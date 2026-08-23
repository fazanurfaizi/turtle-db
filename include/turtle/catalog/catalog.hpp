#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/database.hpp"
#include "turtle/catalog/metadata.hpp"
#include "turtle/catalog/table_info.hpp"
#include "turtle/common/config.hpp"
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

  /** Table runtime registry
   * Creates a table's physical storage (its own backing file + heap) and
   * registers it, returning a non-owning handle
   */
  TableInfo *create_table(const std::string &table_name, ColumnSchema schema);

  // Resolve a registered table by OID or by name; nullptr if not found.
  TableInfo *get_table(TableOid oid);
  TableInfo *get_table(const std::string &table_name);

private:
  buffer::BufferPoolManager *bpm_;
  storage::disk::DiskManager *disk_manager_;
  std::string catalog_file_path_;
  std::string db_dir_;

  std::unordered_map<std::string, std::unique_ptr<Metadata>> metadatas_;
  std::unordered_map<std::string, std::unique_ptr<Database>> databases_;

  // Runtime table registry
  std::unordered_map<TableOid, std::unique_ptr<TableInfo>> tables_;
  std::unordered_map<std::string, TableOid> table_oids_;
  TableOid next_table_oid_{0};

  void load();
  void flush();
  void save_database_metadata(const Database &db, const std::string &meta_file);
  void load_database_metadata(Database &db, const std::string &meta_file);
};

} // namespace turtle::catalog
