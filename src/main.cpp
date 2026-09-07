#include <cstdint>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "fmt/base.h"
#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/catalog.hpp"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/catalog/table_info.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/datatype/value_factory.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/insert_executor.hpp"
#include "turtle/execution/executors/seq_scan_executor.hpp"
#include "turtle/execution/executors/values_executor.hpp"
#include "turtle/execution/expressions/constant_value_expression.hpp"
#include "turtle/execution/plans/insert_plan.hpp"
#include "turtle/execution/plans/seq_scan_plan.hpp"
#include "turtle/execution/plans/values_plan.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/tuple.hpp"

using namespace turtle;

int main() {
  auto disk_manager = std::make_unique<storage::disk::DiskManager>();
  auto bpm =
      std::make_unique<buffer::BufferPoolManager>(10, disk_manager.get());
  auto catalog = std::make_unique<catalog::Catalog>(
      bpm.get(), disk_manager.get(), "test_db/turtle.catalog", "test_db");

  // Define the table schema: (id INTEGER, age INTEGER).
  std::vector<catalog::Column> cols;
  cols.emplace_back("id", datatype::DataType::BIGINT);
  cols.emplace_back("name", datatype::DataType::VARCHAR);
  cols.emplace_back("age", datatype::DataType::SMALLINT);
  cols.emplace_back("weight", datatype::DataType::INTEGER);
  cols.emplace_back("amount", datatype::DataType::DECIMAL);
  cols.emplace_back("is_lived", datatype::DataType::TINYINT);
  cols.emplace_back("is_active", datatype::DataType::BOOLEAN);
  cols.emplace_back("date_of_birth", datatype::DataType::TIMESTAMP);
  auto schema_ref = std::make_shared<const catalog::ColumnSchema>(cols);

  // Register the table with the catalog; this creates its backing heap.
  catalog::TableInfo *table_info =
      catalog->create_table("people", catalog::ColumnSchema(cols));

  // Build values to insert using the executor pipeline.
  // Create a row of ConstantValueExpressions for each tuple.
  std::vector<std::vector<execution::expressions::AbstractExpressionRef>>
      values;

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<int> dist(
      0, static_cast<int>(datatype::CmpBool::CmpNull) - 1);

  for (int id = 1; id <= 1000; ++id) {
    std::vector<execution::expressions::AbstractExpressionRef> row;
    row.push_back(
        std::make_shared<execution::expressions::ConstantValueExpression>(
            datatype::ValueFactory::get_big_int_value(id)));

    std::string name = "user_" + std::to_string(id);
    row.push_back(
        std::make_shared<execution::expressions::ConstantValueExpression>(
            datatype::ValueFactory::get_varchar_value(name)));

    row.push_back(
        std::make_shared<execution::expressions::ConstantValueExpression>(
            datatype::ValueFactory::get_small_int_value(
                static_cast<int16_t>(id + 1))));

    row.push_back(
        std::make_shared<execution::expressions::ConstantValueExpression>(
            datatype::ValueFactory::get_integer_value(id * 7)));

    row.push_back(
        std::make_shared<execution::expressions::ConstantValueExpression>(
            datatype::ValueFactory::get_decimal_value(id * 1000)));

    row.push_back(
        std::make_shared<execution::expressions::ConstantValueExpression>(
            datatype::ValueFactory::get_tiny_int_value(1)));

    row.push_back(
        std::make_shared<execution::expressions::ConstantValueExpression>(
            datatype::ValueFactory::get_boolean_value(
                static_cast<datatype::CmpBool>(dist(gen)))));

    row.push_back(
        std::make_shared<execution::expressions::ConstantValueExpression>(
            datatype::ValueFactory::cast_as_timestamp(
                datatype::ValueFactory::get_varchar_value(
                    "2026-09-07 12:30:00.000000+07"))));

    values.push_back(row);
  }

  // Build the executor context and pipeline.
  execution::ExecutorContext exec_ctx(catalog.get(), bpm.get(), false);

  // Create ValuesPlanNode to emit the constant tuples.
  execution::plans::ValuesPlanNode values_plan(schema_ref, values);

  // Create output schema for InsertExecutor: single INTEGER column for row
  // count.
  std::vector<catalog::Column> insert_out_cols;
  insert_out_cols.emplace_back("count", datatype::DataType::INTEGER);
  auto insert_output_schema =
      std::make_shared<const catalog::ColumnSchema>(insert_out_cols);

  // Create InsertPlanNode with the values as child.
  execution::plans::InsertPlanNode insert_plan(
      insert_output_schema,
      std::make_shared<execution::plans::ValuesPlanNode>(values_plan),
      table_info->oid_);

  // Create ValuesExecutor to produce the constant tuples.
  auto values_exec = std::make_unique<execution::executors::ValuesExecutor>(
      &exec_ctx, &values_plan);

  // Create InsertExecutor to insert the tuples.
  execution::executors::InsertExecutor insert_exec(&exec_ctx, &insert_plan,
                                                   std::move(values_exec));

  // Execute the insert.
  std::cout << "--- Inserting 100 rows via executor pipeline ---\n";
  insert_exec.init();
  std::vector<storage::table::Tuple> insert_batch;
  std::vector<RecordId> insert_rids;
  while (insert_exec.next(&insert_batch, &insert_rids, 256)) {
    if (!insert_batch.empty()) {
      auto count_val =
          insert_batch[0].value(&insert_exec.get_output_schema(), 0);
      fmt::print("Inserted {} rows\n", count_val);
    }
  }

  // Build the plan for scan, then run the plan-driven scan to verify insertion.
  execution::plans::SeqScanPlanNode plan(schema_ref, table_info->oid_,
                                         "people");
  execution::executors::SeqScanExecutor scan(&exec_ctx, &plan);

  std::cout << "--- SeqScan over table='" << plan.table_name_
            << "' (oid=" << plan.get_table_oid() << ") ---\n";

  scan.init();
  std::vector<storage::table::Tuple> batch;
  std::vector<RecordId> rids;
  size_t count = 0;
  while (scan.next(&batch, &rids, 256)) {
    for (size_t i = 0; i < batch.size(); ++i) {
      const auto &t = batch[i];
      const auto &schema = scan.get_output_schema();
      const auto &columns = schema.get_columns();

      fmt::print("row {}: [", count++);
      for (uint32_t col = 0; col < columns.size(); ++col) {
        auto val = t.value(&schema, col);
        fmt::print("{}", val);
        if (col < columns.size() - 1)
          fmt::print(", ");
      }
      std::cout << "]" << " at " << rids[i].to_string() << '\n';
    }
  }
  std::cout << "--- Scan complete (" << count << " rows) ---\n";
  return 0;
}
