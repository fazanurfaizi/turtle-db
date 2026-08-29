// -----------------------------------------------------------------------------
// Tier 1 (Foundation) — turtle::storage::disk::DiskManager
//
// Scope: the lowest-level I/O primitive. Every page the buffer pool caches is
// ultimately read/written through here, so its byte-exactness and its file-id
// bookkeeping are load-bearing for the entire engine.
//
// Fixture: DiskManagerTest owns a unique scratch directory under test_db/ that
// is wiped in SetUp and removed in TearDown, so no test can observe leftover
// files from another (matches the project convention of a disposable test_db/).
//
// Note on API surface tested: create_file() -> write_page() -> read_page() is
// the real hot path (BufferPoolManager only ever uses ids returned by
// create_file). open_file() is deliberately NOT exercised for read-back: its
// current implementation mints a fresh id from path_to_file_id_.size()+1 and
// never registers the fstream in files_, so reads through a freshly-opened
// (never-created) file throw. We only assert the one open_file guarantee that
// holds: an already-created path resolves back to its original id.
// -----------------------------------------------------------------------------
#include <array>
#include <cstring>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "turtle/common/config.hpp"
#include "turtle/storage/disk/disk_manager.hpp"

namespace turtle::storage::disk {
namespace {

class DiskManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    dir_ = std::filesystem::path("test_db") / "disk_manager_gtest";
    std::filesystem::remove_all(dir_);
    std::filesystem::create_directories(dir_);
    dm_ = std::make_unique<DiskManager>();
  }

  void TearDown() override {
    // Destroy the manager first so all fstreams flush/close, then wipe files.
    dm_.reset();
    std::filesystem::remove_all(dir_);
  }

  std::string path(const std::string &name) const {
    return (dir_ / name).string();
  }

  // A PAGE_SIZE buffer filled with a recognizable, position-dependent pattern.
  static std::array<char, PAGE_SIZE> make_pattern(unsigned char seed) {
    std::array<char, PAGE_SIZE> buf{};
    for (uint32_t i = 0; i < PAGE_SIZE; ++i) {
      buf[i] = static_cast<char>((seed + i) & 0xFF);
    }
    return buf;
  }

  std::filesystem::path dir_;
  std::unique_ptr<DiskManager> dm_;
};

// ---- happy path ------------------------------------------------------------

TEST_F(DiskManagerTest, WriteThenReadRoundTripsPageZero) {
  FileId fid = dm_->create_file(path("t0.db"));
  auto written = make_pattern(0x11);

  dm_->write_page(fid, 0, written.data());

  std::array<char, PAGE_SIZE> read_back{};
  dm_->read_page(fid, 0, read_back.data());

  EXPECT_EQ(std::memcmp(written.data(), read_back.data(), PAGE_SIZE), 0);
}

TEST_F(DiskManagerTest, DistinctPagesAreIndependent) {
  FileId fid = dm_->create_file(path("t1.db"));
  auto p0 = make_pattern(0x01);
  auto p5 = make_pattern(0xA0);

  // Writing page 5 first forces the file to grow past page 0's slot.
  dm_->write_page(fid, 5, p5.data());
  dm_->write_page(fid, 0, p0.data());

  std::array<char, PAGE_SIZE> r0{};
  std::array<char, PAGE_SIZE> r5{};
  dm_->read_page(fid, 0, r0.data());
  dm_->read_page(fid, 5, r5.data());

  EXPECT_EQ(std::memcmp(p0.data(), r0.data(), PAGE_SIZE), 0);
  EXPECT_EQ(std::memcmp(p5.data(), r5.data(), PAGE_SIZE), 0);
}

TEST_F(DiskManagerTest, OverwriteReplacesContents) {
  FileId fid = dm_->create_file(path("t2.db"));
  auto first = make_pattern(0x22);
  auto second = make_pattern(0x77);

  dm_->write_page(fid, 3, first.data());
  dm_->write_page(fid, 3, second.data());

  std::array<char, PAGE_SIZE> r{};
  dm_->read_page(fid, 3, r.data());
  EXPECT_EQ(std::memcmp(second.data(), r.data(), PAGE_SIZE), 0);
}

// ---- file-id bookkeeping ---------------------------------------------------

TEST_F(DiskManagerTest, CreateFileHandsOutIdsStartingAtZero) {
  FileId a = dm_->create_file(path("a.db"));
  FileId b = dm_->create_file(path("b.db"));
  FileId c = dm_->create_file(path("c.db"));
  EXPECT_EQ(a, 0u);
  EXPECT_EQ(b, 1u);
  EXPECT_EQ(c, 2u);
}

TEST_F(DiskManagerTest, OpenFileResolvesAnAlreadyCreatedPathToSameId) {
  FileId created = dm_->create_file(path("shared.db"));
  FileId opened = dm_->open_file(path("shared.db"));
  EXPECT_EQ(created, opened);
}

// ---- edge cases ------------------------------------------------------------

TEST_F(DiskManagerTest, ReadingNeverWrittenPageReturnsZeroFilled) {
  FileId fid = dm_->create_file(path("sparse.db"));

  // Prefill the destination with non-zero bytes to prove read_page clears them.
  std::array<char, PAGE_SIZE> buf{};
  buf.fill(static_cast<char>(0xEE));

  dm_->read_page(fid, 9, buf.data());

  std::array<char, PAGE_SIZE> zeros{};
  zeros.fill('\0');
  EXPECT_EQ(std::memcmp(buf.data(), zeros.data(), PAGE_SIZE), 0);
}

TEST_F(DiskManagerTest, ReadPageWithBogusFileIdThrows) {
  const FileId bogus = 999u;
  std::array<char, PAGE_SIZE> buf{};
  EXPECT_THROW(dm_->read_page(bogus, 0, buf.data()), std::runtime_error);
}

TEST_F(DiskManagerTest, WritePageWithBogusFileIdThrows) {
  const FileId bogus = 999u;
  auto buf = make_pattern(0x33);
  EXPECT_THROW(dm_->write_page(bogus, 0, buf.data()), std::runtime_error);
}

} // namespace
} // namespace turtle::storage::disk
