#pragma once

#include <string>
#include <utility>
#include <vector>

#include "fmt/format.h"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"

namespace turtle::execution::plans {

/**
 * The DeletePlanNode identifies a table from which tuples should be deleted.
 * The tuple(s) to be updated come from the child of the DeleteExecutor.
 */
class DeletePlanNode : public AbstractPlanNode {
public:
  /**
   * Construct a new DeletePlanNode.
   * @param child The child plan to obtain values from
   * @param table_id The identifier of the table that should be inserted into
   */
  DeletePlanNode(catalog::ColumnSchemaRef output, AbstractPlanNodeRef child,
                 TableOid table_oid)
      : AbstractPlanNode(std::move(output), {std::move(child)}),
        table_oid_(table_oid) {}

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::Delete; }

  /** @return The identifier of the table into which tuples are inserted */
  auto get_table_oid() const -> TableOid { return this->table_oid_; }

  /** @return The child plan providing tuples to be inserted */
  auto get_child_plan() const -> AbstractPlanNodeRef {
    TURTLE_ASSERT(this->get_children().size() == 1,
                  "Delete should have only one child plan.");
    return this->get_child_at(0);
  }

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(DeletePlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    return fmt::format("Delete {{ table_oid={} }}", this->table_oid_);
  }

private:
  TableOid table_oid_;
};

} // namespace turtle::execution::plans
