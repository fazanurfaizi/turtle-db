#include "turtle/execution/executors/limit_executor.hpp"
#include <algorithm>
#include <utility>

namespace turtle::execution::executors {

/**
 * Construct a new LimitExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The delete to be executed
 * @param child_executor The child executor from which deleted tuples are
 * pulled
 */
LimitExecutor::LimitExecutor(ExecutorContext *exec_ctx,
                             const plans::LimitPlanNode *plan,
                             std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan),
      child_executor_(std::move(child_executor)) {}

/** Initialize the delete */
void LimitExecutor::init() {
  this->child_executor_->init();
  this->emitted_count_ = 0;
}

/**
 * Pulls tuples from the child executor and delets them into the target table.
 * @param tuple_batch Filled with a marker tuple indicating the total number of
 * rows deleted
 * @param rid_batch Corresponding record IDs (typically unused for DELETE
 * output)
 * @param batch_size Maximum tuples to process from the child per call
 * @return true if rows were deleted, false if the child executor is exhausted
 */
auto LimitExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                         std::vector<RecordId> *rid_batch, size_t batch_size)
    -> bool {
  tuple_batch->clear();
  rid_batch->clear();

  const size_t limit = this->plan_->get_limit();

  if (this->emitted_count_ > limit) {
    return false;
  }

  // Fetch and delete all tuples from the child executor
  std::vector<storage::table::Tuple> child_batch;
  std::vector<RecordId> child_rids;

  // Request at most what is left to reach the limit
  const size_t remaining = limit - this->emitted_count_;
  const size_t fetch_size = std::min(batch_size, remaining);

  while (this->child_executor_->next(&child_batch, &child_rids, batch_size)) {
    for (size_t i = 0; i < fetch_size && this->emitted_count_ < limit; ++i) {
      tuple_batch->push_back(std::move(child_batch[i]));
      rid_batch->push_back(child_rids[i]);
      this->emitted_count_++;
    }
    return !tuple_batch->empty();
  }

  return false;
}
} // namespace turtle::execution::executors
