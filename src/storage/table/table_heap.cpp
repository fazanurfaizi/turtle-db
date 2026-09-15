#include <stdexcept>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/storage/page/page.hpp"
#include "turtle/storage/page/slotted_page.hpp"
#include "turtle/storage/page/write_page_guard.hpp"
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
  page::WritePageGuard first_page_guard(this->bpm_, first_page);
  first_page_guard.as_mut<page::SlottedPage>()->init(first_page_id);

  // Keep track of first and last pages
  this->first_page_id_ = first_page_id;
  this->last_page_id_ = first_page_id;
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
    auto read_page = this->bpm_->read_page(this->file_id_, current_page_id);
    if (!read_page.has_value()) {
      return;
    }
    const auto *slotted_page = read_page->as<page::SlottedPage>();

    // Find out what the next page is
    next_page_id = slotted_page->get_next_page_id();

    if (next_page_id != INVALID_PAGE_ID) {
      current_page_id = next_page_id;
    }
  } while (next_page_id != INVALID_PAGE_ID);

  this->last_page_id_ = current_page_id;
}

bool TableHeap::insert_tuple(const Tuple &tuple, RecordId *rid) {
  // Fetch the last page of the table where we usually have free space.
  auto write_page = this->bpm_->write_page(this->file_id_, this->last_page_id_);
  if (!write_page.has_value()) {
    return false;
  }

  while (true) {
    auto *slotted_page(write_page->as_mut<page::SlottedPage>());

    // Try to insert the tuple into this page.
    if (slotted_page->insert_tuple(tuple, rid)) {
      return true;
    }

    TURTLE_ENSURE(slotted_page->tuple_count() != 0,
                  "Tuple too large to fit in a single page");

    // If it failed, the page is full. Create a new page.
    PageId new_page_id;
    page::Page *new_page = this->bpm_->new_page(this->file_id_, &new_page_id);
    if (new_page == nullptr) {
      // BPM is completely full and nothing can be evicted.
      return false;
    }
    slotted_page->set_next_page_id(new_page_id);

    page::WritePageGuard next_page(this->bpm_, new_page);
    next_page.as_mut<page::SlottedPage>()->init(new_page_id);
    // Update TableHeap's tracker
    this->last_page_id_ = new_page_id;

    // Unpin the new page and mark dirty
    write_page = std::move(next_page);
  }
}

bool TableHeap::get_tuple(const RecordId &rid, Tuple *tuple) {
  // Fetch specific page containing the tuple
  auto read_page = this->bpm_->read_page(this->file_id_, rid.page_id);
  if (!read_page.has_value()) {
    return false;
  }

  const auto *slotted_page = read_page->as<page::SlottedPage>();
  // Read the tuple data into the provided pointer
  slotted_page->tuple(rid, tuple);

  return true;
}

TableIterator TableHeap::begin() {
  return TableIterator(this, RecordId{first_page_id_, 0});
}

TableIterator TableHeap::end() {
  return TableIterator(this, RecordId{INVALID_PAGE_ID, 0});
}

} // namespace turtle::storage::table
