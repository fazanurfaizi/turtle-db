#pragma once

#include "turtle/common/config.hpp"

namespace turtle::buffer {
class BufferPoolManager;
}

namespace turtle::storage::page {
class Page;

/**
 * RAII handle for a *read-pinned* page. Pins on construction (via the BPM),
 * unpins on destruction. Move-only: ownership of the pin transfers, so exactly
 * one guard ever releases it. When concurrency lands, the ctor/dtor also take
 * and drop the frame's shared_mutex in *shared* mode (see notes below).
 */
class ReadPageGuard {
public:
  ReadPageGuard() = default;
  ReadPageGuard(buffer::BufferPoolManager *bpm, Page *page);
  ~ReadPageGuard();

  ReadPageGuard(const ReadPageGuard &) = delete;
  auto operator=(const ReadPageGuard &) -> ReadPageGuard & = delete;

  ReadPageGuard(ReadPageGuard &&other) noexcept;
  auto operator=(ReadPageGuard &&other) noexcept -> ReadPageGuard &;

  auto is_valid() const -> bool { return this->page_ != nullptr; }

  auto page_id() const -> PageId;

  auto page() const -> const Page *;

  auto data() const -> const char *;

  template <typename T> auto as() const -> const T * {
    return reinterpret_cast<const T *>(this->data());
  }

  void drop();

private:
  buffer::BufferPoolManager *bpm_{nullptr};
  Page *page_{nullptr};
};

} // namespace turtle::storage::page
