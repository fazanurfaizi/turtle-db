#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "turtle/common/config.hpp"
#include "turtle/storage/disk/disk_manager.hpp"

namespace turtle::storage::disk {

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
      file_path, std::ios::in | std::ios::out | std::ios::binary);

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

  auto fs = std::make_unique<std::fstream>(
      file_path, std::ios::in | std::ios::out | std::ios::binary);

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

  std::fstream &file = *it->second;
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
}

void DiskManager::write_page(FileId file_id, PageId page_id,
                             const char *page_data) {
  std::lock_guard<std::mutex> guard(this->latch_);

  auto it = this->files_.find(file_id);
  if (it == this->files_.end()) {
    throw std::runtime_error("Invalid file id in write_page");
  }

  std::fstream &file = *it->second;
  std::streamoff offset = static_cast<std::streamoff>(page_id) * PAGE_SIZE;

  file.seekp(offset, std::ios::beg);
  file.write(page_data, PAGE_SIZE);

  if (file.fail()) {
    throw std::runtime_error("Disk write failed");
  }

  file.flush();
}

void DiskManager::delete_page(FileId file_id, PageId page_id) {
  std::lock_guard<std::mutex> guard(this->latch_);

  auto it = this->files_.find(file_id);
  if (it == this->files_.end()) {
    throw std::runtime_error("Invalid file id in delete_page");
  }

  // No free-space map yet, so we cannot truly reclaim the slot. Zero the page
  // in place so its contents are gone and it reads back as an empty page.
  std::fstream &file = *it->second;
  std::streamoff offset = static_cast<std::streamoff>(page_id) * PAGE_SIZE;

  char zeros[PAGE_SIZE] = {};
  file.seekp(offset, std::ios::beg);
  file.write(zeros, PAGE_SIZE);

  if (file.fail()) {
    throw std::runtime_error("Disk delete (zero-fill) failed");
  }

  file.flush();
}

} // namespace turtle::storage::disk
