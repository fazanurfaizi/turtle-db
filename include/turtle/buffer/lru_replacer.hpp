#pragma once

#include <list>
#include <mutex>
#include <unordered_map>

#include "turtle/buffer/replacer.hpp"
#include "turtle/common/config.hpp"

namespace turtle::buffer {

/**
 * Least Recently Used" (LRU) replacement policy
 */
class LRUReplacer : public Replacer {
public:
  explicit LRUReplacer(size_t num_pages);
  ~LRUReplacer() override = default;

  // Prevent copying
  LRUReplacer(const LRUReplacer &) = delete;
  LRUReplacer &operator=(const LRUReplacer &) = delete;

  auto victim(FrameId *frame_id) -> bool override;
  void pin(FrameId frame_id) override;
  void unpin(FrameId frame_id) override;
  auto size() -> size_t override;

private:
  std::list<FrameId> lru_list_;
  std::unordered_map<FrameId, std::list<FrameId>::iterator> lru_map_;
  std::mutex latch_;
  size_t capacity_;
};

} // namespace turtle::buffer
