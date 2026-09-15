#include "turtle/storage/table/table_iterator.hpp"
#include "turtle/storage/page/slotted_page.hpp"
#include "turtle/storage/table/table_heap.hpp"

namespace turtle::storage::table {

TableIterator::TableIterator(TableHeap *table_heap, RecordId rid)
    : table_heap_(table_heap), current_id_(rid) {
  // Position on the first live tuple at or after `rid`. This handles an empty
  // first page (begin() == end()) and leading deleted slots without throwing.
  if (this->current_id_.page_id != INVALID_PAGE_ID) {
    this->advance_to_valid();
  }
}

const Tuple &TableIterator::get_tuple() { return this->current_tuple_; }

void TableIterator::advance_to_valid() {
  auto *bpm = this->table_heap_->get_buffer_pool_manager();
  FileId file_id = this->table_heap_->get_file_id();

  while (this->current_id_.page_id != INVALID_PAGE_ID) {
    auto read_page = bpm->read_page(file_id, this->current_id_.page_id);
    if (!read_page.has_value()) {
      return;
    }

    const auto *slotted_page = read_page->as<page::SlottedPage>();

    // Skip over deleted slots on this page.
    uint32_t slot_count = slotted_page->slot_count();
    while (this->current_id_.slot_num < slot_count &&
           !slotted_page->is_slot_occupied(this->current_id_.slot_num)) {
      this->current_id_.slot_num++;
    }

    if (this->current_id_.slot_num < slot_count) {
      // Found a live tuple on this page.
      slotted_page->tuple(this->current_id_, &this->current_tuple_);
      return;
    }

    // Exhausted this page; step to the first slot of the next one.
    this->current_id_.page_id = slotted_page->get_next_page_id();
    this->current_id_.slot_num = 0;
  }

  // No more live tuples anywhere: become end().
  this->current_id_ = RecordId{INVALID_PAGE_ID, 0};
}

TableIterator &TableIterator::operator++() {
  this->current_id_.slot_num++;
  this->advance_to_valid();
  return *this;
}

bool TableIterator::operator==(const TableIterator &other) const {
  return this->current_id_ == other.current_id_;
}

bool TableIterator::operator!=(const TableIterator &other) const {
  return !(*this == other);
}

} // namespace turtle::storage::table
