#pragma once

#include <chrono>
#include <string>

namespace turtle::catalog {

class Metadata {
public:
  explicit Metadata(std::string db_name, std::string db_file_path,
                    std::string meta_file_path);

  const std::string &get_db_name() const { return this->db_name_; }
  const std::string &get_db_file_path() const { return this->db_file_path_; }
  const std::string &get_meta_file_path() const {
    return this->meta_file_path_;
  }
  const std::chrono::system_clock::time_point &get_created_at() const {
    return this->created_at_;
  }
  const std::chrono::system_clock::time_point &get_updated_at() const {
    return this->updated_at_;
  }
  void touch() { this->updated_at_ = std::chrono::system_clock::now(); }

  const size_t &get_schema_count() const { return this->schema_count_; }
  void set_schema_count(size_t total) { this->schema_count_ = total; }
  void increase_schema() { this->schema_count_++; }
  void decrease_schema() { this->schema_count_--; }

  const size_t &get_table_count() const { return this->table_count_; }
  void set_table_count(size_t total) { this->table_count_ = total; }
  void increase_table() { this->table_count_++; }
  void descrease_table() { this->table_count_--; }

private:
  std::string db_name_;
  std::string db_file_path_;
  std::string meta_file_path_;
  std::chrono::system_clock::time_point created_at_;
  std::chrono::system_clock::time_point updated_at_;

  std::string meta_checksum;
  std::string db_checksum;

  size_t schema_count_{0};
  size_t table_count_{0};
};

} // namespace Turtle::Catalog
