#pragma once

#include "turtle/execution/aggregation/accumulator/moment_accumulator.hpp"
#include "turtle/execution/aggregation/aggregate_state.hpp"
#include <vector>

namespace turtle::execution::aggregation {

// ==========================================
// AVG(column)
// ==========================================
class AvgFunction : public MomentAccumulatorBase {
  void update(AggregateState *state,
              const std::vector<datatype::Value> &args) const override;

  void merge(AggregateState *dest, const AggregateState *src) const override;

  auto finalize(const AggregateState *state) const -> datatype::Value override;
};

} // namespace turtle::execution::aggregation
