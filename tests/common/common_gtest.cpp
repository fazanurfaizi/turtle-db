// -----------------------------------------------------------------------------
// Tier 1 (Foundation) — turtle::common
//
// Scope: the zero-dependency value types every other layer is built on:
//   - turtle::RecordId  (tuple address: {PageId page_id, uint32_t slot_num})
//   - turtle::config    (PageId/FrameId aliases, sentinels, PAGE_SIZE)
//
// These have no I/O and no heap ownership, so no fixture is required; the tests
// are pure value-semantics / invariant checks. Getting these wrong silently
// corrupts every higher tier (a RecordId is how the whole engine names a row).
// -----------------------------------------------------------------------------
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"

namespace turtle {
namespace {

// ---- config.hpp invariants -------------------------------------------------

TEST(ConfigTest, PageSizeIsFourKiB) {
  // The slotted-page layout and the disk manager both hard-assume 4 KiB pages.
  EXPECT_EQ(PAGE_SIZE, 4096u);
}

TEST(ConfigTest, SentinelsAreAllOnes) {
  // Sentinels are defined as static_cast<T>(-1); verify they land on the
  // max value of their unsigned alias (this is what "invalid" compares against
  // throughout the buffer pool and table heap).
  EXPECT_EQ(INVALID_PAGE_ID, std::numeric_limits<PageId>::max());
  EXPECT_EQ(INVALID_FRAME_ID, std::numeric_limits<FrameId>::max());
  EXPECT_EQ(INVALID_TXN_ID, std::numeric_limits<TxnId>::max());
}

TEST(ConfigTest, InvalidPageIdIsDistinctFromFirstRealPage) {
  // Page ids are handed out starting at 0; the sentinel must never collide.
  EXPECT_NE(INVALID_PAGE_ID, static_cast<PageId>(0));
}

// ---- RecordId: construction & defaults ------------------------------------

TEST(RecordIdTest, DefaultConstructsToInvalidPageZeroSlot) {
  RecordId rid;
  EXPECT_EQ(rid.page_id, INVALID_PAGE_ID);
  EXPECT_EQ(rid.slot_num, 0u);
}

TEST(RecordIdTest, ValueConstructorStoresFields) {
  RecordId rid{7u, 42u};
  EXPECT_EQ(rid.page_id, 7u);
  EXPECT_EQ(rid.slot_num, 42u);
}

// ---- RecordId: equality (happy path + every mismatch axis) -----------------

TEST(RecordIdTest, EqualityIsFieldwise) {
  RecordId a{3u, 9u};
  RecordId b{3u, 9u};
  EXPECT_TRUE(a == b);
}

TEST(RecordIdTest, DiffersWhenPageIdDiffers) {
  RecordId a{3u, 9u};
  RecordId b{4u, 9u};
  EXPECT_FALSE(a == b);
}

TEST(RecordIdTest, DiffersWhenSlotDiffers) {
  RecordId a{3u, 9u};
  RecordId b{3u, 10u};
  EXPECT_FALSE(a == b);
}

// ---- RecordId: edge values -------------------------------------------------

TEST(RecordIdTest, HandlesMaxFieldValues) {
  RecordId rid{std::numeric_limits<PageId>::max(),
               std::numeric_limits<uint32_t>::max()};
  EXPECT_EQ(rid.page_id, std::numeric_limits<PageId>::max());
  EXPECT_EQ(rid.slot_num, std::numeric_limits<uint32_t>::max());
  EXPECT_TRUE((rid == RecordId{std::numeric_limits<PageId>::max(),
                               std::numeric_limits<uint32_t>::max()}));
}

// ---- RecordId: to_string ---------------------------------------------------

TEST(RecordIdTest, ToStringRendersBothFields) {
  RecordId rid{12u, 5u};
  EXPECT_EQ(rid.to_string(), "RecordId(12, 5)");
}

TEST(RecordIdTest, ToStringOnDefaultShowsInvalidPageId) {
  // INVALID_PAGE_ID is 2^32-1; make sure to_string surfaces the real number
  // rather than -1, since the field is unsigned.
  RecordId rid;
  EXPECT_EQ(rid.to_string(), "RecordId(4294967295, 0)");
}

} // namespace
} // namespace turtle
