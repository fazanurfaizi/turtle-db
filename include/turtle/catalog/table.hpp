#pragma once

#include <string>

#include "column_schema.hpp"
#include "turtle/common/config.hpp"

namespace turtle::catalog {

class Table {
public:
  Table(std::string table_name, ColumnSchema schema, PageId root_page_id)
      : name_(std::move(table_name)), schema_(std::move(schema)),
        root_page_id_(root_page_id) {}

  const std::string &get_name() const { return this->name_; }
  const ColumnSchema &get_schema() const { return this->schema_; }
  PageId get_root_page_id() const { return this->root_page_id_; }

private:
  std::string name_;
  ColumnSchema schema_;
  PageId root_page_id_;
};

} // namespace Turtle::Catalog

