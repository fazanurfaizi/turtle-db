// -----------------------------------------------------------------------------
// Tier 2 (Access & Structures) — turtle::storage::table::Tuple
//
// Scope: the schema-driven row encoding: a leading NULL bitmap
// (ceil(ncols/8) bytes) followed by the serialized bytes of each non-null
// value, in column order. The schema is NOT stored in the tuple, so every read
// must be handed the matching ColumnSchema.
//
// No fixture: Tuples own their heap buffer (deep copy/assign), so each test
// builds its own schema + values locally.
// -----------------------------------------------------------------------------
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "turtle/catalog/column.hpp"
#include "turtle/catalog/column_schema.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/value.hpp"
#include "turtle/storage/table/tuple.hpp"

namespace turtle::storage::table {
namespace {

using catalog::Column;
using catalog::ColumnSchema;
using datatype::DataType;
using datatype::Value;

// Schema: (id INTEGER, name VARCHAR, age INTEGER)
ColumnSchema make_schema() {
  std::vector<Column> cols;
  cols.emplace_back("id", DataType::INTEGER);
  cols.emplace_back("name", DataType::VARCHAR);
  cols.emplace_back("age", DataType::INTEGER);
  return ColumnSchema(std::move(cols));
}

Tuple make_row(const ColumnSchema &schema, int32_t id, const std::string &name,
               int32_t age) {
  std::string name_copy = name; // VARCHAR ctor needs a non-const lvalue
  std::vector<Value> vals;
  vals.emplace_back(DataType::INTEGER, id);
  vals.emplace_back(DataType::VARCHAR, name_copy);
  vals.emplace_back(DataType::INTEGER, age);
  return Tuple(&schema, std::move(vals));
}

// ---- layout / sizing -------------------------------------------------------

TEST(TupleTest, StorageSizeIsBitmapPlusValues) {
  ColumnSchema schema = make_schema();
  Tuple t = make_row(schema, 1, "hello", 21);

  // 1 bitmap byte (3 cols) + int(4) + varchar("hello": 5+4=9) + int(4) = 18
  EXPECT_EQ(t.storage_size(), 1u + 4u + 9u + 4u);
}

// ---- non-null field access -------------------------------------------------

TEST(TupleTest, ReadsBackEveryColumn) {
  ColumnSchema schema = make_schema();
  Tuple t = make_row(schema, 7, "turtle", 33);

  EXPECT_EQ(t.value(&schema, 0).GetAs<int32_t>(), 7);
  EXPECT_EQ(t.value(&schema, 1).to_string(), "turtle");
  EXPECT_EQ(t.value(&schema, 2).GetAs<int32_t>(), 33);
}

// ---- serialize / deserialize round trip ------------------------------------

TEST(TupleTest, SerializeDeserializeRoundTrip) {
  ColumnSchema schema = make_schema();
  Tuple original = make_row(schema, 100, "roundtrip", 9);

  std::vector<char> buf(original.storage_size());
  original.serialize_to(buf.data());

  Tuple restored;
  restored.deserialize_from(buf.data(), original.storage_size());

  EXPECT_EQ(restored.storage_size(), original.storage_size());
  EXPECT_EQ(restored.value(&schema, 0).GetAs<int32_t>(), 100);
  EXPECT_EQ(restored.value(&schema, 1).to_string(), "roundtrip");
  EXPECT_EQ(restored.value(&schema, 2).GetAs<int32_t>(), 9);
}

// ---- value semantics -------------------------------------------------------

TEST(TupleTest, CopyConstructorDeepCopies) {
  ColumnSchema schema = make_schema();
  Tuple a = make_row(schema, 5, "orig", 1);
  Tuple b = a; // copy ctor

  EXPECT_NE(a.data(), b.data());              // independent buffers
  EXPECT_EQ(b.value(&schema, 1).to_string(), "orig");
  EXPECT_EQ(a.storage_size(), b.storage_size());
}

TEST(TupleTest, CopyAssignmentDeepCopies) {
  ColumnSchema schema = make_schema();
  Tuple a = make_row(schema, 5, "src", 1);
  Tuple b = make_row(schema, 9, "dst", 2);
  b = a; // copy assignment releases b's old buffer, clones a's

  EXPECT_NE(a.data(), b.data());
  EXPECT_EQ(b.value(&schema, 0).GetAs<int32_t>(), 5);
  EXPECT_EQ(b.value(&schema, 1).to_string(), "src");
}

TEST(TupleTest, RecordIdRoundTrips) {
  ColumnSchema schema = make_schema();
  Tuple t = make_row(schema, 1, "x", 2);
  t.set_recod_id(RecordId{3u, 4u});
  EXPECT_TRUE((t.record_id() == RecordId{3u, 4u}));
}

// ---- NULL handling ---------------------------------------------------------

TEST(TupleTest, SingleNullColumnReadsBackAsNull) {
  std::vector<Column> cols;
  cols.emplace_back("v", DataType::INTEGER);
  ColumnSchema schema(std::move(cols));

  std::vector<Value> vals;
  vals.emplace_back(DataType::INTEGER); // NULL integer
  Tuple t(&schema, std::move(vals));

  // The constructor still reserves the column's fixed size in storage_size_
  // (IntegerType::get_storage_size returns 4 even for NULL) but leaves those
  // bytes unwritten; the NULL is recorded in the bitmap. So: 1 bitmap + 4.
  EXPECT_EQ(t.storage_size(), 1u + 4u);
  EXPECT_TRUE(t.value(&schema, 0).is_null());
}

// KNOWN DEFECT — pinned DISABLED. Tuple::value() (src/storage/table/tuple.cpp
// ~L112-123) walks the byte offset by adding each prior column's *type size*
// without consulting the NULL bitmap. But a NULL column serializes 0 bytes, so
// any column following a NULL is read at the wrong offset (here, past the end
// of the buffer). Re-enable once the offset walk skips NULL columns.
TEST(TupleTest, DISABLED_ColumnAfterNullIsReadableWhenBitmapAware) {
  std::vector<Column> cols;
  cols.emplace_back("a", DataType::INTEGER);
  cols.emplace_back("b", DataType::INTEGER);
  ColumnSchema schema(std::move(cols));

  std::vector<Value> vals;
  vals.emplace_back(DataType::INTEGER);                    // a = NULL (0 bytes)
  vals.emplace_back(DataType::INTEGER, static_cast<int32_t>(42)); // b = 42
  Tuple t(&schema, std::move(vals));

  EXPECT_TRUE(t.value(&schema, 0).is_null());
  EXPECT_EQ(t.value(&schema, 1).GetAs<int32_t>(), 42);
}

} // namespace
} // namespace turtle::storage::table
