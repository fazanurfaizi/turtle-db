#pragma once

#include <deque>
#include <memory>
#include <utility>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/catalog.hpp"
#include "turtle/common/macros.hpp"
#include "turtle/execution/check_options.hpp"

namespace turtle::execution {

class AbstractExecutor;

using NljChecker =
    std::deque<std::pair<AbstractExecutor *, AbstractExecutor *>>;

class ExecutorContext {
public:
  /**
   * Creates an ExecutorContext for the transaction that is executing the query.
   * @param catalog The catalog that the executor uses
   * @param bpm The buffer pool manager that the executor uses
   */
  ExecutorContext(catalog::Catalog *catalog, buffer::BufferPoolManager *bpm,
                  bool is_delete)
      : catalog_{catalog}, bpm_{bpm}, is_delete_{is_delete} {
    this->nlj_check_exec_set_ =
        std::deque<std::pair<AbstractExecutor *, AbstractExecutor *>>(
            std::deque<std::pair<AbstractExecutor *, AbstractExecutor *>>());
    this->check_options_ = std::make_shared<CheckOptions>();
  }

  ~ExecutorContext() = default;

  DISALLOW_COPY_AND_MOVE(ExecutorContext)

  auto get_catalog() -> catalog::Catalog * { return this->catalog_; }

  auto get_buffer_pool_manager() -> buffer::BufferPoolManager * {
    return this->bpm_;
  }

  auto get_nlj_check_executor_set() -> NljChecker & {
    return this->nlj_check_exec_set_;
  }

  auto get_check_options() -> std::shared_ptr<CheckOptions> {
    return this->check_options_;
  }

  void add_check_executor(AbstractExecutor *left_exec,
                          AbstractExecutor *right_exec) {
    this->nlj_check_exec_set_.emplace_back(left_exec, right_exec);
  }

  void init_check_options(std::shared_ptr<CheckOptions> &&check_options) {
    TURTLE_ASSERT(check_options, "nullptr");
    this->check_options_ = std::move(check_options);
  }

  auto is_delete() const -> bool { return this->is_delete_; }

private:
  /** The database catalog associated with this executor context */
  catalog::Catalog *catalog_;
  /** The buffer pool manager associated with this executor context */
  buffer::BufferPoolManager *bpm_;
  /** The set of NLJ check executors associated with this executor context */
  NljChecker nlj_check_exec_set_;
  /** The set of check options associated with this executor context */
  std::shared_ptr<CheckOptions> check_options_;
  bool is_delete_;
};

} // namespace turtle::execution
