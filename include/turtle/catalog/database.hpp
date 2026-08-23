#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "turtle/catalog/metadata.hpp"
#include "turtle/catalog/namespace.hpp"

namespace turtle::catalog {

class Database {
public:
  Database(Metadata &meta) : meta_(meta) {}

  const Metadata &get_metadata() const { return this->meta_; }
  Metadata &get_metadata() { return this->meta_; }

  Namespace *create_namespace(const std::string &namespace_name);

  const std::unordered_map<std::string, std::unique_ptr<Namespace>> &
  get_namespaces() const noexcept {
    return this->namespaces_;
  }

  Namespace *get_namespace(const std::string &namespace_name) {
    auto it = this->namespaces_.find(namespace_name);
    return (it == this->namespaces_.end()) ? nullptr : it->second.get();
  }

  // read-only access
  const Namespace *get_namespace(const std::string &namespace_name) const {
    auto it = this->namespaces_.find(namespace_name);
    return (it == this->namespaces_.end()) ? nullptr : it->second.get();
  }

private:
  Metadata &meta_;
  std::unordered_map<std::string, std::unique_ptr<Namespace>> namespaces_;
};

} // namespace turtle::catalog
