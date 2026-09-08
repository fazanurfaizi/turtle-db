#include "turtle/execution/executors/projection_executor.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <vector>

namespace turtle::execution::executors {

/**
 * Construct a new ProjectionExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The projection to be executed
 */
ProjectionExecutor::ProjectionExecutor(
    ExecutorContext *exec_ctx, const plans::ProjectionPlanNode *plan,
    std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan),
      child_executor_(std::move(child_executor)) {}

/** Initialize the projection */
void ProjectionExecutor::init() {
  // Initialize the child executor
  this->child_executor_->init();
}

/**
 * Yield the next tuple batch from the projection.
 * @param[out] tuple_batch The next tuple batch produced by the projection
 * @param[out] rid_batch The next tuple RecordId batch produced by the
 * projection
 * @param batch_size The number of tuples to be included in the batch
 * @return `true` if a tuple was produced, `false` if there are no more tuples
 */
auto ProjectionExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                              std::vector<RecordId> *rid_batch,
                              size_t batch_size) -> bool {
  tuple_batch->clear();
  rid_batch->clear();

  if (this->child_offset_ != 0) {
    for (size_t i = this->child_offset_; i < this->child_tuples_.size(); i++) {
      auto child_tuple = this->child_tuples_[i];
      auto child_rid = this->child_rids_[i];

      // Compute expressions
      std::vector<datatype::Value> values{};
      values.reserve(this->get_output_schema().get_column_count());
      for (const auto &expr : this->plan_->get_expressions()) {
        values.push_back(expr->evaluate(
            &child_tuple, this->child_executor_->get_output_schema()));
      }

      tuple_batch->push_back(
          storage::table::Tuple{&this->get_output_schema(), values});
      rid_batch->push_back(child_rid);
    }
  }

  this->child_offset_ = 0;

  const auto status = this->child_executor_->next(
      &this->child_tuples_, &this->child_rids_, batch_size);

  // If no more tuples and output batch is empty, return false
  if (!status && tuple_batch->empty()) {
    return false;
  }

  // If no more tuples but output batch is not empty, return true
  if (!status && !tuple_batch->empty()) {
    return true;
  }

  for (size_t i = 0; i < this->child_tuples_.size(); i++) {
    auto child_tuple = this->child_tuples_[i];
    auto child_rid = this->child_rids_[i];

    // Compute expressions
    std::vector<datatype::Value> values{};
    values.reserve(this->get_output_schema().get_column_count());
    for (const auto &expr : this->plan_->get_expressions()) {
      values.push_back(expr->evaluate(
          &child_tuple, this->child_executor_->get_output_schema()));
    }

    tuple_batch->push_back(
        storage::table::Tuple{&this->get_output_schema(), values});
    rid_batch->push_back(child_rid);

    if (tuple_batch->size() >= batch_size) {
      // If we have filled the output batch but not yet reached the end of the
      // current child batch, update the offset and return
      if (i + 1 < child_tuples_.size()) {
        child_offset_ = i + 1;
      } else {
        child_offset_ = 0;
      }

      return true;
    }
  }

  return !tuple_batch->empty();
}

} // namespace turtle::execution::executors
