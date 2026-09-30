#pragma once

#include <memory>
#include <vector>

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/limit_plan.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::executors {

class LimitExecutor : public AbstractExecutor {
public:
  LimitExecutor(ExecutorContext *exec_ctx, const plans::LimitPlanNode *plan,
                std::unique_ptr<AbstractExecutor> &&child_executor);

  void init() override;

  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  const plans::LimitPlanNode *plan_;

  std::unique_ptr<AbstractExecutor> child_executor_;

  size_t emitted_count_;
};

} // namespace turtle::execution::executors
