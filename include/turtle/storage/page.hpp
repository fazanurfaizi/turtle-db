#pragma once

#include <cstring>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"

namespace turtle::storage {

class Page {
public:
  Page() = default;

  char *data() { return this->data_; }
  PageId page_id() const { return this->page_id_; }
  FileId file_id() const { return this->file_id_; }
  int pin_count() const { return this->pin_count_; }
  bool is_dirty() const { return this->is_dirty_; }

  void reset_memory() {
    std::memset(this->data_, 0, PAGE_SIZE);
  }

private:
  friend class turtle::buffer::BufferPoolManager;

  char *data_{nullptr};
  FileId file_id_{0};
  PageId page_id_{INVALID_PAGE_ID};
  int pin_count_{0};
  bool is_dirty_{false};
};

} // namespace Turtle::Storage

