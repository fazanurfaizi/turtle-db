#pragma once

#include <memory>
#include <string>

#include "turtle/catalog/column_schema.hpp"
#include "turtle/common/config.hpp"
#include "turtle/storage/table/table_heap.hpp"

namespace turtle::catalog {

struct TableInfo {
  /**
   * Construct a new TableInfo.
   * @param name The table name
   * @param schema The table schema
   * @param table An owning pointer to the table heap
   * @param oid The unique OID for the table
   */
  TableInfo(std::string name, ColumnSchema schema,
            std::unique_ptr<storage::table::TableHeap> &&table, TableOid oid)
      : name_{std::move(name)}, schema_{std::move(schema)},
        table_{std::move(table)}, oid_(oid) {}

  /** The table name */
  const std::string name_;
  /** The table schema (columns) */
  ColumnSchema schema_;
  /** An owning pointer to the table heap */
  std::unique_ptr<storage::table::TableHeap> table_;
  /** The table OID */
  const TableOid oid_;
};

} // namespace turtle::catalog
