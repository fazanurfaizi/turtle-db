#pragma once

#include "fmt/format.h"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"
#include <vector>

namespace turtle::execution::plans {

/**
 * The ValuesPlanNode represents rows of values. For exampel,
 * `INSERT INTO table VALUES ((0, 1), (1, 2))`, where we will have
 * `(0, 1)` and `(1, 2)` as the output of this executor.
 */
class ValuesPlanNode : public AbstractPlanNode {
public:
  std::vector<std::vector<expressions::AbstractExpressionRef>> values_;

  /**
   * Construct a new ValuesPlanNode instance.
   * @param output The output schema of this values plan node
   * @param values The values produced by this plan node
   */
  explicit ValuesPlanNode(
      catalog::ColumnSchemaRef output,
      std::vector<std::vector<expressions::AbstractExpressionRef>> values)
      : AbstractPlanNode(std::move(output), {}), values_(std::move(values)) {}

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::Values; }

  auto get_values() const
      -> const std::vector<std::vector<expressions::AbstractExpressionRef>> & {
    return this->values_;
  }

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(ValuesPlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    return fmt::format("Values {{ rows={} }}", this->values_.size());
  }
};

} // namespace turtle::execution::plans
