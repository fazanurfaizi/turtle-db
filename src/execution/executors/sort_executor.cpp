#include "turtle/execution/executors/sort_executor.hpp"
#include "turtle/datatype/type.hpp"
#include <algorithm>
#include <utility>

namespace turtle::execution::executors {

/**
 * Construct a new SortExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The sort plan to be executed
 */
SortExecutor::SortExecutor(ExecutorContext *exec_ctx,
                           const plans::SortPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan),
      child_executor_(std::move(child_executor)) {}

/** Initialize the sort */
void SortExecutor::init() {
  this->child_executor_->init();
  this->sorted_entries_.clear();
  this->cursor_ = 0;

  std::vector<storage::table::Tuple> child_batch;
  std::vector<RecordId> child_rids;
  size_t batch_size = 256;

  child_batch.reserve(batch_size);
  child_rids.reserve(batch_size);

  const auto &order_bys = this->plan_->get_order_bys();
  const auto &child_schema = this->child_executor_->get_output_schema();

  while (this->child_executor_->next(&child_batch, &child_rids, batch_size)) {
    for (size_t i = 0; i < child_batch.size(); ++i) {
      const auto &tuple = child_batch[i];
      const auto &rid = child_rids[i];

      std::vector<datatype::Value> keys;
      keys.reserve(order_bys.size());

      for (const auto &[order_type, order_null_type, expr] : order_bys) {
        keys.push_back(expr->evaluate(&tuple, child_schema));
      }

      this->sorted_entries_.push_back(
          SortEntry{std::move(child_batch[i]), rid, std::move(keys)});
    }
  }

  SortComparator comparator(order_bys);
  std::sort(this->sorted_entries_.begin(), this->sorted_entries_.end(),
            comparator);
}

/**
 * Yield the next tuple batch from the sort.
 * @param[out] tuple_batch The next tuple batch produced by the sort
 * @param[out] rid_batch The next tuple RID batch produced by the sort
 * @param batch_size The number of tuples to be included in the batch
 * @return `true` if a tuple was produced, `false` if there are no more tuples
 */
auto SortExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                        std::vector<RecordId> *rid_batch, size_t batch_size)
    -> bool {
  tuple_batch->clear();
  rid_batch->clear();

  const size_t total_entries = this->sorted_entries_.size();
  if (this->cursor_ >= total_entries) {
    return false;
  }

  const size_t count = std::min(batch_size, total_entries - this->cursor_);
  tuple_batch->reserve(count);
  rid_batch->reserve(count);

  for (size_t i = 0; i < count; ++i) {
    auto &entry = this->sorted_entries_[this->cursor_++];
    tuple_batch->push_back(std::move(entry.tuple));
    rid_batch->push_back(entry.rid);
  }

  return true;
}

} // namespace turtle::execution::executors
