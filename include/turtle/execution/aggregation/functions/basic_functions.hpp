#pragma once

#include "turtle/datatype/type.hpp"
#include "turtle/execution/aggregation/accumulator/basic_accumulators.hpp"

namespace turtle::execution::aggregation {

// ==========================================
// COUNT(column)
// ==========================================
class CountFunction : public BasicAccumulatorBase {
public:
  void update(AggregateState *state,
              const std::vector<datatype::Value> &args) const override;

  void merge(AggregateState *dest, const AggregateState *src) const override;

  auto finalize(const AggregateState *state) const -> datatype::Value override;
};

// ==========================================
// COUNT(*)
// ==========================================
class CountStarFunction : public BasicAccumulatorBase {
public:
  void update(AggregateState *state,
              const std::vector<datatype::Value> & /*args*/) const override;

  void merge(AggregateState *dest, const AggregateState *src) const override;

  auto finalize(const AggregateState *state) const -> datatype::Value override;
};

// ==========================================
// SUM(column)
// ==========================================
class SumFunction : public BasicAccumulatorBase {
public:
  void update(AggregateState *state,
              const std::vector<datatype::Value> &args) const override;

  void merge(AggregateState *dest, const AggregateState *src) const override;

  auto finalize(const AggregateState *state) const -> datatype::Value override;
};

// ==========================================
// MIN(column)
// ==========================================
class MinFunction : public BasicAccumulatorBase {
public:
  void update(AggregateState *state,
              const std::vector<datatype::Value> &args) const override;

  void merge(AggregateState *dest, const AggregateState *src) const override;

  auto finalize(const AggregateState *state) const -> datatype::Value override;
};

// ==========================================
// MAX(column)
// ==========================================
class MaxFunction : public BasicAccumulatorBase {
public:
  void update(AggregateState *state,
              const std::vector<datatype::Value> &args) const override;

  void merge(AggregateState *dest, const AggregateState *src) const override;

  auto finalize(const AggregateState *state) const -> datatype::Value override;
};

} // namespace turtle::execution::aggregation
