#pragma once

#include <memory>

#include "turtle/common/record_id.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/table_iterator.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution {

class SeqScanExecutor : public executors::AbstractExecutor {
public:
  SeqScanExecutor(storage::table::TableHeap *table_heap);
  ~SeqScanExecutor() override = default;

  void init() override;
  bool next(storage::table::Tuple *tuple, RecordId *rid) override;

private:
  storage::table::TableHeap *table_heap_;
  std::unique_ptr<storage::table::TableIterator> iterator_;
};

} // namespace turtle::execution
