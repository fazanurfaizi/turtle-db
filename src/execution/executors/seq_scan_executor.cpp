#include <stdexcept>
#include <string>

#include "turtle/catalog/catalog.hpp"
#include "turtle/catalog/table_info.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/seq_scan_executor.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/table_iterator.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::executors {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx,
                                 const plans::SeqScanPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan) {}

void SeqScanExecutor::init() {
  // Resolve the table heap from the catalog using the plan's table OID.
  auto *info =
      this->exec_ctx_->get_catalog()->get_table(this->plan_->get_table_oid());
  if (info == nullptr) {
    throw std::runtime_error("SeqScanExecutor: unknown table oid " +
                             std::to_string(this->plan_->get_table_oid()));
  }
  this->table_heap_ = info->table_.get();
  this->iter_ = std::make_unique<storage::table::TableIterator>(
      this->table_heap_->begin());
}

bool SeqScanExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                           std::vector<RecordId> *rid_batch,
                           size_t batch_size) {
  // Contract: clear the outputs, append up to batch_size tuples, and return
  // true if at least one tuple was produced (false marks the scan exhausted).
  tuple_batch->clear();
  rid_batch->clear();

  auto end = this->table_heap_->end();
  while (tuple_batch->size() < batch_size && *this->iter_ != end) {
    tuple_batch->push_back(this->iter_->get_tuple());
    rid_batch->push_back(this->iter_->get_rid());
    ++(*this->iter_);
  }
  return !tuple_batch->empty();
}

} // namespace turtle::execution::executors
