#pragma once

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/execution/plans/values_plan.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <vector>

namespace turtle::execution::executors {

/**
 * The ValuesExecutor executor produces rows of values.
 */
class ValuesExecutor : public AbstractExecutor {
public:
  ValuesExecutor(ExecutorContext *exec_ctx, const plans::ValuesPlanNode *plan);

  void init() override;
  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  /** @return The output schema for the values */
  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  const plans::ValuesPlanNode *plan_;
  const catalog::ColumnSchema dummy_schema_;
  size_t cursor_{0};
};

} // namespace turtle::execution::executors
