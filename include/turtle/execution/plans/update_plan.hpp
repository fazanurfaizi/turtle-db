#pragma once

#include "fmt/format.h"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"
#include <vector>

namespace turtle::execution::plans {

/**
 * The UpdatePlanNode identifies a table that should be updated.
 * The tuple(s) to be updated come from the child of the UpdateExecutor.
 */
class UpdatePlanNode : public AbstractPlanNode {
public:
  /**
   * Construct a new UpdatePlanNode instance.
   * @param child The child plan to obtain tuple from
   * @param table_oid The identifier of the table that should be updated
   * @param target_expressions The target expression for new tuples
   */
  UpdatePlanNode(
      catalog::ColumnSchemaRef output, AbstractPlanNodeRef child,
      TableOid table_oid,
      std::vector<expressions::AbstractExpressionRef> target_expressions)
      : AbstractPlanNode(std::move(output), {std::move(child)}),
        table_oid_(table_oid),
        target_expressions_(std::move(target_expressions)) {}

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::Update; }

  /** @return The identifier of the table that should be updated */
  auto get_table_oid() const -> TableOid { return this->table_oid_; }

  /** @return The child plan providing tuples to be inserted */
  auto get_child_plan() const -> AbstractPlanNodeRef {
    TURTLE_ASSERT(this->get_children().size() == 1,
                  "UPDATE should have exactly one child plan.");
    return this->get_child_at(0);
  }

  auto get_target_expressions() const
      -> std::vector<expressions::AbstractExpressionRef> {
    return this->target_expressions_;
  }

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(UpdatePlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    return fmt::format("Update {{ table_oid={} }}", this->table_oid_);
  }

private:
  /** The table to be updated. */
  TableOid table_oid_;

  /** The new expression at each column */
  std::vector<expressions::AbstractExpressionRef> target_expressions_;
};

} // namespace turtle::execution::plans
