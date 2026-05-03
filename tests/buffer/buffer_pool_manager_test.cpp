#include <cstddef>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/page/page.hpp"

using turtle::FileId;
using turtle::FrameId;
using turtle::PageId;

void print_test_result(const std::string &test_name, bool success) {
  std::cout << "[TEST] " << test_name << ": " << (success ? "PASSED" : "FAILED")
            << std::endl;
}

/**
 * @test
 * Base new_page -> write -> unpin -> fetch -> verify
 */
void test_new_page_and_fetch(turtle::buffer::BufferPoolManager *bpm,
                             FileId file_id) {
  std::cout << "\n-- Running test_new_page_and_fetch --" << std::endl;
  bool success = true;

  PageId page_id_0;
  auto *page_0 = bpm->new_page(file_id, &page_id_0);

  if (page_0 == nullptr || page_id_0 != 0) {
    success = false;
  }

  print_test_result("new_page returns valud page", success);

  if (!success)
    return;

  const std::string test_str = "Hello from page 0!";
  std::strcpy(page_0->data(), test_str.c_str());

  // Unpin dirty
  bpm->unpin_page(file_id, page_id_0, true);

  // Fetch again
  auto *fetched = bpm->fetch_page(file_id, page_id_0);

  if (fetched == nullptr ||
      std::strcmp(fetched->data(), test_str.c_str()) != 0) {
    success = false;
  }

  print_test_result("Fetch page contains correct data", success);

  if (fetched) {
    bpm->unpin_page(file_id, page_id_0, false);
  }
}

/**
 * @test
 * File buffer pool, force eviction, verify data survives
 */
void test_buffer_pool_full(turtle::buffer::BufferPoolManager *bpm,
                           FileId file_id, size_t pool_size) {
  std::cout << "\n-- Running test_buffer_pool_full --" << std::endl;
  bool success = true;

  std::vector<PageId> page_ids;
  std::vector<turtle::storage::page::Page *> pages;

  // Fill buffer
  for (size_t i = 0; i < pool_size; ++i) {
    PageId pid;
    auto *page = bpm->new_page(file_id, &pid);

    if (!page) {
      success = false;
      std::cout << "[ERROR] new_page returned nullptr at i = " << i
                << std::endl;
      break;
    }

    std::string data = "Data for page " + std::to_string(pid);
    std::strcpy(page->data(), data.c_str());

    page_ids.push_back(pid);
    pages.push_back(page);
  }

  // Unpin all so eviction is possible
  for (size_t i = 0; i < pages.size(); ++i) {
    bpm->unpin_page(file_id, page_ids[i], true);
  }

  // Fetch a page again
  auto *fetched = bpm->fetch_page(file_id, 1);

  if (fetched == nullptr ||
      std::strcmp(fetched->data(), "Data for page 1") != 0) {
    success = false;
  }

  print_test_result("Evicted and re-fetched page retains data", success);

  if (fetched) {
    bpm->unpin_page(file_id, 1, false);
  }
}

/**
 * @test
 * All pages pinned → cannot allocate new page
 */
void test_pinning(turtle::buffer::BufferPoolManager *bpm, FileId file_id,
                  size_t pool_size) {
  std::cout << "\n--- Running test_pinning ---" << std::endl;
  bool success = true;

  std::vector<turtle::storage::page::Page *> pinned_pages;

  // Pin all pages
  for (size_t i = 0; i < pool_size; ++i) {
    auto *page = bpm->fetch_page(file_id, static_cast<PageId>(i));
    if (!page) {
      success = false;
      std::cout << "[ERROR] fetch_page failed for page " << i << std::endl;
      break;
    }
    pinned_pages.push_back(page);
  }

  // Try to allocate new page (should fail)
  PageId new_page_id;
  auto *should_be_null = bpm->new_page(file_id, &new_page_id);

  if (should_be_null != nullptr) {
    success = false;
    bpm->unpin_page(file_id, new_page_id, false);
  }

  print_test_result("Cannot allocate new page when all pages pinned", success);

  // Cleanup
  for (size_t i = 0; i < pinned_pages.size(); ++i) {
    bpm->unpin_page(file_id, pinned_pages[i]->page_id(), false);
  }
}

int main() {
  std::cout << "Starting BufferPoolManager tests...\n";

  const std::string test_dir = "test_db";
  const std::string test_file = test_dir + "/buffer_pool_test.db";
  const size_t buffer_pool_size = 10;

  // Clean previous run
  std::filesystem::remove_all(test_dir);
  std::filesystem::create_directories(test_dir);

  auto disk_manager = std::make_unique<turtle::storage::disk::DiskManager>();

  // Create file and get file_id
  disk_manager->create_file(test_file);
  FileId file_id = disk_manager->open_file(test_file);

  auto bpm = std::make_unique<turtle::buffer::BufferPoolManager>(
      buffer_pool_size, disk_manager.get());

  // Run tests
  test_new_page_and_fetch(bpm.get(), file_id);
  test_buffer_pool_full(bpm.get(), file_id, buffer_pool_size);
  test_pinning(bpm.get(), file_id, buffer_pool_size);

  std::cout << "\nAll tests finished.\n";
  return 0;
}
