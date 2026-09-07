// -----------------------------------------------------------------------------
// Tier 3 (Execution) — turtle::execution::expressions
//
// Scope: the leaf expression nodes the executors evaluate:
//   - ConstantValueExpression: returns a fixed Value, ignoring the input tuple.
//   - ColumnValueExpression:   projects one column out of a tuple (and, for
//     joins, selects the left or right tuple by index).
//
// No fixture: expressions are cheap value objects. ColumnValueExpression is
// driven against a locally-built Tuple + ColumnSchema.
// -----------------------------------------------------------------------------
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/expressions/column_value_expression.hpp"
#include "turtle/execution/expressions/constant_value_expression.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::expressions {
namespace {

using catalog::Column;
using catalog::ColumnSchema;
using datatype::DataType;
using datatype::Value;
using storage::table::Tuple;

ColumnSchema two_col_schema() {
  std::vector<Column> cols;
  cols.emplace_back("a", DataType::INTEGER);
  cols.emplace_back("b", DataType::VARCHAR);
  return ColumnSchema(std::move(cols));
}

Tuple two_col_row(const ColumnSchema &schema, int32_t a, const std::string &b) {
  std::string b_copy = b;
  std::vector<Value> vals;
  vals.emplace_back(DataType::INTEGER, a);
  vals.emplace_back(DataType::VARCHAR, b_copy);
  return Tuple(&schema, std::move(vals));
}

// ---- ConstantValueExpression ----------------------------------------------

TEST(ConstantValueExpressionTest, EvaluateReturnsTheConstantIgnoringTuple) {
  ConstantValueExpression expr(
      Value(DataType::INTEGER, static_cast<int32_t>(7)));
  ColumnSchema schema = two_col_schema();

  // A constant ignores the tuple entirely (nullptr is a valid input here).
  Value v = expr.evaluate(nullptr, schema);
  EXPECT_EQ(v.get_as<int32_t>(), 7);
}

TEST(ConstantValueExpressionTest, EvaluateJoinAlsoReturnsTheConstant) {
  ConstantValueExpression expr(
      Value(DataType::INTEGER, static_cast<int32_t>(9)));
  ColumnSchema schema = two_col_schema();
  Value v = expr.evaluate_join(nullptr, schema, nullptr, schema);
  EXPECT_EQ(v.get_as<int32_t>(), 9);
}

TEST(ConstantValueExpressionTest, ReturnTypeMatchesConstantsDataType) {
  std::string s = "hello";
  ConstantValueExpression vc(Value(DataType::VARCHAR, s));
  EXPECT_EQ(vc.get_return_type().get_type(), DataType::VARCHAR);

  ConstantValueExpression vi(Value(DataType::INTEGER, static_cast<int32_t>(0)));
  EXPECT_EQ(vi.get_return_type().get_type(), DataType::INTEGER);
}

TEST(ConstantValueExpressionTest, ToStringRendersTheValue) {
  ConstantValueExpression expr(
      Value(DataType::INTEGER, static_cast<int32_t>(42)));
  EXPECT_EQ(expr.to_string(), "42");
}

// ---- ColumnValueExpression -------------------------------------------------

TEST(ColumnValueExpressionTest, ProjectsRequestedColumnFromTuple) {
  ColumnSchema schema = two_col_schema();
  Tuple row = two_col_row(schema, 5, "hi");

  ColumnValueExpression col0(0, 0, schema.get_columns()[0]);
  ColumnValueExpression col1(0, 1, schema.get_columns()[1]);

  EXPECT_EQ(col0.evaluate(&row, schema).get_as<int32_t>(), 5);
  EXPECT_EQ(col1.evaluate(&row, schema).to_string(), "hi");
}

TEST(ColumnValueExpressionTest, AccessorsExposeIndices) {
  ColumnSchema schema = two_col_schema();
  ColumnValueExpression expr(1, 3, schema.get_columns()[0]);
  EXPECT_EQ(expr.get_tuple_idx(), 1u);
  EXPECT_EQ(expr.get_col_idx(), 3u);
  EXPECT_EQ(expr.to_string(), "#1.3");
}

TEST(ColumnValueExpressionTest, EvaluateJoinPicksSideByTupleIndex) {
  ColumnSchema schema = two_col_schema();
  Tuple left = two_col_row(schema, 100, "L");
  Tuple right = two_col_row(schema, 200, "R");

  // tuple_idx 0 -> left side, tuple_idx 1 -> right side.
  ColumnValueExpression from_left(0, 0, schema.get_columns()[0]);
  ColumnValueExpression from_right(1, 0, schema.get_columns()[0]);

  EXPECT_EQ(
      from_left.evaluate_join(&left, schema, &right, schema).get_as<int32_t>(),
      100);
  EXPECT_EQ(
      from_right.evaluate_join(&left, schema, &right, schema).get_as<int32_t>(),
      200);
}

} // namespace
} // namespace turtle::execution::expressions
