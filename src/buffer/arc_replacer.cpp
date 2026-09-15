#include "turtle/buffer/arc_replacer.hpp"
#include "turtle/common/macros.hpp"

namespace turtle::buffer {

ArcReplacer::ArcReplacer(size_t num_frames) : replacer_size_(num_frames) {}

/*
 * @return frame id of the evicted frame, or std::nullopt if cannot evict
 */
auto ArcReplacer::evict() -> std::optional<FrameId> {
  std::lock_guard<std::mutex> guard(this->latch_);
  if (this->curr_size_ == 0) {
    return std::nullopt;
  }

  // Demote the first evictable frame (scanning LRU end -> MRU end) out of the
  // live list `live` into `ghost`, re-tagging it `ghost_status`. The frame
  // data is feed by BPM; just keep metadata-id. Returns the evicted frame id
  // or nullopt if every entry in `lived` is pinned.
  auto try_evict = [this](std::list<FrameId> &live, std::list<PageId> &ghost,
                          ArcStatus ghost_status) -> std::optional<FrameId> {
    for (auto it = live.end(); it != live.begin();) {
      --it; // walk backward from the LRU end
      auto map_it = this->alive_map_.find(*it);
      if (map_it == this->alive_map_.end() || !map_it->second->evictable_) {
        continue; // pinned (or stale) -> skip toward MRU
      }

      FrameId fid = *it;
      std::shared_ptr<FrameStatus> status = map_it->second;

      // Move metadata to the ghost list
      PageId pid = status->page_id_;
      status->arc_status_ = ghost_status;
      status->frame_id_ = INVALID_FRAME_ID;
      ghost.push_front(pid);
      this->ghost_map_[pid] = status;

      // Unlink from live list
      live.erase(it);
      this->alive_map_.erase(fid);
      --this->curr_size_;
      return fid;
    }
    return std::nullopt;
  };

  // REPLACE policy: if mru_ is over its adaptive target, evict from the recency
  // side; otherwise the frequency side. Fall back to the other side if the
  // preferred one is fully pinned. The tie at mru_.size() == target is
  // arbitrary.
  if (this->mru_.size() > this->mru_target_size_) {
    if (auto v =
            try_evict(this->mru_, this->mfu_ghost_, ArcStatus::MRU_GHOST)) {
      return v;
    }
    return try_evict(this->mfu_, this->mfu_ghost_, ArcStatus::MFU_GHOST);
  }
  if (auto v = try_evict(this->mfu_, this->mfu_ghost_, ArcStatus::MFU_GHOST)) {
    return v;
  }
  return try_evict(this->mru_, this->mfu_ghost_, ArcStatus::MRU_GHOST);
}

/**
 * @param frame_id id of frame that received a new access.
 * @param page_id id of page that is mapped to the frame.
 * @param access_type type of access that was received.
 */
void ArcReplacer::record_access(FrameId frame_id, PageId page_id,
                                [[maybe_unused]] AccessType access_type) {
  std::lock_guard<std::mutex> guard(this->latch_);

  // hit on a live list (mfu or mru) -> promote to MFU front.
  if (auto it = this->alive_map_.find(frame_id); it != this->alive_map_.end()) {
    std::shared_ptr<FrameStatus> status = it->second;
    if (status->arc_status_ == ArcStatus::MRU) {
      this->mru_.remove(frame_id);
    } else {
      this->mfu_.remove(frame_id);
    }
    status->arc_status_ = ArcStatus::MFU;
    this->mfu_.push_front(frame_id);
    return;
  }

  // hit in a ghost list -> adapt target, resurrent into MFU.
  if (auto it = this->ghost_map_.find(page_id); it != this->ghost_map_.end()) {
    std::shared_ptr<FrameStatus> status = it->second;
    size_t b1 = this->mru_ghost_.size();
    size_t b2 = this->mfu_ghost_.size();

    if (status->arc_status_ == ArcStatus::MRU_GHOST) {
      // B1 Hit: recency was under-served -> grow target toward capacity
      size_t delta = std::max<size_t>(1, b2 / std::max<size_t>(b1, 1));
      this->mru_target_size_ =
          std::min(this->mru_target_size_ + delta, this->replacer_size_);
      this->mru_ghost_.remove(page_id);
    } else { // MFU_GHOST
      // B2 Hit: frequency was under-server -> shrink targer toward zero
      size_t delta = std::max<size_t>(1, b1 / std::max<size_t>(b2, 1));
      this->mru_target_size_ =
          (this->mru_target_size_ > delta) ? this->mru_target_size_ - delta : 0;
      this->mfu_ghost_.remove(page_id);
    }
    this->ghost_map_.erase(it);

    // Resurrent: a live frame backs the page again; ARC places it in MFU.
    // Freshly fetched -> pinned by the BPM, so not yet evictable.
    status->frame_id_ = frame_id;
    status->page_id_ = page_id;
    status->arc_status_ = ArcStatus::MFU;
    status->evictable_ = false;
    this->mfu_.push_front(frame_id);
    this->alive_map_[frame_id] = status;
    return;
  }

  // Total miss -> trim ghost metadata, then enter MRU.
  size_t t1 = this->mru_.size();
  size_t b1 = this->mfu_ghost_.size();
  if (t1 + b1 == this->replacer_size_) {
    if (b1 > 0) { // drop the LRU (back) B1 ghost entry
      PageId dropped = this->mru_ghost_.back();
      this->mru_ghost_.pop_back();
      this->ghost_map_.erase(dropped);
    }
  } else {
    size_t total = t1 + this->mfu_.size() + b1 + this->mfu_ghost_.size();
    if (t1 + b1 < this->replacer_size_ && total >= 2 * this->replacer_size_ &&
        !this->mfu_ghost_.empty()) {
      PageId dropped = this->mfu_ghost_.back(); // drop LRU B2 ghost entry
      this->mfu_ghost_.pop_back();
      this->ghost_map_.erase(dropped);
    }
  }

  auto status =
      std::make_shared<FrameStatus>(page_id, frame_id, false, ArcStatus::MRU);
  this->mru_.push_front(frame_id);
  this->alive_map_[frame_id] = status;
}

void ArcReplacer::set_evictable(FrameId frame_id, bool set_evictable) {
  std::lock_guard<std::mutex> guard(this->latch_);

  auto it = this->alive_map_.find(frame_id);
  if (it == this->alive_map_.end()) {
    return; // Unknown / ghost frame -> nothing to toggle
  }
  std::shared_ptr<FrameStatus> status = it->second;
  if (status->evictable_ == set_evictable) {
    return; // no change, size unaffected
  }
  status->evictable_ = set_evictable;
  if (set_evictable) {
    ++this->curr_size_;
  } else {
    --this->curr_size_;
  }
}

void ArcReplacer::remove(FrameId frame_id) {
  std::lock_guard<std::mutex> guard(this->latch_);

  auto it = this->alive_map_.find(frame_id);
  if (it == this->alive_map_.end()) {
    return; // not present: no-op
  }

  std::shared_ptr<FrameStatus> status = it->second;
  TURTLE_ENSURE(status->evictable_,
                "ArcReplacer::remove called on a non-evictable frame.");

  if (status->arc_status_ == ArcStatus::MRU) {
    this->mru_.remove(frame_id);
  } else {
    this->mfu_.remove(frame_id);
  }
  this->alive_map_.erase(it);
  --this->curr_size_;
}

auto ArcReplacer::size() -> size_t {
  std::lock_guard<std::mutex> guard(this->latch_);
  return this->replacer_size_;
}

} // namespace turtle::buffer
