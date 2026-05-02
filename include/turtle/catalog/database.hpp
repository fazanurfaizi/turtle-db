#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "turtle/catalog/metadata.hpp"
#include "turtle/catalog/schema.hpp"

namespace turtle::catalog {

class Database {
public:
  Database(Metadata &meta) : meta_(meta) {}

  const Metadata &get_metadata() const { return this->meta_; }
  Metadata &get_metadata() { return this->meta_; }

  Schema *create_schema(const std::string &schema_name);

  const std::unordered_map<std::string, std::unique_ptr<Schema>> &
  get_schemas() const noexcept {
    return this->schemas_;
  }

  Schema *get_schema(const std::string &schema_name) {
    auto it = this->schemas_.find(schema_name);
    return (it == this->schemas_.end()) ? nullptr : it->second.get();
  }

  // read-only access
  const Schema *get_schema(const std::string &schema_name) const {
    auto it = this->schemas_.find(schema_name);
    return (it == this->schemas_.end()) ? nullptr : it->second.get();
  }

private:
  Metadata &meta_;
  std::unordered_map<std::string, std::unique_ptr<Schema>> schemas_;
};

} // namespace Turtle::Catalog
