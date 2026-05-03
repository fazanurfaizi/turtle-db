#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/database.hpp"
#include "turtle/catalog/metadata.hpp"
#include "turtle/storage/disk/disk_manager.hpp"

namespace turtle::catalog {

class Catalog {
public:
  Catalog(buffer::BufferPoolManager *bpm,
          storage::disk::DiskManager *disk_manager,
          const std::string &catalog_file, const std::string &db_dir);
  ~Catalog();
  Catalog(const Catalog &) = delete;
  Catalog &operator=(const Catalog &) = delete;
  Catalog(Catalog &&) noexcept = default;
  Catalog &operator=(Catalog &&) noexcept = default;

  Database *create_database(const std::string &database_name);

  bool drop_database(const std::string &database_name);

  std::vector<Metadata> list_databases() const;
  Metadata get_database_info(const std::string &db_name) const;
  Database *get_database(const std::string &database_name);
  const Database *get_database(const std::string &database_name) const;

private:
  buffer::BufferPoolManager *bpm_;
  storage::disk::DiskManager *disk_manager_;
  std::string catalog_file_path_;
  std::string db_dir_;

  std::unordered_map<std::string, std::unique_ptr<Metadata>> metadatas_;
  std::unordered_map<std::string, std::unique_ptr<Database>> databases_;

  void load();
  void flush();
  void save_database_metadata(const Database &db, const std::string &meta_file);
  void load_database_metadata(Database &db, const std::string &meta_file);
};

} // namespace turtle::catalog
