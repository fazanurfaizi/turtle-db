#pragma once

#include <memory>
#include <vector>

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/delete_plan.hpp"
#include "turtle/storage/table/table_heap.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::executors {

class DeleteExecutor : public AbstractExecutor {
public:
  DeleteExecutor(ExecutorContext *exec_ctx, const plans::DeletePlanNode *plan,
                 std::unique_ptr<AbstractExecutor> &&child_executor);

  void init() override;

  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  const plans::DeletePlanNode *plan_;
  std::unique_ptr<AbstractExecutor> child_executor_;
  storage::table::TableHeap *table_heap_;

  size_t total_rows_deleted_;
  bool has_emitted_count_;
};

} // namespace turtle::execution::executors
