#pragma once

#include "turtle/common/config.hpp"

namespace turtle::storage::table {

class TmpTuple {
public:
  TmpTuple(PageId page_id, size_t offset)
      : page_id_(page_id), offset_(offset) {}

  inline auto operator==(const TmpTuple &other) const -> bool {
    return this->page_id_ == other.page_id_ && this->offset_ == other.offset_;
  }

  auto get_page_id() const -> PageId { return this->page_id_; }

  auto get_offset() const -> size_t { return this->offset_; }

private:
  PageId page_id_;
  size_t offset_;
};

} // namespace turtle::storage::table
