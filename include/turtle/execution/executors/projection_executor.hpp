#pragma once

#include <memory>
#include <vector>

#include "turtle/catalog/column_schema.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/projection_plan.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::executors {

/*
 * The ProjectionExecutor executor executes a projection.
 */
class ProjectionExecutor : public AbstractExecutor {
public:
  ProjectionExecutor(ExecutorContext *exec_ctx,
                     const plans::ProjectionPlanNode *plan,
                     std::unique_ptr<AbstractExecutor> &&child_executor);

  void init() override;
  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  /** @return The output schema for the projection plan */
  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  /** The projection plan node to be executed */
  const plans::ProjectionPlanNode *plan_;

  /** The child executor from which tuples are obtained */
  std::unique_ptr<AbstractExecutor> child_executor_;

  /** child tuple batch & child RecordId batch */
  std::vector<storage::table::Tuple> child_tuples_{};
  std::vector<RecordId> child_rids_{};

  /** Child tuple batch offset */
  size_t child_offset_ = 0;
};

} // namespace turtle::execution::executors
