#include "turtle/storage/page/slotted_page.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"
#include <cstdint>
#include <cstring>

namespace turtle::storage::page {

void SlottedPage::init(PageId page_id) {
  Header *header = this->header();
  header->page_id_ = page_id;
  header->next_page_id_ = INVALID_PAGE_ID;
  header->slot_count_ = 0;
  header->tuple_count_ = 0;
  header->free_space_pointer_ = PAGE_SIZE;
}

void SlottedPage::delete_tuple(const RecordId &rid) {
  this->validate_record_id(rid);
  Slot &slot = this->slots()[rid.slot_num];

  // Set length to 0 to mark as deleted
  // In a real engine, we would claim space back or use a bitmap.
  if (slot.length_ > 0) {
    Header *header = this->header();
    header->dead_space_ += slot.length_;
    slot.length_ = 0;
    header->tuple_count_--;
  }
}

uint32_t SlottedPage::free_space_remaining() const {
  const Header *header = this->header();
  // Free space = (Start of Data Area) - (End of Slot Array)
  uint32_t slot_array_end =
      sizeof(Header) + (header->slot_count_ * sizeof(Slot));
  return header->free_space_pointer_ - slot_array_end;
}

PageId SlottedPage::page_id() const { return this->header()->page_id_; }
uint32_t SlottedPage::tuple_count() const {
  return this->header()->tuple_count_;
}

uint32_t SlottedPage::slot_count() const { return this->header()->slot_count_; }

uint32_t SlottedPage::dead_space() const { return this->header()->dead_space_; }

uint32_t SlottedPage::usable_space() const {
  return this->free_space_remaining() + this->header()->dead_space_;
}

bool SlottedPage::is_slot_occupied(uint32_t slot_num) const {
  if (slot_num >= this->header()->slot_count_) {
    return false;
  }
  return this->slots()[slot_num].length_ != 0;
}

void SlottedPage::validate_record_id(const RecordId &rid) const {
  if (rid.page_id != page_id() || rid.slot_num >= header()->slot_count_) {
    throw std::runtime_error("Invalid RID for this page");
  }
}

void SlottedPage::compact() {
  Header *header = this->header();
  if (header->dead_space_ == 0)
    return;

  // Temporary memory buffer to copy live tuples
  char temp_data[PAGE_SIZE];
  uint32_t new_free_space_ptr = PAGE_SIZE;
  Slot *slot_arr = this->slots();

  // Pack live tuples to the end of temp data
  for (uint32_t i = 0; i < header->slot_count_; ++i) {
    if (slot_arr[i].length_ > 0) {
      new_free_space_ptr -= slot_arr[i].length_;

      // Copy tuple payload into temporary packed location
      std::memcpy(temp_data + new_free_space_ptr,
                  this->data() + slot_arr[i].offset_, slot_arr[i].length_);

      // Update slot offset to point to the new location
      slot_arr[i].offset_ = new_free_space_ptr;
    }
  }

  // Copy packed tuple region back to the actual page buffer
  uint32_t packed_bytes = PAGE_SIZE - new_free_space_ptr;
  if (packed_bytes > 0) {
    std::memcpy(this->data() + new_free_space_ptr,
                temp_data + new_free_space_ptr, packed_bytes);
  }

  // Reset pointer and counters
  header->free_space_pointer_ = new_free_space_ptr;
  header->dead_space_ = 0;
}
} // namespace turtle::storage::page
