#include <cstdint>
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

  // Keep track of first and last pages
  this->first_page_id_ = first_page_id;
  PageId curr_page_id = first_page_id;

  while (curr_page_id != INVALID_PAGE_ID) {
    // Initialize the page as a Slotted Page
    page::WritePageGuard first_page_guard(this->bpm_, first_page);
    auto *slotted_page = first_page_guard.as_mut<page::SlottedPage>();
    slotted_page->init(first_page_id);

    // Track usable space in the lookup map
    this->free_space_map_[curr_page_id] = slotted_page->usable_space();

    PageId next_pid = slotted_page->get_next_page_id();
    this->last_page_id_ = curr_page_id;
    // this->bpm_->unpin_page(this->file_id_, curr_page_id, false);
    curr_page_id = next_pid;
  }
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
  uint32_t size = tuple.storage_size();
  PageId target_page_id = INVALID_PAGE_ID;

  // Check free space map for a candidate page
  for (const auto &[page_id, usable_bytes] : this->free_space_map_) {
    if (usable_bytes >= size + sizeof(page::SlottedPage::SLOT_SIZE)) {
      target_page_id = page_id;
      break;
    }
  }

  // Fallback to last page if no candidate was found in the map
  if (target_page_id == INVALID_PAGE_ID) {
    target_page_id = this->last_page_id_;
  }

  // Fetch the last page of the table where we usually have free space.
  auto write_page_opt =
      this->bpm_->write_page(this->file_id_, this->last_page_id_);
  if (!write_page_opt.has_value()) {
    return false;
  }

  page::WritePageGuard write_page = std::move(*write_page_opt);

  while (true) {
    auto *slotted_page = write_page.as_mut<page::SlottedPage>();

    // Try to insert the tuple into this page.
    if (slotted_page->insert_tuple(tuple, rid)) {
      this->free_space_map_[write_page.page_id()] =
          slotted_page->usable_space();
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

    this->free_space_map_[write_page.page_id()] = slotted_page->usable_space();

    page::WritePageGuard next_page(this->bpm_, new_page);
    next_page.as_mut<page::SlottedPage>()->init(new_page_id);

    // Update TableHeap's tracker
    this->last_page_id_ = new_page_id;

    // Unpin the new page and mark dirty
    write_page = std::move(next_page);
  }
}

bool TableHeap::mark_delete(const RecordId &rid) {
  auto write_page_opt = this->bpm_->write_page(this->file_id_, rid.page_id);
  if (!write_page_opt.has_value()) {
    return false;
  }

  page::WritePageGuard write_page = std::move(*write_page_opt);

  auto *slotted_page(write_page.as_mut<page::SlottedPage>());
  slotted_page->delete_tuple(rid);

  this->free_space_map_[rid.page_id] = slotted_page->usable_space();

  return true;
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
