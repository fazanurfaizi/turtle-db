#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "turtle/catalog/catalog.hpp"
#include "turtle/catalog/database.hpp"
#include "turtle/catalog/metadata.hpp"
#include "turtle/common/util/binary_serializer.hpp"
#include "turtle/storage/disk_manager.hpp"

namespace turtle::catalog {

constexpr uint32_t MAGIC_NUMBER = 0xDEADBEEF;
constexpr uint32_t DATABASE_VERSION = 1;

Catalog::Catalog(buffer::BufferPoolManager *bpm,
                 storage::DiskManager *disk_manager,
                 const std::string &catalog_file, const std::string &db_dir)
    : bpm_(bpm), disk_manager_(disk_manager),
      catalog_file_path_(std::move(catalog_file)), db_dir_(std::move(db_dir)) {
  this->load();
}

Catalog::~Catalog() { this->flush(); }

std::vector<Metadata> Catalog::list_databases() const {
  std::vector<Metadata> db_infos;
  for (const auto &db : this->metadatas_) {
    db_infos.push_back(this->get_database_info(db.first));
  }
  return db_infos;
}

Metadata Catalog::get_database_info(const std::string &database_name) const {
  // Find logical database object.
  auto it = this->metadatas_.find(database_name);
  if (it == this->metadatas_.end()) {
    throw std::runtime_error("Database not found: " + database_name);
  }
  return *it->second.get();
  // const Metadata *meta = it->second.get();

  // return {.name = meta->get_db_name(),
  //         .db_file_path = meta->get_db_file_path(),
  //   .meta_file_path
  //         .schema_count = meta->get_schema_count(),
  //         .table_count = meta->get_table_count(),
  //         .created_at = meta->get_created_at(),
  //         .updated_at = meta->get_updated_at()};
}

Database *Catalog::get_database(const std::string &database_name) {
  auto it = this->databases_.find(database_name);
  return (it == this->databases_.end()) ? nullptr : it->second.get();
}

// read-only access
const Database *Catalog::get_database(const std::string &database_name) const {
  auto it = this->databases_.find(database_name);
  return (it == this->databases_.end()) ? nullptr : it->second.get();
}

Database *Catalog::create_database(const std::string &database_name) {
  if (this->databases_.count(database_name) > 0 ||
      this->metadatas_.count(database_name) > 0) {
    return nullptr;
  }

  auto db_file_path =
      std::filesystem::path(this->db_dir_) / (database_name + ".db");

  auto meta_file_path =
      std::filesystem::path(this->db_dir_) / (database_name + ".meta");

  try {
    this->disk_manager_->create_file(db_file_path.string());
    this->disk_manager_->create_file(meta_file_path.string());

    auto metadata = std::make_unique<Metadata>(std::move(database_name),
                                               std::move(db_file_path),
                                               std::move(meta_file_path));

    auto db = std::make_unique<Database>(*metadata);
    Database *ptr = db.get();

    this->metadatas_.emplace(database_name, std::move(metadata));
    this->databases_.emplace(database_name, std::move(db));

    ptr->create_schema("public");
    return ptr;
  } catch (const std::exception &e) {
    std::cerr << "Error creating database file: " << e.what() << std::endl;
    return nullptr;
  }
}

void Catalog::load() {
  std::cout << "Load databases from catalog" << std::endl;

  std::ifstream ifs(this->catalog_file_path_, std::ios::binary);
  if (!ifs.is_open()) {
    return;
  }

  uint32_t magic;
  if (!util::read_binary(ifs, magic) || magic != MAGIC_NUMBER) {
    std::cerr
        << "Error: Catalog file is corrupted or not a valid TurtleDB catalog."
        << std::endl;
    return;
  }

  uint32_t version = 0;
  if (!util::read_binary(ifs, version) || version != DATABASE_VERSION) {
    std::cerr << "Error: Database version is not supported." << std::endl;
    return;
  }

  uint32_t db_count = 0;
  if (!util::read_binary(ifs, db_count)) {
    throw std::runtime_error("Failed to read database count from catalog.");
  }

  for (uint32_t i = 0; i < db_count; ++i) {
    std::string db_name = util::read_string(ifs);
    std::string db_file_path = util::read_string(ifs);
    std::string meta_file_path = util::read_string(ifs);

    auto metadata =
        std::make_unique<Metadata>(db_name, db_file_path, meta_file_path);
    auto db = std::make_unique<Database>(*metadata);

    try {
      this->load_database_metadata(*db, meta_file_path);
    } catch (const std::exception &e) {
      std::cerr << "[Catalog] Warning: could not load metadata for DB "
                << db_name << ": " << e.what() << '\n';
    }

    this->metadatas_.emplace(db_name, std::move(metadata));
    this->databases_.emplace(db_name, std::move(db));
  }
}

void Catalog::flush() {
  std::ofstream ofs(this->catalog_file_path_,
                    std::ios::binary | std::ios::trunc);
  if (!ofs.is_open()) {
    std::cerr << "Error: Could not open catalog file for writing: "
              << catalog_file_path_ << std::endl;
    return;
  }

  util::write_binary(ofs, MAGIC_NUMBER);
  util::write_binary(ofs, DATABASE_VERSION);

  util::write_binary(ofs, static_cast<uint32_t>(this->databases_.size()));

  for (const auto &pair : this->databases_) {
    const auto &name = pair.first;
    const Database &database = *pair.second;

    util::write_string(ofs, name);
    util::write_string(ofs, database.get_metadata().get_db_file_path());
    util::write_string(ofs, database.get_metadata().get_meta_file_path());

    // dump per-database metadata into its own file
    const auto meta_path =
        std::filesystem::path(database.get_metadata().get_meta_file_path());

    this->save_database_metadata(database, meta_path.string());
  }
}

void Catalog::save_database_metadata(const Database &db,
                                     const std::string &meta_file) {
  const Metadata &metadata = db.get_metadata();

  std::ofstream ofs(meta_file, std::ios::binary | std::ios::trunc);
  if (!ofs.is_open()) {
    throw std::runtime_error("Cannot open metadata file for writing: " +
                             meta_file);
  }

  // File Header
  util::write_binary(ofs, MAGIC_NUMBER);

  util::write_string(ofs, metadata.get_db_name());
  util::write_string(ofs, metadata.get_meta_file_path());
  util::write_string(ofs, metadata.get_db_file_path());
  util::write_binary(ofs, metadata.get_created_at());
  util::write_binary(ofs, metadata.get_updated_at());

  // Schemas
  uint32_t schema_count = static_cast<uint32_t>(metadata.get_schema_count());
  util::write_binary(ofs, schema_count);

  for (const auto &schema_entry : db.get_schemas()) {
    util::write_string(ofs, schema_entry.first);

    uint32_t table_count = static_cast<uint32_t>(metadata.get_table_count());
    util::write_binary(ofs, table_count);
  }
}

void Catalog::load_database_metadata(Database &db,
                                     const std::string &meta_file) {
  std::ifstream ifs(meta_file, std::ios::binary);
  if (!ifs.is_open()) {
    throw std::runtime_error("Cannot open metadata file for reading: " +
                             meta_file);
  }

  uint32_t magic;
  if (!util::read_binary(ifs, magic) || magic != MAGIC_NUMBER) {
    throw std::runtime_error("Invalid or corrupted metadata file: " +
                             meta_file);
  }

  std::string db_name = util::read_string(ifs);
  std::string meta_file_path = util::read_string(ifs);
  std::string db_file_path = util::read_string(ifs);

  std::chrono::system_clock::time_point created_at, updated_at;
  util::read_binary(ifs, created_at);
  util::read_binary(ifs, updated_at);

  uint32_t schema_count;
  util::read_binary(ifs, schema_count);

  for (uint32_t i = 0; i < schema_count; ++i) {
    const std::string schema_name = util::read_string(ifs);
    // Schema *schema =
    db.create_schema(schema_name);

    uint32_t table_count;
    util::read_binary(ifs, table_count);
  }

  db.get_metadata().touch();
}

} // namespace Turtle::Catalog
