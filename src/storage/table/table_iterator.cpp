#include "turtle/storage/table/table_iterator.hpp"
#include "turtle/storage/page/slotted_page.hpp"
#include "turtle/storage/table/table_heap.hpp"

namespace turtle::storage::table {

TableIterator::TableIterator(TableHeap *table_heap, RecordId rid)
    : table_heap_(table_heap), current_id_(rid) {
  // If the RID is valid, pre-fetch the first tuple so it's ready.
  if (this->current_id_.page_id != INVALID_PAGE_ID) {
    this->table_heap_->get_tuple(this->current_id_, &this->current_tuple_);
  }
}

const Tuple &TableIterator::get_tuple() { return this->current_tuple_; }

TableIterator &TableIterator::operator++() {
  // Fetch the current page to see how many slots it has
  auto *bpm = this->table_heap_->get_buffer_pool_manager();
  FileId file_id = this->table_heap_->get_file_id();

  page::Page *page = bpm->fetch_page(file_id, this->current_id_.page_id);
  page::SlottedPage slotted_page(page);

  // Move to the next slot on the current page
  this->current_id_.slot_num++;

  // Check if we have stepped off the edge of the current page
  if (this->current_id_.slot_num >= slotted_page.tuple_count()) {
    // Reached the end of the page. Find the next page.
    PageId next_page_id = slotted_page.get_next_page_id();

    // Unpun current page before move
    bpm->unpin_page(file_id, this->current_id_.page_id, false);

    if (next_page_id == INVALID_PAGE_ID) {
      // reached the end of the ENTIRE table. Set RID to invalid to mark
      // 'end()'.
      this->current_id_ = RecordId{INVALID_PAGE_ID, 0};
      return *this;
    } else {
      // Move to the very first slot of the next page
      this->current_id_.page_id = next_page_id;
      this->current_id_.slot_num = 0;
    }
  } else {
    // Unpin current page
    bpm->unpin_page(file_id, this->current_id_.page_id, false);
  }

  // Fetch the actual tuple data at our new position
  this->table_heap_->get_tuple(this->current_id_, &this->current_tuple_);

  return *this;
}

bool TableIterator::operator==(const TableIterator &other) const {
  return this->current_id_ == other.current_id_;
}

bool TableIterator::operator!=(const TableIterator &other) const {
  return !(*this == other);
}

} // namespace turtle::storage::table
