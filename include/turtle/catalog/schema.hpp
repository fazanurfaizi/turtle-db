#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "table.hpp"

namespace turtle::catalog {

class Schema {
public:
  explicit Schema(std::string schema_name) : name_(std::move(schema_name)) {}

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

} // namespace Turtle::Catalog

