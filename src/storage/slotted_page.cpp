#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/storage/page.hpp"
#include <cstdint>
#include "turtle/storage/slotted_page.hpp"

namespace turtle::storage {

SlottedPage::SlottedPage(Page *page) : page_(page){}

void SlottedPage::init(PageId page_id) {
  Header *header = this->header();
  header->page_id_ = page_id;
  header->slot_count_ = 0;
  header->tuple_count_ = 0;
  header->free_space_pointer_ = PAGE_SIZE;
}

void SlottedPage::delete_tuple(const RecordId &rid) {
  this->validate_record_id(rid);
  Slot *slots = this->slots();
  
  // Set length to 0 to mark as deleted
  // In a real engine, we would claim space back or use a bitmap.
  slots[rid.slot_num].length_ = 0;
  
  Header *header = this->header();
  header->tuple_count_--;
}

uint32_t SlottedPage::free_space_remaining() const {
  Header *header = this->header();
  // Free space = (Start of Data Area) - (End of Slot Array)
  uint32_t slot_array_end = sizeof(Header) + (header->slot_count_ * sizeof(Slot));
  return header->free_space_pointer_ - slot_array_end;
}

PageId SlottedPage::page_id() const { return this->header()->page_id_; }
uint32_t SlottedPage::tuple_count() const { return this->header()->tuple_count_; }

SlottedPage::Header *SlottedPage::header() const {
  return reinterpret_cast<Header *>(page_->data());
}

SlottedPage::Slot *SlottedPage::slots() const {
  return reinterpret_cast<Slot *>(page_->data() + sizeof(SlottedPage::Header));
}

char *SlottedPage::data() const {
  return page_->data();
}

void SlottedPage::validate_record_id(const RecordId &rid) const {
  if (rid.page_id != page_id() || rid.slot_num >= header()->slot_count_) {
    throw std::runtime_error("Invalid RID for this page");
  }
}
}
