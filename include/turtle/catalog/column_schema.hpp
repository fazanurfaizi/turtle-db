#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "fmt/format.h"

#include "column.hpp"
#include "turtle/datatype/type.hpp"

namespace turtle::catalog {

class ColumnSchema;
using ColumnSchemaRef = std::shared_ptr<const ColumnSchema>;

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

  auto to_string() const -> std::string {
    std::string out = "(";
    for (std::size_t i = 0; i < this->columns_.size(); ++i) {
      const auto &col = this->columns_[i];
      out += fmt::format("{}:{}", col.get_name(),
                         datatype::Type::data_type_to_string(col.get_type()));
      if (i + 1 < this->columns_.size())
        out += ", ";
    }
    out += ")";
    return out;
  }

private:
  std::vector<Column> columns_;
};

} // namespace turtle::catalog

template <>
struct fmt::formatter<turtle::catalog::ColumnSchema>
    : fmt::formatter<std::string, char> {
  template <typename FormatCtx>
  auto format(const turtle::catalog::ColumnSchema &schema,
              FormatCtx &ctx) const {
    return fmt::formatter<std::string, char>::format(schema.to_string(), ctx);
  }
};
