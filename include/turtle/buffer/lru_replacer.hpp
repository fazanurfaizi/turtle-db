#pragma once
// Least Recently Used" (LRU) replacement policy

#include <list>
#include <mutex>
#include <unordered_map>

#include "turtle/common/config.hpp"

namespace turtle::buffer {

class LRUReplacer {
public:
  explicit LRUReplacer(size_t num_pages);
  ~LRUReplacer() = default;

  // Prevent copying
  LRUReplacer(const LRUReplacer &) = delete;
  LRUReplacer &operator=(const LRUReplacer &) = delete;

  bool victim(FrameId *frame_id);
  void pin(FrameId frame_id);
  void unpin(FrameId frame_id);
  size_t size();

private:
  std::list<FrameId> lru_list_;
  std::unordered_map<FrameId, std::list<FrameId>::iterator> lru_map_;
  std::mutex latch_;
  size_t capacity_;
};

} // namespace Turtle::Buffer

