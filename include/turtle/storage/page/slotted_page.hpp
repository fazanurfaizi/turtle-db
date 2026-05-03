#pragma once

#include <cstdint>
#include <cstring>

#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/storage/page/page.hpp"

namespace turtle::storage::page {

class Tuple;

class SlottedPage {
public:
  SlottedPage() = default;
  explicit SlottedPage(Page *page);
  ~SlottedPage();

  void init(PageId page_id);

  template <typename T> bool insert_tuple(const T &tuple, RecordId *record_id);

  template <typename T> void tuple(const RecordId &record_id, T *tuple);

  void delete_tuple(const RecordId &record_id);

  inline PageId get_next_page_id() const { return header()->next_page_id_; };

  inline void set_next_page_id(PageId &next_page_id) {
    header()->next_page_id_ = next_page_id;
  };

  uint32_t free_space_remaining() const;
  PageId page_id() const;
  uint32_t tuple_count() const;

private:
  struct Header {
    PageId page_id_;
    PageId next_page_id_{INVALID_PAGE_ID};
    uint32_t slot_count_;         // Total slots (active + empty)
    uint32_t tuple_count_;        // Active tuples
    uint32_t free_space_pointer_; // Offset to the start of free space
  };

  struct Slot {
    uint32_t offset_;
    uint32_t length_;
  };

  Header *header() const;
  Slot *slots() const;
  char *data() const;
  void validate_record_id(const RecordId &record_id) const;

  Page *page_;
};

// Template implementations
template <typename T>
bool SlottedPage::insert_tuple(const T &tuple, RecordId *record_id) {
  uint32_t size = tuple.storage_size();
  if (this->free_space_remaining() < size + sizeof(Slot)) {
    return false; // Not enough space
  }

  Header *header = this->header();

  // Prepare the new slot
  uint32_t slot_id = header->slot_count_;

  // Allocate space in the data area (grow backwards)
  header->free_space_pointer_ -= size;
  uint32_t offset = header->free_space_pointer_;

  // Write data
  tuple.serialize_to(this->data() + offset);

  Slot *slots = this->slots();
  // Update slot info
  slots[slot_id].offset_ = offset;
  slots[slot_id].length_ = size;

  // Update header info
  header->slot_count_++;
  header->tuple_count_++;

  if (record_id) {
    record_id->page_id = header->page_id_;
    record_id->slot_num = slot_id;
  }

  return true;
}

template <typename T>
void SlottedPage::tuple(const RecordId &record_id, T *tuple) {
  this->validate_record_id(record_id);
  Slot *slots = this->slots();
  Slot &slot = slots[record_id.slot_num];

  if (slot.length_ == 0) {
    throw std::runtime_error("Slot is empty (deleted tuple)");
  }

  // Deserialize from the offset
  tuple->deserialize_from(this->data() + slot.offset_, slot.length_);
}

} // namespace turtle::storage::page
