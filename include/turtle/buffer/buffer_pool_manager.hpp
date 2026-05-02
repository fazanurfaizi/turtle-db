#pragma once
#include <atomic>
#include <cstddef>
#include <list>
#include <mutex>

#include "turtle/common/pagekey.hpp"
#include "lru_replacer.hpp"
#include "turtle/storage/disk_manager.hpp"

namespace turtle::storage {
class Page;
}

namespace turtle::buffer {

class BufferPoolManager {
public:
  BufferPoolManager(size_t pool_size, storage::DiskManager *disk_manager);
  ~BufferPoolManager();

  // Prevent copying
  BufferPoolManager(const BufferPoolManager &) = delete;
  BufferPoolManager &operator=(const BufferPoolManager &) = delete;

  storage::Page *fetch_page(FileId file_id, PageId page_id);
  bool unpin_page(FileId file_id, PageId page_id, bool is_dirty);
  bool flush_page(FileId file_id, PageId page_id);
  void flush_all_pages();
  storage::Page *new_page(FileId file_id, PageId *page_id);
  bool delete_page(FileId file_id, PageId page_id);

private:
  size_t pool_size_;
  storage::Page *pages_;
  char *frame_data_;
  std::unordered_map<PageKey, FrameId, PageKeyHash> page_table_;
  std::list<FrameId> free_list_;
  LRUReplacer replacer_;
  storage::DiskManager *disk_manager_;
  std::mutex latch_;
  std::atomic<PageId> next_page_id_{0};
};

} // namespace Turtle::Buffer

