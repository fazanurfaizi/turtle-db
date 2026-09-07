#include "turtle/datatype/timestamp_type.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/datatype/value_factory.hpp"
#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace turtle::datatype {

TimestampType::TimestampType() : Type(DataType::TIMESTAMP) {}

auto TimestampType::compare_equals(const Value &left, const Value &right) const
    -> CmpBool {
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return get_cmp_bool(left.get_as<uint64_t>() == right.get_as<uint64_t>());
}

auto TimestampType::compare_not_equals(const Value &left,
                                       const Value &right) const -> CmpBool {
  assert(left.check_comparable(right));
  if (right.is_null()) {
    return CmpBool::CmpNull;
  }
  return get_cmp_bool(left.get_as<uint64_t>() != right.get_as<uint64_t>());
}

auto TimestampType::compare_less_than(const Value &left,
                                      const Value &right) const -> CmpBool {
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return get_cmp_bool(left.get_as<uint64_t>() < right.get_as<uint64_t>());
}

auto TimestampType::compare_less_than_equals(const Value &left,
                                             const Value &right) const
    -> CmpBool {
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return get_cmp_bool(left.get_as<uint64_t>() <= right.get_as<uint64_t>());
}

auto TimestampType::compare_greater_than(const Value &left,
                                         const Value &right) const -> CmpBool {
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return get_cmp_bool(left.get_as<int64_t>() > right.get_as<int64_t>());
}

auto TimestampType::compare_greater_than_equals(const Value &left,
                                                const Value &right) const
    -> CmpBool {
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return CmpBool::CmpNull;
  }
  return get_cmp_bool(left.get_as<uint64_t>() >= right.get_as<uint64_t>());
}

auto TimestampType::min(const Value &left, const Value &right) const -> Value {
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }
  if (left.compare_less_than(right) == CmpBool::CmpTrue) {
    return left.copy();
  }
  return right.copy();
}

auto TimestampType::max(const Value &left, const Value &right) const -> Value {
  assert(left.check_comparable(right));
  if (left.is_null() || right.is_null()) {
    return left.operate_null(right);
  }
  if (left.compare_greater_than_equals(right) == CmpBool::CmpTrue) {
    return left.copy();
  }
  return right.copy();
}

auto TimestampType::to_string(const Value &val) const -> std::string {
  if (val.is_null()) {
    return "timestamp_null";
  }
  uint64_t tm = val.value_.timestamp_;
  auto micro = static_cast<uint32_t>(tm % 1000000);
  tm /= 1000000;
  auto second = static_cast<uint32_t>(tm % 100000);
  auto sec = static_cast<uint16_t>(second % 60);
  second /= 60;
  auto min = static_cast<uint16_t>(second % 60);
  second /= 60;
  auto hour = static_cast<uint16_t>(second % 24);
  tm /= 100000;
  auto year = static_cast<uint16_t>(tm % 10000);
  tm /= 10000;
  auto tz = static_cast<int8_t>(tm % 27);
  tz -= 12;
  tm /= 27;
  auto day = static_cast<uint16_t>(tm % 32);
  tm /= 32;
  auto month = static_cast<uint16_t>(tm);
  const size_t date_str_len = 30;
  const size_t zone_len = 5;
  char str[date_str_len];
  char zone[zone_len];
  snprintf(str, date_str_len, "%04d-%02d-%02d %02d:%02d:%02d.%06d", year, month,
           day, hour, min, sec, micro);
  if (tz >= 0) {
    str[26] = '+';
  } else {
    str[26] = '-';
  }
  if (tz < 0) {
    tz = -tz;
  }
  snprintf(zone, zone_len, "%02d", tz); // NOLINT
  str[27] = 0;
  return std::string(std::string(str) + std::string(zone));
}

/**
 * Serialize this value into the given storage space
 */
void TimestampType::serialize(const Value &val, char *storage) const {
  *reinterpret_cast<uint64_t *>(storage) = val.value_.timestamp_;
}

/**
 * Deserialize a value of the given type from the given storage space.
 */
auto TimestampType::deserialize(const char *storage) const -> Value {
  uint64_t val = *reinterpret_cast<const uint64_t *>(storage);
  return {data_type_, val};
}

auto TimestampType::copy(const Value &val) const -> Value { return {val}; }

auto TimestampType::cast_as(const Value &val, const DataType data_type) const
    -> Value {
  switch (data_type) {
  case DataType::TIMESTAMP:
    return copy(val);
  case DataType::VARCHAR:
    if (val.is_null()) {
      return ValueFactory::get_varchar_value(nullptr, false);
    }
    return ValueFactory::get_varchar_value(val.to_string());
  default:
    break;
  }
  throw Exception("TIMESTAMP is not coercable to " +
                  Type::get_instance(data_type)->to_string(val));
}

auto TimestampType::get_storage_size(const Value &val
                                     __attribute__((unused))) const
    -> uint32_t {
  return sizeof(int64_t);
}

auto TimestampType::get_data(const Value & /*val*/) const -> const char * {
  // Inlined type: access via get_as<uint64_t>(), not raw bytes (VARCHAR only).
  throw std::runtime_error(
      "get_data() should not be called on inlined Timestamp types.");
}

auto TimestampType::is_coercable_from(DataType data_type) const -> bool {
  switch (data_type) {
  case DataType::VARCHAR:
  case DataType::TIMESTAMP:
    return true;
  default:
    return false;
  }
}

} // namespace turtle::datatype
