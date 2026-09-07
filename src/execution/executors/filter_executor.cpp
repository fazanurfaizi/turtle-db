#include "turtle/execution/executors/filter_executor.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/filter_plan.hpp"
#include <memory>

namespace turtle::execution::executors {

/**
 * Construct a new FilterExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The filter plan to be executed
 * @param child_executor The child executor that feeds the filter
 */
FilterExecutor::FilterExecutor(
    ExecutorContext *exec_ctx, const plans::FilterPlanNode *plan,
    std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan),
      child_executor_(std::move(child_executor)) {}

/** Initialize the filter */
void FilterExecutor::init() {
  // Initialize the child executor
  this->child_executor_->init();
}

/**
 * Yield the next tuple batch from the filter.
 * @param[out] tuple_batch The next tuple batch produced by the filter
 * @param[out] rid_batch The next tuple RID batch produced by the filter
 * @param batch_size The number of tuples to be included in the batch)
 * @return `true` if a tuple was produced, `false` if there are no more tuples
 */
auto FilterExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                          std::vector<RecordId> *rid_batch, size_t batch_size)
    -> bool {
  tuple_batch->clear();
  rid_batch->clear();

  auto filter_expr = this->plan_->get_predicate();
  const auto &child_schema = this->child_executor_->get_output_schema();

  // A null predicate passes every tuple through. Guard before evaluating so a
  // missing predicate never dereferences a null expression.
  auto passes = [&](const storage::table::Tuple &tuple) -> bool {
    if (filter_expr == nullptr) {
      return true;
    }
    auto value = filter_expr->evaluate(&tuple, child_schema);
    return !value.is_null() && value.get_as<bool>();
  };

  while (true) {
    // If the child offset is not zero, process remaining tuples in the last
    // fetched batch
    if (this->child_offset_ != 0) {
      for (size_t i = this->child_offset_; i < this->child_tuples_.size();
           ++i) {
        auto &tuple = this->child_tuples_[i];
        auto &rid = this->child_rids_[i];

        if (passes(tuple)) {
          tuple_batch->push_back(tuple);
          rid_batch->push_back(rid);
        }
      }
    }

    this->child_offset_ = 0;

    // Get the next tuple batch from the child executor
    const auto status = this->child_executor_->next(
        &this->child_tuples_, &this->child_rids_, batch_size);

    // If no more tuples and output batch is empty, return false
    if (!status && tuple_batch->empty()) {
      return false;
    }

    // If not more tuples but output batch is not empty, return true
    if (!status && !tuple_batch->empty()) {
      return true;
    }

    for (size_t i = 0; i < this->child_tuples_.size(); ++i) {
      auto &tuple = this->child_tuples_[i];
      auto &rid = this->child_rids_[i];

      if (passes(tuple)) {
        tuple_batch->push_back(tuple);
        rid_batch->push_back(rid);
        if (tuple_batch->size() >= batch_size) {
          // If we have filled the output batch but not yet reached the end of
          // the child batch, update the offset and return
          if (i + 1 < this->child_tuples_.size()) {
            this->child_offset_ = i + 1;
          } else {
            this->child_offset_ = 0;
          }
          return true;
        }
      }
    }
  }
}

} // namespace turtle::execution::executors
