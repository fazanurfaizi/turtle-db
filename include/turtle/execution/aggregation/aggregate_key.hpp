#pragma once

#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/aggregation/aggregate_state.hpp"
#include <memory>
#include <vector>

namespace turtle::execution::aggregation {

/**
 * @brief Key used to group rows in the Hash table.
 */
struct AggregateKey {
  std::vector<datatype::Value> group_bys_;

  auto operator==(const AggregateKey &other) const -> bool {
    for (uint32_t i = 0; i < group_bys_.size(); ++i) {
      if (group_bys_[i].compare_equals(other.group_bys_[i]) !=
          datatype::CmpBool::CmpTrue) {
        return false;
      }
    }
    return true;
  }
};

/**
 * @brief Holds polymorphic states for all running aggregate functions for a
 * single key.
 */
struct AggregateValue {
  std::vector<std::unique_ptr<AggregateState>> states_;
};

} // namespace turtle::execution::aggregation

namespace std {

template <> struct hash<turtle::execution::aggregation::AggregateKey> {
  auto
  operator()(const turtle::execution::aggregation::AggregateKey &agg_key) const
      -> std::size_t {
    size_t curr_hash = 0;
    for (const auto &key : agg_key.group_bys_) {
      if (!key.is_null()) {
        curr_hash ^= std::hash<std::string>{}(key.to_string()) + 0x9e3779b9 +
                     (curr_hash << 6) + (curr_hash >> 2);
      }
    }
    return curr_hash;
  }
};

} // namespace std
