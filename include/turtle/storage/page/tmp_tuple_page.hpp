#pragma once

#include "turtle/common/config.hpp"
#include "turtle/storage/page/page.hpp"
#include "turtle/storage/table/tmp_tuple.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <cstdint>
#include <cstring>

namespace turtle::storage::page {

class TmpTuplePage : public Page {
public:
  void init(PageId page_id, uint32_t page_size) {
    memcpy(this->data(), &page_id, sizeof(PageId));
    memcpy(this->data() + sizeof(PageId), &page_size, sizeof(uint32_t));
  }

  auto get_table_page_id() -> PageId { return INVALID_PAGE_ID; }

  auto insert(const table::Tuple &tuple, table::TmpTuple *out) -> bool {
    return false;
  }

private:
  static_assert(sizeof(PageId) == 4);
};

} // namespace turtle::storage::page
