#include "turtle/datatype/type.hpp"
#include "turtle/datatype/integer_type.hpp"

namespace turtle::datatype {

Type *Type::k_types[] = {
    nullptr,                            // INVALID = 0
    nullptr,                            // BOOLEAN
    new IntegerType(DataType::INTEGER), // INTEGER
    nullptr,                            // TINYINT
    nullptr,                            // SMALLINT
    nullptr,                            // BIGINT
    nullptr,                            // DECIMAL
    nullptr,                            // VARCHAR
    nullptr,                            // TIMESTAMP
    nullptr                             // Padding for safety
};

auto Type::get_type_size(DataType data_type) -> uint64_t { return 4; }
auto Type::is_coercable_from(DataType data_type) const -> bool { return false; }
auto Type::data_type_to_string(DataType data_type) -> std::string {
  return "UNKNOWN";
}
auto Type::get_min_value(DataType data_type) -> Value { return Value(); }
auto Type::get_max_value(DataType data_type) -> Value { return Value(); }

auto Type::compare_equals(const Value &left, const Value &right) const
    -> CmpBool {
  throw std::runtime_error("Not implemented");
}
auto Type::compare_not_equals(const Value &left, const Value &right) const
    -> CmpBool {
  throw std::runtime_error("Not implemented");
}
auto Type::compare_less_than(const Value &left, const Value &right) const
    -> CmpBool {
  throw std::runtime_error("Not implemented");
}
auto Type::compare_less_than_equals(const Value &left, const Value &right) const
    -> CmpBool {
  throw std::runtime_error("Not implemented");
}
auto Type::compare_greater_than(const Value &left, const Value &right) const
    -> CmpBool {
  throw std::runtime_error("Not implemented");
}
auto Type::compare_greater_than_equals(const Value &left,
                                       const Value &right) const -> CmpBool {
  throw std::runtime_error("Not implemented");
}

auto Type::add(const Value &left, const Value &right) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::subtract(const Value &left, const Value &right) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::multiply(const Value &left, const Value &right) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::divide(const Value &left, const Value &right) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::modulo(const Value &left, const Value &right) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::min(const Value &left, const Value &right) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::max(const Value &left, const Value &right) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::sqrt(const Value &value) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::operate_null(const Value &left, const Value &right) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::is_zero(const Value &value) const -> bool {
  throw std::runtime_error("Not implemented");
}

auto Type::is_inlined(const Value &value) const -> bool {
  throw std::runtime_error("Not implemented");
}
auto Type::to_string(const Value &value) const -> std::string {
  throw std::runtime_error("Not implemented");
}
void Type::serialize(const Value &value, char *storage) const {
  throw std::runtime_error("Not implemented");
}
auto Type::deserialize(const char *storage) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::copy(const Value &value) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::cast_as(const Value &value, DataType data_type) const -> Value {
  throw std::runtime_error("Not implemented");
}
auto Type::get_data(const Value &value) const -> const char * {
  throw std::runtime_error("Not implemented");
}
auto Type::get_storage_size(const Value &value) const -> uint32_t {
  throw std::runtime_error("Not implemented");
}

} // namespace turtle::datatype
