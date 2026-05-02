#ifndef TURTLE_CATALOG_COLUMN_H
#define TURTLE_CATALOG_COLUMN_H

#include <string>

#include "turtle/datatype/data_types.hpp"

namespace turtle::catalog {

class Column {
public:
  Column(std::string col_name, datatype::DataType type, int length = 0)
      : name_(std::move(col_name)), type_(type), length_(length) {}

  const std::string &get_name() const { return this->name_; }
  datatype::DataType get_type() const { return this->type_; }
  int get_length() const { return this->length_; }

private:
  std::string name_;
  datatype::DataType type_;
  int length_;
};

} // namespace Turtle::Catalog

#endif // !TURTLE_CATALOG_COLUMN_H
