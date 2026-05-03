#include "turtle/datatype/value.hpp"

#include <cstring>
#include <stdexcept>

namespace turtle::datatype {

// BOOLEAN & TINYINT
Value::Value(DataType type, int8_t i) : manage_data_(false), data_type_(type) {
  value_.boolean_ = i;
  size_.len_ = (i == TURTLE_BOOLEAN_NULL) ? TURTLE_VALUE_NULL : 0;
}

// Decimal
Value::Value(DataType type, double d) : manage_data_(false), data_type_(type) {
  value_.decimal_ = d;
  size_.len_ = (d == TURTLE_DECIMAL_NULL) ? TURTLE_VALUE_NULL : 0;
}

// Float (Converting to Decimal/Double internally based on your union)
Value::Value(DataType type, float f) : manage_data_(false), data_type_(type) {
  value_.decimal_ = static_cast<double>(f);
  size_.len_ = (f == TURTLE_DECIMAL_NULL) ? TURTLE_VALUE_NULL : 0;
}

// SMALLINT
Value::Value(DataType type, int16_t i) : manage_data_(false), data_type_(type) {
  value_.smallint_ = i;
  size_.len_ = (i == TURTLE_INT16_NULL) ? TURTLE_VALUE_NULL : 0;
}

// INTEGER
Value::Value(DataType type, int32_t i) : manage_data_(false), data_type_(type) {
  value_.integer_ = i;
  size_.len_ = (i == TURTLE_INT32_NULL) ? TURTLE_VALUE_NULL : 0;
}

// BIGINT
Value::Value(DataType type, int64_t i) : manage_data_(false), data_type_(type) {
  value_.bigint_ = i;
  size_.len_ = (i == TURTLE_INT64_NULL) ? TURTLE_VALUE_NULL : 0;
}

// TIMESTAMP
Value::Value(DataType type, uint64_t i)
    : manage_data_(false), data_type_(type) {
  value_.timestamp_ = i;
  size_.len_ = (i == TURTLE_TIMESTAMP_NULL) ? TURTLE_VALUE_NULL : 0;
}

// VARCHAR (Raw char array)
Value::Value(DataType type, const char *data, uint32_t len, bool manage_data)
    : manage_data_(manage_data), data_type_(type) {
  if (data == nullptr) {
    size_.len_ = TURTLE_VALUE_NULL;
  } else {
    size_.len_ = len;
    if (manage_data_) {
      value_.varlen_ = new char[len];
      std::memcpy(value_.varlen_, data, len);
    } else {
      value_.const_varlen_ = data;
    }
  }
}

// VARCHAR (std::string)
Value::Value(DataType type, std::string &data)
    : manage_data_(true), data_type_(type) {
  size_.len_ = data.length();
  value_.varlen_ = new char[size_.len_ + 1];
  std::memcpy(value_.varlen_, data.c_str(), size_.len_);
  value_.varlen_[size_.len_] = '\0';
}

Value::Value(DataType type, const std::vector<double> &data)
    : manage_data_(false), data_type_(type) {
  throw std::runtime_error("Vector constructor not fully implemented yet.");
}

// Copy Constructor
Value::Value(const Value &other)
    : manage_data_(other.manage_data_), data_type_(other.data_type_),
      size_(other.size_) {
  if (data_type_ == DataType::VARCHAR && !other.is_null()) {
    if (manage_data_) {
      value_.varlen_ = new char[size_.len_];
      std::memcpy(value_.varlen_, other.value_.varlen_, size_.len_);
    } else {
      value_.const_varlen_ = other.value_.const_varlen_;
    }
  } else {
    value_ = other.value_;
  }
}

// Copy Assignment Operator
Value &Value::operator=(Value other) {
  swap(*this, other);
  return *this;
}

// Destructor
Value::~Value() {
  if (manage_data_ && data_type_ == DataType::VARCHAR && !is_null()) {
    delete[] value_.varlen_;
  }
}

// Boilerplate implementation for check_integer and check_comparable
bool Value::check_integer() const {
  return data_type_ == DataType::TINYINT || data_type_ == DataType::SMALLINT ||
         data_type_ == DataType::INTEGER || data_type_ == DataType::BIGINT;
}

bool Value::check_comparable(const Value &o) const {
  return this->data_type_ == o.data_type_ ||
         (this->check_integer() && o.check_integer());
}

std::vector<double> Value::vector() const {
  throw std::runtime_error("Vector method not fully implemented yet.");
}

} // namespace turtle::datatype
