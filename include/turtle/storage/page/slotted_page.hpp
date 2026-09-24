#pragma once

#include <cstdint>
#include <cstring>

#include "turtle/common/config.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/storage/page/page.hpp"

namespace turtle::storage::page {

class Tuple;

class SlottedPage {
public:
  SlottedPage() = delete;
  DISALLOW_COPY_AND_MOVE(SlottedPage)

  void init(PageId page_id);

  template <typename T> bool insert_tuple(const T &tuple, RecordId *record_id);

  template <typename T> void tuple(const RecordId &record_id, T *tuple) const;

  void delete_tuple(const RecordId &record_id);

  inline PageId get_next_page_id() const { return header()->next_page_id_; };

  inline void set_next_page_id(PageId &next_page_id) {
    header()->next_page_id_ = next_page_id;
  };

  uint32_t free_space_remaining() const;
  PageId page_id() const;
  uint32_t tuple_count() const;

  uint32_t dead_space() const;

  uint32_t usable_space() const;

  // Total number of slots ever allocated (active + deleted). Slot indices are
  // valid in [0, slot_count()); tuple_count() only counts non-deleted slots.
  uint32_t slot_count() const;

  // True if slot_num is in range and holds a live (non-deleted) tuple.
  bool is_slot_occupied(uint32_t slot_num) const;

  void compact();

  static constexpr size_t SLOT_SIZE = sizeof(uint32_t) * 2;

private:
  struct Header {
    PageId page_id_;
    PageId next_page_id_{INVALID_PAGE_ID};
    uint32_t slot_count_;         // Total slots (active + empty)
    uint32_t tuple_count_;        // Active tuples
    uint32_t free_space_pointer_; // Offset to the start of free space
    uint32_t dead_space_{0};
  };

  struct Slot {
    uint32_t offset_;
    uint32_t length_;
  };

  Header *header() { return reinterpret_cast<Header *>(this); };
  const Header *header() const {
    return reinterpret_cast<const Header *>(this);
  }

  Slot *slots() {
    return reinterpret_cast<Slot *>(reinterpret_cast<char *>(this) +
                                    sizeof(Header));
  };
  const Slot *slots() const {
    return reinterpret_cast<const Slot *>(reinterpret_cast<const char *>(this) +
                                          sizeof(Header));
  };

  char *data() { return reinterpret_cast<char *>(this); };
  const char *data() const { return reinterpret_cast<const char *>(this); }

  void validate_record_id(const RecordId &record_id) const;

  Page *page_;
};

// Template implementations
template <typename T>
bool SlottedPage::insert_tuple(const T &tuple, RecordId *record_id) {
  uint32_t size = tuple.storage_size();
  Header *header = this->header();

  // Check if an empty slot to reuse is exists
  int reusable_slot_id = -1;
  Slot *slot_arr = this->slots();
  for (uint32_t i = 0; i < header->slot_count_; ++i) {
    if (slot_arr[i].length_ == 0) {
      reusable_slot_id = static_cast<int>(i);
      break;
    }
  }

  // Calculate needed space
  uint32_t space_needed = size + (reusable_slot_id == -1 ? sizeof(Slot) : 0);

  if (this->free_space_remaining() < space_needed) {
    if (this->usable_space() >= space_needed) {
      this->compact();
    } else {
      return false; // Not enough space
    }
  }

  // Prepare the new slot
  uint32_t slot_id;
  if (reusable_slot_id != -1) {
    slot_id = static_cast<uint32_t>(reusable_slot_id);
  } else {
    slot_id = header->slot_count_;
    header->slot_count_++;
  }

  // Allocate space in the data area (grow backwards)
  header->free_space_pointer_ -= size;
  uint32_t offset = header->free_space_pointer_;

  // Write data
  tuple.serialize_to(this->data() + offset);

  // Update slot info
  Slot *slots = this->slots();
  slots[slot_id].offset_ = offset;
  slots[slot_id].length_ = size;
  header->tuple_count_++;

  if (record_id) {
    record_id->page_id = header->page_id_;
    record_id->slot_num = slot_id;
  }

  return true;
}

template <typename T>
void SlottedPage::tuple(const RecordId &record_id, T *tuple) const {
  this->validate_record_id(record_id);
  const Slot *slots = this->slots();
  const Slot &slot = slots[record_id.slot_num];

  if (slot.length_ == 0) {
    throw std::runtime_error("Slot is empty (deleted tuple)");
  }

  // Deserialize from the offset
  tuple->deserialize_from(this->data() + slot.offset_, slot.length_);
}

} // namespace turtle::storage::page
