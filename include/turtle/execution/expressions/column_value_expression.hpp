#pragma once

#include "fmt/format.h"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/expressions/abstract_expression.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::expressions {

class ColumnValueExpression : public AbstractExpression {
public:
  /**
   * ColumnValueExpression is an abstraction around "Table.member" in terms of
   * indexes.
   * @param tuple_idx {tuple index 0 = left side of join, tuple index 1 = right
   * side of join}
   * @param col_idx the index of the column in the schema
   * @param ret_type the return type of the expression
   */
  ColumnValueExpression(uint32_t tuple_idx, uint32_t col_idx,
                        catalog::Column ret_type)
      : AbstractExpression({}, std::move(ret_type)), tuple_idx_(tuple_idx),
        col_idx_(col_idx) {}

  auto evaluate(const storage::table::Tuple *tuple,
                const catalog::ColumnSchema &schema) const
      -> datatype::Value override {
    return tuple->value(&schema, this->col_idx_);
  }

  auto evaluate_join(const storage::table::Tuple *left_tuple,
                     const catalog::ColumnSchema &left_schema,
                     const storage::table::Tuple *right_tuple,
                     const catalog::ColumnSchema &right_schema) const
      -> datatype::Value override {
    return this->tuple_idx_ == 0
               ? left_tuple->value(&left_schema, this->col_idx_)
               : right_tuple->value(&right_schema, this->col_idx_);
  }

  auto get_tuple_idx() const -> uint32_t { return this->tuple_idx_; }

  auto get_col_idx() const -> uint32_t { return this->col_idx_; }

  /** @return the string representation of the plan node and its children */
  auto to_string() const -> std::string override {
    return fmt::format("#{}.{}", this->tuple_idx_, this->col_idx_);
  }

  TURTLE_EXPR_CLONE_WITH_CHILDREN(ColumnValueExpression);

private:
  /** Tuple index 0 = left side of join, tuple index 1 = right side of join */
  uint32_t tuple_idx_;
  /** Column index refers to the index within schema of the tuple, e.g. schema
   * {A, B, C} has indexes {0, 1, 2} */
  uint32_t col_idx_;
};

} // namespace turtle::execution::expressions
