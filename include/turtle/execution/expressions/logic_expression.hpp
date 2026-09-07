#pragma once

#include "fmt/base.h"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/exceptions.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/datatype/value_factory.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <string>
#include <utility>
#include <vector>

namespace turtle::execution::expressions {

/** ArithmeticType represents the type of logic operation that we want to
 * perform. */
enum class LogicType { And, Or };

} // namespace turtle::execution::expressions

template <>
struct fmt::formatter<turtle::execution::expressions::LogicType>
    : formatter<string_view> {
  template <typename FormatContext>
  auto format(turtle::execution::expressions::LogicType c,
              FormatContext &ctx) const {
    string_view name;
    switch (c) {
    case turtle::execution::expressions::LogicType::And:
      name = "and";
      break;
    case turtle::execution::expressions::LogicType::Or:
      name = "or";
      break;
    default:
      name = "Unknown";
      break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};

namespace turtle::execution::expressions {

/**
 * ArithmeticType represents two expressions being computed.
 */
class LogicExpression : public AbstractExpression {
public:
  LogicType logic_type_;

  /** Creates a new comparison expression reprensting (left comp_type right) */
  LogicExpression(AbstractExpressionRef left, AbstractExpressionRef right,
                  LogicType logic_type)
      : AbstractExpression(
            {std::move(left), std::move(right)},
            catalog::Column{"<val>", datatype::DataType::BOOLEAN}),
        logic_type_(logic_type) {
    if (this->get_child_at(0)->get_return_type().get_type() !=
            datatype::DataType::BOOLEAN ||
        this->get_child_at(1)->get_return_type().get_type() !=
            datatype::DataType::BOOLEAN) {
      throw turtle::NotImplementedException("expect boolean from either side");
    }
  }

  auto evaluate(const storage::table::Tuple *tuple,
                const catalog::ColumnSchema &schema) const
      -> datatype::Value override {
    datatype::Value lhs = this->get_child_at(0)->evaluate(tuple, schema);
    datatype::Value rhs = this->get_child_at(1)->evaluate(tuple, schema);
    return datatype::ValueFactory::get_boolean_value(
        this->perform_computation(lhs, rhs));
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
    return datatype::ValueFactory::get_boolean_value(
        this->perform_computation(lhs, rhs));
  }

  /** @return the string representation of the expression node and its children
   */
  auto to_string() const -> std::string override {
    return fmt::format("({}{}{})", *this->get_child_at(0), logic_type_,
                       *this->get_child_at(1));
  }

  TURTLE_EXPR_CLONE_WITH_CHILDREN(LogicExpression);

private:
  auto get_bool_as_cmp_bool(const datatype::Value &val) const
      -> datatype::CmpBool {
    if (val.is_null()) {
      return datatype::CmpBool::CmpNull;
    }
    if (val.get_as<bool>()) {
      return datatype::CmpBool::CmpTrue;
    }
    return datatype::CmpBool::CmpFalse;
  }

  auto perform_computation(const datatype::Value &left,
                           const datatype::Value &right) const
      -> datatype::CmpBool {
    auto l = this->get_bool_as_cmp_bool(left);
    auto r = this->get_bool_as_cmp_bool(right);
    switch (this->logic_type_) {
    case LogicType::And:
      if (l == datatype::CmpBool::CmpFalse ||
          r == datatype::CmpBool::CmpFalse) {
        return datatype::CmpBool::CmpFalse;
      }
      if (l == datatype::CmpBool::CmpTrue && r == datatype::CmpBool::CmpTrue) {
        return datatype::CmpBool::CmpTrue;
      }
      return datatype::CmpBool::CmpNull;
    case LogicType::Or:
      if (l == datatype::CmpBool::CmpFalse ||
          r == datatype::CmpBool::CmpFalse) {
        return datatype::CmpBool::CmpFalse;
      }
      if (l == datatype::CmpBool::CmpTrue || r == datatype::CmpBool::CmpTrue) {
        return datatype::CmpBool::CmpTrue;
      }
      return datatype::CmpBool::CmpNull;
    }
  }
};

} // namespace turtle::execution::expressions
