#pragma once

#include <atomic>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "turtle/common/config.hpp"

namespace turtle::storage {

class DiskManager {
public:
  DiskManager() = default;
  ~DiskManager();

  // Prevent copying
  DiskManager(const DiskManager &) = delete;
  DiskManager &operator=(const DiskManager &) = delete;

  FileId create_file(const std::string &file_path);

  FileId open_file(const std::string &file_path);

  /**
   * @brief Reads a page from a specific file on disk.
   * @param file_path The path to the file.
   * @param page_id The ID of the page to read.
   * @param page_data A buffer to read the page data into.
   */
  void read_page(FileId file_id, PageId page_id,
                 char *page_data);

  /**
   * @brief Writes a page to a specific file on disk.
   * @param file_path The path to the file.
   * @param page_id The ID of the page to write.
   * @param page_data The data to write to the page.
   */
  void write_page(FileId file_id, PageId page_id,
                  const char *page_data);

private:
  std::unordered_map<FileId, std::unique_ptr<std::fstream>> files_;
  std::unordered_map<FileId, std::unique_ptr<std::fstream>> file_;
  std::unordered_map<std::string, FileId> path_to_file_id_;
  std::mutex latch_;

  std::atomic<FileId> next_file_id_{0};

};

} // namespace Turtle::Storage
