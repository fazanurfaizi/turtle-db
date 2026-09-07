#include <cassert>
#include <cmath>
#include <string>

#include "turtle/common/exceptions.hpp"
#include "turtle/datatype/bigint_type.hpp"
#include "turtle/datatype/data_types.hpp"

namespace turtle::datatype {

#define BIGINT_COMPARE_FUNC(OP)                                                \
  switch (right.data_type()) {                                                 \
  case DataType::TINYINT:                                                      \
    return get_cmp_bool(left.value_.bigint_ OP right.get_as<int8_t>());        \
  case DataType::SMALLINT:                                                     \
    return get_cmp_bool(left.value_.bigint_ OP right.get_as<int16_t>());       \
  case DataType::INTEGER:                                                      \
    return get_cmp_bool(left.value_.bigint_ OP right.get_as<int32_t>());       \
  case DataType::BIGINT:                                                       \
    return get_cmp_bool(left.value_.bigint_ OP right.get_as<int64_t>());       \
  case DataType::DECIMAL:                                                      \
    return get_cmp_bool(static_cast<double>(left.value_.bigint_) OP            \
                        right.get_as<double>());                               \
  case DataType::VARCHAR: {                                                    \
    auto r_value = right.cast_as(DataType::BIGINT);                            \
    return get_cmp_bool(left.value_.bigint_ OP r_value.get_as<int64_t>());     \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  } // SWITCH

#define BIGINT_MODIFY_FUNC(METHOD, OP)                                         \
  switch (right.data_type()) {                                                 \
  case DataType::TINYINT:                                                      \
    /* NOLINTNEXTLINE */                                                       \
    return METHOD<int64_t, int8_t>(left, right);                               \
  case DataType::SMALLINT:                                                     \
    /* NOLINTNEXTLINE */                                                       \
    return METHOD<int64_t, int16_t>(left, right);                              \
  case DataType::INTEGER:                                                      \
    /* NOLINTNEXTLINE */                                                       \
    return METHOD<int64_t, int32_t>(left, right);                              \
  case DataType::BIGINT:                                                       \
    /* NOLINTNEXTLINE */                                                       \
    return METHOD<int64_t, int64_t>(left, right);                              \
  case DataType::DECIMAL:                                                      \
    /* NOLINTNEXTLINE */                                                       \
    return Value(DataType::DECIMAL,                                            \
                 static_cast<double>(left.value_.bigint_)                      \
                     OP right.get_as<double>());                               \
  case DataType::VARCHAR: {                                                    \
    auto r_value = right.cast_as(DataType::BIGINT);                            \
    /* NOLINTNEXTLINE */                                                       \
    return METHOD<int64_t, int64_t>(left, r_value);                            \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  } // SWITCH

BigintType::BigintType() : IntegerParentType(DataType::BIGINT) {}

auto BigintType::is_zero(const Value &val) const -> bool {
  return (val.value_.bigint_ == 0);
}

auto BigintType::add(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  BIGINT_MODIFY_FUNC(add_value, +);

  throw Exception("type error");
}

auto BigintType::subtract(const Value &left, const Value &right) const
    -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  BIGINT_MODIFY_FUNC(subtract_value, -);

  throw Exception("type error");
}

auto BigintType::multiply(const Value &left, const Value &right) const
    -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  BIGINT_MODIFY_FUNC(multiply_value, *);

  throw Exception("type error");
}

auto BigintType::divide(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  if (right.is_zero()) {
    throw Exception(ExceptionType::DIVIDE_BY_ZERO,
                    "Division by zero on right-hand side");
  }

  BIGINT_MODIFY_FUNC(divide_value, /);
  throw Exception("type error");
}

auto BigintType::modulo(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
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
    return modulo_value<int64_t, int8_t>(left, right);
  case DataType::SMALLINT:
    return modulo_value<int64_t, int16_t>(left, right);
  case DataType::INTEGER:
    return modulo_value<int64_t, int32_t>(left, right);
  case DataType::BIGINT:
    return modulo_value<int64_t, int64_t>(left, right);
  case DataType::DECIMAL:
    return {DataType::DECIMAL,
            val_mod(static_cast<double>(left.value_.bigint_),
                    right.get_as<double>())};
  case DataType::VARCHAR: {
    auto r_value = right.cast_as(DataType::BIGINT);
    return modulo_value<int64_t, int64_t>(left, r_value);
  }
  default:
    break;
  }
  throw Exception("type error");
}

auto BigintType::sqrt(const Value &val) const -> Value {
  assert(val.check_integer());
  if (val.is_null()) {
    return {DataType::DECIMAL, TURTLE_DECIMAL_NULL};
  }

  if (val.value_.bigint_ < 0) {
    throw Exception(ExceptionType::DECIMAL,
                    "Cannot take square root of a negative number.");
  }
  return {DataType::DECIMAL, std::sqrt(val.value_.bigint_)};
}

auto BigintType::min(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));

  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  if (left.compare_less_than(right) == CmpBool::CmpTrue) {
    return left.copy();
  }
  return right.copy();
}

auto BigintType::max(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));

  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  if (left.compare_greater_than(right) == CmpBool::CmpTrue) {
    return left.copy();
  }
  return right.copy();
}

auto BigintType::get_storage_size(const Value & /*val*/) const -> uint32_t {
  return sizeof(int64_t);
}

auto BigintType::get_data(const Value & /*val*/) const -> const char * {
  // Inlined type: access via get_as<int64_t>(), not raw bytes (VARCHAR only).
  throw std::runtime_error(
      "get_data() should not be called on inlined Bigint types.");
}

auto BigintType::operate_null(const Value &left __attribute__((unused)),
                              const Value &right) const -> Value {
  switch (right.data_type()) {
  case DataType::TINYINT:
  case DataType::SMALLINT:
  case DataType::INTEGER:
  case DataType::BIGINT:
    return {DataType::BIGINT, static_cast<int64_t>(TURTLE_INT64_NULL)};
  case DataType::DECIMAL:
    return {DataType::DECIMAL, static_cast<double>(TURTLE_DECIMAL_NULL)};
  default:
    break;
  }
  throw Exception("type error");
}

auto BigintType::compare_equals(const Value &left, const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));

  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  BIGINT_COMPARE_FUNC(==); // NOLINT

  throw Exception("type error");
}

auto BigintType::compare_not_equals(const Value &left, const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  BIGINT_COMPARE_FUNC(!=); // NOLINT

  throw Exception("type error");
}

auto BigintType::compare_less_than(const Value &left, const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  BIGINT_COMPARE_FUNC(<); // NOLINT

  throw Exception("type error");
}

auto BigintType::compare_less_than_equals(const Value &left,
                                          const Value &right) const -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  BIGINT_COMPARE_FUNC(<=); // NOLINT

  throw Exception("type error");
}

auto BigintType::compare_greater_than(const Value &left,
                                      const Value &right) const -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  BIGINT_COMPARE_FUNC(>); // NOLINT

  throw Exception("type error");
}

auto BigintType::compare_greater_than_equals(const Value &left,
                                             const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  BIGINT_COMPARE_FUNC(>=); // NOLINT
  throw Exception("type error");
}

auto BigintType::to_string(const Value &val) const -> std::string {
  assert(val.check_integer());

  if (val.is_null()) {
    return "bigint_null";
  }
  return std::to_string(val.value_.bigint_);
}

/**
 * Serialize this value into the given storage space
 */
void BigintType::serialize(const Value &val, char *storage) const {
  *reinterpret_cast<int64_t *>(storage) = val.value_.bigint_;
}

/**
 * Deserialize a value of the given type from the given storage space.
 */
auto BigintType::deserialize(const char *storage) const -> Value {
  int64_t val = *reinterpret_cast<const int64_t *>(storage);
  return {data_type_, val};
}

/**
 * Create a copy of this value
 */
auto BigintType::copy(const Value &val) const -> Value {
  return {DataType::BIGINT, val.value_.bigint_};
}

auto BigintType::cast_as(const Value &val, const DataType data_type) const
    -> Value {
  switch (data_type) {
  case DataType::TINYINT: {
    if (val.is_null()) {
      return {data_type, TURTLE_INT8_NULL};
    }
    if (val.get_as<int64_t>() > TURTLE_INT8_MAX ||
        val.get_as<int64_t>() < TURTLE_INT8_MIN) {
      throw Exception(ExceptionType::OUT_OF_RANGE,
                      "Numeric value out of range.");
    }
    return {data_type, static_cast<int8_t>(val.get_as<int64_t>())};
  }
  case DataType::SMALLINT: {
    if (val.is_null()) {
      return {data_type, TURTLE_INT16_NULL};
    }
    if (val.get_as<int64_t>() > TURTLE_INT16_MAX ||
        val.get_as<int64_t>() < TURTLE_INT16_MIN) {
      throw Exception(ExceptionType::OUT_OF_RANGE,
                      "Numeric value out of range.");
    }
    return {data_type, static_cast<int16_t>(val.get_as<int64_t>())};
  }
  case DataType::INTEGER: {
    if (val.is_null()) {
      return {data_type, TURTLE_INT32_NULL};
    }
    if (val.get_as<int64_t>() > TURTLE_INT32_MAX ||
        val.get_as<int64_t>() < TURTLE_INT32_MIN) {
      throw Exception(ExceptionType::OUT_OF_RANGE,
                      "Numeric value out of range.");
    }
    return {data_type, static_cast<int32_t>(val.get_as<int64_t>())};
  }

  case DataType::BIGINT: {
    if (val.is_null()) {
      return {data_type, TURTLE_INT64_NULL};
    }
    return copy(val);
  }

  case DataType::DECIMAL: {
    if (val.is_null()) {
      return {data_type, TURTLE_DECIMAL_NULL};
    }
    return {data_type, static_cast<double>(val.get_as<int64_t>())};
  }

  case DataType::VARCHAR: {
    if (val.is_null()) {
      return {DataType::VARCHAR, nullptr, 0, false};
    }
    std::string str = val.to_string();
    return {DataType::VARCHAR, str};
  }
  default:
    break;
  }
  throw Exception(ExceptionType::CONVERSION,
                  "bigint is not coercable to " +
                      Type::data_type_to_string(data_type));
}

} // namespace turtle::datatype
