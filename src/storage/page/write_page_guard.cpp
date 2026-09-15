#include "turtle/storage/page/write_page_guard.hpp"
#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/storage/page/page.hpp"

namespace turtle::storage::page {

WritePageGuard::WritePageGuard(buffer::BufferPoolManager *bpm, Page *page)
    : bpm_(bpm), page_(page) {
  if (this->page_ != nullptr) {
    this->page_->rw_latch_->lock();
  }
}

WritePageGuard::WritePageGuard(WritePageGuard &&other) noexcept
    : bpm_(other.bpm_), page_(other.page_) {
  other.bpm_ = nullptr;
  other.page_ = nullptr;
}

auto WritePageGuard::operator=(WritePageGuard &&other) noexcept
    -> WritePageGuard & {
  if (this != &other) {
    this->drop();
    this->bpm_ = other.bpm_;
    this->page_ = other.page_;
    other.bpm_ = nullptr;
    other.page_ = nullptr;
  }
  return *this;
}

WritePageGuard::~WritePageGuard() { this->drop(); }

auto WritePageGuard::page_id() const -> PageId {
  return this->page_->page_id();
}

auto WritePageGuard::page() -> Page * { return this->page_; }

auto WritePageGuard::data() -> char * { return this->page_->data_mut(); }

void WritePageGuard::flush() {
  if (this->bpm_ != nullptr && this->page_ != nullptr) {
    this->bpm_->flush_page(this->page_->file_id(), this->page_->page_id());
  }
}

void WritePageGuard::drop() {
  if (this->bpm_ != nullptr && this->page_ != nullptr) {
    this->page_->rw_latch_->unlock();
    this->bpm_->unpin_page(this->page_->file_id(), this->page_->page_id(),
                           true);
  }
  this->bpm_ = nullptr;
  this->page_ = nullptr;
}

} // namespace turtle::storage::page
