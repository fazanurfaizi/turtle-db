// -----------------------------------------------------------------------------
// Tier 1 (Foundation) — turtle::storage::page::SlottedPage
//
// Scope: the on-page record layout (header + forward-growing slot array +
// backward-growing tuple bytes) that every TableHeap page is. SlottedPage is a
// *view* over a Page* and stores no data of its own, so it is exercised over a
// real frame handed out by the BufferPoolManager (Page's data buffer is only
// valid when owned by the pool).
//
// insert_tuple/tuple are templated on any T exposing storage_size() /
// serialize_to() / deserialize_from(); we supply a minimal FakeTuple to test
// the page mechanics in isolation from the concrete storage::table::Tuple.
//
// Fixture: SlottedPageTest owns DiskManager + BPM + file and yields freshly
// init()'d pages. Free-space assertions are made *relatively* (delta per
// insert) so they validate the accounting without hardcoding sizeof(Header).
// -----------------------------------------------------------------------------
#include <cstring>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/page/page.hpp"
#include "turtle/storage/page/slotted_page.hpp"

namespace turtle::storage::page {
namespace {

// Minimal record type satisfying the SlottedPage template contract.
struct FakeTuple {
  std::string payload;

  FakeTuple() = default;
  explicit FakeTuple(std::string p) : payload(std::move(p)) {}

  uint32_t storage_size() const {
    return static_cast<uint32_t>(payload.size());
  }
  void serialize_to(char *dst) const {
    std::memcpy(dst, payload.data(), payload.size());
  }
  void deserialize_from(const char *src, uint32_t len) {
    payload.assign(src, len);
  }
};

// sizeof(Slot) is private to SlottedPage; the layout is {offset_, length_},
// i.e. two uint32_t. Insert cost is storage_size() + this.
constexpr uint32_t kSlotSize = 2 * sizeof(uint32_t); // 8

class SlottedPageTest : public ::testing::Test {
protected:
  void SetUp() override {
    dir_ = std::filesystem::path("test_db") / "slotted_page_gtest";
    std::filesystem::remove_all(dir_);
    std::filesystem::create_directories(dir_);

    disk_ = std::make_unique<storage::disk::DiskManager>();
    file_ = disk_->create_file((dir_ / "sp.db").string());
    bpm_ = std::make_unique<buffer::BufferPoolManager>(4, disk_.get());
  }

  void TearDown() override {
    bpm_.reset();
    disk_.reset();
    std::filesystem::remove_all(dir_);
  }

  // Allocate a fresh page and wrap+init a SlottedPage over it. The page stays
  // pinned for the duration of the test; the pool frees frames in its dtor.
  Page *fresh_page(PageId *out_pid) {
    Page *p = bpm_->new_page(file_, out_pid);
    return p;
  }

  std::filesystem::path dir_;
  std::unique_ptr<storage::disk::DiskManager> disk_;
  std::unique_ptr<buffer::BufferPoolManager> bpm_;
  FileId file_{0};
};

// ---- init ------------------------------------------------------------------

TEST_F(SlottedPageTest, InitEstablishesEmptyPage) {
  PageId pid = INVALID_PAGE_ID;
  Page *p = fresh_page(&pid);
  SlottedPage sp(p);
  sp.init(pid);

  EXPECT_EQ(sp.page_id(), pid);
  EXPECT_EQ(sp.tuple_count(), 0u);
  EXPECT_EQ(sp.slot_count(), 0u);
  EXPECT_GT(sp.free_space_remaining(), 0u);
  EXPECT_LT(sp.free_space_remaining(), PAGE_SIZE); // header consumes some
  EXPECT_EQ(sp.get_next_page_id(), INVALID_PAGE_ID);
}

// ---- insert / read round trip ---------------------------------------------

TEST_F(SlottedPageTest, InsertThenReadRoundTrips) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);

  FakeTuple in{"hello slotted page"};
  RecordId rid;
  ASSERT_TRUE(sp.insert_tuple(in, &rid));

  EXPECT_EQ(rid.page_id, pid);
  EXPECT_EQ(rid.slot_num, 0u);
  EXPECT_EQ(sp.tuple_count(), 1u);
  EXPECT_EQ(sp.slot_count(), 1u);
  EXPECT_TRUE(sp.is_slot_occupied(0));

  FakeTuple out;
  sp.tuple(rid, &out);
  EXPECT_EQ(out.payload, in.payload);
}

TEST_F(SlottedPageTest, MultipleInsertsGetSequentialSlots) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);

  for (uint32_t i = 0; i < 3; ++i) {
    FakeTuple t{"row-" + std::to_string(i)};
    RecordId rid;
    ASSERT_TRUE(sp.insert_tuple(t, &rid));
    EXPECT_EQ(rid.slot_num, i);
  }
  EXPECT_EQ(sp.tuple_count(), 3u);
  EXPECT_EQ(sp.slot_count(), 3u);

  // Read the middle row back to confirm slots are independent.
  FakeTuple out;
  sp.tuple(RecordId{pid, 1u}, &out);
  EXPECT_EQ(out.payload, "row-1");
}

// ---- free-space accounting (relative, layout-agnostic) --------------------

TEST_F(SlottedPageTest, EachInsertConsumesPayloadPlusSlot) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);

  FakeTuple t{std::string(100, 'x')}; // 100-byte payload
  const uint32_t before = sp.free_space_remaining();
  RecordId rid;
  ASSERT_TRUE(sp.insert_tuple(t, &rid));
  const uint32_t after = sp.free_space_remaining();

  EXPECT_EQ(before - after, t.storage_size() + kSlotSize);
}

// ---- capacity / overflow ---------------------------------------------------

TEST_F(SlottedPageTest, InsertFailsWhenPageIsFull) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);

  // ~2000-byte payloads: two fit on a 4 KiB page, the third cannot.
  FakeTuple big{std::string(2000, 'z')};
  RecordId rid;
  ASSERT_TRUE(sp.insert_tuple(big, &rid));
  ASSERT_TRUE(sp.insert_tuple(big, &rid));
  EXPECT_FALSE(sp.insert_tuple(big, &rid)); // no room

  // A rejected insert must not mutate counts.
  EXPECT_EQ(sp.tuple_count(), 2u);
  EXPECT_EQ(sp.slot_count(), 2u);
}

// ---- delete (soft) ---------------------------------------------------------

TEST_F(SlottedPageTest, DeleteMarksSlotEmptyButKeepsSlotCount) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);

  RecordId r0, r1;
  ASSERT_TRUE(sp.insert_tuple(FakeTuple{"keep"}, &r0));
  ASSERT_TRUE(sp.insert_tuple(FakeTuple{"drop"}, &r1));

  sp.delete_tuple(r1);

  EXPECT_EQ(sp.tuple_count(), 1u);       // live tuples decremented
  EXPECT_EQ(sp.slot_count(), 2u);        // slot array not compacted
  EXPECT_FALSE(sp.is_slot_occupied(1));  // slot 1 now empty
  EXPECT_TRUE(sp.is_slot_occupied(0));   // slot 0 still live
}

TEST_F(SlottedPageTest, ReadingDeletedTupleThrows) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);

  RecordId rid;
  ASSERT_TRUE(sp.insert_tuple(FakeTuple{"gone"}, &rid));
  sp.delete_tuple(rid);

  FakeTuple out;
  EXPECT_THROW(sp.tuple(rid, &out), std::runtime_error);
}

// ---- RecordId validation (error handling) ----------------------------------

TEST_F(SlottedPageTest, ReadWithForeignPageIdThrows) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);
  RecordId rid;
  ASSERT_TRUE(sp.insert_tuple(FakeTuple{"data"}, &rid));

  FakeTuple out;
  RecordId foreign{pid + 100u, 0u}; // page id does not match this page
  EXPECT_THROW(sp.tuple(foreign, &out), std::runtime_error);
}

TEST_F(SlottedPageTest, ReadWithOutOfRangeSlotThrows) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);
  ASSERT_TRUE(sp.insert_tuple(FakeTuple{"only"}, nullptr));

  FakeTuple out;
  RecordId beyond{pid, 5u}; // only slot 0 exists
  EXPECT_THROW(sp.tuple(beyond, &out), std::runtime_error);
}

TEST_F(SlottedPageTest, IsSlotOccupiedFalseForOutOfRange) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);
  EXPECT_FALSE(sp.is_slot_occupied(0));   // nothing inserted yet
  EXPECT_FALSE(sp.is_slot_occupied(999)); // wildly out of range
}

// ---- page chaining ---------------------------------------------------------

TEST_F(SlottedPageTest, NextPageIdRoundTrips) {
  PageId pid = INVALID_PAGE_ID;
  SlottedPage sp(fresh_page(&pid));
  sp.init(pid);
  EXPECT_EQ(sp.get_next_page_id(), INVALID_PAGE_ID);

  PageId next = 77u;
  sp.set_next_page_id(next);
  EXPECT_EQ(sp.get_next_page_id(), 77u);
}

} // namespace
} // namespace turtle::storage::page
