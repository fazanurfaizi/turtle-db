#include "turtle/execution/executors/update_executor.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/plans/update_plan.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <cstdint>
#include <vector>

namespace turtle::execution::executors {

/**
 * Construct a new UpdateExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The update plan to be executed
 * @param child_executor The child executor that feeds the update
 */
UpdateExecutor::UpdateExecutor(
    ExecutorContext *exec_ctx, const plans::UpdatePlanNode *plan,
    std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan),
      child_executor_(std::move(child_executor)) {}

/** Initialize the update */
void UpdateExecutor::init() {
  this->table_info_ =
      this->exec_ctx_->get_catalog()->get_table(this->plan_->get_table_oid());
  if (this->table_info_ == nullptr) {
    throw std::runtime_error("DeleteExecutor: unknown table oid " +
                             std::to_string(this->plan_->get_table_oid()));
  }
  this->executed_ = false;

  this->child_executor_->init();
}

/**
 * Yield the number of rows updated in the table.
 * @param[out] tuple_batch The tuple batch with one integer indicating the
 * number of rows updated in the table
 * @param[out] rid_batch The next tuple RecordId batch produced by the update
 * (ignore, not used)
 * @param batch_size The number of tuples to be included in the batch
 * @return `true` if a tuple was produced, `false` if there are no more tuples
 *
 * NOTE: UpdateExecutor::next() does not use the `rid_batch` out-parameter.
 * NOTE: UpdateExecutor::next() returns true with the number of updated rows
 * produced only once.
 */
auto UpdateExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                          std::vector<RecordId> *rid_batch, size_t batch_size)
    -> bool {
  if (this->executed_) {
    return false;
  }

  tuple_batch->clear();
  rid_batch->clear();

  auto *table_heap = this->table_info_->table_.get();
  uint32_t updated_count = 0;

  // Fetch and update all tuples from the child executor
  std::vector<storage::table::Tuple> all_tuples;
  std::vector<RecordId> all_rids;

  std::vector<storage::table::Tuple> chunk_tuples;
  std::vector<RecordId> chunk_rids;

  while (this->child_executor_->next(&chunk_tuples, &chunk_rids, batch_size)) {
    all_tuples.insert(all_tuples.end(), chunk_tuples.begin(),
                      chunk_tuples.end());
    all_rids.insert(all_rids.end(), chunk_rids.begin(), chunk_rids.end());
  }

  for (size_t i = 0; i < all_tuples.size(); ++i) {
    const auto &old_tuple = all_tuples[i];
    const auto &old_rid = all_rids[i];

    std::vector<datatype::Value> new_values;
    new_values.reserve(this->plan_->get_target_expressions().size());
    for (const auto &expr : this->plan_->get_target_expressions()) {
      new_values.push_back(
          expr->evaluate(&old_tuple, this->table_info_->schema_));
    }

    storage::table::Tuple updated_tuple(&this->table_info_->schema_,
                                        new_values);

    RecordId new_rid;
    if (table_heap->mark_delete(old_rid)) {
      if (table_heap->insert_tuple(updated_tuple, &new_rid)) {
        updated_count++;
      }
    }
  }

  // Create a count tuple containing the total rows inserted
  std::vector<datatype::Value> count_vals;
  count_vals.emplace_back(datatype::DataType::INTEGER,
                          static_cast<int32_t>(updated_count));

  storage::table::Tuple count_tuple(&this->get_output_schema(), count_vals);
  tuple_batch->push_back(count_tuple);
  rid_batch->push_back(RecordId{});

  this->executed_ = true;
  return true;
}

} // namespace turtle::execution::executors
