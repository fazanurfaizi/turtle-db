#include "turtle/execution/executors/aggregation_executor.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/aggregation/aggregate_key.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <vector>

namespace turtle::execution::executors {
/**
 * Construct a new AggregationExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The insert plan to be executed
 * @param child_executor The child executor from which inserted tuples are
 * pulled (may be `nullptr`)
 */
AggregationExecutor::AggregationExecutor(
    ExecutorContext *exec_ctx, const plans::AggregationPlanNode *plan,
    std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan),
      child_executor_(std::move(child_executor)),
      aht_(plan->get_aggregate_functions()) {}

/** Initialize the aggregation */
void AggregationExecutor::init() {
  this->aht_.clear();
  this->child_executor_->init();

  // Consume all child tuples into the hash table
  std::vector<storage::table::Tuple> child_batch;
  std::vector<RecordId> child_rids;
  constexpr size_t batch_size = 256;

  while (this->child_executor_->next(&child_batch, &child_rids, batch_size)) {
    for (size_t i = 0; i < child_batch.size(); ++i) {
      auto key = this->make_aggregate_key(&child_batch[i]);
      auto val_args = this->make_aggregate_values(&child_batch[i]);
      this->aht_.insert_combine(key, val_args);
    }
  }

  // If child returned no tuples and there are no GROUP BY keys,
  // initialize a single empty group so aggregate functions emit default results
  // (e.g. COUNT(*) = 0)
  if (this->aht_.empty() && this->plan_->get_group_bys().empty()) {
    aggregation::AggregateKey empty_key{};
    std::vector<datatype::Value> empty_args(
        this->plan_->get_aggregate_functions().size());
    this->aht_.insert_combine(empty_key, empty_args);
  }

  this->aht_iterator_ = this->aht_.begin();
}

/**
 * Yield the next tuple batch from the aggregation.
 * @param[out] tuple_batch The next batch of tuples produced by the aggregation
 * @param[out] rid_batch The next batch of tuple RecordIds produced by the
 * aggregation
 * @param batch_size The number of tuples to be included in the batch
 * @return `true` if any tuples were produced, `false` if there are no more
 * tuples
 */

auto AggregationExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                               std::vector<RecordId> *rid_batch,
                               size_t batch_size) -> bool {
  tuple_batch->clear();
  rid_batch->clear();

  while (this->aht_iterator_ != this->aht_.end() &&
         tuple_batch->size() < batch_size) {
    std::vector<datatype::Value> values;

    // Add group-by key fields
    for (const auto &gb_val : this->aht_iterator_->first.group_bys_) {
      values.push_back(gb_val);
    }

    // Add finalized aggregate calculations
    const auto &states = this->aht_iterator_->second.states_;
    const auto &funcs = this->plan_->get_aggregate_functions();
    for (size_t i = 0; i < funcs.size(); ++i) {
      values.push_back(funcs[i]->finalize(states[i].get()));
    }

    storage::table::Tuple agg_tuple(&this->plan_->output_schema(), values);
    tuple_batch->push_back(agg_tuple);
    rid_batch->push_back(RecordId{});

    ++this->aht_iterator_;
  }

  return !tuple_batch->empty();
}

/** Do not use or remove this function; otherwise, you will get zero points. */
auto AggregationExecutor::get_child_executor() const
    -> const AbstractExecutor * {
  return this->child_executor_.get();
}
} // namespace turtle::execution::executors
