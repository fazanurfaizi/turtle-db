#pragma once

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/aggregation/aggregate_function.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"
#include <vector>

namespace turtle::execution::plans {

/**
 * @brief AggregationPLanNode represents GROUP BY and aggregate functions.
 */
class AggregationPlanNode : public AbstractPlanNode {
public:
  /**
   * Construct a new AggregationPlanNode.
   * @param output_schema The output format of this plan node
   * @param child The child plan to aggregate data over
   * @param group_bys The group by clause of the aggregation
   * @param aggregates The expressions that we are aggregating
   * @param agg_types The types that we are aggregating
   */
  AggregationPlanNode(
      catalog::ColumnSchemaRef output_schema, AbstractPlanNodeRef child,
      std::vector<expressions::AbstractExpressionRef> group_bys,
      std::vector<expressions::AbstractExpressionRef> aggregates,
      std::vector<const aggregation::AggregateFunction *> agg_funcs)
      : AbstractPlanNode(std::move(output_schema), {std::move(child)}),
        group_bys_(std::move(group_bys)), aggregates_(std::move(aggregates)),
        agg_funcs_(std::move(agg_funcs)) {}

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::Aggregation; }

  /** @return the child of this aggregation plan node */
  auto get_child_plan() const -> AbstractPlanNodeRef {
    TURTLE_ASSERT(this->get_children().size() == 1,
                  "Aggregation expected to only have one child.");
    return this->get_child_at(0);
  }

  /** @return The idx'th group by expression */
  auto get_group_by_at(uint32_t idx) const
      -> const expressions::AbstractExpressionRef & {
    return this->group_bys_[idx];
  }

  /** @return The group by expressions */
  auto get_group_bys() const
      -> const std::vector<expressions::AbstractExpressionRef> & {
    return this->group_bys_;
  }

  /** @return The idx'th aggregate expression */
  auto get_aggregate_at(uint32_t idx) const
      -> const expressions::AbstractExpressionRef & {
    return this->aggregates_[idx];
  }

  /** @return The aggregate expressions */
  auto get_aggregates() const
      -> const std::vector<expressions::AbstractExpressionRef> & {
    return this->aggregates_;
  }

  /** @return The aggregate functions */
  auto get_aggregate_functions() const
      -> const std::vector<const aggregation::AggregateFunction *> & {
    return this->agg_funcs_;
  }

  static auto infer_agg_schema(
      const std::vector<expressions::AbstractExpressionRef> &group_bys,
      const std::vector<expressions::AbstractExpressionRef> &aggregates,
      const std::vector<const aggregation::AggregateFunction *> &agg_funcs)
      -> catalog::ColumnSchema;

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(AggregationPlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    std::string result = "Aggregation { ";

    result += "group_bys=[";
    for (size_t i = 0; i < group_bys_.size(); ++i) {
      if (i > 0)
        result += ", ";
      result += group_bys_[i]->to_string();
    }
    result += "], aggregates=[";
    for (size_t i = 0; i < aggregates_.size(); ++i) {
      if (i > 0)
        result += ", ";
      result += aggregates_[i]->to_string();
    }
    result += "] }";

    return result;
  }

private:
  std::vector<expressions::AbstractExpressionRef> group_bys_;
  std::vector<expressions::AbstractExpressionRef> aggregates_;
  std::vector<const aggregation::AggregateFunction *> agg_funcs_;
};

} // namespace turtle::execution::plans
