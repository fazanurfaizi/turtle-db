#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "fmt/base.h"
#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/catalog.hpp"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/catalog/table_info.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/datatype/value_factory.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/executors/filter_executor.hpp"
#include "turtle/execution/executors/insert_executor.hpp"
#include "turtle/execution/executors/projection_executor.hpp"
#include "turtle/execution/executors/seq_scan_executor.hpp"
#include "turtle/execution/executors/values_executor.hpp"
#include "turtle/execution/expressions/column_value_expression.hpp"
#include "turtle/execution/expressions/comparison_expression.hpp"
#include "turtle/execution/expressions/constant_value_expression.hpp"
#include "turtle/execution/expressions/logic_expression.hpp"
#include "turtle/execution/plans/filter_plan.hpp"
#include "turtle/execution/plans/insert_plan.hpp"
#include "turtle/execution/plans/projection_plan.hpp"
#include "turtle/execution/plans/seq_scan_plan.hpp"
#include "turtle/execution/plans/values_plan.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/table/tuple.hpp"

using namespace turtle;

namespace {

namespace expr = execution::expressions;
namespace plans = execution::plans;
namespace executors = execution::executors;

using expr::AbstractExpressionRef;

constexpr int k_row_count = 1000;
constexpr size_t k_batch_size = 256;

// -----------------------------------------------------------------------------
// Schema / data construction
// -----------------------------------------------------------------------------

// The full 8-column schema for the demo `people` table.
auto make_people_columns() -> std::vector<catalog::Column> {
  std::vector<catalog::Column> cols;
  cols.emplace_back("id", datatype::DataType::BIGINT);
  cols.emplace_back("name", datatype::DataType::VARCHAR);
  cols.emplace_back("age", datatype::DataType::SMALLINT);
  cols.emplace_back("weight", datatype::DataType::INTEGER);
  cols.emplace_back("amount", datatype::DataType::DECIMAL);
  cols.emplace_back("is_lived", datatype::DataType::TINYINT);
  cols.emplace_back("is_active", datatype::DataType::BOOLEAN);
  cols.emplace_back("date_of_birth", datatype::DataType::TIMESTAMP);
  return cols;
}

// One row of constant-value expressions per generated tuple, matching the
// people schema column-for-column.
auto build_value_rows(int row_count)
    -> std::vector<std::vector<AbstractExpressionRef>> {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(
      0, static_cast<int>(datatype::CmpBool::CmpNull) - 1);
  std::uniform_int_distribution dist8(0, 1);

  auto constant = [](const datatype::Value &value) -> AbstractExpressionRef {
    return std::make_shared<expr::ConstantValueExpression>(value);
  };

  std::vector<std::vector<AbstractExpressionRef>> values;
  values.reserve(static_cast<size_t>(row_count));
  for (int id = 1; id <= row_count; ++id) {
    values.push_back({
        constant(datatype::ValueFactory::get_big_int_value(id)),
        constant(datatype::ValueFactory::get_varchar_value("user_" +
                                                           std::to_string(id))),
        constant(datatype::ValueFactory::get_small_int_value(
            static_cast<int16_t>(id + 1))),
        constant(datatype::ValueFactory::get_integer_value(id * 7)),
        constant(datatype::ValueFactory::get_decimal_value(id * 1000)),
        constant(datatype::ValueFactory::get_tiny_int_value(
            static_cast<int8_t>(dist(gen)))),
        constant(datatype::ValueFactory::get_boolean_value(
            static_cast<datatype::CmpBool>(dist(gen)))),
        constant(datatype::ValueFactory::cast_as_timestamp(
            datatype::ValueFactory::get_varchar_value(
                "2026-09-07 12:30:00.000000+07"))),
    });
  }
  return values;
}

// -----------------------------------------------------------------------------
// Output helpers
// -----------------------------------------------------------------------------

void print_schema_header(const std::vector<catalog::Column> &cols) {
  for (const auto &col : cols) {
    std::cout << col.get_name() << " "
              << datatype::Type::data_type_to_string(col.get_type()) << "  ";
  }
  std::cout << "\n";
}

// Drain `exec` to completion, printing each row and a final row count.
void print_rows(executors::AbstractExecutor &exec) {
  const auto &schema = exec.get_output_schema();
  const auto &columns = schema.get_columns();

  std::vector<storage::table::Tuple> batch;
  std::vector<RecordId> rids;
  size_t count = 0;
  while (exec.next(&batch, &rids, k_batch_size)) {
    for (size_t i = 0; i < batch.size(); ++i) {
      fmt::print("row {}: [", count++);
      for (uint32_t col = 0; col < columns.size(); ++col) {
        fmt::print("{}", batch[i].value(&schema, col));
        if (col < columns.size() - 1) {
          fmt::print(", ");
        }
      }
      std::cout << "] at " << rids[i].to_string() << '\n';
    }
  }
  std::cout << "--- Scan complete (" << count << " rows) ---\n";
}

// -----------------------------------------------------------------------------
// Pipelines
// -----------------------------------------------------------------------------

// Insert `values` into `table_oid` through a Values -> Insert pipeline and
// report how many rows landed.
void run_insert(execution::ExecutorContext *ctx,
                const catalog::ColumnSchemaRef &schema,
                std::vector<std::vector<AbstractExpressionRef>> values,
                TableOid table_oid) {
  plans::ValuesPlanNode values_plan(schema, std::move(values));

  // InsertExecutor emits a single INTEGER column holding the inserted count.
  std::vector<catalog::Column> count_cols;
  count_cols.emplace_back("count", datatype::DataType::INTEGER);
  auto count_schema = std::make_shared<const catalog::ColumnSchema>(count_cols);

  plans::InsertPlanNode insert_plan(
      count_schema, std::make_shared<plans::ValuesPlanNode>(values_plan),
      table_oid);

  auto values_exec =
      std::make_unique<executors::ValuesExecutor>(ctx, &values_plan);
  executors::InsertExecutor insert_exec(ctx, &insert_plan,
                                        std::move(values_exec));

  std::cout << "--- Inserting rows via executor pipeline ---\n";
  insert_exec.init();
  std::vector<storage::table::Tuple> batch;
  std::vector<RecordId> rids;
  while (insert_exec.next(&batch, &rids, k_batch_size)) {
    if (!batch.empty()) {
      fmt::print("Inserted {} rows\n",
                 batch[0].value(&insert_exec.get_output_schema(), 0));
    }
  }
}

// Predicate for the demo query: is_active == true OR (column 2) == 1.
auto build_predicate(const std::vector<catalog::Column> &cols)
    -> AbstractExpressionRef {
  auto is_active_true = std::make_shared<expr::ComparisonExpression>(
      std::make_shared<expr::ColumnValueExpression>(0, 6, cols[6]),
      std::make_shared<expr::ConstantValueExpression>(
          datatype::ValueFactory::get_boolean_value(true)),
      expr::ComparisonType::Equal);

  auto is_lived_one = std::make_shared<expr::ComparisonExpression>(
      std::make_shared<expr::ColumnValueExpression>(0, 5, cols[5]),
      std::make_shared<expr::ConstantValueExpression>(
          datatype::ValueFactory::get_tiny_int_value(1)),
      expr::ComparisonType::Equal);

  return std::make_shared<expr::LogicExpression>(is_active_true, is_lived_one,
                                                 expr::LogicType::And);
}

// The five columns projected out of the scanned table.
auto make_projection_columns() -> std::vector<catalog::Column> {
  std::vector<catalog::Column> cols;
  cols.emplace_back("id", datatype::DataType::BIGINT);
  cols.emplace_back("name", datatype::DataType::VARCHAR);
  cols.emplace_back("is_lived", datatype::DataType::TINYINT);
  cols.emplace_back("is_active", datatype::DataType::BOOLEAN);
  cols.emplace_back("date_of_birth", datatype::DataType::TIMESTAMP);
  return cols;
}

// Build and run SeqScan -> Filter -> Projection over `people`, printing the
// projected rows.
void run_query(execution::ExecutorContext *ctx,
               const catalog::ColumnSchemaRef &schema,
               const std::vector<catalog::Column> &cols, TableOid table_oid) {
  // Leaf: sequential scan streaming every row.
  plans::SeqScanPlanNode scan_plan(schema, table_oid, "people");
  auto scan_exec =
      std::make_unique<executors::SeqScanExecutor>(ctx, &scan_plan);

  // Filter: keep rows matching the predicate (schema is unchanged).
  plans::FilterPlanNode filter_plan(
      schema, build_predicate(cols),
      std::make_shared<plans::SeqScanPlanNode>(scan_plan));
  auto filter_exec = std::make_unique<executors::FilterExecutor>(
      ctx, &filter_plan, std::move(scan_exec));

  // Projection: id, name, is_lived, is_active, date_of_birth. Column indices
  // reference the child (filter) schema — the full 8-col table:
  // id=0, name=1, age=2, weight=3, amount=4, is_lived=5, is_active=6, dob=7.
  auto out_cols = make_projection_columns();
  auto out_schema = std::make_shared<const catalog::ColumnSchema>(out_cols);

  std::vector<AbstractExpressionRef> projections;
  projections.push_back(
      std::make_shared<expr::ColumnValueExpression>(0, 0, out_cols[0]));
  projections.push_back(
      std::make_shared<expr::ColumnValueExpression>(0, 1, out_cols[1]));
  projections.push_back(
      std::make_shared<expr::ColumnValueExpression>(0, 5, out_cols[2]));
  projections.push_back(
      std::make_shared<expr::ColumnValueExpression>(0, 6, out_cols[3]));
  projections.push_back(
      std::make_shared<expr::ColumnValueExpression>(0, 7, out_cols[4]));

  plans::ProjectionPlanNode projection_plan(
      out_schema, projections,
      std::make_shared<plans::FilterPlanNode>(filter_plan));
  executors::ProjectionExecutor projection_exec(ctx, &projection_plan,
                                                std::move(filter_exec));

  std::cout << "--- Filtered SeqScan over table='" << scan_plan.table_name_
            << "' (oid=" << scan_plan.get_table_oid() << ") ---\n";
  print_schema_header(out_cols);

  projection_exec.init();
  print_rows(projection_exec);
}

} // namespace

int main() {
  auto disk_manager = std::make_unique<storage::disk::DiskManager>();
  auto bpm =
      std::make_unique<buffer::BufferPoolManager>(10, disk_manager.get());
  auto catalog = std::make_unique<catalog::Catalog>(
      bpm.get(), disk_manager.get(), "test_db/turtle.catalog", "test_db");

  // Define and register the `people` table (this creates its backing heap).
  auto cols = make_people_columns();
  auto schema = std::make_shared<const catalog::ColumnSchema>(cols);
  catalog::TableInfo *table_info =
      catalog->create_table("people", catalog::ColumnSchema(cols));

  execution::ExecutorContext ctx(catalog.get(), bpm.get(), false);

  run_insert(&ctx, schema, build_value_rows(k_row_count), table_info->oid_);
  run_query(&ctx, schema, cols, table_info->oid_);

  return 0;
}
