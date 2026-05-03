#pragma once

#include "turtle/common/record_id.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::execution::executors {

class AbstractExecutor {
public:
  virtual ~AbstractExecutor() = default;

  // Prepare the executors (e.g., resets iterators)
  virtual void init() = 0;

  // Yields the next tuple. Returns false if there are no more tuples.
  virtual bool next(storage::table::Tuple *tuple, RecordId *rid) = 0;
};

} // namespace turtle::execution::executors
