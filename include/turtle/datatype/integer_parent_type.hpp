#pragma once

#include <string>

#include "data_types.hpp"
#include "numeric_type.hpp"

namespace turtle::datatype {

class IntegerParentType : public NumericType {
public:
  explicit IntegerParentType(DataType type);
  ~IntegerParentType() override = default;

  auto add(const Value &left, const Value &right) const -> Value override = 0;
  auto subtract(const Value &left, const Value &right) const
      -> Value override = 0;
  auto multiply(const Value &left, const Value &right) const
      -> Value override = 0;
  auto divide(const Value &left, const Value &right) const
      -> Value override = 0;
  auto modulo(const Value &left, const Value &right) const
      -> Value override = 0;
  auto min(const Value &left, const Value &right) const -> Value override = 0;
  auto max(const Value &left, const Value &right) const -> Value override = 0;
  auto sqrt(const Value &val) const -> Value override = 0;

  auto compare_equals(const Value &left, const Value &right) const
      -> CmpBool override = 0;
  auto compare_not_equals(const Value &left, const Value &right) const
      -> CmpBool override = 0;
  auto compare_less_than(const Value &left, const Value &right) const
      -> CmpBool override = 0;
  auto compare_less_than_equals(const Value &left, const Value &right) const
      -> CmpBool override = 0;
  auto compare_greater_than(const Value &left, const Value &right) const
      -> CmpBool override = 0;
  auto compare_greater_than_equals(const Value &left, const Value &right) const
      -> CmpBool override = 0;

  auto cast_as(const Value &value, DataType data_type) const
      -> Value override = 0;

  auto is_inlined(const Value &value) const -> bool override { return true; }

  auto to_string(const Value &value) const -> std::string override = 0;

  void serialize(const Value &value, char *storage) const override = 0;

  auto deserialize(const char *storage) const -> Value override = 0;

  auto copy(const Value &value) const -> Value override = 0;

protected:
  template <class T1, class T2>
  auto add_value(const Value &left, const Value &right) const -> Value;
  template <class T1, class T2>
  auto subtract_value(const Value &left, const Value &right) const -> Value;
  template <class T1, class T2>
  auto multiply_value(const Value &left, const Value &right) const -> Value;
  template <class T1, class T2>
  auto divide_value(const Value &left, const Value &right) const -> Value;
  template <class T1, class T2>
  auto modulo_value(const Value &left, const Value &right) const -> Value;

  auto operate_null(const Value &left, const Value &right) const
      -> Value override = 0;

  auto is_zero(const Value &value) const -> bool override = 0;
};

template <class T1, class T2>
auto IntegerParentType::add_value(const Value &left, const Value &right) const
    -> Value {
  // auto x = left.Get
}

} // namespace Turtle::Type
