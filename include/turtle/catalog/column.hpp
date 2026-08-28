#ifndef TURTLE_CATALOG_COLUMN_H
#define TURTLE_CATALOG_COLUMN_H

#include <cstdint>
#include <string>

#include "turtle/datatype/data_types.hpp"

namespace turtle::catalog {

class Column {
public:
  Column(std::string col_name, datatype::DataType type, uint32_t length = 0)
      : name_(std::move(col_name)), type_(type), length_(length) {}

  const std::string &get_name() const { return this->name_; }
  datatype::DataType get_type() const { return this->type_; }
  uint32_t get_length() const { return this->length_; }

private:
  std::string name_;
  datatype::DataType type_;
  uint32_t length_;
};

} // namespace turtle::catalog

#endif // !TURTLE_CATALOG_COLUMN_H
