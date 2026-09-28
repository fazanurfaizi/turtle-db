#include "turtle/execution/aggregation/aggregate_function_registry.hpp"
#include "turtle/execution/aggregation/functions/basic_functions.hpp"
#include "turtle/execution/aggregation/functions/moment_functions.hpp"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace turtle::execution::aggregation {

auto AggregateFunctionRegistry::get(const std::string &name)
    -> const AggregateFunction * {
  static std::unordered_map<std::string, std::unique_ptr<AggregateFunction>>
      registry;
  static std::once_flag flag;

  std::call_once(flag, []() {
    // Basic Functions
    registry["count"] = std::make_unique<CountFunction>();
    registry["count_star"] = std::make_unique<CountStarFunction>();
    registry["sum"] = std::make_unique<SumFunction>();
    registry["min"] = std::make_unique<MinFunction>();
    registry["max"] = std::make_unique<MaxFunction>();

    // Moment Functions
    registry["avg"] = std::make_unique<AvgFunction>();
  });

  auto it = registry.find(name);
  return it != registry.end() ? it->second.get() : nullptr;
}

} // namespace turtle::execution::aggregation
