#pragma once

#include "turtle/catalog/table_info.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/update_plan.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <memory>
#include <vector>

namespace turtle::execution::executors {

/**
 * UpdateExecutor executes an update on a table.
 * Updated value are always pulled from a child.
 */
class UpdateExecutor : public AbstractExecutor {
public:
  UpdateExecutor(ExecutorContext *exec_ctx, const plans::UpdatePlanNode *plan,
                 std::unique_ptr<AbstractExecutor> &&child_executor);

  void init() override;

  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  /** @return The output schema for the update */
  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  /** The update plan node to be executed */
  const plans::UpdatePlanNode *plan_;

  /** Metadata identifying the table that should be updated */
  const catalog::TableInfo *table_info_;

  /** The child executor to obtain value from */
  std::unique_ptr<AbstractExecutor> child_executor_;

  bool executed_;
};

} // namespace turtle::execution::executors
