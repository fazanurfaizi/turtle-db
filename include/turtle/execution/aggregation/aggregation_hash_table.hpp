#pragma once

#include "turtle/datatype/value.hpp"
#include "turtle/execution/aggregation/aggregate_function.hpp"
#include "turtle/execution/aggregation/aggregate_key.hpp"
#include <unordered_map>
#include <vector>

namespace turtle::execution::aggregation {

/**
 * @brief Dynamic Hash Table for DB Aggregation
 */
class AggregationHashTable {
public:
  explicit AggregationHashTable(
      std::vector<const AggregateFunction *> agg_funcs)
      : agg_funcs_(agg_funcs) {}

  /**
   * @brief Evaluates expressions and updates state accumulators for an
   * AggregateKey
   */
  void insert_combine(const AggregateKey &key,
                      const std::vector<datatype::Value> &args) {
    auto it = this->ht_.find(key);
    if (it == this->ht_.end()) {
      // Allocate new states for a newly encountered key
      AggregateValue val;
      val.states_.reserve(this->agg_funcs_.size());
      for (const auto *func : this->agg_funcs_) {
        val.states_.push_back(func->create_state());
      }
      it = this->ht_.emplace(key, std::move(val)).first;
    }

    // Delegate computation to each function's update method
    for (size_t i = 0; i < this->agg_funcs_.size(); ++i) {
      // input args per aggregate function call
      if (i < args.size()) {
        std::vector<datatype::Value> input_args = {args[i]};
        this->agg_funcs_[i]->update(it->second.states_[i].get(), input_args);
      }
    }
  }

  /** @return Iterator accessors for scan execution */
  auto begin() -> std::unordered_map<AggregateKey, AggregateValue>::iterator {
    return this->ht_.begin();
  }

  auto end() -> std::unordered_map<AggregateKey, AggregateValue>::iterator {
    return this->ht_.end();
  }

  auto empty() const -> bool { return this->ht_.empty(); }
  void clear() { this->ht_.clear(); }

private:
  std::unordered_map<AggregateKey, AggregateValue> ht_;
  std::vector<const AggregateFunction *> agg_funcs_;
};

} // namespace turtle::execution::aggregation
