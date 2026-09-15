#pragma once

#include "turtle/common/config.hpp"

namespace turtle::buffer {

/*
 * Replacer is an abstract class that tracks page usage.
 */
class Replacer {
public:
  Replacer() = default;
  virtual ~Replacer() = default;

  /**
   * Remove the victim frame as defined by the replacement policy.
   * @param frame_id id of frame that was removed, nullptr if no victim was
   * found
   * @return true if a victim frame was found, false otherwise
   */
  virtual auto victim(FrameId *frame_id) -> bool = 0;

  /**
   * Pin a frame, indicating that it should not be victimized until it is
   * unpinned.
   * @param frame_id the id of the frame to pin
   */
  virtual void pin(FrameId frame_id) = 0;

  /**
   * Unpin a frame, indicating that it can now be victimized
   * @param frame_id the id of the frame to unpin
   */
  virtual void unpin(FrameId frame_id) = 0;

  /**
   * @return the number of elements in the replacer that can be victimized
   */
  virtual auto size() -> size_t = 0;
};

} // namespace turtle::buffer
