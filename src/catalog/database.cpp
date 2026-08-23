#include "turtle/catalog/database.hpp"

namespace turtle::catalog {

Namespace *Database::create_namespace(const std::string &namespace_name) {
  // Check if namespace already exists
  auto it = this->namespaces_.find(namespace_name);
  if (it != this->namespaces_.end()) {
    return it->second.get();
  }

  // Insert new namespace
  auto ns = std::make_unique<Namespace>(namespace_name);
  auto *ns_ptr = ns.get();
  this->namespaces_.emplace(namespace_name, std::move(ns));

  this->meta_.increase_namespace();
  this->meta_.touch();
  return ns_ptr;
}

} // namespace turtle::catalog
