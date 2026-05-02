#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "data_types.hpp"

namespace turtle::datatype {

class Value;

enum class CmpBool { CmpFalse = 0, CmpTrue = 1, CmpNull = 2 };

class Type {
public:
  explicit Type(DataType data_type) : data_type_(data_type) {}

  virtual ~Type() = default;

  static auto get_type_size(DataType data_type) -> uint64_t;

  auto is_coercable_from(DataType data_type) const -> bool;

  static auto data_type_to_string(DataType data_type) -> std::string;

  static auto get_min_value(DataType data_type) -> Value;
  static auto get_max_value(DataType data_type) -> Value;

  inline static auto get_instance(DataType data_type) -> Type * {
    return k_types[static_cast<std::size_t>(data_type)];
  }

  inline auto get_data_type() const -> DataType { return this->data_type_; }

  // Comparison functions
  virtual auto compare_equals(const Value &left, const Value &right) const
      -> CmpBool;
  virtual auto compare_not_equals(const Value &left, const Value &right) const
      -> CmpBool;
  virtual auto compare_less_than(const Value &left, const Value &right) const
      -> CmpBool;
  virtual auto compare_less_than_equals(const Value &left,
                                        const Value &right) const -> CmpBool;
  virtual auto compare_greater_than(const Value &left, const Value &right) const
      -> CmpBool;
  virtual auto compare_greater_than_equals(const Value &left,
                                           const Value &right) const -> CmpBool;

  // Mathematical functions
  virtual auto add(const Value &left, const Value &right) const -> Value;
  virtual auto subtract(const Value &left, const Value &right) const -> Value;
  virtual auto multiply(const Value &left, const Value &right) const -> Value;
  virtual auto divide(const Value &left, const Value &right) const -> Value;
  virtual auto modulo(const Value &left, const Value &right) const -> Value;
  virtual auto min(const Value &left, const Value &right) const -> Value;
  virtual auto max(const Value &left, const Value &right) const -> Value;
  virtual auto sqrt(const Value &value) const -> Value;
  virtual auto operate_null(const Value &left, const Value &right) const
      -> Value;
  virtual auto is_zero(const Value &value) const -> bool;

  // Check if is the data inlined into this classes storage space, or must it be
  // accessed through an indirection/pointer
  virtual auto is_inlined(const Value &value) const -> bool;

  virtual auto to_string(const Value &value) const -> std::string;

  // serialize this value into the given storage space.
  virtual void serialize(const Value &value, char *storage) const;

  // deserialize a value of the given type from the given storage space.
  virtual auto deserialize(const char *storage) const -> Value;

  virtual auto copy(const Value &value) const -> Value;

  virtual auto cast_as(const Value &value, DataType data_type) const -> Value;

  // Access the raw varlen value data stored from the tuple storage
  virtual auto get_data(const Value &value) const -> const char *;

  // et the storage size of the value.
  virtual auto get_storage_size(const Value &value) const -> uint32_t;

private:
  DataType data_type_;

  // Singleton instance
  static Type *k_types[10];
};

} // namespace Turtle::Type
