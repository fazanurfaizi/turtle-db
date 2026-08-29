// -----------------------------------------------------------------------------
// Tier 3 (Execution) — turtle::execution::plans
//
// Scope: the plan-tree nodes the planner hands to executors: ValuesPlanNode,
// InsertPlanNode, SeqScanPlanNode. These are lightweight immutable descriptors,
// so the tests cover construction, get_type() tagging, output-schema exposure,
// child wiring, and clone_with_children().
//
// No fixture: plans hold only shared_ptrs to schema/children.
// -----------------------------------------------------------------------------
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/expressions/constant_value_expression.hpp"
#include "turtle/execution/plans/insert_plan.hpp"
#include "turtle/execution/plans/seq_scan_plan.hpp"
#include "turtle/execution/plans/values_plan.hpp"

namespace turtle::execution::plans {
namespace {

using catalog::Column;
using catalog::ColumnSchema;
using catalog::ColumnSchemaRef;
using datatype::DataType;
using datatype::Value;
using expressions::AbstractExpressionRef;
using expressions::ConstantValueExpression;

ColumnSchemaRef make_schema_ref() {
  std::vector<Column> cols;
  cols.emplace_back("id", DataType::INTEGER);
  cols.emplace_back("name", DataType::VARCHAR);
  return std::make_shared<const ColumnSchema>(std::move(cols));
}

// One row of two constants: (id, name).
std::vector<std::vector<AbstractExpressionRef>> one_row_values() {
  std::vector<AbstractExpressionRef> row;
  row.push_back(std::make_shared<ConstantValueExpression>(
      Value(DataType::INTEGER, static_cast<int32_t>(1))));
  std::string name = "a";
  row.push_back(
      std::make_shared<ConstantValueExpression>(Value(DataType::VARCHAR, name)));
  return {std::move(row)};
}

// ---- ValuesPlanNode --------------------------------------------------------

TEST(ValuesPlanNodeTest, TypeAndValuesExposed) {
  auto schema = make_schema_ref();
  ValuesPlanNode plan(schema, one_row_values());

  EXPECT_EQ(plan.get_type(), PlanType::Values);
  EXPECT_EQ(plan.get_values().size(), 1u);
  EXPECT_EQ(plan.output_schema().get_column_count(), 2u);
}

TEST(ValuesPlanNodeTest, HasNoChildren) {
  auto schema = make_schema_ref();
  ValuesPlanNode plan(schema, one_row_values());
  EXPECT_TRUE(plan.get_children().empty());
}

// ---- SeqScanPlanNode -------------------------------------------------------

TEST(SeqScanPlanNodeTest, CarriesTableIdentity) {
  auto schema = make_schema_ref();
  SeqScanPlanNode plan(schema, TableOid{5}, "people");

  EXPECT_EQ(plan.get_type(), PlanType::SeqScan);
  EXPECT_EQ(plan.get_table_oid(), 5u);
  EXPECT_EQ(plan.table_name_, "people");
  EXPECT_EQ(&plan.output_schema(), schema.get()); // shares the same schema
}

// ---- InsertPlanNode --------------------------------------------------------

TEST(InsertPlanNodeTest, WrapsChildAndTargetTable) {
  auto out_schema = make_schema_ref();
  auto child = std::make_shared<ValuesPlanNode>(make_schema_ref(),
                                                one_row_values());
  InsertPlanNode plan(out_schema, child, TableOid{3});

  EXPECT_EQ(plan.get_type(), PlanType::Insert);
  EXPECT_EQ(plan.get_table_oid(), 3u);
  ASSERT_EQ(plan.get_children().size(), 1u);
  EXPECT_EQ(plan.get_child_plan(), child);
}

// ---- clone_with_children ---------------------------------------------------

TEST(PlanCloneTest, SeqScanCloneCarriesNewChildrenAndKeepsIdentity) {
  auto schema = make_schema_ref();
  SeqScanPlanNode plan(schema, TableOid{9}, "t");

  auto clone = plan.clone_with_children({}); // scans have no children
  ASSERT_NE(clone, nullptr);
  EXPECT_EQ(clone->get_type(), PlanType::SeqScan);
  auto *as_scan = dynamic_cast<SeqScanPlanNode *>(clone.get());
  ASSERT_NE(as_scan, nullptr);
  EXPECT_EQ(as_scan->get_table_oid(), 9u);
}

TEST(PlanCloneTest, InsertCloneAcceptsReplacementChild) {
  auto out_schema = make_schema_ref();
  auto child0 = std::make_shared<ValuesPlanNode>(make_schema_ref(),
                                                 one_row_values());
  InsertPlanNode plan(out_schema, child0, TableOid{1});

  auto child1 = std::make_shared<ValuesPlanNode>(make_schema_ref(),
                                                 one_row_values());
  auto clone = plan.clone_with_children({child1});
  ASSERT_EQ(clone->get_children().size(), 1u);
  EXPECT_EQ(clone->get_child_at(0), child1);
}

} // namespace
} // namespace turtle::execution::plans
