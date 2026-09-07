#include "turtle/datatype/varchar_type.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace turtle::datatype {

VarcharType::VarcharType(DataType type) : Type(type) {}

auto VarcharType::compare_equals(const Value &left, const Value &right) const
    -> CmpBool {
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  auto left_str = std::string(left.data(), left.storage_size() - 4);
  auto right_str = std::string(right.data(), right.storage_size() - 4);
  return get_cmp_bool(left_str == right_str);
}

auto VarcharType::compare_not_equals(const Value &left,
                                     const Value &right) const -> CmpBool {
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  auto left_str = std::string(left.data(), left.storage_size() - 4);
  auto right_str = std::string(right.data(), right.storage_size() - 4);
  return get_cmp_bool(left_str != right_str);
}

auto VarcharType::compare_less_than(const Value &left, const Value &right) const
    -> CmpBool {
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  auto left_str = std::string(left.data(), left.storage_size() - 4);
  auto right_str = std::string(right.data(), right.storage_size() - 4);
  return get_cmp_bool(left_str < right_str);
}

auto VarcharType::compare_less_than_equals(const Value &left,
                                           const Value &right) const
    -> CmpBool {
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  auto left_str = std::string(left.data(), left.storage_size() - 4);
  auto right_str = std::string(right.data(), right.storage_size() - 4);
  return get_cmp_bool(left_str <= right_str);
}

auto VarcharType::compare_greater_than(const Value &left,
                                       const Value &right) const -> CmpBool {
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  auto left_str = std::string(left.data(), left.storage_size() - 4);
  auto right_str = std::string(right.data(), right.storage_size() - 4);
  return get_cmp_bool(left_str > right_str);
}

auto VarcharType::compare_greater_than_equals(const Value &left,
                                              const Value &right) const
    -> CmpBool {
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  auto left_str = std::string(left.data(), left.storage_size() - 4);
  auto right_str = std::string(right.data(), right.storage_size() - 4);
  return get_cmp_bool(left_str >= right_str);
}

auto VarcharType::cast_as(const Value &val, DataType type_id) const -> Value {
  if (val.is_null()) {
    return Value(type_id);
  }
  throw std::runtime_error("VARCHAR cannot be cast to other types.");
}

auto VarcharType::to_string(const Value &val) const -> std::string {
  if (val.is_null()) {
    return "varchar_null";
  }
  auto len = val.storage_size() - 4;
  return std::string(val.data(), len);
}

void VarcharType::serialize(const Value &val, char *storage) const {
  if (val.is_null()) {
    uint32_t len = 0;
    std::memcpy(storage, &len, sizeof(uint32_t));
    return;
  }
  auto str_len = val.size_.len_;
  std::memcpy(storage, &str_len, sizeof(uint32_t));
  std::memcpy(storage + 4, val.data(), str_len);
}

auto VarcharType::deserialize(const char *storage) const -> Value {
  uint32_t len = *reinterpret_cast<const uint32_t *>(storage);
  if (len == 0) {
    return Value(DataType::VARCHAR);
  }
  auto data = storage + 4;
  return Value(DataType::VARCHAR, data, len, false);
}

auto VarcharType::copy(const Value &val) const -> Value {
  if (val.is_null()) {
    return Value(DataType::VARCHAR);
  }
  auto len = val.storage_size();
  char *buf = new char[len];
  std::memcpy(buf, val.data(), len);
  return Value(DataType::VARCHAR, buf, len, true);
}

auto VarcharType::get_storage_size(const Value &val) const -> uint32_t {
  if (val.is_null()) {
    return 4;
  }
  return val.size_.len_ + 4;
}

auto VarcharType::get_data(const Value &val) const -> const char * {
  return val.value_.const_varlen_;
}

auto VarcharType::add(const Value &, const Value &) const -> Value {
  throw std::runtime_error("VARCHAR does not support addition.");
}

auto VarcharType::subtract(const Value &, const Value &) const -> Value {
  throw std::runtime_error("VARCHAR does not support subtraction.");
}

auto VarcharType::multiply(const Value &, const Value &) const -> Value {
  throw std::runtime_error("VARCHAR does not support multiplication.");
}

auto VarcharType::divide(const Value &, const Value &) const -> Value {
  throw std::runtime_error("VARCHAR does not support division.");
}

auto VarcharType::modulo(const Value &, const Value &) const -> Value {
  throw std::runtime_error("VARCHAR does not support modulo.");
}

auto VarcharType::min(const Value &left, const Value &right) const -> Value {
  if (compare_less_than(left, right) == CmpBool::CmpTrue) {
    return copy(left);
  }
  return copy(right);
}

auto VarcharType::max(const Value &left, const Value &right) const -> Value {
  if (compare_greater_than(left, right) == CmpBool::CmpTrue) {
    return copy(left);
  }
  return copy(right);
}

auto VarcharType::sqrt(const Value &) const -> Value {
  throw std::runtime_error("VARCHAR does not support sqrt.");
}

auto VarcharType::operate_null(const Value &left, const Value &right) const
    -> Value {
  return Value(DataType::VARCHAR);
}

auto VarcharType::is_zero(const Value &) const -> bool {
  throw std::runtime_error("VARCHAR does not support is_zero.");
}

auto VarcharType::is_inlined(const Value &val) const -> bool {
  return false; // VARCHAR is not inlined, stored as pointer
}

} // namespace turtle::datatype
