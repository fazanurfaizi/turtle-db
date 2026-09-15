#pragma once
#include <atomic>
#include <cstddef>
#include <list>
#include <mutex>
#include <optional>

#include "turtle/buffer/arc_replacer.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/pagekey.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/page/read_page_guard.hpp"
#include "turtle/storage/page/write_page_guard.hpp"

namespace turtle::storage::page {
class Page;
}

namespace turtle::buffer {

class BufferPoolManager {
public:
  BufferPoolManager(size_t pool_size, storage::disk::DiskManager *disk_manager);
  ~BufferPoolManager();

  // Prevent copying
  BufferPoolManager(const BufferPoolManager &) = delete;
  BufferPoolManager &operator=(const BufferPoolManager &) = delete;

  auto read_page(FileId file_id, PageId page_id,
                 AccessType access_type = AccessType::Unknown)
      -> std::optional<storage::page::ReadPageGuard>;

  auto write_page(FileId file_id, PageId page_id,
                  AccessType access_type = AccessType::Unknown)
      -> std::optional<storage::page::WritePageGuard>;

  bool unpin_page(FileId file_id, PageId page_id, bool is_dirty);
  bool flush_page(FileId file_id, PageId page_id);
  void flush_all_pages();
  storage::page::Page *new_page(FileId file_id, PageId *page_id);
  bool delete_page(FileId file_id, PageId page_id);

private:
  /** @brief The numver of pool in the buffer pool*/
  size_t pool_size_;

  std::shared_ptr<std::mutex> bpm_latch_;

  /** @brief The pages that in this buffer pool manager */
  storage::page::Page *pages_;

  char *frame_data_;

  std::unordered_map<PageKey, FrameId, PageKeyHash> page_table_;

  std::list<FrameId> free_frames_;

  ArcReplacer replacer_;

  storage::disk::DiskManager *disk_manager_;

  std::atomic<PageId> next_page_id_{0};

  storage::page::Page *fetch_page(FileId file_id, PageId page_id);
};

} // namespace turtle::buffer
