#pragma once

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/filter_plan.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <memory>
#include <vector>
namespace turtle::execution::executors {

/**
 * The FilterExecutor executor executes a filter.
 */
class FilterExecutor : public AbstractExecutor {
public:
  FilterExecutor(ExecutorContext *exec_ctx, const plans::FilterPlanNode *plan,
                 std::unique_ptr<AbstractExecutor> &&child_executor);

  void init() override;

  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  /** @return The output schema for the filter plan */
  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  /** The filter plan node to be executed. */
  const plans::FilterPlanNode *plan_;

  /** The child executor from which tuples are obtained */
  std::unique_ptr<AbstractExecutor> child_executor_;

  /** Child tuple batch & child RecordId batch */
  std::vector<storage::table::Tuple> child_tuples_{};
  std::vector<RecordId> child_rids_{};

  /** child tuple batch offset */
  size_t child_offset_ = 0;
};

} // namespace turtle::execution::executors
