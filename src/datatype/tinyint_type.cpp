#include "turtle/datatype/tinyint_type.hpp"
#include "turtle/common/exceptions.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/integer_parent_type.hpp"
#include "turtle/datatype/limits.hpp"
#include "turtle/datatype/value.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <string>

namespace turtle::datatype {

#define TINYINT_COMPARE_FUNC(OP)                                               \
  switch (right.data_type()) {                                                 \
  case DataType::TINYINT:                                                      \
    return get_cmp_bool(left.value_.tinyint_ OP right.get_as<int8_t>());       \
  case DataType::SMALLINT:                                                     \
    return get_cmp_bool(left.value_.tinyint_ OP right.get_as<int16_t>());      \
  case DataType::INTEGER:                                                      \
    return get_cmp_bool(left.value_.tinyint_ OP right.get_as<int32_t>());      \
  case DataType::BIGINT:                                                       \
    return get_cmp_bool(left.value_.tinyint_ OP right.get_as<int64_t>());      \
  case DataType::DECIMAL:                                                      \
    return get_cmp_bool(left.value_.tinyint_ OP right.get_as<double>());       \
  case DataType::VARCHAR: {                                                    \
    auto r_value = right.cast_as(DataType::TINYINT);                           \
    return get_cmp_bool(left.value_.tinyint_ OP r_value.get_as<int8_t>());     \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  } // SWITCH

#define TINYINT_MODIFY_FUNC(METHOD, OP)                                        \
  switch (right.data_type()) {                                                 \
  case DataType::TINYINT:                                                      \
    return METHOD<int8_t, int8_t>(left, right);                                \
  case DataType::SMALLINT:                                                     \
    return METHOD<int8_t, int16_t>(left, right);                               \
  case DataType::INTEGER:                                                      \
    return METHOD<int8_t, int32_t>(left, right);                               \
  case DataType::BIGINT:                                                       \
    return METHOD<int8_t, int64_t>(left, right);                               \
  case DataType::DECIMAL:                                                      \
    return Value(DataType::DECIMAL,                                            \
                 left.value_.tinyint_ OP right.get_as<double>());              \
  case DataType::VARCHAR: {                                                    \
    auto r_value = right.cast_as(DataType::TINYINT);                           \
    return METHOD<int8_t, int8_t>(left, r_value);                              \
  }                                                                            \
  default:                                                                     \
    break;                                                                     \
  } // SWITCH
// SWITCH

TinyintType::TinyintType() : IntegerParentType(DataType::TINYINT) {}

auto TinyintType::add(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  TINYINT_MODIFY_FUNC(add_value, +);

  throw Exception("type error");
}

auto TinyintType::subtract(const Value &left, const Value &right) const
    -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  TINYINT_MODIFY_FUNC(subtract_value, -);
  throw Exception("type error");
}

auto TinyintType::multiply(const Value &left, const Value &right) const
    -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  TINYINT_MODIFY_FUNC(multiply_value, *);
  throw Exception("type error");
}

auto TinyintType::divide(const Value &left, const Value &right) const -> Value {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }

  if (right.is_zero()) {
    throw Exception(ExceptionType::DIVIDE_BY_ZERO,
                    "Division by zero on right-hand side");
  }

  TINYINT_MODIFY_FUNC(divide_value, /);
  throw Exception("type error");
}

auto TinyintType::modulo(const Value &left, const Value &right) const -> Value {
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
    return modulo_value<int8_t, int8_t>(left, right);
  case DataType::SMALLINT:
    return modulo_value<int8_t, int16_t>(left, right);
  case DataType::INTEGER:
    return modulo_value<int8_t, int32_t>(left, right);
  case DataType::BIGINT:
    return modulo_value<int8_t, int64_t>(left, right);
  case DataType::DECIMAL:
    return {DataType::DECIMAL,
            val_mod(left.value_.tinyint_, right.get_as<double>())};
  case DataType::VARCHAR: {
    auto r_value = right.cast_as(DataType::TINYINT);
    return modulo_value<int8_t, int8_t>(left, r_value);
  }
  default:
    break;
  }

  throw Exception("type error");
}

auto TinyintType::sqrt(const Value &val) const -> Value {
  assert(val.check_integer());
  if (val.is_null()) {
    return {DataType::DECIMAL, static_cast<double>(TURTLE_DECIMAL_NULL)};
  }

  if (val.value_.tinyint_ < 0) {
    throw Exception(ExceptionType::DECIMAL,
                    "Cannot take square root of a negative number.");
  }
  return {DataType::DECIMAL, std::sqrt(val.value_.tinyint_)};
}

auto TinyintType::min(const Value &left, const Value &right) const -> Value {
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

auto TinyintType::max(const Value &left, const Value &right) const -> Value {
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

auto TinyintType::compare_equals(const Value &left, const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));

  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  TINYINT_COMPARE_FUNC(==);

  throw Exception("type error");
}

auto TinyintType::compare_not_equals(const Value &left,
                                     const Value &right) const -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  TINYINT_COMPARE_FUNC(!=);

  throw Exception("type error");
}

auto TinyintType::compare_less_than(const Value &left, const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  TINYINT_COMPARE_FUNC(<);

  throw Exception("type error");
}

auto TinyintType::compare_less_than_equals(const Value &left,
                                           const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  TINYINT_COMPARE_FUNC(<=);

  throw Exception("type error");
}

auto TinyintType::compare_greater_than(const Value &left,
                                       const Value &right) const -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  TINYINT_COMPARE_FUNC(>);

  throw Exception("type error");
}

auto TinyintType::compare_greater_than_equals(const Value &left,
                                              const Value &right) const
    -> CmpBool {
  assert(left.check_integer());
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }

  TINYINT_COMPARE_FUNC(>=);

  throw Exception("type error");
}

auto TinyintType::cast_as(const Value &val, const DataType type_id) const
    -> Value {
  switch (type_id) {
  case DataType::TINYINT: {
    if (val.is_null()) {
      return {type_id, TURTLE_INT8_NULL};
    }
    return this->copy(val);
  }
  case DataType::SMALLINT: {
    if (val.is_null()) {
      return {type_id, TURTLE_INT16_NULL};
    }
    return {type_id, static_cast<int16_t>(val.get_as<int8_t>())};
  }
  case DataType::INTEGER: {
    if (val.is_null()) {
      return {type_id, TURTLE_INT32_NULL};
    }
    return {type_id, static_cast<int32_t>(val.get_as<int8_t>())};
  }
  case DataType::BIGINT: {
    if (val.is_null()) {
      return {type_id, TURTLE_INT64_NULL};
    }
    return {type_id, static_cast<int64_t>(val.get_as<int8_t>())};
  }
  case DataType::DECIMAL: {
    if (val.is_null()) {
      return {type_id, TURTLE_DECIMAL_NULL};
    }
    return {type_id, static_cast<double>(val.get_as<int8_t>())};
  }
  case DataType::VARCHAR: {
    if (val.is_null()) {
      return {DataType::VARCHAR, nullptr, 0, false};
    }
    return {DataType::VARCHAR, val.to_string()};
  }
  default:
    break;
  }
  throw Exception("tinyint is not coercable to " +
                  Type::data_type_to_string(type_id));
}
auto TinyintType::to_string(const Value &val) const -> std::string {
  assert(val.check_integer());
  if (val.is_null()) {
    return "tinyint_null";
  }
  return std::to_string(val.value_.tinyint_);
}

void TinyintType::serialize(const Value &val, char *storage) const {
  *reinterpret_cast<int8_t *>(storage) = val.value_.tinyint_;
}

auto TinyintType::deserialize(const char *storage) const -> Value {
  int16_t val = *reinterpret_cast<const int8_t *>(storage);
  return {data_type_, val};
}

auto TinyintType::copy(const Value &val) const -> Value {
  assert(val.check_integer());
  return {DataType::TINYINT, val.value_.tinyint_};
}

auto TinyintType::operate_null(const Value &left __attribute__((unused)),
                               const Value &right) const -> Value {
  switch (right.data_type()) {
  case DataType::TINYINT:
    return {DataType::TINYINT, TURTLE_INT8_NULL};
  case DataType::SMALLINT:
    return {DataType::INTEGER, TURTLE_INT16_NULL};
  case DataType::INTEGER:
    return {DataType::INTEGER, TURTLE_INT32_NULL};
  case DataType::BIGINT:
    return {DataType::BIGINT, TURTLE_INT64_NULL};
  case DataType::DECIMAL:
    return {DataType::DECIMAL, static_cast<double>(TURTLE_DECIMAL_NULL)};
  default:
    break;
  }

  throw Exception("type error");
}

auto TinyintType::is_zero(const Value &val) const -> bool {
  return (val.value_.tinyint_ == 0);
}

auto TinyintType::get_storage_size(const Value & /*val*/) const -> uint32_t {
  return sizeof(int8_t);
}

} // namespace turtle::datatype
