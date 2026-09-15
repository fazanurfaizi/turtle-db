#include <cstddef>
#include <mutex>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"
#include "turtle/storage/page/page.hpp"

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
    this->free_list_.push_back(i);
  }
}

BufferPoolManager::~BufferPoolManager() {
  this->flush_all_pages();
  delete[] this->frame_data_;
  delete[] this->pages_;
}

storage::page::Page *BufferPoolManager::fetch_page(FileId file_id,
                                                   PageId page_id) {
  // std::lock_guard<std::mutex> guard(this->latch_);
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);
  // this->bpm_latch_->lock();

  PageKey key{file_id, page_id};

  // Page is in buffer pool
  if (this->page_table_.count(key)) {
    FrameId frame_id = this->page_table_[key];
    storage::page::Page &page = this->pages_[frame_id];

    page.pin_count_++;
    // this->replacer_.pin(frame_id);
    this->replacer_.record_access(frame_id, page_id);
    this->replacer_.set_evictable(frame_id, false);

    return &page;
  }

  // Page not in buffer pool, find a frame
  FrameId frame_id;
  if (!this->free_list_.empty()) {
    frame_id = this->free_list_.front();
    this->free_list_.pop_front();
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
  this->disk_manager_->read_page(file_id, page_id, frame.data());

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
  // std::lock_guard<std::mutex> guard(this->latch_);
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);
  // this->bpm_latch_->lock();

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
    // this->replacer_.unpin(frame_id);
    this->replacer_.set_evictable(frame_id, true);
  }

  return true;
}

bool BufferPoolManager::flush_page(FileId file_id, PageId page_id) {
  // std::lock_guard<std::mutex> guard(this->latch_);
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);
  // this->bpm_latch_->lock();

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
  // std::lock_guard<std::mutex> guard(this->latch_);
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);
  // this->bpm_latch_->lock();

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
  // std::lock_guard<std::mutex> guard(this->latch_);
  std::scoped_lock<std::mutex> guard(*this->bpm_latch_);
  // this->bpm_latch_->lock();

  FrameId frame_id;
  if (!this->free_list_.empty()) {
    frame_id = this->free_list_.front();
    this->free_list_.pop_front();
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
  // this->bpm_latch_->lock();

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
  // this->replacer_.pin(frame_id);
  this->free_list_.push_back(frame_id);

  page.reset_memory();
  page.page_id_ = INVALID_PAGE_ID;
  page.file_id_ = 0;
  page.pin_count_ = 0;
  page.is_dirty_ = false;

  return true;
}

} // namespace turtle::buffer
