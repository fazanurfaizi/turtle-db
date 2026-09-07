#pragma once

#include <string>
#include <utility>
#include <vector>

#include "fmt/format.h"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/datatype/value_factory.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::expressions {

/** ComparisonType represents the type of comparison that we want to perform. */
enum class ComparisonType {
  Equal,
  NotEqual,
  LessThan,
  LessThanOrEqual,
  GreaterThan,
  GreaterThanOrEqual,
};

} // namespace turtle::execution::expressions

template <>
struct fmt::formatter<turtle::execution::expressions::ComparisonType>
    : formatter<string_view> {
  template <typename FormatContext>
  auto format(turtle::execution::expressions::ComparisonType c,
              FormatContext &ctx) const {
    string_view name;
    switch (c) {
    case turtle::execution::expressions::ComparisonType::Equal:
      name = "=";
      break;
    case turtle::execution::expressions::ComparisonType::NotEqual:
      name = "!=";
      break;
    case turtle::execution::expressions::ComparisonType::LessThan:
      name = "<";
      break;
    case turtle::execution::expressions::ComparisonType::LessThanOrEqual:
      name = "<=";
      break;
    case turtle::execution::expressions::ComparisonType::GreaterThan:
      name = ">";
      break;
    case turtle::execution::expressions::ComparisonType::GreaterThanOrEqual:
      name = ">=";
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
 * ComparisonExpression represents two expressions being compared.
 */
class ComparisonExpression : public AbstractExpression {
public:
  ComparisonType comp_type_;

  /**
   * Creates a new comparison expresison representing (left comp_type right).
   */
  ComparisonExpression(AbstractExpressionRef left, AbstractExpressionRef right,
                       ComparisonType comp_type)
      : AbstractExpression(
            {std::move(left), std::move(right)},
            catalog::Column{"<val>", datatype::DataType::BOOLEAN}),
        comp_type_{comp_type} {}

  auto evaluate(const storage::table::Tuple *tuple,
                const catalog::ColumnSchema &schema) const
      -> datatype::Value override {
    datatype::Value lhs = this->get_child_at(0)->evaluate(tuple, schema);
    datatype::Value rhs = this->get_child_at(1)->evaluate(tuple, schema);
    return datatype::ValueFactory::get_boolean_value(
        this->perform_comparison(lhs, rhs));
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
        this->perform_comparison(lhs, rhs));
  }

  auto to_string() const -> std::string override {
    return fmt::format("({}{}{})", *this->get_child_at(0), this->comp_type_,
                       *this->get_child_at(1));
  }

  TURTLE_EXPR_CLONE_WITH_CHILDREN(ComparisonExpression);

private:
  auto perform_comparison(const datatype::Value &left,
                          const datatype::Value &right) const
      -> datatype::CmpBool {
    switch (this->comp_type_) {
    case ComparisonType::Equal:
      return left.compare_equals(right);
    case ComparisonType::NotEqual:
      return left.compare_not_equals(right);
    case ComparisonType::LessThan:
      return left.compare_less_than(right);
    case ComparisonType::LessThanOrEqual:
      return left.compare_less_than_equals(right);
    case ComparisonType::GreaterThan:
      return left.compare_greater_than(right);
    case ComparisonType::GreaterThanOrEqual:
      return left.compare_greater_than_equals(right);
    default:
      TURTLE_ASSERT(false, "Unsupported comparison type.");
    }
  }
};

} // namespace turtle::execution::expressions
