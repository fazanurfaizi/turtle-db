#pragma once

#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/aggregation/aggregate_function.hpp"
#include "turtle/execution/aggregation/aggregate_state.hpp"
#include <memory>

namespace turtle::execution::aggregation {

/**
 * @brief Single-value state used by statistical moments functions (AVG).
 */
struct MomentAccumulatorState : public AggregateState {
  datatype::Value sum{datatype::DataType::INVALID};
  uint64_t count{0};
  datatype::CmpBool is_null{datatype::CmpBool::CmpTrue};
};

/**
 * @brief Abstract base class for single-argument moment aggregates.
 */
class MomentAccumulatorBase : public AggregateFunction {
public:
  auto create_state() const -> std::unique_ptr<AggregateState> override {
    return std::make_unique<MomentAccumulatorState>();
  }

  void merge(AggregateState *dest,
             const AggregateState *src) const override = 0;

  void update(AggregateState *state,
              const std::vector<datatype::Value> &args) const override = 0;

  auto finalize(const AggregateState *state) const
      -> datatype::Value override = 0;
};

} // namespace turtle::execution::aggregation
