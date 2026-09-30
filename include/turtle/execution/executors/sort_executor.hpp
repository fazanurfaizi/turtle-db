#pragma once

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/execution/executors/abstract_executor.hpp"
#include "turtle/execution/plans/sort_plan.hpp"
#include "turtle/storage/table/tuple.hpp"
#include <memory>
#include <vector>

namespace turtle::execution::executors {

struct SortEntry {
  storage::table::Tuple tuple;
  RecordId rid;
  std::vector<datatype::Value> sort_keys;
};

class SortComparator {
public:
  explicit SortComparator(std::vector<plans::OrderBy> order_bys)
      : order_bys_(order_bys) {}

  bool operator()(const SortEntry &entry_a, const SortEntry &entry_b) {
    for (size_t i = 0; i < this->order_bys_.size(); ++i) {
      auto &val_a = entry_a.sort_keys[i];
      auto &val_b = entry_b.sort_keys[i];

      const auto order_type = std::get<0>(this->order_bys_[i]);
      const auto null_type = std::get<1>(this->order_bys_[i]);

      const bool a_is_null = val_a.is_null();
      const bool b_is_null = val_b.is_null();

      // Handle cases where nulls are involved
      if (a_is_null || b_is_null) {
        if (a_is_null && b_is_null) {
          continue;
        }

        // Determine if NULLS should come FIRST
        bool nulls_first = false;
        if (null_type == plans::OrderByNullType::NULLS_FIRST) {
          nulls_first = true;
        } else if (null_type == plans::OrderByNullType::NULLS_LAST) {
          nulls_first = false;
        } else {
          // Default SQL behavior:
          // ASC -> NULLS LAST, DESC -> NULL FIRST
          nulls_first = (order_type == plans::OrderByType::DESC);
        }

        if (a_is_null) {
          return nulls_first;
        }
        return !nulls_first;
      }

      // Check equality for non-null values
      if (val_a.compare_equals(val_a) == datatype::CmpBool::CmpTrue) {
        continue;
      }

      const bool is_asc = (order_type == plans::OrderByType::ASC ||
                           order_type == plans::OrderByType::DEFAULT);

      if (is_asc) {
        return val_a.compare_less_than(val_b) == datatype::CmpBool::CmpTrue;
      } else {
        return val_a.compare_greater_than(val_b) == datatype::CmpBool::CmpTrue;
      }
    }

    return false;
  };

private:
  std::vector<plans::OrderBy> order_bys_;
};

/**
 * The SortExecutor executor executes a sort.
 */
class SortExecutor : public AbstractExecutor {
public:
  SortExecutor(ExecutorContext *exec_ctx, const plans::SortPlanNode *plan,
               std::unique_ptr<AbstractExecutor> &&child_executor);

  void init() override;

  auto next(std::vector<storage::table::Tuple> *tuple_batch,
            std::vector<RecordId> *rid_batch, size_t batch_size)
      -> bool override;

  /** @return The output schema for the sort */
  auto get_output_schema() const -> const catalog::ColumnSchema & override {
    return this->plan_->output_schema();
  }

private:
  /** The sort plan node to be executed */
  const plans::SortPlanNode *plan_;

  /** The child executor from which tuples are obtained */
  std::unique_ptr<AbstractExecutor> child_executor_;

  std::vector<SortEntry> sorted_entries_;

  /** Track current position for emit phase */
  size_t cursor_{0};
};

} // namespace turtle::execution::executors
