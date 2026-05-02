#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "column.hpp"

namespace turtle::catalog {

class ColumnSchema {
public:
  explicit ColumnSchema(std::vector<Column> columns)
      : columns_(std::move(columns)) {}

  const std::vector<Column> &get_columns() const { return this->columns_; }

  const Column &get_column(const std::string &col_name) const {
    for (const auto &col : this->columns_) {
      if (col.get_name() == col_name)
        return col;
    }
    throw std::runtime_error("Column not found: " + col_name);
  }

private:
  std::vector<Column> columns_;
};

} // namespace Turtle::Catalog

