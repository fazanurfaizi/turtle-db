#include <stdexcept>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"
#include "turtle/storage/page/page.hpp"
#include "turtle/storage/page/slotted_page.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/table_iterator.hpp"

namespace turtle::storage::table {

// Creating a brand new table
TableHeap::TableHeap(buffer::BufferPoolManager *bpm, FileId file_id)
    : bpm_(bpm), file_id_(file_id) {
  PageId first_page_id;

  // Ask the BPM for a brand new page
  // This automatically "pins" the page in memory.
  page::Page *first_page = this->bpm_->new_page(this->file_id_, &first_page_id);
  if (first_page == nullptr) {
    throw std::runtime_error(
        "Out of memory: Cannot allocate first page for TableHeap.");
  }

  // Initialize the page as a Slotted Page
  page::SlottedPage slotted_page(first_page);
  slotted_page.init(first_page_id);

  // Keep track of first and last pages
  this->first_page_id_ = first_page_id;
  this->last_page_id_ = first_page_id;

  // Unpin the page
  this->bpm_->unpin_page(this->file_id_, first_page_id, true);
}

// Opening an existing table from disk
TableHeap::TableHeap(buffer::BufferPoolManager *bpm, FileId file_id,
                     PageId first_page_id)
    : bpm_(bpm), file_id_(file_id), first_page_id_(first_page_id) {
  // To find the last_page_id_, we must traverse the linked list of pages.
  PageId current_page_id = this->first_page_id_;
  PageId next_page_id = INVALID_PAGE_ID;

  do {
    // Fetch the current page
    page::Page *page = this->bpm_->fetch_page(this->file_id_, current_page_id);
    page::SlottedPage slotted_page(page);

    // Find out what the next page is
    next_page_id = slotted_page.get_next_page_id();

    // Unpin because we are just reading, not modifying
    this->bpm_->unpin_page(this->file_id_, current_page_id, false);

    if (next_page_id != INVALID_PAGE_ID) {
      current_page_id = next_page_id;
    }
  } while (next_page_id != INVALID_PAGE_ID);

  this->last_page_id_ = current_page_id;
}

bool TableHeap::insert_tuple(const Tuple &tuple, RecordId *rid) {
  // Fetch the last page of the table where we usually have free space.
  page::Page *page = this->bpm_->fetch_page(0, this->last_page_id_);
  page::SlottedPage slotted_page(page);

  // Try to insert the tuple into this page.
  if (slotted_page.insert_tuple(tuple, rid)) {
    this->bpm_->unpin_page(0, this->last_page_id_, true);
    return true;
  }

  // If it failed, the page is full. Create a new page.
  PageId new_page_id;
  page::Page *new_page = this->bpm_->new_page(0, &new_page_id);
  if (new_page == nullptr) {
    // BPM is completely full and nothing can be evicted.
    this->bpm_->unpin_page(0, this->last_page_id_, false);
    return false;
  }

  // Initialize the new page
  page::SlottedPage new_slotted_page(new_page);
  new_slotted_page.init(new_page_id);

  // Link the old last page to this new page
  slotted_page.set_next_page_id(new_page_id);

  this->bpm_->unpin_page(0, this->last_page_id_, true);

  // Insert the tuple into the new page
  bool success = new_slotted_page.insert_tuple(tuple, rid);

  // Update TableHeap's tracker
  this->last_page_id_ = new_page_id;

  // Unpin the new page and mark dirty
  this->bpm_->unpin_page(0, new_page_id, true);

  return success;
}

bool TableHeap::get_tuple(const RecordId &rid, Tuple *tuple) {
  // Fetch specific page containing the tuple
  page::Page *page = this->bpm_->fetch_page(0, rid.page_id);
  if (page == nullptr)
    return false;

  page::SlottedPage slotted_page(page);

  // Read the tuple data into the provided pointer
  slotted_page.tuple(rid, tuple);

  this->bpm_->unpin_page(0, rid.page_id, false);
  return true;
}

TableIterator TableHeap::begin() {
  return TableIterator(this, RecordId{first_page_id_, 0});
}

TableIterator TableHeap::end() {
  return TableIterator(this, RecordId{INVALID_PAGE_ID, 0});
}

} // namespace turtle::storage::table
