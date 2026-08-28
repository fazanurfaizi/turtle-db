#pragma once

#include <memory>
#include <vector>

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/insert_plan.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::executors {

class InsertExecutor : public AbstractExecutor {
public:
  InsertExecutor(ExecutorContext *exec_ctx, const plans::InsertPlanNode *plan,
                 std::unique_ptr<AbstractExecutor> &&child_executor);

  void init() override;

  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  const plans::InsertPlanNode *plan_;
  storage::table::TableHeap *table_heap_;
  std::unique_ptr<AbstractExecutor> child_executor_;

  size_t total_rows_inserted_;
  bool has_emitted_count_;
};

} // namespace turtle::execution::executors
