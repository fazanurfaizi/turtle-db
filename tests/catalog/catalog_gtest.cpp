// -----------------------------------------------------------------------------
// Tier 2 (Access & Structures) — turtle::catalog (Catalog runtime table
// registry
// + Column / ColumnSchema)
//
// Scope: the metadata layer executors resolve tables through. Focus is the
// runtime table registry (create_table / get_table by OID and by name), which
// is what ExecutorContext depends on. Each created table gets its own backing
// .tbl file and TableHeap, so this is also a light integration test of the
// catalog -> heap wiring.
//
// Fixture owns DiskManager + BPM + a scratch dir that also hosts the catalog
// file; the Catalog is rebuilt per test. The Catalog dtor flushes to disk, so
// the scratch dir must exist for the lifetime of the object.
// -----------------------------------------------------------------------------
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "turtle/buffer/buffer_pool_manager.hpp"
#include "turtle/catalog/catalog.hpp"
#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/storage/disk/disk_manager.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::catalog {
namespace {

using datatype::DataType;
using datatype::Value;

ColumnSchema people_schema() {
  std::vector<Column> cols;
  cols.emplace_back("id", DataType::INTEGER);
  cols.emplace_back("name", DataType::VARCHAR);
  return ColumnSchema(std::move(cols));
}

class CatalogTest : public ::testing::Test {
protected:
  void SetUp() override {
    dir_ = std::filesystem::path("test_db") / "catalog_gtest";
    std::filesystem::remove_all(dir_);
    std::filesystem::create_directories(dir_);

    disk_ = std::make_unique<storage::disk::DiskManager>();
    bpm_ = std::make_unique<buffer::BufferPoolManager>(16, disk_.get());
    catalog_ = std::make_unique<Catalog>(bpm_.get(), disk_.get(),
                                         (dir_ / "catalog.cat").string(),
                                         dir_.string());
  }

  void TearDown() override {
    catalog_.reset(); // flushes to disk while dir_ still exists
    bpm_.reset();
    disk_.reset();
    std::filesystem::remove_all(dir_);
  }

  std::filesystem::path dir_;
  std::unique_ptr<storage::disk::DiskManager> disk_;
  std::unique_ptr<buffer::BufferPoolManager> bpm_;
  std::unique_ptr<Catalog> catalog_;
};

// ---- create_table ----------------------------------------------------------

TEST_F(CatalogTest, CreateTableReturnsPopulatedInfo) {
  TableInfo *info = catalog_->create_table("people", people_schema());
  ASSERT_NE(info, nullptr);
  EXPECT_EQ(info->name_, "people");
  EXPECT_EQ(info->oid_, 0u); // first OID
  ASSERT_NE(info->table_, nullptr);
  EXPECT_EQ(info->schema_.get_column_count(), 2u);
  EXPECT_EQ(info->schema_.get_columns()[0].get_name(), "id");
}

TEST_F(CatalogTest, OidsAreAssignedMonotonically) {
  TableInfo *a = catalog_->create_table("a", people_schema());
  TableInfo *b = catalog_->create_table("b", people_schema());
  TableInfo *c = catalog_->create_table("c", people_schema());
  EXPECT_EQ(a->oid_, 0u);
  EXPECT_EQ(b->oid_, 1u);
  EXPECT_EQ(c->oid_, 2u);
}

TEST_F(CatalogTest, CreateTableIsIdempotentByName) {
  TableInfo *first = catalog_->create_table("dup", people_schema());
  TableInfo *again = catalog_->create_table("dup", people_schema());
  EXPECT_EQ(first, again); // same handle
  EXPECT_EQ(first->oid_, again->oid_);
}

// ---- lookup ----------------------------------------------------------------

TEST_F(CatalogTest, GetTableResolvesByOidAndName) {
  TableInfo *created = catalog_->create_table("people", people_schema());
  EXPECT_EQ(catalog_->get_table(created->oid_), created);
  EXPECT_EQ(catalog_->get_table("people"), created);
}

TEST_F(CatalogTest, GetUnknownTableReturnsNullptr) {
  EXPECT_EQ(catalog_->get_table(TableOid{999}), nullptr);
  EXPECT_EQ(catalog_->get_table("nope"), nullptr);
}

// ---- catalog -> heap wiring (integration) ----------------------------------

TEST_F(CatalogTest, TableHeapFromCatalogIsUsable) {
  TableInfo *info = catalog_->create_table("people", people_schema());

  std::string name = "carol";
  std::vector<Value> vals;
  vals.emplace_back(DataType::INTEGER, static_cast<int32_t>(7));
  vals.emplace_back(DataType::VARCHAR, name);
  storage::table::Tuple t(&info->schema_, std::move(vals));

  RecordId rid;
  ASSERT_TRUE(info->table_->insert_tuple(t, &rid));

  storage::table::Tuple fetched;
  ASSERT_TRUE(info->table_->get_tuple(rid, &fetched));
  EXPECT_EQ(fetched.value(&info->schema_, 0).get_as<int32_t>(), 7);
  EXPECT_EQ(fetched.value(&info->schema_, 1).to_string(), "carol");
}

// ---- database registry (secondary) -----------------------------------------

TEST_F(CatalogTest, CreateDatabaseThenLookup) {
  Database *db = catalog_->create_database("shop");
  ASSERT_NE(db, nullptr);
  EXPECT_EQ(catalog_->get_database("shop"), db);
  // Re-creating the same database name is rejected (returns nullptr).
  EXPECT_EQ(catalog_->create_database("shop"), nullptr);
}

} // namespace
} // namespace turtle::catalog
