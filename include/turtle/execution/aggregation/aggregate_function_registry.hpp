#include "turtle/execution/aggregation/aggregate_function.hpp"

namespace turtle::execution::aggregation {

class AggregateFunctionRegistry {
public:
  static auto get(const std::string &name) -> const AggregateFunction *;
};

} // namespace turtle::execution::aggregation
