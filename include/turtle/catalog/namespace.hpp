#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "table.hpp"

namespace turtle::catalog {

// A Namespace is a container of tables (Postgres calls this a schema; the SQL
// keyword stays CREATE SCHEMA, but the type is named Namespace to avoid
// colliding with ColumnSchema, which describes a row's column layout).
class Namespace {
public:
  explicit Namespace(std::string namespace_name)
      : name_(std::move(namespace_name)) {}

  const std::string &get_name() const { return this->name_; }

  void add_table(std::unique_ptr<Table> table) {
    this->tables_.emplace(table->get_name(), std::move(table));
  }

  Table *table(const std::string &table_name) {
    auto it = this->tables_.find(table_name);
    return (it == this->tables_.end()) ? nullptr : it->second.get();
  }

  size_t total_tables() const { return this->tables_.size(); }

private:
  std::string name_;
  std::unordered_map<std::string, std::unique_ptr<Table>> tables_;
};

} // namespace turtle::catalog

