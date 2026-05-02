#pragma once

#include "turtle/common/config.hpp"
#include <cstdint>
#include <sstream>
#include <string>

namespace turtle {

struct RecordId {
  PageId page_id = INVALID_PAGE_ID;
  uint32_t slot_num = 0;

  RecordId() = default;
  RecordId(PageId page_id, uint32_t slot_num)
    : page_id(page_id), slot_num(slot_num) {}

  inline bool operator==(const RecordId &other) const {
    return page_id == other.page_id && slot_num == other.slot_num;
  }

  inline std::string to_string() const {
    std::stringstream os;
    os << "RecordId(" << page_id << ", " << slot_num << ")";
    return os.str();
  }
};

}
