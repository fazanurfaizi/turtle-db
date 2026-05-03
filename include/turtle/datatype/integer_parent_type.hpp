#pragma once

#include <string>

#include "data_types.hpp"
#include "numeric_type.hpp"
#include "turtle/common/exceptions.hpp"

namespace turtle::datatype {

class IntegerParentType : public NumericType {
public:
  explicit IntegerParentType(DataType type) : NumericType(type) {}

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

  auto is_inlined(const Value & /*value*/) const -> bool override {
    return true;
  }

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
  // auto x = left.GetAs<T1>();
  // auto y = left.GetAs<T2>();
  // auto sum1 = static_cast<T1>(x + y);
  // auto sum2 = static_cast<T2>(x + y);
  //
  // if ((x + y) != sum1 && (x + y) != sum2) {
  //   throw Exception(ExceptionType::OUT_OF_RANGE, "Numeric value out of
  //   range.");
  // }
  // // Overflow detection
  // if (sizeof(x) >= sizeof(y)) {
  //   if ((x > 0 && y > 0 && sum1 < 0) || (x < 0 && y < 0 && sum1 > 0)) {
  //     throw Exception(ExceptionType::OUT_OF_RANGE,
  //                     "Numeric value out of range.");
  //   }
  //   return Value(left.GetTypeId(), sum1);
  // }
  // if ((x > 0 && y > 0 && sum2 < 0) || (x < 0 && y < 0 && sum2 > 0)) {
  //   throw Exception(ExceptionType::OUT_OF_RANGE, "Numeric value out of
  //   range.");
  // }
  // return Value(right.GetTypeId(), sum2);
}

template <class T1, class T2>
auto IntegerParentType::subtract_value(const Value &left,
                                       const Value &right) const -> Value {
  // auto x = left.GetAs<T1>();
  // auto y = right.GetAs<T2>();
  // auto diff1 = static_cast<T1>(x - y);
  // auto diff2 = static_cast<T2>(x - y);
  // if ((x - y) != diff1 && (x - y) != diff2) {
  //   throw Exception(ExceptionType::OUT_OF_RANGE, "Numeric value out of
  //   range.");
  // }
  // // Overflow detection
  // if (sizeof(x) >= sizeof(y)) {
  //   if ((x > 0 && y < 0 && diff1 < 0) || (x < 0 && y > 0 && diff1 > 0)) {
  //     throw Exception(ExceptionType::OUT_OF_RANGE,
  //                     "Numeric value out of range.");
  //   }
  //   return Value(left.GetTypeId(), diff1);
  // }
  // if ((x > 0 && y < 0 && diff2 < 0) || (x < 0 && y > 0 && diff2 > 0)) {
  //   throw Exception(ExceptionType::OUT_OF_RANGE, "Numeric value out of
  //   range.");
  // }
  // return Value(right.GetTypeId(), diff2);
}

template <class T1, class T2>
auto IntegerParentType::multiply_value(const Value &left,
                                       const Value &right) const -> Value {
  // auto x = left.GetAs<T1>();
  // auto y = right.GetAs<T2>();
  // auto prod1 = static_cast<T1>(x * y);
  // auto prod2 = static_cast<T2>(x * y);
  // if ((x * y) != prod1 && (x * y) != prod2) {
  //   throw Exception(ExceptionType::OUT_OF_RANGE, "Numeric value out of
  //   range.");
  // }
  // // Overflow detection
  // if (sizeof(x) >= sizeof(y)) {
  //   if ((y != 0 && prod1 / y != x)) {
  //     throw Exception(ExceptionType::OUT_OF_RANGE,
  //                     "Numeric value out of range.");
  //   }
  //   return Value(left.GetTypeId(), prod1);
  // }
  // if (y != 0 && prod2 / y != x) {
  //   throw Exception(ExceptionType::OUT_OF_RANGE, "Numeric value out of
  //   range.");
  // }
  // return Value(right.GetTypeId(), prod2);
}

template <class T1, class T2>
auto IntegerParentType::divide_value(const Value &left,
                                     const Value &right) const -> Value {
  // auto x = left.GetAs<T1>();
  // auto y = right.GetAs<T2>();
  // if (y == 0) {
  //   throw Exception(ExceptionType::DIVIDE_BY_ZERO, "Division by zero.");
  // }
  // auto quot1 = static_cast<T1>(x / y);
  // auto quot2 = static_cast<T2>(x / y);
  // if (sizeof(x) >= sizeof(y)) {
  //   return Value(left.GetTypeId(), quot1);
  // }
  // return Value(right.GetTypeId(), quot2);
}

template <class T1, class T2>
auto IntegerParentType::modulo_value(const Value &left,
                                     const Value &right) const -> Value {
  // auto x = left.GetAs<T1>();
  // auto y = right.GetAs<T2>();
  // if (y == 0) {
  //   throw Exception(ExceptionType::DIVIDE_BY_ZERO, "Division by zero.");
  // }
  // auto quot1 = static_cast<T1>(x % y);
  // auto quot2 = static_cast<T2>(x % y);
  // if (sizeof(x) >= sizeof(y)) {
  //   return Value(left.GetTypeId(), quot1);
  // }
  // return Value(right.GetTypeId(), quot2);
}

} // namespace turtle::datatype
