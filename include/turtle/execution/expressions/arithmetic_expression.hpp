#pragma once

#include "fmt/base.h"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/exceptions.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
namespace turtle::execution::expressions {

/** ArithmeticType represents the type of computation that we want to perform.
 */
enum class ArithmeticType {
  Addition,
  Subtraction,
  Multiplication,
  Division,
  Modulo,
  Exponentiation,
  FloorDivision
};
} // namespace turtle::execution::expressions

template <>
struct fmt::formatter<turtle::execution::expressions::ArithmeticType>
    : formatter<string_view> {
  template <typename FormatContext>
  auto format(turtle::execution::expressions::ArithmeticType c,
              FormatContext &ctx) const {
    string_view name;
    switch (c) {
    case turtle::execution::expressions::ArithmeticType::Addition:
      name = "+";
      break;
    case turtle::execution::expressions::ArithmeticType::Subtraction:
      name = "-";
      break;
    case turtle::execution::expressions::ArithmeticType::Multiplication:
      name = "*";
      break;
    case turtle::execution::expressions::ArithmeticType::Division:
      name = "/";
      break;
    case turtle::execution::expressions::ArithmeticType::Modulo:
      name = "%";
      break;
    case turtle::execution::expressions::ArithmeticType::Exponentiation:
      name = "**";
      break;
    case turtle::execution::expressions::ArithmeticType::FloorDivision:
      name = "//";
      break;
    default:
      name = "Unknown";
      break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};

namespace turtle::execution::expressions {

class ArithmeticExpression : public AbstractExpression {
public:
  ArithmeticType arithmetic_type_;

  /** Creates a new arithmetic expression representing (left op right).
   *
   */
  ArithmeticExpression(AbstractExpressionRef left, AbstractExpressionRef right,
                       ArithmeticType arithmetic_type)
      : AbstractExpression(
            {left, right},
            catalog::Column{"<val>", infer_return_type(left, right)}),
        arithmetic_type_(arithmetic_type) {}

  auto evaluate(const storage::table::Tuple *tuple,
                const catalog::ColumnSchema &schema) const
      -> datatype::Value override {
    datatype::Value lhs = this->get_child_at(0)->evaluate(tuple, schema);
    datatype::Value rhs = this->get_child_at(1)->evaluate(tuple, schema);
    return this->perform_computation(lhs, rhs);
  }

  auto evaluate_join(const storage::table::Tuple *left_tuple,
                     const catalog::ColumnSchema &left_schema,
                     const storage::table::Tuple *right_tuple,
                     const catalog::ColumnSchema &right_schema) const
      -> datatype::Value override {
    datatype::Value lhs = this->get_child_at(0)->evaluate_join(
        left_tuple, left_schema, right_tuple, right_schema);
    datatype::Value rhs = this->get_child_at(1)->evaluate_join(
        left_tuple, left_schema, right_tuple, right_schema);
    return this->perform_computation(lhs, rhs);
  }

  auto to_string() const -> std::string override {
    return fmt::format("({}{}{})", *this->get_child_at(0),
                       this->arithmetic_type_, *this->get_child_at(1));
  }

  TURTLE_EXPR_CLONE_WITH_CHILDREN(ArithmeticExpression);

private:
  static auto is_numeric(datatype::DataType t) -> bool {
    switch (t) {
    case datatype::DataType::TINYINT:
    case datatype::DataType::SMALLINT:
    case datatype::DataType::INTEGER:
    case datatype::DataType::BIGINT:
    case datatype::DataType::DECIMAL:
      return true;
    default:
      return false;
    }
  }

  static auto numeric_rank(datatype::DataType t) -> int {
    switch (t) {
    case datatype::DataType::TINYINT:
      return 1;
    case datatype::DataType::SMALLINT:
      return 2;
    case datatype::DataType::INTEGER:
      return 3;
    case datatype::DataType::BIGINT:
      return 4;
    case datatype::DataType::DECIMAL:
      return 5;
    default:
      return 0;
    }
  }

  /** Result type of (left op right): the higher-ranked operand type */
  static auto infer_return_type(const AbstractExpressionRef &left,
                                const AbstractExpressionRef &right)
      -> datatype::DataType {
    datatype::DataType lt = left->get_return_type().get_type();
    datatype::DataType rt = right->get_return_type().get_type();
    if (!is_numeric(lt) || !is_numeric(rt)) {
      throw NotImplementedException(
          "ArithmeticExpression only supports numeric operands");
    }
    return numeric_rank(lt) >= numeric_rank(rt) ? lt : rt;
  }

  auto perform_computation(const datatype::Value &left,
                           const datatype::Value &right) const
      -> datatype::Value {
    switch (this->arithmetic_type_) {
    case ArithmeticType::Addition:
      return left.add(right);
    case ArithmeticType::Subtraction:
      return left.subtract(right);
    case ArithmeticType::Multiplication:
      return left.multiply(right);
    case ArithmeticType::Division:
      return left.divide(right);
    case ArithmeticType::Modulo:
      return left.modulo(right);
    case ArithmeticType::Exponentiation:
    case ArithmeticType::FloorDivision:
      throw NotImplementedException("exponentiation and floor-division are not "
                                    "supported by the numeric Type layer yet");
    }
    UNREACHABLE("unhandled ArithmeticType");
  }
};

} // namespace turtle::execution::expressions
