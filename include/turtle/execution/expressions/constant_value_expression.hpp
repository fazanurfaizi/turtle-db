#pragma once

#include "fmt/format.h"
#include "turtle/catalog/column.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::expressions {

class ConstantValueExpression : public AbstractExpression {
public:
  datatype::Value value_;

  explicit ConstantValueExpression(const datatype::Value &value)
      : AbstractExpression({}, value.column()), value_(value) {}

  auto evaluate(const storage::table::Tuple * /*tuple*/,
                const catalog::ColumnSchema & /*schema*/) const
      -> datatype::Value override {
    return this->value_;
  }

  auto evaluate_join(const storage::table::Tuple * /*left_tuple*/,
                     const catalog::ColumnSchema & /*left_schema*/,
                     const storage::table::Tuple * /*right_tuple*/,
                     const catalog::ColumnSchema & /*right_schema*/) const
      -> datatype::Value override {
    return this->value_;
  }

  auto to_string() const -> std::string override {
    return this->value_.to_string();
  }

  TURTLE_EXPR_CLONE_WITH_CHILDREN(ConstantValueExpression);
};

} // namespace turtle::execution::expressions
