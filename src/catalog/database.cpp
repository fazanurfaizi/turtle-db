#include "turtle/catalog/database.hpp"

namespace turtle::catalog {

Schema *Database::create_schema(const std::string &schema_name) {
  // Check if schema already exists
  auto it = this->schemas_.find(schema_name);
  if (it != this->schemas_.end()) {
    return it->second.get();
  }

  // Insert new schema
  auto schema = std::make_unique<Schema>(schema_name);
  auto *schema_ptr = schema.get();
  this->schemas_.emplace(schema_name, std::move(schema));

  this->meta_.increase_schema();
  this->meta_.touch();
  return schema_ptr;
}

} // namespace Turtle::Catalog
