#pragma once

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/common/config.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::storage::table {

class TableIterator;

class TableHeap {
public:
  // Create a brand new table heap (allocates the first page)
  TableHeap(buffer::BufferPoolManager *bpm, FileId file_id);

  // Open an existing table heap
  TableHeap(buffer::BufferPoolManager *bpm, FileId file_id,
            PageId first_page_id);

  ~TableHeap() = default;

  // Insert a tuple. Retunrs true if successful, and populates the rid.
  bool insert_tuple(const Tuple &tuple, RecordId *rid);

  // Marks a tuple as deleted (sets length to 0 in the slotted page)
  bool mark_delete(const RecordId &rid);

  // Retrieves a tuple by its Record ID
  bool get_tuple(const RecordId &rid, Tuple *tuple);

  // Get the first page ID to start a Sequential Scan
  inline PageId get_first_page_id() const { return this->first_page_id_; }

  // Get the last page ID to end a Sequential Scan
  inline PageId get_last_page_id() const { return this->last_page_id_; }

  inline buffer::BufferPoolManager *get_buffer_pool_manager() const {
    return this->bpm_;
  }

  inline FileId get_file_id() const { return file_id_; }

  TableIterator begin();
  TableIterator end();

private:
  buffer::BufferPoolManager *bpm_;
  FileId file_id_;
  PageId first_page_id_{INVALID_PAGE_ID};
  PageId last_page_id_{INVALID_PAGE_ID};
};

} // namespace turtle::storage::table
