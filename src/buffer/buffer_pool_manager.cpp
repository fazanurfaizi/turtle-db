#include <cstddef>
#include <fmt/base.h>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>

#include "turtle/buffer/arc_replacer.hpp"
#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"
#include "turtle/storage/page/page.hpp"
#include "turtle/storage/page/read_page_guard.hpp"
#include "turtle/storage/page/write_page_guard.hpp"

namespace turtle::buffer {

BufferPoolManager::BufferPoolManager(size_t pool_size,
                                     storage::disk::DiskManager *disk_manager)
    : pool_size_(pool_size), bpm_latch_(std::make_shared<std::mutex>()),
      replacer_(pool_size), disk_manager_(disk_manager) {

  std::scoped_lock latch(*this->bpm_latch_);

  this->pages_ = new storage::page::Page[this->pool_size_];
  this->frame_data_ = new char[this->pool_size_ * PAGE_SIZE];

  for (FrameId i = 0; i < static_cast<FrameId>(this->pool_size_); ++i) {
    this->pages_[i].data_ = this->frame_data_ + i * PAGE_SIZE;
    this->pages_[i].rw_latch_ = std::make_shared<std::shared_mutex>();
    this->free_frames_.push_back(i);
  }
}

BufferPoolManager::~BufferPoolManager() {
  this->flush_all_pages();
  delete[] this->frame_data_;
  delete[] this->pages_;
}

/**
 * @brief Acquires an optional write-locked guard over a page of data. The user
 * can specify an `AccessType` if needed.
 *
 * If it is not possible to bring the page of data into memory, this function
 * will return a `std::nullopt`.
 *
 * Page data can _only_ be accessed via page guards. Users of this
 * `BufferPoolManager` are expected to acquire either a `ReadPageGuard` or a
 * `WritePageGuard` depending on the mode in which they would like to access the
 * data, which ensures that any access of data is thread-safe.
 *
 * There can only be 1 `WritePageGuard` reading/writing a page at a time. This
 * allows data access to be both immutable and mutable, meaning the thread that
 * owns the `WritePageGuard` is allowed to manipulate the page's data however
 * they want. If a user wants to have multiple threads reading the page at the
 * same time, they must acquire a `ReadPageGuard` with `CheckedReadPage`
 * instead.
 *
 * ### Implementation
 *
 * There are three main cases that you will have to implement. The first two are
 * relatively simple: one is when there is plenty of available memory, and the
 * other is when we don't actually need to perform any additional I/O. Think
 * about what exactly these two cases entail.
 *
 * The third case is the trickiest, and it is when we do not have any _easily_
 * available memory at our disposal. The buffer pool is tasked with finding
 * memory that it can use to bring in a page of memory, using the replacement
 * algorithm you implemented previously to find candidate frames for eviction.
 *
 * Once the buffer pool has identified a frame for eviction, several I/O
 * operations may be necessary to bring in the page of data we want into the
 * frame.
 *
 * There is likely going to be a lot of shared code with `CheckedReadPage`, so
 * you may find creating helper functions useful.
 *
 * These two functions are the crux of this project, so we won't give you more
 * hints than this. Good luck!
 *
 * TODO(P1): Add implementation.
 *
 * @param file_id The ID of the file we want to write to.
 * @param page_id The ID of the page we want to write to.
 * @param access_type The type of page access.
 * @return std::optional<WritePageGuard> An optional latch guard where if there
 * are no more free frames (out of memory) returns `std::nullopt`; otherwise,
 * returns a `WritePageGuard` ensuring exclusive and mutable access to a page's
 * data.
 */
auto BufferPoolManager::write_page(FileId file_id, PageId page_id,
                                   [[maybe_unused]] AccessType access_type)
    -> std::optional<storage::page::WritePageGuard> {
  storage::page::Page *page = this->fetch_page(file_id, page_id);
  if (page != nullptr) {
    return storage::page::WritePageGuard(this, page);
  }

  return std::nullopt;
}

/**
 * @brief Acquires an optional write-locked guard over a page of data. The
 * user can specify an `AccessType` if needed.
 *
 * If it is not possible to bring the page of data into memory, this function
 * will return a `std::nullopt`.
 *
 * Page data can _only_ be accessed via page guards. Users of this
 * `BufferPoolManager` are expected to acquire either a `ReadPageGuard` or a
 * `WritePageGuard` depending on the mode in which they would like to access
 * the data, which ensures that any access of data is thread-safe.
 *
 * There can only be 1 `WritePageGuard` reading/writing a page at a time. This
 * allows data access to be both immutable and mutable, meaning the thread
 * that owns the `WritePageGuard` is allowed to manipulate the page's data
 * however they want. If a user wants to have multiple threads reading the
 * page at the same time, they must acquire a `ReadPageGuard` with
 * `CheckedReadPage` instead.
 *
 * ### Implementation
 *
 * There are three main cases that you will have to implement. The first two
 * are relatively simple: one is when there is plenty of available memory, and
 * the other is when we don't actually need to perform any additional I/O.
 * Think about what exactly these two cases entail.
 *
 * The third case is the trickiest, and it is when we do not have any _easily_
 * available memory at our disposal. The buffer pool is tasked with finding
 * memory that it can use to bring in a page of memory, using the replacement
 * algorithm you implemented previously to find candidate frames for eviction.
 *
 * Once the buffer pool has identified a frame for eviction, several I/O
 * operations may be necessary to bring in the page of data we want into the
 * frame.
 *
 * There is likely going to be a lot of shared code with `CheckedReadPage`, so
 * you may find creating helper functions useful.
 *
 * These two functions are the crux of this project, so we won't give you more
 * hints than this. Good luck!
 *
 * TODO(P1): Add implementation.
 *
 * @param page_id The ID of the page we want to write to.
 * @param access_type The type of page access.
 * @return std::optional<WritePageGuard> An optional latch guard where if
 * there are no more free frames (out of memory) returns `std::nullopt`;
 * otherwise, returns a `WritePageGuard` ensuring exclusive and mutable access
 * to a page's data.
 */
auto BufferPoolManager::read_page(FileId file_id, PageId page_id,
                                  [[maybe_unused]] AccessType access_type)
    -> std::optional<storage::page::ReadPageGuard> {
  storage::page::Page *page = this->fetch_page(file_id, page_id);
  if (page != nullptr) {
    return storage::page::ReadPageGuard(this, page);
  }

  return std::nullopt;
}

storage::page::Page *BufferPoolManager::fetch_page(FileId file_id,
                                                   PageId page_id) {
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);

  PageKey key{file_id, page_id};

  // Page is in buffer pool
  if (this->page_table_.count(key)) {
    FrameId frame_id = this->page_table_[key];
    storage::page::Page &page = this->pages_[frame_id];

    page.pin_count_++;
    this->replacer_.record_access(frame_id, page_id);
    this->replacer_.set_evictable(frame_id, false);

    return &page;
  }

  // Page not in buffer pool, find a frame
  FrameId frame_id;
  if (!this->free_frames_.empty()) {
    frame_id = this->free_frames_.front();
    this->free_frames_.pop_front();
  } else {
    auto victim = this->replacer_.evict();
    if (!victim.has_value()) {
      return nullptr; // All pages pinned
    }
    frame_id = *victim;
  }

  storage::page::Page &frame = this->pages_[frame_id];

  // Evict old page if needed
  if (frame.page_id_ != INVALID_PAGE_ID) {
    if (frame.is_dirty()) {
      this->disk_manager_->write_page(frame.file_id(), frame.page_id(),
                                      frame.data());
    }
    this->page_table_.erase({frame.file_id(), frame.page_id()});
  }

  // fetch new page from disk
  this->disk_manager_->read_page(file_id, page_id, frame.data_mut());

  frame.file_id_ = file_id;
  frame.page_id_ = page_id;
  frame.pin_count_ = 1;
  frame.is_dirty_ = false;

  this->page_table_[key] = frame_id;
  // this->replacer_.pin(frame_id);
  this->replacer_.record_access(frame_id, page_id);
  this->replacer_.set_evictable(frame_id, false);

  return &frame;
}

bool BufferPoolManager::unpin_page(FileId file_id, PageId page_id,
                                   bool is_dirty) {
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);

  PageKey key{file_id, page_id};
  if (!this->page_table_.count(key)) {
    return false;
  }

  FrameId frame_id = this->page_table_[key];
  storage::page::Page &page = this->pages_[frame_id];

  if (page.pin_count_ <= 0) {
    return false;
  }

  page.pin_count_--;
  if (is_dirty) {
    page.is_dirty_ = true;
  }

  if (page.pin_count_ == 0) {
    this->replacer_.set_evictable(frame_id, true);
  }

  return true;
}

bool BufferPoolManager::flush_page(FileId file_id, PageId page_id) {
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);

  PageKey key{file_id, page_id};
  if (!this->page_table_.count(key)) {
    // Page not in buffer pool
    return false;
  }

  FrameId frame_id = this->page_table_[key];
  storage::page::Page &page = this->pages_[frame_id];

  this->disk_manager_->write_page(page.file_id(), page.page_id(), page.data());
  page.is_dirty_ = false;

  return true;
}

void BufferPoolManager::flush_all_pages() {
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);

  for (FrameId i = 0; i < static_cast<FrameId>(this->pool_size_); ++i) {
    storage::page::Page &page = this->pages_[i];

    if (page.page_id() != INVALID_PAGE_ID && page.is_dirty()) {
      this->disk_manager_->write_page(page.file_id(), page.page_id(),
                                      page.data());
      page.is_dirty_ = false;
    }
  }
}

storage::page::Page *BufferPoolManager::new_page(FileId file_id,
                                                 PageId *page_id) {
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);

  FrameId frame_id;
  if (!this->free_frames_.empty()) {
    frame_id = this->free_frames_.front();
    this->free_frames_.pop_front();
  } else {
    auto victim = this->replacer_.evict();
    if (!victim.has_value()) {
      return nullptr;
    }
    frame_id = *victim;
  }

  storage::page::Page &frame = this->pages_[frame_id];

  if (frame.page_id() != INVALID_PAGE_ID) {
    if (frame.is_dirty()) {
      this->disk_manager_->write_page(frame.file_id(), frame.page_id(),
                                      frame.data());
    }
    this->page_table_.erase({frame.file_id(), frame.page_id()});
  }

  // Allocate new Page ID
  *page_id = this->next_page_id_++;

  frame.reset_memory();
  frame.file_id_ = file_id;
  frame.page_id_ = *page_id;
  frame.pin_count_ = 1;
  frame.is_dirty_ = false;

  this->page_table_[{file_id, *page_id}] = frame_id;
  // this->replacer_.pin(frame_id);
  this->replacer_.record_access(frame_id, *page_id);
  this->replacer_.set_evictable(frame_id, false);

  // Write the empty page to disk to "reserve" its space
  this->disk_manager_->write_page(file_id, *page_id, frame.data());

  return &frame;
}

bool BufferPoolManager::delete_page(FileId file_id, PageId page_id) {
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);

  PageKey key{file_id, page_id};
  if (!this->page_table_.count(key)) {
    return true;
  }

  FrameId frame_id = this->page_table_[key];
  storage::page::Page &page = this->pages_[frame_id];

  if (page.pin_count() > 0) {
    // Cannot delete a pinned page
    return false;
  }

  this->page_table_.erase(key);
  this->replacer_.remove(frame_id);
  this->free_frames_.push_back(frame_id);

  page.reset_memory();
  page.page_id_ = INVALID_PAGE_ID;
  page.file_id_ = 0;
  page.pin_count_ = 0;
  page.is_dirty_ = false;

  return true;
}

} // namespace turtle::buffer
