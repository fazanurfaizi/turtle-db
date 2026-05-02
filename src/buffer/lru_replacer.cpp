#include "turtle/buffer/lru_replacer.hpp"

namespace turtle::buffer {

LRUReplacer::LRUReplacer(size_t num_pages) : capacity_(num_pages) {}

bool LRUReplacer::victim(FrameId *frame_id) {
  std::lock_guard<std::mutex> guard(this->latch_);
  if (this->lru_list_.empty()) {
    return false;
  }

  *frame_id = this->lru_list_.back();
  this->lru_list_.pop_back();
  this->lru_map_.erase(*frame_id);
  return true;
}

void LRUReplacer::pin(FrameId frame_id) {
  std::lock_guard<std::mutex> guard(this->latch_);
  if (this->lru_map_.count(frame_id)) {
    this->lru_list_.erase(this->lru_map_[frame_id]);
    this->lru_map_.erase(frame_id);
  }
}

void LRUReplacer::unpin(FrameId frame_id) {
  std::lock_guard<std::mutex> guard(this->latch_);
  if (this->lru_map_.count(frame_id) == 0) {
    this->lru_list_.push_front(frame_id);
    this->lru_map_[frame_id] = this->lru_list_.begin();
  }
}

size_t LRUReplacer::size() {
  std::lock_guard<std::mutex> guard(this->latch_);
  return this->lru_list_.size();
}

} // namespace Turtle::Buffer
