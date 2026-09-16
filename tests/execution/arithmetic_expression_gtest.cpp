// Tests for ArithmeticExpression routed through the datatype NumericType path.
// Covers same-type math, cross-type promotion, NULL propagation, divide-by-zero,
// and the unsupported-operand / unsupported-op guards.

#include <memory>

#include "gtest/gtest.h"

#include "turtle/catalog/column_schema.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/datatype/value_factory.hpp"
#include "turtle/execution/expressions/arithmetic_expression.hpp"
#include "turtle/execution/expressions/constant_value_expression.hpp"

namespace turtle::execution::expressions {

using datatype::DataType;
using datatype::Value;
using datatype::ValueFactory;

namespace {

auto konst(const Value &v) -> AbstractExpressionRef {
  return std::make_shared<ConstantValueExpression>(v);
}

auto integer(int32_t v) -> AbstractExpressionRef {
  return konst(ValueFactory::get_integer_value(v));
}

auto bigint(int64_t v) -> AbstractExpressionRef {
  return konst(Value(DataType::BIGINT, v));
}

// Constants ignore tuple/schema, so evaluation only needs throwaway args.
auto eval(const AbstractExpression &expr) -> Value {
  static const catalog::ColumnSchema kEmpty{std::vector<catalog::Column>{}};
  return expr.evaluate(nullptr, kEmpty);
}

} // namespace

TEST(ArithmeticExpressionTest, IntegerAddition) {
  ArithmeticExpression expr(integer(7), integer(5), ArithmeticType::Addition);
  Value res = eval(expr);
  EXPECT_EQ(expr.get_return_type().get_type(), DataType::INTEGER);
  EXPECT_EQ(res.get_as<int32_t>(), 12);
}

TEST(ArithmeticExpressionTest, IntegerSubtractMultiplyDivideModulo) {
  EXPECT_EQ(eval(ArithmeticExpression(integer(7), integer(5),
                                      ArithmeticType::Subtraction))
                .get_as<int32_t>(),
            2);
  EXPECT_EQ(eval(ArithmeticExpression(integer(7), integer(5),
                                      ArithmeticType::Multiplication))
                .get_as<int32_t>(),
            35);
  EXPECT_EQ(eval(ArithmeticExpression(integer(20), integer(5),
                                      ArithmeticType::Division))
                .get_as<int32_t>(),
            4);
  EXPECT_EQ(
      eval(ArithmeticExpression(integer(7), integer(5), ArithmeticType::Modulo))
          .get_as<int32_t>(),
      2);
}

TEST(ArithmeticExpressionTest, PromotesToBigintWhenMixed) {
  // INTEGER + BIGINT must widen to BIGINT, both statically and at runtime.
  ArithmeticExpression expr(integer(1000000), bigint(3000000000LL),
                            ArithmeticType::Addition);
  EXPECT_EQ(expr.get_return_type().get_type(), DataType::BIGINT);
  Value res = eval(expr);
  EXPECT_EQ(res.data_type(), DataType::BIGINT);
  EXPECT_EQ(res.get_as<int64_t>(), 3001000000LL);
}

TEST(ArithmeticExpressionTest, NullOperandPropagates) {
  auto null_int = konst(ValueFactory::get_null_value_by_type(DataType::INTEGER));
  ArithmeticExpression expr(null_int, integer(5), ArithmeticType::Addition);
  Value res = eval(expr);
  EXPECT_TRUE(res.is_null());
}

TEST(ArithmeticExpressionTest, DivideByZeroThrows) {
  ArithmeticExpression expr(integer(9), integer(0), ArithmeticType::Division);
  EXPECT_ANY_THROW(eval(expr));
}

TEST(ArithmeticExpressionTest, UnsupportedOpThrows) {
  ArithmeticExpression expr(integer(2), integer(3),
                            ArithmeticType::Exponentiation);
  EXPECT_THROW(eval(expr), NotImplementedException);
}

TEST(ArithmeticExpressionTest, NonNumericOperandRejectedAtConstruction) {
  auto boolean = konst(ValueFactory::get_boolean_value(true));
  EXPECT_THROW(
      ArithmeticExpression(boolean, integer(1), ArithmeticType::Addition),
      NotImplementedException);
}

} // namespace turtle::execution::expressions
