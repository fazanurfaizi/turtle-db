#pragma once

namespace turtle::execution::aggregation {

// Represents the state stored inside the Hash Table for a single group
struct AggregateState {
  virtual ~AggregateState() = default;
};

} // namespace turtle::execution::aggregation
