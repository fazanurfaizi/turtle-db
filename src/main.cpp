#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/catalog.hpp"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/catalog/table_info.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/seq_scan_executor.hpp"
#include "turtle/execution/plans/seq_scan_plan.hpp"
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
  cols.emplace_back("id", datatype::DataType::INTEGER);
  cols.emplace_back("age", datatype::DataType::INTEGER);
  auto schema_ref = std::make_shared<const catalog::ColumnSchema>(cols);

  // Register the table with the catalog; this creates its backing heap.
  catalog::TableInfo *table_info =
      catalog->create_table("people", catalog::ColumnSchema(cols));

  // Insert a couple of tuples directly into the table heap.
  auto make_tuple = [&](int32_t id, int32_t age) {
    std::vector<datatype::Value> vals;
    vals.emplace_back(datatype::DataType::INTEGER, id);
    vals.emplace_back(datatype::DataType::INTEGER, age);
    return storage::table::Tuple(&table_info->schema_, vals);
  };

  RecordId rid;
  auto t1 = make_tuple(1, 25);
  auto t2 = make_tuple(2, 50);
  table_info->table_->insert_tuple(t1, &rid);
  table_info->table_->insert_tuple(t2, &rid);

  // Build the plan + executor context, then run the plan-driven scan.
  execution::plans::SeqScanPlanNode plan(schema_ref, table_info->oid_, "people");
  execution::ExecutorContext exec_ctx(catalog.get(), bpm.get(), false);
  execution::executors::SeqScanExecutor scan(&exec_ctx, &plan);

  std::cout << "--- SeqScan over table='" << plan.table_name_
            << "' (oid=" << plan.get_table_oid() << ") ---\n";

  scan.init();
  std::vector<storage::table::Tuple> batch;
  std::vector<RecordId> rids;
  size_t count = 0;
  while (scan.next(&batch, &rids, 256)) {
    for (size_t i = 0; i < batch.size(); ++i) {
      std::cout << "row " << count++ << " at " << rids[i].to_string() << '\n';
    }
  }
  std::cout << "--- Scan complete (" << count << " rows) ---\n";
  return 0;
}
