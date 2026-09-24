#pragma once

#include <cstring>
#include <shared_mutex>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"

namespace turtle::storage::page {

class ReadPageGuard;
class WritePageGuard;

class Page {
  friend class turtle::buffer::BufferPoolManager;
  friend class ReadPageGuard;
  friend class WritePageGuard;

public:
  Page() = default;

  auto data() const -> const char * { return this->data_; }
  auto data_mut() -> char * { return this->data_; }

  PageId page_id() const { return this->page_id_; }
  FileId file_id() const { return this->file_id_; }
  int pin_count() const { return this->pin_count_; }
  bool is_dirty() const { return this->is_dirty_; }

  void reset_memory() { std::memset(this->data_, 0, PAGE_SIZE); }

private:
  std::shared_ptr<std::shared_mutex> rw_latch_;

  char *data_{nullptr};

  FileId file_id_{0};

  PageId page_id_{INVALID_PAGE_ID};

  int pin_count_{0};

  bool is_dirty_{false};
};

} // namespace turtle::storage::page
