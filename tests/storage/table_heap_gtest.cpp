// -----------------------------------------------------------------------------
// Tier 2 (Access & Structures) — turtle::storage::table::TableHeap + TableIterator
//
// Scope: a SQL table as a singly-linked list of slotted pages. Exercises the
// insert -> get_tuple round trip, automatic page growth when a page fills, and
// a full sequential scan via begin()/end(). This is an integration test over
// the whole storage stack (DiskManager -> BPM -> SlottedPage -> Tuple), which
// is deliberate: the heap only has meaning on top of real pages.
//
// Fixture owns DiskManager + BPM + backing file; the heap is created per test.
// mark_delete() is intentionally never called: it is declared but has no
// definition in table_heap.cpp, so referencing it would fail to link.
// -----------------------------------------------------------------------------
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/config.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/table_iterator.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::storage::table {
namespace {

using catalog::Column;
using catalog::ColumnSchema;
using datatype::DataType;
using datatype::Value;

class TableHeapTest : public ::testing::Test {
protected:
  void SetUp() override {
    dir_ = std::filesystem::path("test_db") / "table_heap_gtest";
    std::filesystem::remove_all(dir_);
    std::filesystem::create_directories(dir_);

    disk_ = std::make_unique<storage::disk::DiskManager>();
    file_ = disk_->create_file((dir_ / "heap.db").string());
    bpm_ = std::make_unique<buffer::BufferPoolManager>(16, disk_.get());

    std::vector<Column> cols;
    cols.emplace_back("id", DataType::INTEGER);
    cols.emplace_back("name", DataType::VARCHAR);
    schema_ = std::make_unique<ColumnSchema>(std::move(cols));
  }

  void TearDown() override {
    bpm_.reset();
    disk_.reset();
    std::filesystem::remove_all(dir_);
  }

  Tuple row(int32_t id, const std::string &name) {
    std::string name_copy = name;
    std::vector<Value> vals;
    vals.emplace_back(DataType::INTEGER, id);
    vals.emplace_back(DataType::VARCHAR, name_copy);
    return Tuple(schema_.get(), std::move(vals));
  }

  std::filesystem::path dir_;
  std::unique_ptr<storage::disk::DiskManager> disk_;
  std::unique_ptr<buffer::BufferPoolManager> bpm_;
  std::unique_ptr<ColumnSchema> schema_;
  FileId file_{0};
};

// ---- construction ----------------------------------------------------------

TEST_F(TableHeapTest, NewHeapHasSingleValidPage) {
  TableHeap heap(bpm_.get(), file_);
  EXPECT_NE(heap.get_first_page_id(), INVALID_PAGE_ID);
  EXPECT_EQ(heap.get_first_page_id(), heap.get_last_page_id());
  EXPECT_EQ(heap.get_file_id(), file_);
}

TEST_F(TableHeapTest, EmptyHeapScanIsImmediatelyEnd) {
  TableHeap heap(bpm_.get(), file_);
  EXPECT_TRUE(heap.begin() == heap.end());
}

// ---- insert / get round trip ----------------------------------------------

TEST_F(TableHeapTest, InsertThenGetRoundTrips) {
  TableHeap heap(bpm_.get(), file_);

  RecordId rid;
  ASSERT_TRUE(heap.insert_tuple(row(42, "alice"), &rid));
  EXPECT_EQ(rid.page_id, heap.get_first_page_id());
  EXPECT_EQ(rid.slot_num, 0u);

  Tuple fetched;
  ASSERT_TRUE(heap.get_tuple(rid, &fetched));
  EXPECT_EQ(fetched.value(schema_.get(), 0).GetAs<int32_t>(), 42);
  EXPECT_EQ(fetched.value(schema_.get(), 1).to_string(), "alice");
}

TEST_F(TableHeapTest, MultipleInsertsGetDistinctSlots) {
  TableHeap heap(bpm_.get(), file_);
  RecordId r0, r1, r2;
  ASSERT_TRUE(heap.insert_tuple(row(1, "a"), &r0));
  ASSERT_TRUE(heap.insert_tuple(row(2, "b"), &r1));
  ASSERT_TRUE(heap.insert_tuple(row(3, "c"), &r2));

  EXPECT_EQ(r0.slot_num, 0u);
  EXPECT_EQ(r1.slot_num, 1u);
  EXPECT_EQ(r2.slot_num, 2u);

  Tuple t;
  ASSERT_TRUE(heap.get_tuple(r2, &t));
  EXPECT_EQ(t.value(schema_.get(), 1).to_string(), "c");
}

// ---- page growth -----------------------------------------------------------

TEST_F(TableHeapTest, GrowsToNewPageWhenFull) {
  TableHeap heap(bpm_.get(), file_);
  const PageId first = heap.get_first_page_id();

  // Big payloads (~2 KiB) so only ~2 fit per 4 KiB page; 6 inserts => >=2 pages.
  const std::string big(2000, 'x');
  RecordId rid;
  for (int i = 0; i < 6; ++i) {
    ASSERT_TRUE(heap.insert_tuple(row(i, big), &rid));
  }

  EXPECT_NE(heap.get_last_page_id(), first) << "table should have spilled";
}

// ---- sequential scan -------------------------------------------------------

TEST_F(TableHeapTest, ScanVisitsEveryInsertedTupleAcrossPages) {
  TableHeap heap(bpm_.get(), file_);

  constexpr int kRows = 250; // enough small rows to span multiple pages
  for (int i = 0; i < kRows; ++i) {
    RecordId rid;
    ASSERT_TRUE(heap.insert_tuple(row(i, "user_" + std::to_string(i)), &rid));
  }

  int count = 0;
  int first_id = -1;
  for (auto it = heap.begin(); it != heap.end(); ++it) {
    const Tuple &t = it.get_tuple();
    if (count == 0) {
      first_id = t.value(schema_.get(), 0).GetAs<int32_t>();
    }
    ++count;
  }

  EXPECT_EQ(count, kRows);
  EXPECT_EQ(first_id, 0); // insertion order preserved from the first slot
}

} // namespace
} // namespace turtle::storage::table
