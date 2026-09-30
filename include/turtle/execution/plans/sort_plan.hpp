#pragma once

#include "turtle/binder/bound_order_by.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"
#include <vector>

namespace turtle::execution::plans {

/**
 * The SortPlanNode represents a sort operation. It wioll sort the input with
 * the given predicate.
 */
class SortPlanNode : public AbstractPlanNode {
public:
  /**
   * Construct a new SortPlanNode instance.
   * @param output The output schema of this sort plan node
   * @param child The child plan node
   * @param order_bys The sort expressions and ther order by types.
   */
  SortPlanNode(catalog::ColumnSchemaRef output, AbstractPlanNodeRef child,
               std::vector<binder::OrderBy> order_bys)
      : AbstractPlanNode(std::move(output), {std::move(child)}),
        order_bys_(order_bys) {}

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::Sort; }

  /** @return The child plan node */
  auto get_child_plan() const -> AbstractPlanNodeRef {
    TURTLE_ASSERT(this->get_children().size() == 1,
                  "Sort should have exactly one child plan.");
    return this->get_child_at(0);
  }

  /** @return Get sort by expressions */
  auto get_order_bys() const -> const std::vector<binder::OrderBy> & {
    return this->order_bys_;
  }

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(SortPlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    std::string result = "Sort { order_bys=[";

    for (size_t i = 0; i < order_bys_.size(); ++i) {
      if (i > 0) {
        result += ", ";
      }

      // Format Direction (ASC / DESC)
      const auto &order_type = std::get<binder::OrderByType>(order_bys_[i]);
      std::string order_str;
      switch (order_type) {
      case binder::OrderByType::ASC:
      case binder::OrderByType::DEFAULT:
        order_str = "ASC";
        break;
      case binder::OrderByType::DESC:
        order_str = "DESC";
        break;
      default:
        order_str = "INVALID";
        break;
      }

      // Format Expression + Direction
      const auto &expr =
          std::get<expressions::AbstractExpressionRef>(order_bys_[i]);
      result +=
          "(" + (expr ? expr->to_string() : "null") + ", " + order_str + ")";
    }

    result += "] }";
    return result;
  }

private:
  std::vector<binder::OrderBy> order_bys_;
};

} // namespace turtle::execution::plans
