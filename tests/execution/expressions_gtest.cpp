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
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/datatype/value_factory.hpp"
#include "turtle/execution/expressions/column_value_expression.hpp"
#include "turtle/execution/expressions/comparison_expression.hpp"
#include "turtle/execution/expressions/constant_value_expression.hpp"
#include "turtle/execution/expressions/logic_expression.hpp"
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

// ---- NULL semantics / three-valued logic ----------------------------------
//
// SQL 3VL: a comparison touching NULL is "unknown" (not true/false), and AND/OR
// only collapse unknown to a concrete answer when the other operand is decisive
// (false AND unknown = false, true OR unknown = true). These lock that in — the
// datatype `compare_*` + LogicExpression already implement it, and were
// previously untested.

using datatype::ValueFactory;

// Tri mirrors the three logical outcomes so truth tables read declaratively.
enum class Tri { True, False, Null };

AbstractExpressionRef konst(const Value &v) {
  return std::make_shared<ConstantValueExpression>(v);
}
AbstractExpressionRef int_val(int32_t v) {
  return konst(ValueFactory::get_integer_value(v));
}
AbstractExpressionRef null_int() {
  return konst(ValueFactory::get_null_value_by_type(DataType::INTEGER));
}
AbstractExpressionRef boolean(bool b) {
  return konst(ValueFactory::get_boolean_value(b));
}
AbstractExpressionRef null_bool() {
  return konst(ValueFactory::get_null_value_by_type(DataType::BOOLEAN));
}

// Constants ignore the tuple/schema, so a throwaway schema is fine.
Tri eval_tri(const AbstractExpression &expr) {
  static const ColumnSchema kSchema = two_col_schema();
  Value v = expr.evaluate(nullptr, kSchema);
  if (v.is_null()) {
    return Tri::Null;
  }
  return v.get_as<bool>() ? Tri::True : Tri::False;
}

Tri cmp(AbstractExpressionRef l, AbstractExpressionRef r, ComparisonType t) {
  return eval_tri(ComparisonExpression(std::move(l), std::move(r), t));
}

Tri logic(AbstractExpressionRef l, AbstractExpressionRef r, LogicType t) {
  return eval_tri(LogicExpression(std::move(l), std::move(r), t));
}

// --- Comparisons with NULL yield unknown, for every operator ---------------

TEST(ComparisonNullTest, EveryOperatorWithNullOperandIsUnknown) {
  for (auto op : {ComparisonType::Equal, ComparisonType::NotEqual,
                  ComparisonType::LessThan, ComparisonType::LessThanOrEqual,
                  ComparisonType::GreaterThan,
                  ComparisonType::GreaterThanOrEqual}) {
    EXPECT_EQ(cmp(int_val(5), null_int(), op), Tri::Null);
    EXPECT_EQ(cmp(null_int(), int_val(5), op), Tri::Null);
    EXPECT_EQ(cmp(null_int(), null_int(), op), Tri::Null);
  }
}

TEST(ComparisonNullTest, BooleanComparedWithNullIsUnknown) {
  EXPECT_EQ(cmp(boolean(true), null_bool(), ComparisonType::Equal), Tri::Null);
  EXPECT_EQ(cmp(null_bool(), boolean(false), ComparisonType::NotEqual),
            Tri::Null);
}

// Non-NULL sanity: the true/false paths still work (guards against a "return
// Null for everything" regression that would trivially pass the above).
TEST(ComparisonNullTest, NonNullComparisonsAreDefinite) {
  EXPECT_EQ(cmp(int_val(5), int_val(5), ComparisonType::Equal), Tri::True);
  EXPECT_EQ(cmp(int_val(5), int_val(6), ComparisonType::Equal), Tri::False);
  EXPECT_EQ(cmp(int_val(5), int_val(6), ComparisonType::LessThan), Tri::True);
  EXPECT_EQ(cmp(int_val(6), int_val(5), ComparisonType::LessThan), Tri::False);
}

// --- AND / OR full 3x3 truth tables ----------------------------------------

TEST(LogicNullTest, AndTruthTable) {
  EXPECT_EQ(logic(boolean(true), boolean(true), LogicType::And), Tri::True);
  EXPECT_EQ(logic(boolean(true), boolean(false), LogicType::And), Tri::False);
  EXPECT_EQ(logic(boolean(false), boolean(false), LogicType::And), Tri::False);
  // The cases humans get wrong: false is decisive, so it beats unknown.
  EXPECT_EQ(logic(boolean(false), null_bool(), LogicType::And), Tri::False);
  EXPECT_EQ(logic(null_bool(), boolean(false), LogicType::And), Tri::False);
  // true AND unknown stays unknown (depends on the unknown).
  EXPECT_EQ(logic(boolean(true), null_bool(), LogicType::And), Tri::Null);
  EXPECT_EQ(logic(null_bool(), boolean(true), LogicType::And), Tri::Null);
  EXPECT_EQ(logic(null_bool(), null_bool(), LogicType::And), Tri::Null);
}

TEST(LogicNullTest, OrTruthTable) {
  EXPECT_EQ(logic(boolean(true), boolean(true), LogicType::Or), Tri::True);
  EXPECT_EQ(logic(boolean(true), boolean(false), LogicType::Or), Tri::True);
  EXPECT_EQ(logic(boolean(false), boolean(false), LogicType::Or), Tri::False);
  // true is decisive, so it beats unknown.
  EXPECT_EQ(logic(boolean(true), null_bool(), LogicType::Or), Tri::True);
  EXPECT_EQ(logic(null_bool(), boolean(true), LogicType::Or), Tri::True);
  // false OR unknown stays unknown.
  EXPECT_EQ(logic(boolean(false), null_bool(), LogicType::Or), Tri::Null);
  EXPECT_EQ(logic(null_bool(), boolean(false), LogicType::Or), Tri::Null);
  EXPECT_EQ(logic(null_bool(), null_bool(), LogicType::Or), Tri::Null);
}

} // namespace
} // namespace turtle::execution::expressions
