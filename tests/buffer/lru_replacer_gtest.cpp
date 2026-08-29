// -----------------------------------------------------------------------------
// Tier 1 (Foundation) — turtle::buffer::LRUReplacer
//
// Scope: the eviction policy the buffer pool consults when every frame is
// taken. Pure in-memory data structure with no I/O, so it is tested directly
// (no fixture needed beyond a fresh instance per test).
//
// Semantics under test (from the implementation):
//   unpin(f)   -> makes frame f a victim candidate (push_front); idempotent
//   pin(f)     -> removes f from the candidate set;               idempotent
//   victim(&f) -> pops the *least recently unpinned* frame (list back)
//   size()     -> number of current victim candidates
// The net effect is classic LRU: the frame unpinned earliest is evicted first.
// -----------------------------------------------------------------------------
#include <gtest/gtest.h>

#include "turtle/buffer/lru_replacer.hpp"
#include "turtle/common/config.hpp"

namespace turtle::buffer {
namespace {

// ---- empty / trivial -------------------------------------------------------

TEST(LRUReplacerTest, StartsEmpty) {
  LRUReplacer r(4);
  EXPECT_EQ(r.size(), 0u);
}

TEST(LRUReplacerTest, VictimOnEmptyReturnsFalseAndLeavesOutParamUntouched) {
  LRUReplacer r(4);
  FrameId out = 123u; // sentinel we expect to remain untouched
  EXPECT_FALSE(r.victim(&out));
  EXPECT_EQ(out, 123u);
}

// ---- basic candidate accounting -------------------------------------------

TEST(LRUReplacerTest, UnpinAddsCandidate) {
  LRUReplacer r(4);
  r.unpin(0);
  r.unpin(1);
  EXPECT_EQ(r.size(), 2u);
}

TEST(LRUReplacerTest, UnpinIsIdempotentForSameFrame) {
  LRUReplacer r(4);
  r.unpin(2);
  r.unpin(2); // must not create a duplicate candidate
  EXPECT_EQ(r.size(), 1u);
}

TEST(LRUReplacerTest, PinRemovesCandidate) {
  LRUReplacer r(4);
  r.unpin(0);
  r.unpin(1);
  r.pin(0);
  EXPECT_EQ(r.size(), 1u);
}

TEST(LRUReplacerTest, PinOnUnknownFrameIsNoOp) {
  LRUReplacer r(4);
  r.unpin(1);
  r.pin(3); // 3 was never a candidate
  EXPECT_EQ(r.size(), 1u);
}

// ---- eviction order (the actual LRU contract) ------------------------------

TEST(LRUReplacerTest, EvictsInLeastRecentlyUnpinnedOrder) {
  LRUReplacer r(4);
  r.unpin(0); // unpinned first  -> oldest -> evicted first
  r.unpin(1);
  r.unpin(2); // unpinned last   -> newest -> evicted last

  FrameId v = INVALID_FRAME_ID;
  ASSERT_TRUE(r.victim(&v));
  EXPECT_EQ(v, 0u);
  ASSERT_TRUE(r.victim(&v));
  EXPECT_EQ(v, 1u);
  ASSERT_TRUE(r.victim(&v));
  EXPECT_EQ(v, 2u);

  EXPECT_EQ(r.size(), 0u);
  EXPECT_FALSE(r.victim(&v)); // now exhausted
}

TEST(LRUReplacerTest, PinnedFrameIsSkippedByVictim) {
  LRUReplacer r(4);
  r.unpin(0);
  r.unpin(1);
  r.pin(0); // frame 0 is back in use; must not be chosen

  FrameId v = INVALID_FRAME_ID;
  ASSERT_TRUE(r.victim(&v));
  EXPECT_EQ(v, 1u);
  EXPECT_FALSE(r.victim(&v)); // only frame 1 was a candidate
}

TEST(LRUReplacerTest, ReUnpinningAfterVictimMakesFrameACandidateAgain) {
  LRUReplacer r(4);
  r.unpin(0);
  FrameId v = INVALID_FRAME_ID;
  ASSERT_TRUE(r.victim(&v));
  EXPECT_EQ(v, 0u);

  r.unpin(0); // frame recycled and unpinned again
  EXPECT_EQ(r.size(), 1u);
  ASSERT_TRUE(r.victim(&v));
  EXPECT_EQ(v, 0u);
}

} // namespace
} // namespace turtle::buffer
