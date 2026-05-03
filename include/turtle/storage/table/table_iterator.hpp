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
  TableHeap *table_heap_;
  RecordId current_id_;
  Tuple current_tuple_;
};

} // namespace turtle::storage::table
