#include <cassert>
#include <cmath>
#include <stdexcept>
#include <string>

#include "turtle/datatype/integer_type.hpp"
#include "turtle/datatype/value.hpp"

namespace turtle::datatype {

#define INT_COMPARE_FUNC(OP)                                                   \
  switch (right.data_type()) {                                                 \
  case DataType::TINYINT:                                                      \
    return get_cmp_bool(left.GetAs<int32_t>() OP right.GetAs<int8_t>());       \
  case DataType::SMALLINT:                                                     \
    return get_cmp_bool(left.GetAs<int32_t>() OP right.GetAs<int16_t>());      \
  case DataType::INTEGER:                                                      \
    return get_cmp_bool(left.GetAs<int32_t>() OP right.GetAs<int32_t>());      \
  case DataType::BIGINT:                                                       \
    return get_cmp_bool(left.GetAs<int32_t>() OP right.GetAs<int64_t>());      \
  case DataType::DECIMAL:                                                      \
    return get_cmp_bool(left.GetAs<int32_t>() OP right.GetAs<double>());       \
  case DataType::VARCHAR: {                                                    \
    auto r_value = right.cast_as(DataType::INTEGER);                           \
    return get_cmp_bool(left.GetAs<int32_t>() OP r_value.GetAs<int32_t>());    \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  } // SWITCH

#define INT_MODIFY_FUNC(METHOD, OP)                                            \
  switch (right.data_type()) {                                                 \
  case DataType::TINYINT:                                                      \
    return METHOD<int32_t, int8_t>(left, right);                               \
  case DataType::SMALLINT:                                                     \
    return METHOD<int32_t, int16_t>(left, right);                              \
  case DataType::INTEGER:                                                      \
    return METHOD<int32_t, int32_t>(left, right);                              \
  case DataType::BIGINT:                                                       \
    return METHOD<int32_t, int64_t>(left, right);                              \
  case DataType::DECIMAL:                                                      \
    return Value(DataType::DECIMAL,                                            \
                 left.GetAs<int32_t>() OP right.GetAs<double>());              \
  case DataType::VARCHAR: {                                                    \
    auto r_value = right.cast_as(DataType::INTEGER);                           \
    return METHOD<int32_t, int32_t>(left, r_value);                            \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  } // SWITCH

IntegerType::IntegerType(DataType type) : IntegerParentType(type) {}

auto IntegerType::is_zero(const Value &val) const -> bool {
  return (val.GetAs<int32_t>() == 0);
}

auto IntegerType::add(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  INT_MODIFY_FUNC(add_value, +);
  throw std::runtime_error("type error");
}

auto IntegerType::subtract(const Value &left, const Value &right) const
    -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  INT_MODIFY_FUNC(subtract_value, -);
  throw std::runtime_error("type error");
}

auto IntegerType::multiply(const Value &left, const Value &right) const
    -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  INT_MODIFY_FUNC(multiply_value, *);
  throw std::runtime_error("type error");
}

auto IntegerType::divide(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  if (right.is_zero()) {
    throw std::runtime_error("Division by zero on right-hand side");
  }

  INT_MODIFY_FUNC(divide_value, /);
  throw std::runtime_error("type error");
}

auto IntegerType::modulo(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  if (right.is_zero()) {
    throw std::runtime_error("Division by zero on right-hand side");
  }

  switch (right.data_type()) {
  case DataType::TINYINT:
    return modulo_value<int32_t, int8_t>(left, right);
  case DataType::SMALLINT:
    return modulo_value<int32_t, int16_t>(left, right);
  case DataType::INTEGER:
    return modulo_value<int32_t, int32_t>(left, right);
  case DataType::BIGINT:
    return modulo_value<int32_t, int64_t>(left, right);
  case DataType::DECIMAL:
    return Value(DataType::DECIMAL,
                 val_mod(left.GetAs<int32_t>(), right.GetAs<double>()));
  case DataType::VARCHAR: {
    auto r_value = right.cast_as(DataType::INTEGER);
    return modulo_value<int32_t, int32_t>(left, r_value);
  }
  default:
    break;
  }

  throw std::runtime_error("type error");
}

auto IntegerType::sqrt(const Value &val) const -> Value {
  assert(val.check_integer());
  if (val.is_null()) {
    return operate_null(val, val);
  }

  if (val.GetAs<int32_t>() < 0) {
    throw std::runtime_error("Cannot take square root of a negative number.");
  }
  return Value(DataType::DECIMAL, std::sqrt(val.GetAs<int32_t>()));
}

auto IntegerType::min(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));

  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  // Reuse our comparison logic!
  if (left.compare_less_than(right) == CmpBool::CmpTrue) {
    return left.copy();
  }
  return right.copy();
}

auto IntegerType::max(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));

  if (left.is_null() || right.is_null()) {
    return operate_null(left, right);
  }

  // Reuse our comparison logic!
  if (left.compare_greater_than(right) == CmpBool::CmpTrue) {
    return left.copy();
  }
  return right.copy();
}

auto IntegerType::operate_null(const Value &left, const Value &right) const
    -> Value {
  switch (right.data_type()) {
  case DataType::TINYINT:
  case DataType::SMALLINT:
  case DataType::INTEGER:
    return Value(DataType::INTEGER, static_cast<int32_t>(TURTLE_INT32_NULL));
  case DataType::BIGINT:
    return Value(DataType::BIGINT, static_cast<int64_t>(TURTLE_INT64_NULL));
  case DataType::DECIMAL:
    return Value(DataType::DECIMAL, static_cast<double>(TURTLE_DECIMAL_NULL));
  default:
    break;
  }

  throw std::runtime_error("type error");
}

auto IntegerType::compare_equals(const Value &left, const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));

  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  INT_COMPARE_FUNC(==); // NOLINT

  throw std::runtime_error("type error");
}

auto IntegerType::compare_not_equals(const Value &left,
                                     const Value &right) const -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  INT_COMPARE_FUNC(!=); // NOLINT

  throw std::runtime_error("type error");
}

auto IntegerType::compare_less_than(const Value &left, const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  INT_COMPARE_FUNC(<); // NOLINT

  throw std::runtime_error("type error");
}

auto IntegerType::compare_less_than_equals(const Value &left,
                                           const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  INT_COMPARE_FUNC(<=); // NOLINT

  throw std::runtime_error("type error");
}

auto IntegerType::compare_greater_than(const Value &left,
                                       const Value &right) const -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  INT_COMPARE_FUNC(>); // NOLINT

  throw std::runtime_error("type error");
}

auto IntegerType::compare_greater_than_equals(const Value &left,
                                              const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  INT_COMPARE_FUNC(>=); // NOLINT

  throw std::runtime_error("type error");
}

auto IntegerType::to_string(const Value &val) const -> std::string {
  assert(val.check_integer());

  if (val.is_null()) {
    return "integer_null";
  }
  return std::to_string(val.GetAs<int32_t>());
}

void IntegerType::serialize(const Value &val, char *storage) const {
  int32_t int_val = val.GetAs<int32_t>();
  std::memcpy(storage, &int_val, sizeof(int32_t));
}

auto IntegerType::deserialize(const char *storage) const -> Value {
  int32_t val;
  std::memcpy(&val, storage, sizeof(int32_t));
  return Value(get_data_type(), val);
}

auto IntegerType::copy(const Value &val) const -> Value {
  assert(val.check_integer());
  return Value(val.data_type(), val.GetAs<int32_t>());
}

auto IntegerType::cast_as(const Value &val, const DataType type_id) const
    -> Value {
  switch (type_id) {
  case DataType::TINYINT: {
    if (val.is_null()) {
      return Value(type_id, static_cast<int8_t>(TURTLE_INT8_NULL));
    }
    if (val.GetAs<int32_t>() > TURTLE_INT8_MAX ||
        val.GetAs<int32_t>() < TURTLE_INT8_MIN) {
      throw std::runtime_error("Numeric value out of range.");
    }
    return Value(type_id, static_cast<int8_t>(val.GetAs<int32_t>()));
  }
  case DataType::SMALLINT: {
    if (val.is_null()) {
      return Value(type_id, static_cast<int16_t>(TURTLE_INT16_NULL));
    }
    if (val.GetAs<int32_t>() > TURTLE_INT16_MAX ||
        val.GetAs<int32_t>() < TURTLE_INT16_MIN) {
      throw std::runtime_error("Numeric value out of range.");
    }
    return Value(type_id, static_cast<int16_t>(val.GetAs<int32_t>()));
  }
  case DataType::INTEGER: {
    if (val.is_null()) {
      return Value(type_id, static_cast<int32_t>(TURTLE_INT32_NULL));
    }
    return Value(type_id, static_cast<int32_t>(val.GetAs<int32_t>()));
  }
  case DataType::BIGINT: {
    if (val.is_null()) {
      return Value(type_id, static_cast<int64_t>(TURTLE_INT64_NULL));
    }
    return Value(type_id, static_cast<int64_t>(val.GetAs<int32_t>()));
  }
  case DataType::DECIMAL: {
    if (val.is_null()) {
      return Value(type_id, static_cast<double>(TURTLE_DECIMAL_NULL));
    }
    return Value(type_id, static_cast<double>(val.GetAs<int32_t>()));
  }
  case DataType::VARCHAR: {
    if (val.is_null()) {
      return Value(DataType::VARCHAR, nullptr, 0, false);
    }
    // return Value(DataType::VARCHAR, val.to_string());
  }
  default:
    break;
  }
  throw std::runtime_error("Integer is not coercable.");
}

auto IntegerType::get_storage_size(const Value &val) const -> uint32_t {
  return sizeof(int32_t);
}

auto IntegerType::get_data(const Value &val) const -> const char * {
  // We throw here because inlined types (like integers) should be accessed
  // via GetAs<int32_t>(), not via raw char pointers (which are for VARCHARs).
  throw std::runtime_error(
      "get_data() should not be called on inlined Integer types.");
}

} // namespace turtle::datatype
