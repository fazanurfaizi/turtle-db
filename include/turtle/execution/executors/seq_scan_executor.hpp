#pragma once

#include <memory>
#include <vector>

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/seq_scan_plan.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/table_iterator.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::executors {

class SeqScanExecutor : public AbstractExecutor {
public:
  SeqScanExecutor(ExecutorContext *exec_ctx,
                  const plans::SeqScanPlanNode *plan);

  void init() override;
  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  /** @return The output schema for the sequential scan */
  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  /** The sequential scan plan node to be executed */
  const plans::SeqScanPlanNode *plan_;
  /** The table heap resolved from the catalog during init() (non-owning) */
  storage::table::TableHeap *table_heap_{nullptr};
  /** Iterator over the table heap's tuples */
  std::unique_ptr<storage::table::TableIterator> iter_;
};

} // namespace turtle::execution::executors
