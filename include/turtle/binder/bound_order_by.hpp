#pragma once

#include "turtle/execution/expressions/abstract_expression.hpp"
#include <cstdint>
#include <tuple>

namespace turtle::binder {

/**
 * @brief All types of order-bys.
 */
enum class OrderByType : uint8_t {
  INVALID = 0, // Invalid order by type
  DEFAULT = 1, // Default order by type
  ASC = 2,     // Ascending order by type
  DESC = 3,    // Descending order by type
};

/**
 * @brief All types order by nulls.
 */
enum class OrderByNullType : uint8_t {
  DEFAULT = 0,     // Default order by type
  NULLS_FIRST = 1, // Ascending order by type
  NULLS_LAST = 2,  // Descending order by type
};

using OrderBy = std::tuple<OrderByType, OrderByNullType,
                           execution::expressions::AbstractExpressionRef>;

} // namespace turtle::binder
