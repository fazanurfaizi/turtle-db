#include "turtle/storage/page/read_page_guard.hpp"
#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/storage/page/page.hpp"

namespace turtle::storage::page {

ReadPageGuard::ReadPageGuard(buffer::BufferPoolManager *bpm, Page *page)
    : bpm_(bpm), page_(page) {
  if (this->page_ != nullptr) {
    this->page_->rw_latch_->lock_shared();
  }
}

ReadPageGuard::ReadPageGuard(ReadPageGuard &&other) noexcept
    : bpm_(other.bpm_), page_(other.page_) {
  other.bpm_ = nullptr;
  other.page_ = nullptr;
}

auto ReadPageGuard::operator=(ReadPageGuard &&other) noexcept
    -> ReadPageGuard & {
  if (this != &other) {
    this->drop();
    this->bpm_ = other.bpm_;
    this->page_ = other.page_;
    other.bpm_ = nullptr;
    other.page_ = nullptr;
  }
  return *this;
}

ReadPageGuard::~ReadPageGuard() { this->drop(); }

auto ReadPageGuard::page_id() const -> PageId { return this->page_->page_id(); }

auto ReadPageGuard::page() const -> const Page * { return this->page_; }

auto ReadPageGuard::data() const -> const char * { return this->page_->data(); }

void ReadPageGuard::drop() {
  if (this->bpm_ != nullptr && this->page_ != nullptr) {
    this->page_->rw_latch_->unlock_shared();
    this->bpm_->unpin_page(this->page_->file_id(), this->page_->page_id(),
                           false);
  }
  this->bpm_ = nullptr;
  this->page_ = nullptr;
}

} // namespace turtle::storage::page
