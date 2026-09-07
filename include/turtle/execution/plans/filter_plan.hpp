#pragma once

#include "fmt/format.h"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"
#include <memory>
#include <string>
#include <utility>

namespace turtle::execution::plans {

/**
 * The FilterPlanNode represents a filter operation.
 * It retains any tuple that satisfies the predicate in the child.
 */
class FilterPlanNode : public AbstractPlanNode {
public:
  /** The predicate that all returned tuples must satisfy */
  expressions::AbstractExpressionRef predicate_;

  /**
   * Construct a new FilterPlanNode instance.
   * @param output The output schema of this filter plan node
   * @param predicate The predicate applied during the scan operation
   * @param child The child plan node
   */
  FilterPlanNode(catalog::ColumnSchemaRef output,
                 expressions::AbstractExpressionRef predicate,
                 AbstractPlanNodeRef child)
      : AbstractPlanNode(std::move(output), {std::move(child)}),
        predicate_{std::move(predicate)} {}

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::Filter; }

  /** @return The predicate to test tuple against; tuples should only be
   * returned if they evaluated to true */
  auto get_predicate() const -> const expressions::AbstractExpressionRef & {
    return this->predicate_;
  }

  /** @return The child plan node */
  auto get_child_plan() const -> AbstractPlanNodeRef {
    TURTLE_ASSERT(this->get_children().size() == 1,
                  "Filter should have exactly one child plan.");
    return this->get_child_at(0);
  }

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(FilterPlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    return fmt::format("Filter {{ predicate={} }}", this->predicate_);
  }
};

} // namespace turtle::execution::plans
