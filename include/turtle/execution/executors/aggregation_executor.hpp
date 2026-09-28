#pragma once

#include "turtle/datatype/value.hpp"
#include "turtle/execution/aggregation/aggregate_key.hpp"
#include "turtle/execution/aggregation/aggregation_hash_table.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/aggeration_plan.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace turtle::execution::executors {

/**
 * AggregationExecutor executes an aggregation operation (e.g. COUNT, SUM, MIN,
 * MAX) over the tuples produced by a child executor.
 */
class AggregationExecutor : public AbstractExecutor {
public:
  AggregationExecutor(ExecutorContext *exec_ctx,
                      const plans::AggregationPlanNode *plan,
                      std::unique_ptr<AbstractExecutor> &&child_executor);

  void init() override;

  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

  auto get_child_executor() const -> const AbstractExecutor *;

private:
  /** The aggregation plan node */
  const plans::AggregationPlanNode *plan_;

  /** The child executor that produces tuples over which the aggregation is
   * computed */
  std::unique_ptr<AbstractExecutor> child_executor_;

  /** Aggregation hash table */
  aggregation::AggregationHashTable aht_;

  /** Simple aggregation hash table iterator */
  std::unordered_map<aggregation::AggregateKey,
                     aggregation::AggregateValue>::iterator aht_iterator_;

  /** @return The tuple as an AggregateKey */
  auto make_aggregate_key(const storage::table::Tuple *tuple)
      -> aggregation::AggregateKey {
    std::vector<datatype::Value> keys;
    for (const auto &expr : this->plan_->get_group_bys()) {
      keys.push_back(
          expr->evaluate(tuple, this->child_executor_->get_output_schema()));
    }
    return {keys};
  }

  /** @return The tuple as an AggregateValue */
  auto make_aggregate_values(const storage::table::Tuple *tuple)
      -> std::vector<datatype::Value> {
    std::vector<datatype::Value> vals;
    for (const auto &expr : this->plan_->get_aggregates()) {
      vals.push_back(
          expr->evaluate(tuple, this->child_executor_->get_output_schema()));
    }
    return vals;
  }
};

} // namespace turtle::execution::executors
