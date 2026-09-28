#pragma once

#include "turtle/execution/aggregation/functions/moment_functions.hpp"
#include "turtle/datatype/value_factory.hpp"
#include "turtle/execution/aggregation/accumulator/moment_accumulator.hpp"
#include "turtle/execution/aggregation/aggregate_state.hpp"
#include <vector>

namespace turtle::execution::aggregation {

// ==========================================
// AVG(column)
// ==========================================
void AvgFunction::update(AggregateState *state,
                         const std::vector<datatype::Value> &args) const {
  if (args.empty() || this->check_cmp_is_null(args[0]))
    return;

  auto *st = static_cast<MomentAccumulatorState *>(state);
  if (st->is_null == datatype::CmpBool::CmpTrue) {
    st->sum = args[0];
    st->is_null = datatype::CmpBool::CmpFalse;
  } else {
    st->sum = st->sum.add(args[0]);
  }
  st->count++;
}

void AvgFunction::merge(AggregateState *dest, const AggregateState *src) const {
  auto *d = static_cast<MomentAccumulatorState *>(dest);
  auto *s = static_cast<const MomentAccumulatorState *>(src);
  if (s->is_null == datatype::CmpBool::CmpTrue)
    return;

  if (d->is_null == datatype::CmpBool::CmpTrue) {
    d->sum = s->sum;
    d->is_null = datatype::CmpBool::CmpFalse;
  } else {
    d->sum = d->sum.add(s->sum);
  }
  d->count += s->count;
}

auto AvgFunction::finalize(const AggregateState *state) const
    -> datatype::Value {
  auto *st = static_cast<const MomentAccumulatorState *>(state);
  if (st->is_null == datatype::CmpBool::CmpTrue || st->count == 0) {
    return datatype::ValueFactory::get_null_value_by_type(st->sum.data_type());
  }

  auto count_val =
      datatype::ValueFactory::get_decimal_value(static_cast<double>(st->count));

  return st->sum.divide(count_val);
}

} // namespace turtle::execution::aggregation
