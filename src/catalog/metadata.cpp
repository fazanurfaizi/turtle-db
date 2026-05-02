#include "turtle/catalog/metadata.hpp"

namespace turtle::catalog {

Metadata::Metadata(std::string db_name, std::string db_file_path,
                   std::string meta_file_path)
    : db_name_(db_name), db_file_path_(db_file_path),
      meta_file_path_(meta_file_path),
      created_at_(std::chrono::system_clock::now()), updated_at_(created_at_) {}

} // namespace Turtle::Catalog
