#include <cassert>
#include <cmath>
#include <string>

#include "turtle/common/exceptions.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/decimal_type.hpp"
#include "turtle/datatype/limits.hpp"
#include "turtle/datatype/numeric_type.hpp"
#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"

namespace turtle::datatype {

#define DECIMAL_COMPARE_FUNC(OP)                                               \
  switch (right.data_type()) {                                                 \
  case DataType::TINYINT:                                                      \
    return get_cmp_bool(left.value_.decimal_ OP right.get_as<int8_t>());       \
  case DataType::SMALLINT:                                                     \
    return get_cmp_bool(left.value_.decimal_ OP right.get_as<int16_t>());      \
  case DataType::INTEGER:                                                      \
    return get_cmp_bool(left.value_.decimal_ OP right.get_as<int32_t>());      \
  case DataType::BIGINT:                                                       \
    return get_cmp_bool(left.value_.decimal_ OP right.get_as<int64_t>());      \
  case DataType::DECIMAL:                                                      \
    return get_cmp_bool(left.value_.decimal_ OP right.get_as<double>());       \
  case DataType::VARCHAR: {                                                    \
    auto r_value = right.cast_as(DataType::DECIMAL);                           \
    return get_cmp_bool(left.value_.decimal_ OP r_value.get_as<double>());     \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  }

#define DECIMAL_MODIFY_FUNC(OP)                                                \
  switch (right.data_type()) {                                                 \
  case DataType::TINYINT:                                                      \
    return Value(DataType::DECIMAL,                                            \
                 left.value_.decimal_ OP right.get_as<int8_t>());              \
  case DataType::SMALLINT:                                                     \
    return Value(DataType::DECIMAL,                                            \
                 left.value_.decimal_ OP right.get_as<int16_t>());             \
  case DataType::INTEGER:                                                      \
    return Value(DataType::DECIMAL,                                            \
                 left.value_.decimal_ OP right.get_as<int32_t>());             \
  case DataType::BIGINT:                                                       \
    return Value(DataType::DECIMAL,                                            \
                 left.value_.decimal_ OP right.get_as<int64_t>());             \
  case DataType::DECIMAL:                                                      \
    return Value(DataType::DECIMAL,                                            \
                 left.value_.decimal_ OP right.get_as<double>());              \
  case DataType::VARCHAR: {                                                    \
    auto r_value = right.cast_as(DataType::DECIMAL);                           \
    return Value(DataType::DECIMAL,                                            \
                 left.value_.decimal_ OP r_value.get_as<double>());            \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  }

DecimalType::DecimalType() : NumericType(DataType::DECIMAL) {}

auto DecimalType::add(const Value &left, const Value &right) const -> Value {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  DECIMAL_MODIFY_FUNC(+);
  throw Exception("type error");
}

auto DecimalType::subtract(const Value &left, const Value &right) const
    -> Value {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  DECIMAL_MODIFY_FUNC(-);
  throw Exception("type error");
}

auto DecimalType::multiply(const Value &left, const Value &right) const
    -> Value {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  DECIMAL_MODIFY_FUNC(*);
  throw Exception("type error");
}

auto DecimalType::divide(const Value &left, const Value &right) const -> Value {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  if (right.is_zero()) {
    throw Exception(ExceptionType::DIVIDE_BY_ZERO,
                    "Division by zero on right-hand side");
  }

  DECIMAL_MODIFY_FUNC(/);
  throw Exception("type error");
}

auto DecimalType::modulo(const Value &left, const Value &right) const -> Value {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  if (right.is_zero()) {
    throw Exception(ExceptionType::DIVIDE_BY_ZERO,
                    "Division by zero on right-hand side");
  }

  switch (right.data_type()) {
  case DataType::TINYINT:
    return {DataType::DECIMAL,
            val_mod(left.value_.decimal_, right.get_as<int8_t>())};
  case DataType::SMALLINT:
    return {DataType::DECIMAL,
            val_mod(left.value_.decimal_, right.get_as<int16_t>())};
  case DataType::INTEGER:
    return {DataType::DECIMAL,
            val_mod(left.value_.decimal_, right.get_as<int32_t>())};
  case DataType::BIGINT:
    return {DataType::DECIMAL,
            val_mod(left.value_.decimal_, right.get_as<int64_t>())};
  case DataType::DECIMAL:
    return {DataType::DECIMAL,
            val_mod(left.value_.decimal_, right.get_as<double>())};
  case DataType::VARCHAR: {
    auto r_value = right.cast_as(DataType::DECIMAL);
    return {DataType::DECIMAL,
            val_mod(left.value_.decimal_, r_value.get_as<double>())};
  }
  default:
    break;
  }

  throw Exception("type error");
}

auto DecimalType::min(const Value &left, const Value &right) const -> Value {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  if (left.compare_less_than_equals(right) == CmpBool::CmpTrue) {
    return left.copy();
  }
  return right.copy();
}

auto DecimalType::max(const Value &left, const Value &right) const -> Value {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  if (left.compare_greater_than_equals(right) == CmpBool::CmpTrue) {
    return left.copy();
  }
  return right.copy();
}

auto DecimalType::sqrt(const Value &val) const -> Value {
  assert(this->data_type() == DataType::DECIMAL);
  if (val.is_null()) {
    return {DataType::DECIMAL, TURTLE_DECIMAL_NULL};
  }

  if (val.value_.decimal_ < 0) {
    throw Exception(ExceptionType::DECIMAL,
                    "Cannot take square root of a negative number.");
  }
  return {DataType::DECIMAL, std::sqrt(val.value_.decimal_)};
}

auto DecimalType::compare_equals(const Value &left, const Value &right) const
    -> CmpBool {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  DECIMAL_COMPARE_FUNC(==); // NOLINT

  throw Exception("type error");
}

auto DecimalType::compare_not_equals(const Value &left,
                                     const Value &right) const -> CmpBool {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  DECIMAL_COMPARE_FUNC(!=); // NOLINT

  throw Exception("type error");
}

auto DecimalType::compare_less_than(const Value &left, const Value &right) const
    -> CmpBool {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  DECIMAL_COMPARE_FUNC(<); // NOLINT

  throw Exception("type error");
}

auto DecimalType::compare_less_than_equals(const Value &left,
                                           const Value &right) const
    -> CmpBool {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  DECIMAL_COMPARE_FUNC(<=); // NOLINT

  throw Exception("type error");
}

auto DecimalType::compare_greater_than(const Value &left,
                                       const Value &right) const -> CmpBool {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  DECIMAL_COMPARE_FUNC(>); // NOLINT

  throw Exception("type error");
}

auto DecimalType::compare_greater_than_equals(const Value &left,
                                              const Value &right) const
    -> CmpBool {
  assert(this->data_type() == DataType::DECIMAL);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  DECIMAL_COMPARE_FUNC(>=); // NOLINT

  throw Exception("type error");
}

auto DecimalType::is_zero(const Value &val) const -> bool {
  assert(this->data_type() == DataType::DECIMAL);
  return val.value_.decimal_ == 0;
}

auto DecimalType::cast_as(const Value &val, const DataType data_type) const
    -> Value {
  switch (data_type) {
  case DataType::TINYINT: {
    if (val.is_null()) {
      return {data_type, TURTLE_INT8_NULL};
    }
    if (val.get_as<double>() > TURTLE_INT8_MAX ||
        val.get_as<double>() < TURTLE_INT8_MIN) {
      throw Exception(ExceptionType::OUT_OF_RANGE,
                      "Numeric value out of range.");
    }
    return {data_type, static_cast<int8_t>(val.get_as<double>())};
  }
  case DataType::SMALLINT: {
    if (val.is_null()) {
      return {data_type, TURTLE_INT16_NULL};
    }
    if (val.get_as<double>() > TURTLE_INT16_MAX ||
        val.get_as<double>() < TURTLE_INT16_MIN) {
      throw Exception(ExceptionType::OUT_OF_RANGE,
                      "Numeric value out of range.");
    }
    return {data_type, static_cast<int16_t>(val.get_as<double>())};
  }
  case DataType::INTEGER: {
    if (val.is_null()) {
      return {data_type, TURTLE_INT32_NULL};
    }
    if (val.get_as<double>() > TURTLE_INT32_MAX ||
        val.get_as<double>() < TURTLE_INT32_MIN) {
      throw Exception(ExceptionType::OUT_OF_RANGE,
                      "Numeric value out of range.");
    }
    return {data_type, static_cast<int32_t>(val.get_as<double>())};
  }
  case DataType::BIGINT: {
    if (val.is_null()) {
      return {data_type, TURTLE_INT64_NULL};
    }
    if (val.get_as<double>() >= static_cast<double>(TURTLE_INT64_MAX) ||
        val.get_as<double>() < static_cast<double>(TURTLE_INT64_MIN)) {
      throw Exception(ExceptionType::OUT_OF_RANGE,
                      "Numeric value out of range.");
    }
    return {data_type, static_cast<int64_t>(val.get_as<double>())};
  }
  case DataType::DECIMAL:
    return val.copy();
  case DataType::VARCHAR: {
    if (val.is_null()) {
      return {DataType::VARCHAR, nullptr, 0, false};
    }
    return {DataType::VARCHAR, val.to_string()};
  }
  default:
    break;
  }
  throw Exception("DECIMAL is not coercable to " +
                  Type::data_type_to_string(data_type));
}

auto DecimalType::to_string(const Value &val) const -> std::string {
  if (val.is_null()) {
    return "decimal_null";
  }
  return std::to_string(val.value_.decimal_);
}

/**
 * Serialize this value into the given storage space
 */
void DecimalType::serialize(const Value &val, char *storage) const {
  *reinterpret_cast<double *>(storage) = val.value_.decimal_;
}

/**
 * Deserialize a value of the given type from the given storage space.
 */
auto DecimalType::deserialize(const char *storage) const -> Value {
  double val = *reinterpret_cast<const double *>(storage);
  return {data_type_, val};
}

/**
 * Create a copy of this value
 */
auto DecimalType::copy(const Value &val) const -> Value {
  return {DataType::DECIMAL, val.value_.decimal_};
}

auto DecimalType::get_storage_size(const Value &val
                                   __attribute__((unused))) const -> uint32_t {
  return sizeof(int64_t);
}

auto DecimalType::operate_null(const Value &left __attribute__((unused)),
                               const Value &right __attribute__((unused))) const
    -> Value {
  return {DataType::DECIMAL, TURTLE_DECIMAL_NULL};
}

} // namespace turtle::datatype
