#pragma once

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <vector>

namespace turtle::execution {
class ExecutorContext;
}

namespace turtle::execution::executors {

using turtle::execution::ExecutorContext;

class AbstractExecutor {
public:
  explicit AbstractExecutor(ExecutorContext *exec_ctx) : exec_ctx_{exec_ctx} {};
  virtual ~AbstractExecutor() = default;

  // Prepare the executors (e.g., resets iterators)
  virtual void init() = 0;

  // Yields the next tuple. Returns false if there are no more tuples.
  virtual auto next(std::vector<storage::table::Tuple> *tuple_batch,
                    std::vector<RecordId> *record_id_batch, size_t batch_size)
      -> bool = 0;

  virtual auto get_output_schema() const -> const catalog::ColumnSchema & = 0;

  auto get_executor_context() -> ExecutorContext * { return this->exec_ctx_; }

protected:
  ExecutorContext *exec_ctx_;
};

} // namespace turtle::execution::executors
