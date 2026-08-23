#pragma once

#include <string>
#include <utility>

#include "fmt/format.h"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/config.hpp"
#include "turtle/execution/plans/abstract_plan.hpp"

namespace turtle::execution::plans {

/**
 * The SeqScanExecutor represent a sequential table scan operation.
 */
class SeqScanPlanNode : public AbstractPlanNode {
public:
  /** The table whose tuples should be scanned */
  TableOid table_oid_;

  /** The table name */
  std::string table_name_;

  /**
   * The predicate to filter in seqscan.
   */
  // expressions::AbstractExpressionRef filter_predicate_;

  /**
   * Construct a new SeqScanPlanNode instance.
   * @param output The output schema of this sequential scan plan node
   * @param table_oid The identifier of table to be scanne
   */
  SeqScanPlanNode(catalog::ColumnSchemaRef output, TableOid table_oid,
                  std::string table_name)
      : AbstractPlanNode(std::move(output), {}), table_oid_{table_oid},
        table_name_(std::move(table_name)) {}

  /** @return The type of the plan node */
  auto get_type() const -> PlanType override { return PlanType::SeqScan; }

  /** @return The identifier of the table that should be scanned */
  auto get_table_oid() const -> TableOid { return this->table_oid_; }

  // static auto InferScanSchema(const BoundBaseTableRef &table_ref)
  //     -> catalog::ColumnSchema;

  TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(SeqScanPlanNode);

protected:
  auto plan_node_to_string() const -> std::string override {
    // if (this->filter_predicate_) {
    //   return fmt::format("SeqScan {{ table={}, filter={} }}",
    //   this->table_name_,
    //                      this->filter_predicate_);
    // }
    return fmt::format("SeqScan {{ table={} }}", this->table_name_);
  }
};

} // namespace turtle::execution::plans
