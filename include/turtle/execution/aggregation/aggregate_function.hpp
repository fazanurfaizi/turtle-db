
#pragma once

#include "turtle/datatype/value.hpp"
#include "turtle/execution/aggregation/aggregate_state.hpp"
#include <memory>
#include <vector>

namespace turtle::execution::aggregation {

class AggregateFunction {
public:
  virtual ~AggregateFunction() = default;

  // Initialize state in the hash table bucket
  virtual auto create_state() const -> std::unique_ptr<AggregateState> = 0;

  // Process incoming row(s)
  virtual void update(AggregateState *state,
                      const std::vector<datatype::Value> &args) const = 0;

  // Merge another state (for parallel aggregation)
  virtual void merge(AggregateState *dest, const AggregateState *src) const = 0;

  // Compute final output value (e.g., standard deviation from count/sum/sum_sq)
  virtual auto finalize(const AggregateState *state) const
      -> datatype::Value = 0;

  auto check_cmp_is_null(const datatype::Value &value) const -> bool {
    return datatype::get_cmp_bool(value.is_null()) ==
           datatype::CmpBool::CmpTrue;
  }
};

} // namespace turtle::execution::aggregation
