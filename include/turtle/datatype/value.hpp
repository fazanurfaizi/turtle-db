#pragma once

#include "data_types.hpp"
#include "numeric_type.hpp"
#include <cstdint>
#include <fmt/core.h>
#include <type_traits>

#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "limits.hpp"
#include "type.hpp"

namespace turtle::catalog {
class Column;
}

namespace turtle::datatype {

inline auto get_cmp_bool(bool state) -> CmpBool {
  return state ? CmpBool::CmpTrue : CmpBool::CmpFalse;
}

// A value is an abstract class that represents a view over SQL data stored in
// some materialized state. All values have a type and comparison functions, but
// subclasses implement other type-specific functionality.
class Value {
  friend class Type;
  friend class NumericType;
  friend class IntegerParentType;

public:
  explicit Value(const DataType type) : manage_data_(false), data_type_(type) {
    this->size_.len_ = TURTLE_VALUE_NULL;
  }

  // constructor for specific types
  // BOOLEAN & TINYINT
  Value(DataType type, int8_t i);

  // Decimal
  Value(DataType type, double d);

  // Float
  Value(DataType type, float f);

  // SMALLINT
  Value(DataType type, int16_t i);

  // INTEGER
  Value(DataType type, int32_t i);

  // BIGINT
  Value(DataType type, int64_t i);

  // TIMESTAMP
  Value(DataType type, uint64_t i);

  // VARCHAR
  Value(DataType type, const char *data, uint32_t len, bool manage_data);
  Value(DataType type, std::string &data);
  Value(DataType type, const std::vector<double> &data);

  Value() : Value(DataType::INVALID) {}
  Value(const Value &other);
  auto operator=(Value other) -> Value &;
  ~Value();

  friend void swap(Value &first, Value &second) {
    std::swap(first.value_, second.value_);
    std::swap(first.size_, second.size_);
    std::swap(first.manage_data_, second.manage_data_);
    std::swap(first.data_type_, second.data_type_);
  }

  auto check_integer() const -> bool;
  auto check_comparable(const Value &value) const -> bool;

  inline auto data_type() const -> DataType { return this->data_type_; }

  auto column() const -> turtle::catalog::Column;

  // get the length of the variable length data
  inline auto storage_size() const -> uint32_t {
    return Type::get_instance(this->data_type_)->get_storage_size(*this);
  }

  // access the raw variable length data
  inline auto data() const -> const char * {
    return Type::get_instance(this->data_type_)->get_data(*this);
  }

  template <class T> inline auto GetAs() const -> T {
    return *reinterpret_cast<const T *>(&value_);
  }

  auto vector() const -> std::vector<double>;

  inline auto cast_as(const DataType data_type) const -> Value {
    return Type::get_instance(this->data_type_)->cast_as(*this, data_type);
  }
  // You will likely need this in project 4...
  inline auto compare_exactly_equals(const Value &o) const -> bool {
    if (this->is_null() && o.is_null()) {
      return true;
    }
    return (Type::get_instance(this->data_type_)->compare_equals(*this, o)) ==
           CmpBool::CmpTrue;
  }
  // Comparison Methods
  inline auto compare_equals(const Value &o) const -> CmpBool {
    return Type::get_instance(this->data_type_)->compare_equals(*this, o);
  }
  inline auto compare_not_equals(const Value &o) const -> CmpBool {
    return Type::get_instance(this->data_type_)->compare_not_equals(*this, o);
  }
  inline auto compare_less_than(const Value &o) const -> CmpBool {
    return Type::get_instance(this->data_type_)->compare_less_than(*this, o);
  }
  inline auto compare_less_than_equals(const Value &o) const -> CmpBool {
    return Type::get_instance(this->data_type_)
        ->compare_less_than_equals(*this, o);
  }
  inline auto compare_greater_than(const Value &o) const -> CmpBool {
    return Type::get_instance(this->data_type_)->compare_greater_than(*this, o);
  }
  inline auto compare_greater_than_equals(const Value &o) const -> CmpBool {
    return Type::get_instance(this->data_type_)
        ->compare_greater_than_equals(*this, o);
  }

  // Other mathematical functions
  inline auto add(const Value &o) const -> Value {
    return Type::get_instance(this->data_type_)->add(*this, o);
  }
  inline auto subtract(const Value &o) const -> Value {
    return Type::get_instance(this->data_type_)->subtract(*this, o);
  }
  inline auto multiply(const Value &o) const -> Value {
    return Type::get_instance(this->data_type_)->multiply(*this, o);
  }
  inline auto divide(const Value &o) const -> Value {
    return Type::get_instance(this->data_type_)->divide(*this, o);
  }
  inline auto modulo(const Value &o) const -> Value {
    return Type::get_instance(this->data_type_)->modulo(*this, o);
  }
  inline auto min(const Value &o) const -> Value {
    return Type::get_instance(this->data_type_)->min(*this, o);
  }
  inline auto max(const Value &o) const -> Value {
    return Type::get_instance(this->data_type_)->max(*this, o);
  }
  inline auto sqrt() const -> Value {
    return Type::get_instance(this->data_type_)->sqrt(*this);
  }

  inline auto operate_null(const Value &o) const -> Value {
    return Type::get_instance(this->data_type_)->operate_null(*this, o);
  }
  inline auto is_zero() const -> bool {
    return Type::get_instance(this->data_type_)->is_zero(*this);
  }
  inline auto is_null() const -> bool {
    return size_.len_ == TURTLE_VALUE_NULL;
  }

  // Serialize this value into the given storage space. The inlined parameter
  // indicates whether we are allowed to inline this value into the storage
  // space, or whether we must store only a reference to this value. If inlined
  // is false, we may use the provided data pool to allocate space for this
  // value, storing a reference into the allocated pool space in the storage.
  inline void SerializeTo(char *storage) const {
    Type::get_instance(this->data_type_)->serialize(*this, storage);
  }

  // Deserialize a value of the given type from the given storage space.
  inline static auto DeserializeFrom(const char *storage,
                                     const DataType data_type) -> Value {
    return Type::get_instance(data_type)->deserialize(storage);
  }

  // Return a string version of this value
  inline auto to_string() const -> std::string {
    return Type::get_instance(this->data_type_)->to_string(*this);
  }
  // Create a copy of this value
  inline auto copy() const -> Value {
    return Type::get_instance(this->data_type_)->copy(*this);
  }

private:
  bool manage_data_;
  DataType data_type_;

  union Val {
    int8_t boolean_;
    int8_t tinyint_;
    int16_t smallint_;
    int32_t integer_;
    int64_t bigint_;
    double decimal_;
    uint64_t timestamp_;
    char *varlen_;
    const char *const_varlen_;
  } value_;

  union {
    uint32_t len_;
    DataType elem_data_type_;
  } size_;
};

} // namespace turtle::datatype

template <typename T>
struct fmt::formatter<
    T, char, std::enable_if_t<std::is_base_of_v<turtle::datatype::Value, T>>>
    : fmt::formatter<std::string, char> {
  template <typename FormatCtx>
  auto format(const T &val, FormatCtx &ctx) const {
    return fmt::formatter<std::string, char>::format(val.to_string(), ctx);
  }
};

template <typename T>
struct fmt::formatter<
    std::unique_ptr<T>, char,
    std::enable_if_t<std::is_base_of_v<turtle::datatype::Value, T>>>
    : fmt::formatter<std::string, char> {
  template <typename FormatCtx>
  auto format(const std::unique_ptr<turtle::datatype::Value> &value,
              FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(value->to_string(), ctx);
  }
};
