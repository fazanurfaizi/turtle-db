#include <cassert>
#include <stdexcept>
#include <string>

#include "turtle/common/exceptions.hpp"
#include "turtle/datatype/boolean_type.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"

namespace turtle::datatype {

#define BOOLEAN_COMPARE_FUNC(OP)                                               \
  get_cmp_bool(left.value_.boolean_ OP right.cast_as(DataType::BOOLEAN)        \
                   .value_.boolean_)

BooleanType::BooleanType() : Type(DataType::BOOLEAN) {}

auto BooleanType::compare_equals(const Value &left, const Value &right) const
    -> CmpBool {
  assert(this->data_type() == DataType::BOOLEAN);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return BOOLEAN_COMPARE_FUNC(==);
}

auto BooleanType::compare_not_equals(const Value &left,
                                     const Value &right) const -> CmpBool {
  assert(this->data_type() == DataType::BOOLEAN);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return BOOLEAN_COMPARE_FUNC(!=);
}

auto BooleanType::compare_less_than(const Value &left, const Value &right) const
    -> CmpBool {
  assert(this->data_type() == DataType::BOOLEAN);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return BOOLEAN_COMPARE_FUNC(<);
}

auto BooleanType::compare_less_than_equals(const Value &left,
                                           const Value &right) const
    -> CmpBool {
  assert(this->data_type() == DataType::BOOLEAN);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return BOOLEAN_COMPARE_FUNC(<=);
}

auto BooleanType::compare_greater_than(const Value &left,
                                       const Value &right) const -> CmpBool {
  assert(this->data_type() == DataType::BOOLEAN);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return BOOLEAN_COMPARE_FUNC(>);
}

auto BooleanType::compare_greater_than_equals(const Value &left,
                                              const Value &right) const
    -> CmpBool {
  assert(this->data_type() == DataType::BOOLEAN);
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return BOOLEAN_COMPARE_FUNC(>=);
}

auto BooleanType::to_string(const Value &val) const -> std::string {
  assert(this->data_type() == DataType::BOOLEAN);
  if (val.value_.boolean_ == 1) {
    return "true";
  }
  if (val.value_.boolean_ == 0) {
    return "false";
  }
  return "boolean_null";
}

/**
 * Serialize this value into the given storage space.
 */
void BooleanType::serialize(const Value &val, char *storage) const {
  *reinterpret_cast<int8_t *>(storage) = val.value_.boolean_;
}

/**
 * Deserialize a value of the given type from the given storage space.
 */
auto BooleanType::deserialize(const char *storage) const -> Value {
  int8_t val = *reinterpret_cast<const int8_t *>(storage);
  return {DataType::BOOLEAN, val};
}

auto BooleanType::copy(const Value &val) const -> Value {
  return {DataType::BOOLEAN, val.value_.boolean_};
}

auto BooleanType::cast_as(const Value &val, const DataType data_type) const
    -> Value {
  switch (data_type) {
  case DataType::BOOLEAN:
    return this->copy(val);
  case DataType::VARCHAR: {
    if (val.is_null()) {
      return {DataType::VARCHAR, nullptr, 0, false};
    }
    return {DataType::VARCHAR, val.to_string()};
  }
  default:
    break;
  }
  throw Exception("BOOLEAN is not coercable to " +
                  Type::data_type_to_string(data_type));
}

auto BooleanType::get_storage_size(const Value & /*val*/) const -> uint32_t {
  return sizeof(int8_t);
}

auto BooleanType::get_data(const Value & /*val*/) const -> const char * {
  // Inlined type: access via get_as<int8_t>(), not raw bytes (VARCHAR only).
  throw std::runtime_error(
      "get_data() should not be called on inlined Boolean types.");
}

} // namespace turtle::datatype
