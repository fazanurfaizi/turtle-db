#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "fmt/format.h"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/table.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/storage/table/tuple.hpp"

#define TURTLE_EXPR_CLONE_WITH_CHILDREN(cname)                                 \
  auto CloneWithChildren(std::vector<AbstractExpressionRef> children) const    \
      -> std::unique_ptr<AbstractExpression>                                   \
          override {                                                           \
    auto expr = cname(*this);                                                  \
    expr.children_ = children;                                                 \
    return std::make_unique<cname>(std::move(expr));                           \
  }

namespace turtle::execution::expressions {

class AbstractExpression;
using AbstractExpressionRef = std::shared_ptr<AbstractExpression>;

/**
 * AbstractExpression is the base class of all the expressions in the system.
 * Expressions are modeled as trees, i.e. every expression may have a variable
 * number of children.
 */
class AbstractExpression {
public:
  std::vector<AbstractExpressionRef> children_;

  /**
   * Create a new AbstractExpression with the given children and return type.
   * @param children the children of this abstract expression
   * @param ret_type the return type of this abstract expression when it is
   * evaluated
   */
  AbstractExpression(std::vector<AbstractExpressionRef> children,
                     catalog::Column ret_type)
      : children_{std::move(children)}, ret_type_{std::move(ret_type)} {}

  virtual ~AbstractExpression() = default;

  /**
   * @return The Value obtained by evaluating the tuple with given schema
   */
  virtual auto evalute(const storage::table::Tuple *tuple,
                       const catalog::Table &table) const
      -> datatype::Value = 0;

  /**
   * Returns the value obtained by evaluating a JOIN.
   * @param left_tuple the left tuple
   * @param left_table the left tuple's table
   * @param right_tuple the right tuple
   * @param right_table the right tuple's table
   * @return The value obtained by evaluating a JOIN on the left and right
   */
  virtual auto evaluate_join(const storage::table::Tuple *left_tuple,
                             const catalog::Table &left_Table,
                             const storage::table::Tuple *right_tuple,
                             const catalog::Table &right_table) const
      -> datatype::Value = 0;

  /**
   * @return the child_indx'th child of this expression
   */
  auto get_child_at(uint32_t child_idx) const -> const AbstractExpressionRef & {
    return this->children_[child_idx];
  }

  /**
   * @return the children of this expression, ordering may matter
   */
  auto get_children() const -> const std::vector<AbstractExpressionRef> & {
    return this->children_;
  }

  /**
   * @return the type of this expression if it were to be evaluated
   */
  virtual auto get_return_type() const -> catalog::Column {
    return this->ret_type_;
  }

  /**
   * @return the string representation of the plan node and its children
   */
  virtual auto to_string() const -> std::string { return "unknown"; }

  /**
   * @return a new expression with new children
   */
  virtual auto
  clone_with_children(std::vector<AbstractExpressionRef> children) const
      -> std::unique_ptr<AbstractExpression> = 0;

private:
  catalog::Column ret_type_;
};

} // namespace turtle::execution::expressions

template <typename T>
struct fmt::formatter<
    T, std::enable_if_t<
           std::is_base_of<turtle::execution::expressions::AbstractExpression,
                           T>::value,
           char>> : fmt::formatter<std::string> {
  template <typename FormatCtx> auto format(const T &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.ToString(), ctx);
  }
};

template <typename T>
struct fmt::formatter<
    std::unique_ptr<T>,
    std::enable_if_t<
        std::is_base_of<turtle::execution::expressions::AbstractExpression,
                        T>::value,
        char>> : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::unique_ptr<T> &x, FormatCtx &ctx) const {
    if (x != nullptr) {
      return fmt::formatter<std::string>::format(x->ToString(), ctx);
    }
    return fmt::formatter<std::string>::format("", ctx);
  }
};

template <typename T>
struct fmt::formatter<
    std::shared_ptr<T>,
    std::enable_if_t<
        std::is_base_of<turtle::execution::expressions::AbstractExpression,
                        T>::value,
        char>> : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::shared_ptr<T> &x, FormatCtx &ctx) const {
    if (x != nullptr) {
      return fmt::formatter<std::string>::format(x->ToString(), ctx);
    }
    return fmt::formatter<std::string>::format("", ctx);
  }
};
