#pragma once

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"
#include <vector>

namespace turtle::execution::plans {

/**
 * Limit constraints the number of output tuples produced by its child executor.
 */
class LimitPlanNode : public AbstractPlanNode {
public:
  /**
   * Construct a new LimitPlanNode instance.
   * @param output The output schema of this sort plan node
   * @param child The child plan node
   * @param order_bys The sort expressions and ther order by types.
   */
  LimitPlanNode(catalog::ColumnSchemaRef output, AbstractPlanNodeRef child,
                size_t limit)
      : AbstractPlanNode(std::move(output), {std::move(child)}), limit_(limit) {
  }

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::Limit; }

  /** @return The child plan node */
  auto get_child_plan() const -> AbstractPlanNodeRef {
    TURTLE_ASSERT(this->get_children().size() == 1,
                  "Limit should have exactly one child plan.");
    return this->get_child_at(0);
  }

  /** @return Get the limit */
  auto get_limit() const -> size_t { return this->limit_; }

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(LimitPlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    return fmt::format("Filter {{ limit={} }}", this->limit_);
  }

private:
  size_t limit_;
};

} // namespace turtle::execution::plans
