#pragma once

#include <string>

#include "integer_parent_type.hpp"

namespace turtle::datatype {

class IntegerType : public IntegerParentType {
public:
  ~IntegerType() override = default;

  explicit IntegerType(DataType type);

  // Other mathematical functions
  auto add(const Value &left, const Value &right) const -> Value override;
  auto subtract(const Value &left, const Value &right) const -> Value override;
  auto multiply(const Value &left, const Value &right) const -> Value override;
  auto divide(const Value &left, const Value &right) const -> Value override;
  auto modulo(const Value &left, const Value &right) const -> Value override;
  auto sqrt(const Value &val) const -> Value override;

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

  auto cast_as(const Value &val, DataType type_id) const -> Value override;

  auto to_string(const Value &val) const -> std::string override;

  void serialize(const Value &val, char *storage) const override;

  auto deserialize(const char *storage) const -> Value override;

  auto copy(const Value &val) const -> Value override;

  auto get_storage_size(const Value &val) const -> uint32_t override;

  auto get_data(const Value &val) const -> const char * override;

protected:
  auto operate_null(const Value &left, const Value &right) const
      -> Value override;

  auto is_zero(const Value &val) const -> bool override;
};

} // namespace turtle::datatype
