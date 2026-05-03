#include "turtle/execution/seq_scan_executor.hpp"
#include "turtle/storage/table/table_iterator.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <memory>

namespace turtle::execution {

SeqScanExecutor::SeqScanExecutor(storage::table::TableHeap *table_heap)
    : table_heap_(table_heap) {}

void SeqScanExecutor::init() {
  // Reset the iterator to point to the very first tuple in the table
  this->iterator_ = std::make_unique<storage::table::TableIterator>(
      this->table_heap_->begin());
}

bool SeqScanExecutor::next(storage::table::Tuple *tuple, RecordId *rid) {
  // Check if reached the end of the table
  if (*this->iterator_ == this->table_heap_->end()) {
    return false;
  }

  // Grab the current tuple and its  Record ID
  *tuple = this->iterator_->get_tuple();
  *rid = this->iterator_->get_rid();

  ++(*this->iterator_);

  return true;
}

} // namespace turtle::execution
