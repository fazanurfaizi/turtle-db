#include "turtle/execution/aggregation/functions/basic_functions.hpp"
#include "turtle/datatype/value_factory.hpp"

namespace turtle::execution::aggregation {

// ==========================================
// COUNT(column)
// ==========================================
void CountFunction::update(AggregateState *state,
                           const std::vector<datatype::Value> &args) const {
  if (args.empty() || this->check_cmp_is_null(args[0]))
    return;

  auto *st = static_cast<BasicAccumulatorState *>(state);
  st->count++;
  st->is_null = datatype::CmpBool::CmpFalse;
}

void CountFunction::merge(AggregateState *dest,
                          const AggregateState *src) const {
  auto *d = static_cast<BasicAccumulatorState *>(dest);
  auto *s = static_cast<const BasicAccumulatorState *>(src);
  if (s->is_null == datatype::CmpBool::CmpNull)
    return;

  if (d->is_null == datatype::CmpBool::CmpNull) {
    d->value = s->value;
    d->count = s->count;
    d->is_null = datatype::CmpBool::CmpFalse;
  } else {
    d->count += s->count;
  }
}

auto CountFunction::finalize(const AggregateState *state) const
    -> datatype::Value {
  auto *st = static_cast<const BasicAccumulatorState *>(state);
  return datatype::ValueFactory::get_integer_value(
      static_cast<int32_t>(st->count));
}

// ==========================================
// COUNT(*)
// ==========================================
void CountStarFunction::update(
    AggregateState *state,
    const std::vector<datatype::Value> & /*args*/) const {
  auto *st = static_cast<BasicAccumulatorState *>(state);
  st->count++;
  st->is_null = datatype::CmpBool::CmpFalse;
}

void CountStarFunction::merge(AggregateState *dest,
                              const AggregateState *src) const {
  auto *d = static_cast<BasicAccumulatorState *>(dest);
  auto *s = static_cast<const BasicAccumulatorState *>(src);
  if (s->is_null == datatype::CmpBool::CmpNull)
    return;

  if (d->is_null == datatype::CmpBool::CmpNull) {
    d->value = s->value;
    d->count = s->count;
    d->is_null = datatype::CmpBool::CmpFalse;
  } else {
    d->count += s->count;
  }
}

auto CountStarFunction::finalize(const AggregateState *state) const
    -> datatype::Value {
  auto *st = static_cast<const BasicAccumulatorState *>(state);
  return datatype::ValueFactory::get_big_int_value(
      static_cast<int64_t>(st->count));
}

// ==========================================
// SUM(column)
// ==========================================
void SumFunction::update(AggregateState *state,
                         const std::vector<datatype::Value> &args) const {
  if (args.empty() || this->check_cmp_is_null(args[0]))
    return;

  auto *st = static_cast<BasicAccumulatorState *>(state);
  if (st->is_null == datatype::CmpBool::CmpTrue) {
    st->value = args[0];
    st->is_null = datatype::CmpBool::CmpFalse;
  } else {
    st->value = st->value.add(args[0]);
  }
}

void SumFunction::merge(AggregateState *dest, const AggregateState *src) const {
  auto *d = static_cast<BasicAccumulatorState *>(dest);
  auto *s = static_cast<const BasicAccumulatorState *>(src);
  if (s->is_null == datatype::CmpBool::CmpTrue)
    return;

  if (d->is_null == datatype::CmpBool::CmpTrue) {
    d->value = s->value;
    d->is_null = datatype::CmpBool::CmpFalse;
  } else {
    d->value = d->value.add(s->value);
  }
}

auto SumFunction::finalize(const AggregateState *state) const
    -> datatype::Value {
  auto *st = static_cast<const BasicAccumulatorState *>(state);
  if (st->is_null == datatype::CmpBool::CmpTrue) {
    return datatype::ValueFactory::get_null_value_by_type(
        st->value.data_type());
  }
  return st->value;
}

// ==========================================
// MIN(column)
// ==========================================
void MinFunction::update(AggregateState *state,
                         const std::vector<datatype::Value> &args) const {
  if (args.empty() || this->check_cmp_is_null(args[0]))
    return;

  auto *st = static_cast<BasicAccumulatorState *>(state);
  if (st->is_null == datatype::CmpBool::CmpTrue) {
    st->value = args[0];
    st->is_null = datatype::CmpBool::CmpFalse;
  } else {
    st->value = st->value.min(args[0]);
  }
}

void MinFunction::merge(AggregateState *dest, const AggregateState *src) const {
  auto *d = static_cast<BasicAccumulatorState *>(dest);
  auto *s = static_cast<const BasicAccumulatorState *>(src);
  if (s->is_null == datatype::CmpBool::CmpTrue)
    return;

  if (d->is_null == datatype::CmpBool::CmpTrue) {
    d->value = s->value;
    d->is_null = datatype::CmpBool::CmpFalse;
  } else {
    d->value = d->value.min(s->value);
  }
}

auto MinFunction::finalize(const AggregateState *state) const
    -> datatype::Value {
  auto *st = static_cast<const BasicAccumulatorState *>(state);
  if (st->is_null == datatype::CmpBool::CmpTrue) {
    return datatype::ValueFactory::get_null_value_by_type(
        st->value.data_type());
  }
  return st->value;
}

// ==========================================
// MAX(column)
// ==========================================
void MaxFunction::update(AggregateState *state,
                         const std::vector<datatype::Value> &args) const {
  if (args.empty() || this->check_cmp_is_null(args[0]))
    return;

  auto *st = static_cast<BasicAccumulatorState *>(state);
  if (st->is_null == datatype::CmpBool::CmpTrue) {
    st->value = args[0];
    st->is_null = datatype::CmpBool::CmpFalse;
  } else {
    st->value = st->value.max(args[0]);
  }
}

void MaxFunction::merge(AggregateState *dest, const AggregateState *src) const {
  auto *d = static_cast<BasicAccumulatorState *>(dest);
  auto *s = static_cast<const BasicAccumulatorState *>(src);
  if (s->is_null == datatype::CmpBool::CmpTrue)
    return;

  if (d->is_null == datatype::CmpBool::CmpTrue) {
    d->value = s->value;
    d->is_null = datatype::CmpBool::CmpFalse;
  } else {
    d->value = d->value.max(s->value);
  }
}

auto MaxFunction::finalize(const AggregateState *state) const
    -> datatype::Value {
  auto *st = static_cast<const BasicAccumulatorState *>(state);
  if (st->is_null == datatype::CmpBool::CmpTrue) {
    return datatype::ValueFactory::get_null_value_by_type(
        st->value.data_type());
  }
  return st->value;
}

} // namespace turtle::execution::aggregation
