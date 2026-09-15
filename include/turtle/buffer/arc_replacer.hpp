#pragma once

#include "turtle/common/config.hpp"
#include <cstddef>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace turtle::buffer {

enum class AccessType {
  Unknown = 0,
  Lookup,
  Scan,
  Index,
};

/**
 * ArcStatus
 * MRU: Most Recently Used
 * MFU: Most Frequently Used
 */
enum class ArcStatus {
  MRU,
  MFU,
  MRU_GHOST,
  MFU_GHOST,
};

struct FrameStatus {
  PageId page_id_;
  FrameId frame_id_;
  bool evictable_;
  ArcStatus arc_status_;

  FrameStatus(PageId pid, FrameId fid, bool evictable, ArcStatus ar)
      : page_id_(pid), frame_id_(fid), evictable_(evictable), arc_status_(ar) {}
};

/**
 * Adaptive Replacement Cache (ARC) eviction policy.
 *
 *   mru_  (T1)  — pages seen once recently        | live, keyed by FrameId
 *   mfu_  (T2)  — pages seen >= 2 times            | live, keyed by FrameId
 *   mru_ghost_ (B1) — metadata of pages evicted from T1 | dead, keyed by PageId
 *   mfu_ghost_ (B2) — metadata of pages evicted from T2 | dead, keyed by PageId
 *
 * target_ (p) is the adaptive split point: the desired size of mru_. It grows
 * on a B1 ghost hit (recency was under-served) and shrinks on a B2 ghost hit.
 */
class ArcReplacer {
public:
  explicit ArcReplacer(size_t num_frames);

  ~ArcReplacer() = default;

  ArcReplacer(const ArcReplacer &) = delete;
  auto operator=(const ArcReplacer &) -> ArcReplacer & = delete;

  ArcReplacer(ArcReplacer &&) = delete;
  auto operator=(ArcReplacer &&) -> ArcReplacer & = delete;

  auto evict() -> std::optional<FrameId>;

  void record_access(FrameId frame_id, PageId page_id,
                     AccessType access_type = AccessType::Unknown);

  void set_evictable(FrameId frame_id, bool set_evictable);

  void remove(FrameId frame_id);

  auto size() -> size_t;

private:
  std::list<FrameId> mru_;
  std::list<FrameId> mfu_;
  std::list<PageId> mru_ghost_;
  std::list<PageId> mfu_ghost_;

  std::unordered_map<FrameId, std::shared_ptr<FrameStatus>> alive_map_;

  std::unordered_map<PageId, std::shared_ptr<FrameStatus>> ghost_map_;

  [[maybe_unused]] size_t curr_size_{0};
  [[maybe_unused]] size_t mru_target_size_{0};
  [[maybe_unused]] size_t replacer_size_;

  std::mutex latch_;
};

} // namespace turtle::buffer
