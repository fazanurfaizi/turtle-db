#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/executor_context.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/executors/values_executor.hpp"
#include "turtle/execution/plans/values_plan.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <vector>

namespace turtle::execution::executors {

/**
 * Construct a new ValuesExecutor instance.
 * @param exec_ctx The executor context
 * @param plan The values plan to be executed
 */
ValuesExecutor::ValuesExecutor(ExecutorContext *exec_ctx,
                               const plans::ValuesPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan),
      dummy_schema_(catalog::ColumnSchema({})) {}

/** Initialize the values */
void ValuesExecutor::init() { cursor_ = 0; }

/**
 * Yield the next tuple batch from the values.
 * @param tuple_batch The next tuple batch produced by the values
 * @param rid_batch The next tuple RecordID batch produced by the values
 * @param batch_size The number of tuples to be included in the batch
 * @return `true` if tuple was produced, `false` if there are no more tuples
 */
auto ValuesExecutor::next(std::vector<storage::table::Tuple> *tuple_batch,
                          std::vector<RecordId> *rid_batch, size_t batch_size)
    -> bool {
  tuple_batch->clear();
  rid_batch->clear();

  while (tuple_batch->size() < batch_size &&
         this->cursor_ < this->plan_->get_values().size()) {
    std::vector<datatype::Value> values{};
    values.reserve(this->get_output_schema().get_column_count());

    const auto &row_expr = this->plan_->get_values()[this->cursor_];
    for (const auto &col : row_expr) {
      values.push_back(col->evaluate(nullptr, this->dummy_schema_));
    }

    tuple_batch->emplace_back(&this->get_output_schema(), values);
    rid_batch->emplace_back(RecordId{});
    this->cursor_++;
  }
  return !tuple_batch->empty();
}

} // namespace turtle::execution::executors
