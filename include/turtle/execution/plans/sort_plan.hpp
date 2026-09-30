#pragma once

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"
#include <cstdint>
#include <tuple>
#include <vector>

namespace turtle::execution::plans {

/**
 * @brief All types of order-bys.
 */
enum class OrderByType : uint8_t {
  INVALID = 0, // Invalid order by type
  DEFAULT = 1, // Default order by type
  ASC = 2,     // Ascending order by type
  DESC = 3,    // Descending order by type
};

/**
 * @brief All types order by nulls.
 */
enum class OrderByNullType : uint8_t {
  DEFAULT = 0,     // Default order by type
  NULLS_FIRST = 1, // Ascending order by type
  NULLS_LAST = 2,  // Descending order by type
};

using OrderBy = std::tuple<OrderByType, OrderByNullType,
                           expressions::AbstractExpressionRef>;

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
  SortPlanNode(catalog::ColumnSchemaRef output, std::vector<OrderBy> order_bys,
               AbstractPlanNodeRef child)
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
  auto get_order_bys() const -> const std::vector<OrderBy> & {
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
      const auto &order_type = std::get<OrderByType>(order_bys_[i]);
      std::string order_str;
      switch (order_type) {
      case OrderByType::ASC:
      case OrderByType::DEFAULT:
        order_str = "ASC";
        break;
      case OrderByType::DESC:
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
  std::vector<OrderBy> order_bys_;
};

} // namespace turtle::execution::plans
