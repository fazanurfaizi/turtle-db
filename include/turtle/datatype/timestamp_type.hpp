#pragma once

#include "turtle/datatype/type.hpp"
#include <string>

namespace turtle::datatype {

class TimestampType : public Type {
public:
  static constexpr uint64_t K_USECS_PER_DATE = 86400000000UL;

  ~TimestampType() override = default;

  TimestampType();

  // Other mathematical functions
  auto min(const Value &left, const Value &right) const -> Value override;
  auto max(const Value &left, const Value &right) const -> Value override;

  // Comparison functions
  auto compare_equals(const Value &left, const Value &right) const
      -> CmpBool override;
  auto compare_not_equals(const Value &left, const Value &right) const
      -> CmpBool override;
  auto compare_less_than(const Value &left, const Value &right) const
      -> CmpBool override;
  auto compare_less_than_equals(const Value &left, const Value &right) const
      -> CmpBool override;
  auto compare_greater_than(const Value &left, const Value &right) const
      -> CmpBool override;
  auto compare_greater_than_equals(const Value &left, const Value &right) const
      -> CmpBool override;

  auto is_inlined(const Value & /*val*/) const -> bool override { return true; }

  auto cast_as(const Value &val, DataType data_type) const -> Value override;

  auto to_string(const Value &val) const -> std::string override;

  void serialize(const Value &val, char *storage) const override;

  auto deserialize(const char *storage) const -> Value override;

  auto copy(const Value &val) const -> Value override;

  auto get_storage_size(const Value &val) const -> uint32_t override;

  auto get_data(const Value &val) const -> const char * override;

  auto is_coercable_from(DataType data_type) const -> bool override;
};

} // namespace turtle::datatype
