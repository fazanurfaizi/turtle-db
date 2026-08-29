// -----------------------------------------------------------------------------
// Tier 1 (Foundation) — turtle::buffer::BufferPoolManager (+ storage::page::Page
// accessors, which are only observable through the pool since Page's metadata
// is friend-mutable by the BPM alone).
//
// This is the single most load-bearing component in the engine: every table
// scan and every executor pulls pages through here. The tests exercise the full
// pin lifecycle, eviction + dirty write-back, pool exhaustion, and delete rules.
//
// Fixture: BufferPoolManagerTest owns a DiskManager + one backing file, plus a
// pool of a configurable (small) size so exhaustion/eviction paths are cheap to
// drive. Everything is torn down and the scratch dir wiped after each test, so
// pinned pages never leak across tests (the BPM frees its frame arrays in its
// dtor regardless of pin state).
// -----------------------------------------------------------------------------
#include <array>
#include <cstring>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/page/page.hpp"

namespace turtle::buffer {
namespace {

class BufferPoolManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    dir_ = std::filesystem::path("test_db") / "bpm_gtest";
    std::filesystem::remove_all(dir_);
    std::filesystem::create_directories(dir_);

    disk_ = std::make_unique<storage::disk::DiskManager>();
    file_ = disk_->create_file((dir_ / "bpm.db").string());
  }

  void TearDown() override {
    bpm_.reset();  // flush + free frame arrays first
    disk_.reset(); // then close files
    std::filesystem::remove_all(dir_);
  }

  // (Re)create a pool of a given size for the test at hand.
  BufferPoolManager *make_pool(size_t pool_size) {
    bpm_ = std::make_unique<BufferPoolManager>(pool_size, disk_.get());
    return bpm_.get();
  }

  static void fill(char *dst, unsigned char seed) {
    for (uint32_t i = 0; i < PAGE_SIZE; ++i) {
      dst[i] = static_cast<char>((seed + i) & 0xFF);
    }
  }

  static bool matches(const char *data, unsigned char seed) {
    for (uint32_t i = 0; i < PAGE_SIZE; ++i) {
      if (data[i] != static_cast<char>((seed + i) & 0xFF)) {
        return false;
      }
    }
    return true;
  }

  std::filesystem::path dir_;
  std::unique_ptr<storage::disk::DiskManager> disk_;
  std::unique_ptr<BufferPoolManager> bpm_;
  FileId file_{0};
};

// ---- new_page: allocation + pinning ---------------------------------------

TEST_F(BufferPoolManagerTest, NewPageReturnsPinnedPageWithIdStartingAtZero) {
  auto *bpm = make_pool(4);

  PageId pid = INVALID_PAGE_ID;
  auto *page = bpm->new_page(file_, &pid);

  ASSERT_NE(page, nullptr);
  EXPECT_EQ(pid, 0u);              // global next_page_id_ starts at 0
  EXPECT_EQ(page->page_id(), 0u);
  EXPECT_EQ(page->file_id(), file_);
  EXPECT_EQ(page->pin_count(), 1); // freshly created pages arrive pinned
  EXPECT_FALSE(page->is_dirty());
}

TEST_F(BufferPoolManagerTest, NewPageAssignsMonotonicIds) {
  auto *bpm = make_pool(4);
  PageId a = INVALID_PAGE_ID, b = INVALID_PAGE_ID, c = INVALID_PAGE_ID;
  bpm->new_page(file_, &a);
  bpm->new_page(file_, &b);
  bpm->new_page(file_, &c);
  EXPECT_EQ(a, 0u);
  EXPECT_EQ(b, 1u);
  EXPECT_EQ(c, 2u);
}

// ---- fetch: cache hit re-pins & returns the same frame ---------------------

TEST_F(BufferPoolManagerTest, FetchCachedPageIncrementsPinAndReturnsSameFrame) {
  auto *bpm = make_pool(4);
  PageId pid = INVALID_PAGE_ID;
  auto *created = bpm->new_page(file_, &pid); // pin_count == 1

  auto *fetched = bpm->fetch_page(file_, pid); // pin_count == 2
  EXPECT_EQ(created, fetched);
  EXPECT_EQ(fetched->pin_count(), 2);
}

// ---- dirty data survives a round trip through disk -------------------------

TEST_F(BufferPoolManagerTest, DirtyPageIsWrittenBackOnEvictionAndReadBack) {
  auto *bpm = make_pool(1); // size 1 forces eviction on the next new_page

  PageId pid0 = INVALID_PAGE_ID;
  auto *p0 = bpm->new_page(file_, &pid0);
  ASSERT_NE(p0, nullptr);
  fill(p0->data(), 0x5A);
  ASSERT_TRUE(bpm->unpin_page(file_, pid0, /*is_dirty=*/true));

  // Allocating a second page must evict p0 and flush its dirty contents.
  PageId pid1 = INVALID_PAGE_ID;
  auto *p1 = bpm->new_page(file_, &pid1);
  ASSERT_NE(p1, nullptr);
  ASSERT_TRUE(bpm->unpin_page(file_, pid1, /*is_dirty=*/false));

  // Fetch p0 back: its bytes must have persisted through the write-back.
  auto *reloaded = bpm->fetch_page(file_, pid0);
  ASSERT_NE(reloaded, nullptr);
  EXPECT_TRUE(matches(reloaded->data(), 0x5A));
  EXPECT_TRUE(bpm->unpin_page(file_, pid0, /*is_dirty=*/false));
}

// ---- pool exhaustion -------------------------------------------------------

TEST_F(BufferPoolManagerTest, ReturnsNullptrWhenAllFramesArePinned) {
  const size_t kPool = 3;
  auto *bpm = make_pool(kPool);

  for (size_t i = 0; i < kPool; ++i) {
    PageId pid = INVALID_PAGE_ID;
    ASSERT_NE(bpm->new_page(file_, &pid), nullptr) << "alloc " << i;
  }

  // Every frame is pinned; there is no victim to reclaim.
  PageId overflow = INVALID_PAGE_ID;
  EXPECT_EQ(bpm->new_page(file_, &overflow), nullptr);
}

TEST_F(BufferPoolManagerTest, UnpinningFreesAFrameForReuse) {
  const size_t kPool = 2;
  auto *bpm = make_pool(kPool);

  PageId a = INVALID_PAGE_ID, b = INVALID_PAGE_ID;
  bpm->new_page(file_, &a);
  bpm->new_page(file_, &b);

  PageId c = INVALID_PAGE_ID;
  EXPECT_EQ(bpm->new_page(file_, &c), nullptr); // full

  ASSERT_TRUE(bpm->unpin_page(file_, a, false)); // release one victim
  EXPECT_NE(bpm->new_page(file_, &c), nullptr);  // now succeeds
}

// ---- unpin error handling --------------------------------------------------

TEST_F(BufferPoolManagerTest, UnpinUnknownPageReturnsFalse) {
  auto *bpm = make_pool(4);
  EXPECT_FALSE(bpm->unpin_page(file_, /*page_id=*/999u, false));
}

TEST_F(BufferPoolManagerTest, DoubleUnpinReturnsFalseOnSecondCall) {
  auto *bpm = make_pool(4);
  PageId pid = INVALID_PAGE_ID;
  bpm->new_page(file_, &pid); // pin_count == 1

  EXPECT_TRUE(bpm->unpin_page(file_, pid, false));  // -> 0
  EXPECT_FALSE(bpm->unpin_page(file_, pid, false)); // already 0
}

// ---- delete_page rules -----------------------------------------------------

TEST_F(BufferPoolManagerTest, DeletePinnedPageIsRefused) {
  auto *bpm = make_pool(4);
  PageId pid = INVALID_PAGE_ID;
  bpm->new_page(file_, &pid); // still pinned
  EXPECT_FALSE(bpm->delete_page(file_, pid));
}

TEST_F(BufferPoolManagerTest, DeleteUnpinnedPageSucceeds) {
  auto *bpm = make_pool(4);
  PageId pid = INVALID_PAGE_ID;
  bpm->new_page(file_, &pid);
  ASSERT_TRUE(bpm->unpin_page(file_, pid, false));
  EXPECT_TRUE(bpm->delete_page(file_, pid));
}

TEST_F(BufferPoolManagerTest, DeleteUncachedPageIsANoOpSuccess) {
  auto *bpm = make_pool(4);
  // Page 12345 was never brought into the pool; delete is a vacuous success.
  EXPECT_TRUE(bpm->delete_page(file_, 12345u));
}

// ---- flush -----------------------------------------------------------------

TEST_F(BufferPoolManagerTest, FlushUnknownPageReturnsFalse) {
  auto *bpm = make_pool(4);
  EXPECT_FALSE(bpm->flush_page(file_, 4242u));
}

TEST_F(BufferPoolManagerTest, FlushClearsDirtyFlag) {
  auto *bpm = make_pool(4);
  PageId pid = INVALID_PAGE_ID;
  auto *p = bpm->new_page(file_, &pid);
  fill(p->data(), 0x0C);
  ASSERT_TRUE(bpm->unpin_page(file_, pid, /*is_dirty=*/true));
  EXPECT_TRUE(p->is_dirty());

  EXPECT_TRUE(bpm->flush_page(file_, pid));
  EXPECT_FALSE(p->is_dirty());
}

} // namespace
} // namespace turtle::buffer
