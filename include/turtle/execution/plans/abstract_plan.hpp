#pragma once

#include "fmt/format.h"
#include "turtle/catalog/column_schema.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace turtle::execution::plans {

#define TURTLE_PLAN_NODE_CLONE_WITH_CHILDREN(cname)                            \
  auto clone_with_children(std::vector<AbstractPlanNodeRef> children) const    \
      -> std::unique_ptr<AbstractPlanNode>                                     \
          override {                                                           \
    auto plan_node = cname(*this);                                             \
    plan_node.children_ = children;                                            \
    return std::make_unique<cname>(std::move(plan_node));                      \
  }

/** PlanType represents the types of plans that we have in our system. */
enum class PlanType {
  SeqScan,
  IndexScan,
  Insert,
  Update,
  Delete,
  Aggregation,
  Limit,
  NestedLoopJoin,
  NestedIndexJoin,
  HashJoin,
  Filter,
  Values,
  Projection,
  Sort,
  TopN,
  TopNPerGroup,
  MockScan,
  InitCheck,
  Window
};

class AbstractPlanNode;
using AbstractPlanNodeRef = std::shared_ptr<const AbstractPlanNode>;

/**
 * AbstractPlanNode represents all the possible types of plan nodes in our
 * system. Plan nodes are modeled as trees, so each plan node can have a
 * variable number of children. Per the Volcano model, the plan node receives
 * the tuples of its children. The ordering of the children may matter.
 */
class AbstractPlanNode {
public:
  /**
   * The schema for the output of this plan node. In the volcano model, every
   * plan node will spit out tuples, and this tells you what schema this plan
   * node's tuples will have.
   */
  catalog::ColumnSchemaRef output_schema_;

  // The children of this plan node
  std::vector<AbstractPlanNodeRef> children_;

  /**
   * Create a new AbstractPlanNode with the specified output schema and
   * children.
   * @param output_schema The schema for the output of this plan node
   * @param children The children of this plan node
   */
  AbstractPlanNode(catalog::ColumnSchemaRef output_schema,
                   std::vector<AbstractPlanNodeRef> children)
      : output_schema_(std::move(output_schema)),
        children_(std::move(children)) {}

  /** Virtual destructor */
  virtual ~AbstractPlanNode() = default;

  /** @return the schema for the output of this plan node */
  auto output_schema() const -> const catalog::ColumnSchema & {
    return *this->output_schema_;
  }

  /** @return the child of this plan node at index child_idx */
  auto get_child_at(uint32_t child_idx) const -> AbstractPlanNodeRef {
    return this->children_[child_idx];
  }

  /** @return the children of this plan node */
  auto get_children() const -> const std::vector<AbstractPlanNodeRef> & {
    return this->children_;
  }

  /** @return the type of this plan node */
  virtual auto get_type() const -> PlanType = 0;

  /** @return the string representation of the plan node and its children */
  auto to_string(bool with_schema = true) const -> std::string {
    if (with_schema) {
      return fmt::format("{} | {}{}", this->plan_node_to_string(),
                         this->output_schema(),
                         this->children_to_string(2, with_schema));
    }
    return fmt::format("{}{}", this->plan_node_to_string(),
                       this->children_to_string(2, with_schema));
  }

  /** @return the cloned plan node with new children */
  virtual auto
  clone_with_children(std::vector<AbstractPlanNodeRef> children) const
      -> std::unique_ptr<AbstractPlanNode> = 0;

protected:
  /** @return the string representation of the plan node itself */
  virtual auto plan_node_to_string() const -> std::string {
    return "<unknown>";
  }

  /** @return the string representation of the plan node's children */
  auto children_to_string(int indent, bool with_schema = true) const
      -> std::string;
};

} // namespace turtle::execution::plans

template <typename T>
struct fmt::formatter<
    T, std::enable_if<std::is_base_of<
                          turtle::execution::plans::AbstractPlanNode, T>::value,
                      char>> : fmt::formatter<std::string> {
  template <typename FormatCtx> auto format(const T &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.to_string(), ctx);
  }
};

template <typename T>
struct fmt::formatter<
    std::unique_ptr<T>,
    std::enable_if<
        std::is_base_of<turtle::execution::plans::AbstractPlanNode, T>::value,
        char>> : fmt::formatter<std::string> {
  template <typename FormatCtx> auto format(const T &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.to_string(), ctx);
  }
};
