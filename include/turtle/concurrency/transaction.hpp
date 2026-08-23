#pragma once

#include <fmt/format.h>
#include <vector>

#include "turtle/common/config.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::concurrency {

class TransactionManager;

/**
 * Transaction State
 */
enum class TransactionState { RUNNING = 0, TAINTED, COMMITED = 100, ABORTED };

/**
 * Transaction isolation level
 */
enum class IsolationLevel { READ_UNCOMMITTED, SNAPSHOT_ISOLATION, SERIALIZE };

class TableHeap;
class Catalog;

/**
 * Represents a link to a previous version of this tuple
 */
struct UndoLink {
  // Previous version cab be found in which txn
  TxnId prev_txn_{INVALID_TXN_ID};
  // The log index of the previous version in `prev_txn_`
  int prev_log_idx_{0};

  friend auto operator==(const UndoLink &a, const UndoLink &b) {
    return a.prev_txn_ == b.prev_txn_ && a.prev_log_idx_ == b.prev_log_idx_;
  }

  friend auto operator!=(const UndoLink &a, const UndoLink &b) {
    return !(a == b);
  }

  /* Checks if the undo link points to something. */
  auto IsValid() const -> bool { return prev_txn_ != INVALID_TXN_ID; }
};

struct UndoLog {
  // Whether this log is a deletion marker
  bool is_deleted_;
  // the fields modified by this undo log
  std::vector<bool> modified_fields_;
  // the modified fields
  storage::table::Tuple tuple_;
  // Timestamp of this undo log
  TimestampT ts_{INVALID_TS};
  // Undo log prev version
  UndoLink prev_version_{};
};

} // namespace turtle::concurrency
