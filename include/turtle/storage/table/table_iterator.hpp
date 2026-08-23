#pragma once

#include "turtle/common/record_id.hpp"
// #include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::storage::table {

class TableHeap;

class TableIterator {
public:
  // Initializes the iterator at a specific RecordId
  TableIterator(TableHeap *table_heap_, RecordId rid);
  ~TableIterator() = default;

  // Returns the tuple currently pointed to
  const Tuple &get_tuple();

  inline const RecordId get_rid() { return this->current_id_; }

  // Moves the iterator to the next tuple (handles jumping pages)
  TableIterator &operator++();

  // Checks if the iterator has reached the end of the table
  bool operator==(const TableIterator &other) const;
  bool operator!=(const TableIterator &other) const;

private:
  // Advance current_id_ forward (across pages) to the first live tuple at or
  // after its current position, loading current_tuple_. If none remain, sets
  // current_id_ to the sentinel end() position {INVALID_PAGE_ID, 0}.
  void advance_to_valid();

  TableHeap *table_heap_;
  RecordId current_id_;
  Tuple current_tuple_;
};

} // namespace turtle::storage::table
