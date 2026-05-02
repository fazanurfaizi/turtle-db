#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "turtle/common/config.hpp"
#include "turtle/storage/disk_manager.hpp"

namespace turtle::storage {

DiskManager::~DiskManager() {
  std::lock_guard<std::mutex> guard(this->latch_);
  for (auto &[id, file] : this->files_) {
    if (file && file->is_open()) {
      file->flush();
      file->close();
    }
  }
}

FileId DiskManager::create_file(const std::string &file_path) {
  std::scoped_lock<std::mutex> lock(this->latch_);

  // Ensure directory exists
  std::filesystem::path p(file_path);
  if (p.has_parent_path() && !std::filesystem::exists(p.parent_path())) {
    std::filesystem::create_directories(p.parent_path());
  }

  // Create file if not exists
  std::ofstream create(file_path, std::ios::binary | std::ios::app);
  create.close();

  FileId file_id = this->next_file_id_++;
  this->path_to_file_id_[file_path] = file_id;

  auto fs = std::make_unique<std::fstream>(
    file_path, std::ios::in | std::ios::out | std::ios::binary
  );

  if (!fs->is_open()) {
    throw std::runtime_error("Failed to open file: " + file_path);
  }

  this->files_[file_id] = std::move(fs);
  return file_id;
}

FileId DiskManager::open_file(const std::string &file_path) {
  std::lock_guard<std::mutex> guard(this->latch_);

  if (this->path_to_file_id_.count(file_path)) {
    return this->path_to_file_id_[file_path];
  }

  FileId new_id = static_cast<FileId>(this->path_to_file_id_.size() + 1);
  this->path_to_file_id_[file_path] = new_id;

  auto fs = std::make_unique<std::fstream>(file_path, std::ios::in | std::ios::out | std::ios::binary);

  if (!fs->is_open()) {
    throw std::runtime_error("Failed to open file: " + file_path);
  }

  // this->file_[file_path] = std::move(fs);
  return new_id;
}

void DiskManager::read_page(FileId file_id, PageId page_id, char *page_data) {
  std::lock_guard<std::mutex> guard(this->latch_);

  auto it = this->files_.find(file_id);
  if (it == this->files_.end()) {
    throw std::runtime_error("Invalid file id in read_page");
  }

  std::fstream& file = *it->second;
  std::streamoff offset = static_cast<std::streamoff>(page_id) * PAGE_SIZE;

  file.seekg(offset, std::ios::beg);

  if (!file.good()) {
    file.clear();
    std::memset(page_data, 0, PAGE_SIZE);
    return;
  }

  file.read(page_data, PAGE_SIZE);

  std::streamsize ready_bytes = file.gcount();
  if (ready_bytes < static_cast<std::streamsize>(PAGE_SIZE)) {
    std::memset(page_data + ready_bytes, 0, PAGE_SIZE - ready_bytes);
  }

  // if (this->files_.find(file_path) == this->files_.end()) {
  //   // Ensure the directory exists
  //   std::filesystem::path p(file_path);
  //   if (p.has_parent_path() && !std::filesystem::exists(p.parent_path())) {
  //     std::filesystem::create_directories(p.parent_path());
  //   }
  //
  //   this->files_[file_path].open(file_path,
  //                                       std::ios::in | std::ios::out |
  //                                           std::ios::binary | std::ios::app);
  //   if (!this->files_[file_path].is_open()) {
  //     throw std::runtime_error("Could not open file for reading: " + file_path);
  //   }
  // }
  //
  // std::fstream &file = this->files_[file_path];
  // int offset = page_id * PAGE_SIZE;
  //
  // file.seekg(offset);
  // if (file.fail() || file.eof()) {
  //   file.clear();
  //   std::fill(page_data, page_data + PAGE_SIZE, 0);
  //   return;
  // }
  //
  // file.read(page_data, PAGE_SIZE);
  // if (file.gcount() == 0) {
  //   std::fill(page_data, page_data + PAGE_SIZE, 0);
  // } else if (file.gcount() < PAGE_SIZE && file.gcount() > 0) {
  //   std::fill(page_data + file.gcount(), page_data + PAGE_SIZE, 0);
  // }
}

void DiskManager::write_page(FileId file_id, PageId page_id, const char *page_data) {
  std::lock_guard<std::mutex> guard(this->latch_);
  
  auto it = this->files_.find(file_id);
  if (it == this->files_.end()) {
    throw std::runtime_error("Invalid file id in write_page");
  }

  std::fstream& file = *it->second;
  std::streamoff offset = static_cast<std::streamoff>(page_id) * PAGE_SIZE;

  file.seekg(offset, std::ios::beg);
  file.write(page_data, PAGE_SIZE);

  if (file.fail()) {
    throw std::runtime_error("Disk write failed");
  }

  file.flush();
  //   // Ensure directory exists
  //   std::filesystem::path p(file_path);
  //   if (p.has_parent_path() && !std::filesystem::exists(p.parent_path())) {
  //     std::filesystem::create_directories(p.parent_path());
  //   }
  //
  //   // Try to open for read/write; if it fails, create the file first
  //   std::fstream fs(file_path, std::ios::in | std::ios::out | std::ios::binary);
  //   if (!fs.is_open()) {
  //     std::ofstream create(file_path,
  //                          std::ios::out | std::ios::binary | std::ios::trunc);
  //     create.close();
  //     fs.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
  //   }
  //
  //   if (!fs.is_open()) {
  //     throw std::runtime_error("Could not open file for writing: " + file_path);
  //   }
  //   this->files_[file_path] = std::move(fs);
  // }
  //
  // std::fstream &file = this->files_[file_path];
  // std::streamoff offset = static_cast<std::streamoff>(page_id) * PAGE_SIZE;
  //
  // file.seekp(offset);
  // if (!file.good())
  //   file.clear();
  //
  // file.write(page_data, PAGE_SIZE);
  //
  // if (file.fail()) {
  //   throw std::runtime_error("Error writing to file: " + file_path);
  // }
  //
  // file.flush(); // Ensure data is written to OS buffer
}

} // namespace Turtle::Storage
