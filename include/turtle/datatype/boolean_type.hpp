#pragma once

#include "turtle/datatype/type.hpp"

namespace turtle::datatype {

class BooleanType : public Type {
public:
  ~BooleanType() override = default;

  BooleanType();

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

  auto is_inlined(const Value __attribute__((__unused__)) & val) const
      -> bool override {
    return true;
  }

  auto to_string(const Value &val) const -> std::string override;

  void serialize(const Value &val, char *storage) const override;

  auto deserialize(const char *storage) const -> Value override;

  auto copy(const Value &val) const -> Value override;

  auto cast_as(const Value &val, DataType data_type) const -> Value override;

  auto get_storage_size(const Value &val) const -> uint32_t override;

  auto get_data(const Value &val) const -> const char * override;
};

} // namespace turtle::datatype
