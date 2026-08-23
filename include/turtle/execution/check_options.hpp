#pragma once

#include <cstdint>
#include <unordered_set>

namespace turtle::execution {

enum class CheckOption : uint8_t {
  ENABLE_NLJ_CHECK = 0,
  ENABLE_TOPN_CHECK = 1,
};

/**
 * The CheckOptions class contains the set of check options used for testing
 * executor logic.
 */
class CheckOptions {
public:
  std::unordered_set<CheckOption> check_options_set_;
};

} // namespace turtle::execution
