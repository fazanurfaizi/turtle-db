#pragma once

#include <string>
#include <utility>
#include <vector>

#include "fmt/format.h"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"

namespace turtle::execution::plans {

/**
 * The ProjectionPlanNode a represents a project operation.
 * It computes expressions based on the input.
 */
class ProjectionPlanNode : public AbstractPlanNode {
public:
  std::vector<expressions::AbstractExpressionRef> expressions_;

  /**
   * Construct a new ProjectionPlanNode instance.
   * @param output The output schema of this projection node
   * @param expressions The expression to evaluate
   * @param child The child plan node
   */
  ProjectionPlanNode(
      catalog::ColumnSchemaRef output,
      std::vector<expressions::AbstractExpressionRef> expressions,
      AbstractPlanNodeRef child)
      : AbstractPlanNode(std::move(output), {std::move(child)}),
        expressions_(std::move(expressions)) {}

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::Projection; }

  /** @return The child plan node */
  auto get_child_plan() const -> AbstractPlanNodeRef {
    TURTLE_ASSERT(this->get_children().size() == 1,
                  "Projection should have exactly one child plan.");
    return this->get_child_at(0);
  }

  /** @return Projection expressions */
  auto get_expressions() const
      -> const std::vector<expressions::AbstractExpressionRef> & {
    return this->expressions_;
  }

  static auto infer_projection_schema(
      const std::vector<expressions::AbstractExpressionRef> &expressions)
      -> catalog::ColumnSchema;

  static auto rename_schema(const catalog::ColumnSchema &schema,
                            const std::vector<std::string> &col_names)
      -> catalog::ColumnSchema;

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(ProjectionPlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    std::string exprs;
    for (size_t i = 0; i < this->expressions_.size(); ++i) {
      if (i != 0) {
        exprs += ", ";
      }
      exprs += fmt::format("{}", this->expressions_[i]);
    }
    return fmt::format("Projection {{ exprs=[{}] }}", exprs);
  }
};

} // namespace turtle::execution::plans
