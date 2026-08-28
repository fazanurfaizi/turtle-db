#include "turtle/execution/executors/insert_executor.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/insert_plan.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <memory>
#include <vector>

namespace turtle::execution::executors {

/**
 * Construct a new InsertExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The insert to be executed
 * @param child_executor The child executor from which inserted tuples are
 * pulled
 */
InsertExecutor::InsertExecutor(
    ExecutorContext *exec_ctx, const plans::InsertPlanNode *plan,
    std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan),
      child_executor_(std::move(child_executor)) {}

/** Initialize the insert */
void InsertExecutor::init() {
  // Resolve the table heap from the catalog using the plan's table OID.
  auto *info =
      this->exec_ctx_->get_catalog()->get_table(this->plan_->get_table_oid());
  if (info == nullptr) {
    throw std::runtime_error("InsertExecutor: unknown table oid " +
                             std::to_string(this->plan_->get_table_oid()));
  }

  this->table_heap_ = info->table_.get();
  this->total_rows_inserted_ = 0;
  this->has_emitted_count_ = false;

  this->child_executor_->init();
}

/**
 * Pulls tuples from the child executor and inserts them into the target table.
 * @param tuple_batch Filled with a marker tuple indicating the total number of
 * rows inserted
 * @param rid_batch Corresponding record IDs (typically unused for INSERT
 * output)
 * @param batch_size Maximum tuples to process from the child per call
 * @return true if rows were inserted, false if the child executor is exhausted
 */
auto InsertExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                          std::vector<RecordId> *rid_batch, size_t batch_size)
    -> bool {
  tuple_batch->clear();
  rid_batch->clear();

  if (this->has_emitted_count_)
    return false;

  // Fetch and insert all tuples from the child executor
  std::vector<storage::table::Tuple> child_batch;
  std::vector<RecordId> child_rids;

  while (this->child_executor_->next(&child_batch, &child_rids, batch_size)) {
    for (size_t i = 0; i < child_batch.size(); ++i) {
      RecordId rid;
      this->table_heap_->insert_tuple(child_batch[i], &rid);
      this->total_rows_inserted_++;
    }
  }

  // Create a count tuple containing the total rows inserted
  std::vector<datatype::Value> count_vals;
  count_vals.emplace_back(datatype::DataType::INTEGER,
                          static_cast<int32_t>(this->total_rows_inserted_));

  storage::table::Tuple count_tuple(&this->plan_->output_schema(), count_vals);
  tuple_batch->push_back(count_tuple);
  rid_batch->push_back(RecordId{});

  this->has_emitted_count_ = true;
  return true;
}
} // namespace turtle::execution::executors
