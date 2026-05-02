#pragma once
#include "type.hpp"
#include "value.hpp"
#include <cmath>

namespace turtle::datatype {

// A numeric value is an abstract type representing a number. Numerics can be
// either integral or non-integral (decimal), but must provide arithmetic
// operations on its value.
class NumericType : public Type {
public:
  explicit NumericType(DataType type) : Type(type) {}
  ~NumericType() override = default;

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
  auto operate_null(const Value &left, const Value &right) const
      -> Value override = 0;
  auto is_zero(const Value &val) const -> bool override = 0;

protected:
  static inline auto val_mod(double x, double y) -> double {
    return x - std::trunc(static_cast<double>(x) / static_cast<double>(y)) * y;
  }
};

} // namespace Turtle::Type
