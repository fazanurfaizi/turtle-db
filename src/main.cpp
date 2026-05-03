// #include <cstring>
// #include <filesystem>
// #include <iostream>
// #include <memory>
// #include <string>
// #include <vector>
//
// #include "turtle/buffer/buffer_pool_manager.hpp"
// #include "turtle/catalog/catalog.hpp"
// #include "turtle/common/config.hpp"
// #include "turtle/storage/disk_manager.hpp"
// #include "turtle/storage/page.hpp"
//
// void print_test_result(const std::string &test_name, bool success) {
//   std::cout << "[TEST] " << test_name << ": " << (success ? "PASSED" :
//   "FAILED")
//             << std::endl;
// }
//
// // Test case 2: Basic page creation, writing, and fetching.
// void test_new_page_and_fetch(turtle::buffer::BufferPoolManager *bpm,
//                              const std::string &file_path) {
//   std::cout << "\n--- Running test_new_page_and_fetch ---" << std::endl;
//   bool success = true;
//
//   turtle::PageId page_id_1;
//   // Create a new page
//   turtle::storage::Page *page_1 = bpm->new_page(file_path, &page_id_0);
//
//   if (page_1 == nullptr || page_0->page_id() != 0) {
//     success = false;
//   }
//   print_test_result("new_page returns a valid page", success);
//
//   if (success) {
//     std::string test_str = "Hello from Page 1!";
//
//     // Write some data to the page
//     strcpy(page_1->data(), test_str.c_str());
//
//     // unpin the page, marking it as dirty
//     bpm->unpin_page(file_path, page_id_1, true);
//
//     // Fetch the same page again
//     turtle::storage::Page *fetched_page_1 =
//         bpm->fetch_page(file_path, page_id_1);
//
//     // Verify the content
//     if (fetched_page_1 == nullptr ||
//         strcmp(fetched_page_1->get_data(), test_str.c_str()) != 0) {
//       success = false;
//     }
//     print_test_result("Fetched page contains correct data", success);
//
//     // unpin again
//     bpm->unpin_page(file_path, page_id_1, false);
//   }
// }
//
// // Test case 3: Test the LRU replacement policy by filling the buffer pool.
// void test_buffer_pool_full(turtle::buffer::BufferPoolManager *bpm,
//                            const std::string &file_path) {
//   std::cout << "\n--- Running test_buffer_pool_full ---" << std::endl;
//   bool success = true;
//   const size_t pool_size = 11;
//
//   std::vector<turtle::storage::Page *> pages;
//   std::vector<turtle::PageId> page_ids;
//
//   // Create pages to fill the buffer pool + one more to trigger eviction
//   for (size_t i = 1; i < pool_size; ++i) {
//     turtle::PageId new_page_id;
//     turtle::storage::Page *page = bpm->new_page(file_path, &new_page_id);
//
//     if (page == nullptr) {
//       success = false;
//       std::cout << "[ERROR] new_page returned null or wrong id: "
//                 << (page ? page->get_page_id() : 0) << " expected "
//                 << new_page_id << '\n';
//       break;
//     }
//     pages.push_back(page);
//     page_ids.push_back(new_page_id);
//
//     // Write unique data to each page
//     std::string data = "Data for page " + std::to_string(new_page_id);
//     strcpy(page->get_data(), data.c_str());
//   }
//
//   // unpin all pages to make them candidates for eviction
//   for (size_t i = 1; i < pages.size(); ++i) {
//     bpm->unpin_page(file_path, page_ids[i], true);
//   }
//
//   turtle::storage::Page *fetched_page_2 = bpm->fetch_page(file_path, 1);
//
//   if (fetched_page_2 == nullptr ||
//       strcmp(fetched_page_2->get_data(), "Data for page 1") != 0) {
//     success = false;
//   }
//
//   print_test_result("Evicted and re-fetched page retains data", success);
//   if (fetched_page_2) {
//     bpm->unpin_page(file_path, 2, false);
//   }
// }
//
// void test_pinning(turtle::buffer::BufferPoolManager *bpm,
//                   const std::string &file_path) {
//   std::cout << "\n--- Running test_pinning ---" << std::endl;
//   bool success = true;
//   const size_t pool_size = 11;
//
//   // Fetch all pages that should currently be in the buffer pool
//   std::vector<turtle::storage::Page *> pinned_pages;
//   for (size_t i = 1; i < pool_size; ++i) {
//     turtle::storage::Page *page =
//         bpm->fetch_page(file_path, static_cast<turtle::PageId>(i));
//     if (page == nullptr) {
//       success = false;
//       std::cout << "[ERROR] fetch_page returned null unexpectedly for page "
//                 << i << std::endl;
//       break;
//     }
//     pinned_pages.push_back(page);
//   }
//
//   turtle::PageId new_page_id;
//   turtle::storage::Page *should_be_null =
//       bpm->new_page(file_path, &new_page_id);
//
//   if (should_be_null != nullptr) {
//     success = false;
//     bpm->unpin_page(file_path, new_page_id, false);
//   }
//
//   print_test_result("Cannot fetch new page when pool is fully pinned",
//   success);
//
//   // Unpuin all the pages to clean up
//   for (size_t i = 1; i < pinned_pages.size(); ++i) {
//     bpm->unpin_page(file_path, pinned_pages[i]->get_page_id(), false);
//   }
// }
//
// int main() {
//   // Setup
//   const std::string db_dir = "test_db";
//
//   // Clean up previous test runs
//   if (!std::filesystem::exists(db_dir)) {
//     // std::filesystem::remove_all(db_dir);
//     std::filesystem::create_directories(db_dir);
//   }
//
//   // const std::string file_path = db_dir + "/test.db";
//   const size_t buffer_pool_size = 11;
//
//   const std::string catalog_file = db_dir + "/turtle.catalog";
//
//   auto disk_manager = std::make_unique<turtle::storage::DiskManager>();
//   auto bpm = std::make_unique<turtle::buffer::BufferPoolManager>(
//       buffer_pool_size, disk_manager.get());
//   auto catalog = std::make_unique<turtle::catalog::Catalog>(
//       bpm.get(), disk_manager.get(), catalog_file, db_dir);
//
//   const std::string catalog_database = "testing_db";
//   catalog.get()->create_database(catalog_database);
//
//   auto databases = catalog.get()->list_databases();
//   for (const auto &database : databases) {
//     std::cout << database.get_db_name() << std::endl;
//     std::cout << database.get_meta_file_path() << std::endl;
//     std::cout << database.get_db_file_path() << std::endl;
//   }
//   auto fetched_db = catalog.get()->get_database(catalog_database);
//   if (fetched_db) {
//     fetched_db->create_schema("master");
//   }
//
//   std::cout << "Database Name: " << fetched_db->get_metadata().get_db_name()
//             << std::endl;
//   std::cout << "Total schemas: "
//             << std::to_string(fetched_db->get_metadata().get_schema_count())
//             << std::endl;
//
//   // std::cout << "Starting buffer pool manager tests...." << std::endl;
//   //
//   // // Run tests
//   // test_new_page_and_fetch(bpm.get(), file_path);
//   // test_buffer_pool_full(bpm.get(), file_path);
//   // test_pinning(bpm.get(), file_path);
//   //
//   // std::cout << "\nTests finished. Shutting down." << std::endl;
//
//   return 1;
// }
#include <iostream>
#include <memory>
#include <vector>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/execution/seq_scan_executor.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/tuple.hpp"

int main(int argc, char *argv[]) {
  auto disk_manager = std::make_unique<turtle::storage::disk::DiskManager>();
  auto bpm = std::make_unique<turtle::buffer::BufferPoolManager>(
      10, disk_manager.get());

  turtle::FileId table_file_id = disk_manager->create_file("test_db/table.db");

  auto table_heap = std::make_unique<turtle::storage::table::TableHeap>(
      bpm.get(), table_file_id);

  // Create a mock Table Schema
  std::vector<turtle::catalog::Column> cols;
  cols.emplace_back("id", turtle::datatype::DataType::INTEGER);
  cols.emplace_back("age", turtle::datatype::DataType::INTEGER);
  turtle::catalog::ColumnSchema mock_table_schema(cols);

  // Create the raw values
  std::vector<turtle::datatype::Value> values_1;
  values_1.emplace_back(turtle::datatype::DataType::INTEGER, 1);
  values_1.emplace_back(turtle::datatype::DataType::INTEGER, 25);
  turtle::storage::table::Tuple tuple1(&mock_table_schema, values_1);

  std::vector<turtle::datatype::Value> values_2;
  values_2.emplace_back(turtle::datatype::DataType::INTEGER, 2);
  values_2.emplace_back(turtle::datatype::DataType::INTEGER, 50);
  turtle::storage::table::Tuple tuple2(&mock_table_schema, values_2);

  turtle::RecordId rid;

  table_heap->insert_tuple(tuple1, &rid);
  table_heap->insert_tuple(tuple2, &rid);

  // Run a Sequential Scan
  turtle::execution::SeqScanExecutor scanner(table_heap.get());
  scanner.init();

  turtle::storage::table::Tuple result_tuple;
  turtle::RecordId result_rid;

  std::cout << "--- Starting Scan ---" << std::endl;
  while (scanner.next(&result_tuple, &result_rid)) {
    std::cout << "Found Tuple at: " << result_rid.to_string() << std::endl;
  }
  std::cout << "--- Scan Complete ---" << std::endl;

  return 0;
}
