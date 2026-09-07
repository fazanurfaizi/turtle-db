// -----------------------------------------------------------------------------
// Tier 3 (Execution) — turtle::execution::executors
//
// Scope: the Volcano operators ValuesExecutor, SeqScanExecutor, InsertExecutor.
//
// Mocking policy: everything below Tier 3 is concrete and cheap, so those are
// used for real (a genuine Catalog/TableHeap is the insert sink and scan
// source). The ONE pure-virtual seam in the hot path — AbstractExecutor — is
// mocked with GoogleMock to isolate InsertExecutor from its child: we assert
// that Insert pulls its child's batches and reports the correct row count
// without depending on any concrete child executor. A second, end-to-end test
// wires a real ValuesExecutor -> InsertExecutor pipeline for integration cover.
//
// Fixture owns DiskManager + BPM + Catalog + ExecutorContext and a "people"
// table (id INTEGER, name VARCHAR); scratch dir wiped per test.
// -----------------------------------------------------------------------------
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/catalog.hpp"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/executors/insert_executor.hpp"
#include "turtle/execution/executors/seq_scan_executor.hpp"
#include "turtle/execution/executors/values_executor.hpp"
#include "turtle/execution/expressions/constant_value_expression.hpp"
#include "turtle/execution/plans/insert_plan.hpp"
#include "turtle/execution/plans/seq_scan_plan.hpp"
#include "turtle/execution/plans/values_plan.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/table/table_iterator.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution {
namespace {

using catalog::Column;
using catalog::ColumnSchema;
using catalog::ColumnSchemaRef;
using datatype::DataType;
using datatype::Value;
using executors::AbstractExecutor;
using expressions::AbstractExpressionRef;
using expressions::ConstantValueExpression;
using storage::table::Tuple;

// A GoogleMock stand-in for a child executor, used to isolate InsertExecutor.
class MockAbstractExecutor : public AbstractExecutor {
public:
  MockAbstractExecutor(ExecutorContext *ctx, const ColumnSchema *schema)
      : AbstractExecutor(ctx), schema_(schema) {}

  MOCK_METHOD(void, init, (), (override));
  MOCK_METHOD(bool, next,
              (std::vector<Tuple> * tuple_batch,
               std::vector<RecordId> *rid_batch, size_t batch_size),
              (override));

  auto get_output_schema() const -> const ColumnSchema & override {
    return *schema_;
  }

private:
  const ColumnSchema *schema_;
};

class ExecutorTest : public ::testing::Test {
protected:
  void SetUp() override {
    dir_ = std::filesystem::path("test_db") / "executors_gtest";
    std::filesystem::remove_all(dir_);
    std::filesystem::create_directories(dir_);

    disk_ = std::make_unique<storage::disk::DiskManager>();
    bpm_ = std::make_unique<buffer::BufferPoolManager>(32, disk_.get());
    catalog_ = std::make_unique<catalog::Catalog>(
        bpm_.get(), disk_.get(), (dir_ / "catalog.cat").string(),
        dir_.string());
    ctx_ = std::make_unique<ExecutorContext>(catalog_.get(), bpm_.get(), false);

    // people(id INTEGER, name VARCHAR)
    std::vector<Column> cols;
    cols.emplace_back("id", DataType::INTEGER);
    cols.emplace_back("name", DataType::VARCHAR);
    people_ = catalog_->create_table("people", ColumnSchema(cols));

    // Single-column INTEGER "count" schema that InsertExecutor emits.
    std::vector<Column> ccols;
    ccols.emplace_back("count", DataType::INTEGER);
    count_schema_ = std::make_shared<const ColumnSchema>(std::move(ccols));

    // Reusable people schema ref for plans.
    people_schema_ = std::make_shared<const ColumnSchema>(std::move(cols));
  }

  void TearDown() override {
    ctx_.reset();
    catalog_.reset();
    bpm_.reset();
    disk_.reset();
    std::filesystem::remove_all(dir_);
  }

  Tuple people_row(int32_t id, const std::string &name) {
    std::string name_copy = name;
    std::vector<Value> vals;
    vals.emplace_back(DataType::INTEGER, id);
    vals.emplace_back(DataType::VARCHAR, name_copy);
    return Tuple(&people_->schema_, std::move(vals));
  }

  int count_table_rows() {
    int n = 0;
    for (auto it = people_->table_->begin(); it != people_->table_->end();
         ++it) {
      it.get_tuple();
      ++n;
    }
    return n;
  }

  std::filesystem::path dir_;
  std::unique_ptr<storage::disk::DiskManager> disk_;
  std::unique_ptr<buffer::BufferPoolManager> bpm_;
  std::unique_ptr<catalog::Catalog> catalog_;
  std::unique_ptr<ExecutorContext> ctx_;
  catalog::TableInfo *people_{nullptr};
  ColumnSchemaRef count_schema_;
  ColumnSchemaRef people_schema_;
};

// ============================ ValuesExecutor ================================

TEST_F(ExecutorTest, ValuesExecutorEmitsAllRowsRespectingBatchSize) {
  // Three constant rows: (1,"a"), (2,"b"), (3,"c").
  std::vector<std::vector<AbstractExpressionRef>> values;
  for (int i = 1; i <= 3; ++i) {
    std::vector<AbstractExpressionRef> row;
    row.push_back(std::make_shared<ConstantValueExpression>(
        Value(DataType::INTEGER, static_cast<int32_t>(i))));
    std::string nm(1, static_cast<char>('a' + i - 1));
    row.push_back(std::make_shared<ConstantValueExpression>(
        Value(DataType::VARCHAR, nm)));
    values.push_back(std::move(row));
  }

  plans::ValuesPlanNode plan(people_schema_, std::move(values));
  executors::ValuesExecutor exec(ctx_.get(), &plan);
  exec.init();

  std::vector<Tuple> batch;
  std::vector<RecordId> rids;

  // batch_size 2 -> first call yields 2 rows.
  ASSERT_TRUE(exec.next(&batch, &rids, 2));
  EXPECT_EQ(batch.size(), 2u);
  EXPECT_EQ(batch[0].value(&exec.get_output_schema(), 0).get_as<int32_t>(), 1);
  EXPECT_EQ(batch[1].value(&exec.get_output_schema(), 1).to_string(), "b");

  // second call yields the remaining 1 row.
  ASSERT_TRUE(exec.next(&batch, &rids, 2));
  EXPECT_EQ(batch.size(), 1u);
  EXPECT_EQ(batch[0].value(&exec.get_output_schema(), 0).get_as<int32_t>(), 3);

  // third call is exhausted.
  EXPECT_FALSE(exec.next(&batch, &rids, 2));
}

// ============================ SeqScanExecutor ===============================

TEST_F(ExecutorTest, SeqScanStreamsEveryStoredTuple) {
  constexpr int kRows = 120;
  for (int i = 0; i < kRows; ++i) {
    RecordId rid;
    ASSERT_TRUE(people_->table_->insert_tuple(
        people_row(i, "u" + std::to_string(i)), &rid));
  }

  plans::SeqScanPlanNode plan(people_schema_, people_->oid_, "people");
  executors::SeqScanExecutor scan(ctx_.get(), &plan);
  scan.init();

  int seen = 0;
  std::vector<Tuple> batch;
  std::vector<RecordId> rids;
  while (scan.next(&batch, &rids, 32)) {
    seen += static_cast<int>(batch.size());
  }
  EXPECT_EQ(seen, kRows);
}

TEST_F(ExecutorTest, SeqScanOverEmptyTableProducesNothing) {
  catalog::TableInfo *empty = catalog_->create_table("empty", *people_schema_);
  plans::SeqScanPlanNode plan(people_schema_, empty->oid_, "empty");
  executors::SeqScanExecutor scan(ctx_.get(), &plan);
  scan.init();

  std::vector<Tuple> batch;
  std::vector<RecordId> rids;
  EXPECT_FALSE(scan.next(&batch, &rids, 16));
}

TEST_F(ExecutorTest, SeqScanOnUnknownOidThrowsOnInit) {
  plans::SeqScanPlanNode plan(people_schema_, TableOid{9999}, "ghost");
  executors::SeqScanExecutor scan(ctx_.get(), &plan);
  EXPECT_THROW(scan.init(), std::runtime_error);
}

// ============================ InsertExecutor ================================
// gmock-isolated: the child executor is mocked; the insert sink is a real heap.

TEST_F(ExecutorTest, InsertPullsChildBatchAndReportsRowCount) {
  using ::testing::_;
  using ::testing::DoAll;
  using ::testing::Invoke;
  using ::testing::Return;

  auto child =
      std::make_unique<MockAbstractExecutor>(ctx_.get(), &people_->schema_);
  MockAbstractExecutor *child_raw = child.get();

  // Child is initialized exactly once, then yields one batch of 3 rows and
  // signals exhaustion on the following call.
  EXPECT_CALL(*child_raw, init()).Times(1);
  EXPECT_CALL(*child_raw, next(_, _, _))
      .WillOnce(DoAll(Invoke([this](std::vector<Tuple> *tb,
                                    std::vector<RecordId> *rb, size_t) {
                        tb->clear();
                        rb->clear();
                        for (int i = 0; i < 3; ++i) {
                          tb->push_back(people_row(i, "m" + std::to_string(i)));
                          rb->emplace_back();
                        }
                      }),
                      Return(true)))
      .WillOnce(Return(false));

  plans::InsertPlanNode plan(
      count_schema_,
      std::make_shared<plans::ValuesPlanNode>(
          people_schema_, std::vector<std::vector<AbstractExpressionRef>>{}),
      people_->oid_);

  executors::InsertExecutor insert(ctx_.get(), &plan, std::move(child));
  insert.init();

  std::vector<Tuple> out;
  std::vector<RecordId> out_rids;
  ASSERT_TRUE(insert.next(&out, &out_rids, 256));
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(out[0].value(&insert.get_output_schema(), 0).get_as<int32_t>(), 3);

  // Insert must be a single-shot count emitter: second call returns false.
  EXPECT_FALSE(insert.next(&out, &out_rids, 256));

  // And the rows really landed in the table heap.
  EXPECT_EQ(count_table_rows(), 3);
}

// End-to-end: a real ValuesExecutor feeding a real InsertExecutor.
TEST_F(ExecutorTest, ValuesToInsertPipelineInsertsAndCounts) {
  std::vector<std::vector<AbstractExpressionRef>> values;
  for (int i = 0; i < 10; ++i) {
    std::vector<AbstractExpressionRef> row;
    row.push_back(std::make_shared<ConstantValueExpression>(
        Value(DataType::INTEGER, static_cast<int32_t>(i))));
    std::string nm = "user_" + std::to_string(i);
    row.push_back(std::make_shared<ConstantValueExpression>(
        Value(DataType::VARCHAR, nm)));
    values.push_back(std::move(row));
  }

  auto values_plan =
      std::make_shared<plans::ValuesPlanNode>(people_schema_, values);
  auto values_exec = std::make_unique<executors::ValuesExecutor>(
      ctx_.get(), values_plan.get());

  plans::InsertPlanNode plan(count_schema_, values_plan, people_->oid_);
  executors::InsertExecutor insert(ctx_.get(), &plan, std::move(values_exec));

  insert.init();
  std::vector<Tuple> out;
  std::vector<RecordId> out_rids;
  ASSERT_TRUE(insert.next(&out, &out_rids, 256));
  ASSERT_EQ(out.size(), 1u);
  EXPECT_EQ(out[0].value(&insert.get_output_schema(), 0).get_as<int32_t>(), 10);

  EXPECT_EQ(count_table_rows(), 10);
}

} // namespace
} // namespace turtle::execution
