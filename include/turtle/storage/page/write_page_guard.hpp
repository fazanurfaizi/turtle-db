#pragma once

#include "turtle/common/config.hpp"

namespace turtle::buffer {
class BufferPoolManager;
}

namespace turtle::storage::page {
class Page;

/**
 * RAII handle for a *write-pinned* page. Same lifecycle as ReadPageGuard, but
 * unpins with is_dirty=true (any guard handed out for writing marks the frame
 * dirty) and hands back mutable bytes. Concurrency: exclusive lock.
 */
class WritePageGuard {
public:
  WritePageGuard() = default;
  WritePageGuard(buffer::BufferPoolManager *bpm, Page *page);
  ~WritePageGuard();

  WritePageGuard(const WritePageGuard &) = delete;
  auto operator=(const WritePageGuard &) -> WritePageGuard & = delete;

  WritePageGuard(WritePageGuard &&other) noexcept;
  auto operator=(WritePageGuard &&other) noexcept -> WritePageGuard &;

  auto is_valid() const -> bool { return this->page_ != nullptr; }

  auto page_id() const -> PageId;

  auto page() -> Page *;

  auto data() -> char *;

  template <typename T> auto as_mut() -> T * {
    return reinterpret_cast<T *>(this->data());
  }

  void flush();

  void drop();

private:
  buffer::BufferPoolManager *bpm_{nullptr};
  Page *page_{nullptr};
};

} // namespace turtle::storage::page
