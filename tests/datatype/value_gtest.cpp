// -----------------------------------------------------------------------------
// Tier 2 (Access & Structures) — turtle::datatype (Value + Type singletons)
//
// Scope: the dynamic type system every tuple and expression is built on. Value
// holds no logic itself — it forwards to the Type singleton for its DataType —
// so these tests double as coverage of IntegerType and VarcharType.
//
// No fixture: Values are self-contained (VARCHAR owns/copies its bytes). The
// one exception is deserialization, where the decoded VARCHAR is a *non-owning*
// view into the caller's storage buffer, so that buffer is kept alive for the
// duration of the assertion.
//
// Note: the tree builds Release (NDEBUG), so the assert()-based type guards in
// IntegerType are compiled out; tests therefore stick to well-typed operands
// and exercise the runtime-throwing error paths (divide-by-zero, VARCHAR math).
// -----------------------------------------------------------------------------
#include <array>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "turtle/catalog/column.hpp"
#include "turtle/datatype/data_types.hpp"
#include "turtle/datatype/type.hpp"
#include "turtle/datatype/value.hpp"

namespace turtle::datatype {
namespace {

// ============================ INTEGER =======================================

TEST(IntegerValueTest, StoresAndReportsScalar) {
  Value v(DataType::INTEGER, static_cast<int32_t>(42));
  EXPECT_EQ(v.data_type(), DataType::INTEGER);
  EXPECT_FALSE(v.is_null());
  EXPECT_EQ(v.GetAs<int32_t>(), 42);
  EXPECT_EQ(v.storage_size(), sizeof(int32_t));
  EXPECT_EQ(v.to_string(), "42");
}

TEST(IntegerValueTest, TypeOnlyCtorIsNull) {
  Value v(DataType::INTEGER);
  EXPECT_TRUE(v.is_null());
  EXPECT_EQ(v.to_string(), "integer_null");
}

TEST(IntegerValueTest, SerializeDeserializeRoundTrip) {
  Value original(DataType::INTEGER, static_cast<int32_t>(-12345));
  std::array<char, sizeof(int32_t)> buf{};
  original.SerializeTo(buf.data());

  Value restored = Value::DeserializeFrom(buf.data(), DataType::INTEGER);
  EXPECT_EQ(restored.GetAs<int32_t>(), -12345);
  EXPECT_TRUE(original.compare_exactly_equals(restored));
}

TEST(IntegerValueTest, ComparisonsReflectOrdering) {
  Value five(DataType::INTEGER, static_cast<int32_t>(5));
  Value nine(DataType::INTEGER, static_cast<int32_t>(9));
  Value five2(DataType::INTEGER, static_cast<int32_t>(5));

  EXPECT_EQ(five.compare_equals(five2), CmpBool::CmpTrue);
  EXPECT_EQ(five.compare_equals(nine), CmpBool::CmpFalse);
  EXPECT_EQ(five.compare_not_equals(nine), CmpBool::CmpTrue);
  EXPECT_EQ(five.compare_less_than(nine), CmpBool::CmpTrue);
  EXPECT_EQ(nine.compare_greater_than(five), CmpBool::CmpTrue);
  EXPECT_EQ(five.compare_less_than_equals(five2), CmpBool::CmpTrue);
  EXPECT_EQ(five.compare_greater_than_equals(five2), CmpBool::CmpTrue);
}

TEST(IntegerValueTest, ComparisonWithNullYieldsCmpNull) {
  Value real(DataType::INTEGER, static_cast<int32_t>(1));
  Value null_int(DataType::INTEGER);
  EXPECT_EQ(real.compare_equals(null_int), CmpBool::CmpNull);
}

TEST(IntegerValueTest, ExactEqualsTreatsTwoNullsAsEqual) {
  Value a(DataType::INTEGER);
  Value b(DataType::INTEGER);
  EXPECT_TRUE(a.compare_exactly_equals(b));
}

// KNOWN DEFECT — pinned DISABLED (gtest "known bug" idiom).
// The arithmetic kernels add_value/subtract_value/multiply_value/divide_value/
// modulo_value in include/turtle/datatype/integer_parent_type.hpp (lines ~80-189)
// have their ENTIRE bodies commented out. Each is declared `-> Value` but
// contains no return statement, so IntegerType::add/subtract/... return an
// ill-formed Value (undefined behavior). Re-enable this test once the kernels
// are implemented; the expected results are documented here as the spec.
TEST(IntegerValueTest, DISABLED_ArithmeticProducesCorrectResults) {
  Value a(DataType::INTEGER, static_cast<int32_t>(7));
  Value b(DataType::INTEGER, static_cast<int32_t>(5));
  EXPECT_EQ(a.add(b).GetAs<int32_t>(), 12);
  EXPECT_EQ(a.subtract(b).GetAs<int32_t>(), 2);
  EXPECT_EQ(a.multiply(b).GetAs<int32_t>(), 35);
  EXPECT_EQ(a.divide(b).GetAs<int32_t>(), 1);
  EXPECT_EQ(a.modulo(b).GetAs<int32_t>(), 2);
}

// The divide/modulo *guards* ARE implemented (they run before the unimplemented
// kernel), so the by-zero error path is genuinely exercisable and correct.
TEST(IntegerValueTest, DivideByZeroThrows) {
  Value a(DataType::INTEGER, static_cast<int32_t>(10));
  Value zero(DataType::INTEGER, static_cast<int32_t>(0));
  EXPECT_THROW(a.divide(zero), std::runtime_error);
  EXPECT_THROW(a.modulo(zero), std::runtime_error);
}

TEST(IntegerValueTest, MinMax) {
  Value a(DataType::INTEGER, static_cast<int32_t>(3));
  Value b(DataType::INTEGER, static_cast<int32_t>(8));
  EXPECT_EQ(a.min(b).GetAs<int32_t>(), 3);
  EXPECT_EQ(a.max(b).GetAs<int32_t>(), 8);
}

TEST(IntegerValueTest, CopyProducesEqualIndependentValue) {
  Value a(DataType::INTEGER, static_cast<int32_t>(99));
  Value c = a.copy();
  EXPECT_EQ(c.GetAs<int32_t>(), 99);
  EXPECT_TRUE(a.compare_exactly_equals(c));
}

TEST(IntegerValueTest, CastToWiderTypes) {
  Value i(DataType::INTEGER, static_cast<int32_t>(1000));

  Value as_big = i.cast_as(DataType::BIGINT);
  EXPECT_EQ(as_big.data_type(), DataType::BIGINT);
  EXPECT_EQ(as_big.GetAs<int64_t>(), 1000);

  Value as_dec = i.cast_as(DataType::DECIMAL);
  EXPECT_EQ(as_dec.data_type(), DataType::DECIMAL);
  EXPECT_DOUBLE_EQ(as_dec.GetAs<double>(), 1000.0);
}

// ============================ VARCHAR =======================================

TEST(VarcharValueTest, StoresStringAndSize) {
  std::string s = "turtle";
  Value v(DataType::VARCHAR, s);
  EXPECT_EQ(v.data_type(), DataType::VARCHAR);
  EXPECT_FALSE(v.is_null());
  EXPECT_EQ(v.to_string(), "turtle");
  // storage layout is a 4-byte length prefix + the raw bytes.
  EXPECT_EQ(v.storage_size(), s.size() + 4);
}

TEST(VarcharValueTest, TypeOnlyCtorIsNull) {
  Value v(DataType::VARCHAR);
  EXPECT_TRUE(v.is_null());
  EXPECT_EQ(v.to_string(), "varchar_null");
  EXPECT_EQ(v.storage_size(), 4u); // just the length prefix
}

TEST(VarcharValueTest, LexicographicComparisons) {
  std::string sa = "apple";
  std::string sb = "banana";
  std::string sa2 = "apple";
  Value a(DataType::VARCHAR, sa);
  Value b(DataType::VARCHAR, sb);
  Value a2(DataType::VARCHAR, sa2);

  EXPECT_EQ(a.compare_equals(a2), CmpBool::CmpTrue);
  EXPECT_EQ(a.compare_not_equals(b), CmpBool::CmpTrue);
  EXPECT_EQ(a.compare_less_than(b), CmpBool::CmpTrue);
  EXPECT_EQ(b.compare_greater_than(a), CmpBool::CmpTrue);
}

TEST(VarcharValueTest, SerializeDeserializeRoundTrip) {
  std::string s = "round-trip me";
  Value original(DataType::VARCHAR, s);

  std::vector<char> buf(original.storage_size());
  original.SerializeTo(buf.data());

  // The decoded VARCHAR is a non-owning view into buf, so buf must outlive it.
  Value restored = Value::DeserializeFrom(buf.data(), DataType::VARCHAR);
  EXPECT_EQ(restored.to_string(), s);
  EXPECT_EQ(restored.compare_equals(original), CmpBool::CmpTrue);
}

// The copy CONSTRUCTOR is correct: for a managed VARCHAR it allocates
// size_.len_ bytes and copies exactly that many, so the duplicate is an
// independent, faithful copy that outlives the source.
TEST(VarcharValueTest, CopyConstructorIsIndependentOfSource) {
  Value dup(DataType::VARCHAR);
  {
    std::string tmp = "owned";
    Value src(DataType::VARCHAR, tmp);
    dup = Value(src); // exercise the copy constructor, then destroy src
  }
  EXPECT_EQ(dup.to_string(), "owned");
}

// KNOWN DEFECT — pinned DISABLED. VarcharType::copy() (src/datatype/
// varchar_type.cpp:107-115) copies val.storage_size() == len+4 bytes out of a
// buffer that only holds len(+1) bytes, over-reading the heap and then storing
// the inflated length. The result is a string with garbage/NUL padding
// ("owned\0\0\0\0"). Re-enable once copy() uses the raw length instead of
// storage_size(). (ASan would flag this as heap-buffer-overflow.)
TEST(VarcharValueTest, DISABLED_CopyMethodPreservesExactString) {
  std::string tmp = "owned";
  Value src(DataType::VARCHAR, tmp);
  Value copied = src.copy();
  EXPECT_EQ(copied.to_string(), "owned");
}

TEST(VarcharValueTest, ArithmeticIsUnsupported) {
  std::string s = "x";
  std::string t = "y";
  Value a(DataType::VARCHAR, s);
  Value b(DataType::VARCHAR, t);
  EXPECT_THROW(a.add(b), std::runtime_error);
  EXPECT_THROW(a.multiply(b), std::runtime_error);
}

// ======================= Value traits & column() ============================

TEST(ValueTraitsTest, CheckIntegerClassifiesFamilies) {
  EXPECT_TRUE(Value(DataType::INTEGER, static_cast<int32_t>(0)).check_integer());
  EXPECT_TRUE(Value(DataType::BIGINT, static_cast<int64_t>(0)).check_integer());
  std::string s = "no";
  EXPECT_FALSE(Value(DataType::VARCHAR, s).check_integer());
}

TEST(ValueTraitsTest, CheckComparable) {
  Value i(DataType::INTEGER, static_cast<int32_t>(0));
  Value big(DataType::BIGINT, static_cast<int64_t>(0));
  std::string s = "v";
  Value vc(DataType::VARCHAR, s);

  EXPECT_TRUE(i.check_comparable(big)); // integer family is mutually comparable
  EXPECT_FALSE(i.check_comparable(vc)); // integer vs varchar is not
  EXPECT_TRUE(vc.check_comparable(vc));
}

TEST(ValueTraitsTest, ColumnDescriptorForInteger) {
  Value i(DataType::INTEGER, static_cast<int32_t>(1));
  auto col = i.column();
  EXPECT_EQ(col.get_type(), DataType::INTEGER);
  EXPECT_EQ(col.get_name(), "<val>");
}

TEST(ValueTraitsTest, ColumnDescriptorForVarcharCarriesStorageSize) {
  std::string s = "hello"; // 5 bytes -> storage_size 9
  Value v(DataType::VARCHAR, s);
  auto col = v.column();
  EXPECT_EQ(col.get_type(), DataType::VARCHAR);
  EXPECT_EQ(col.get_length(), v.storage_size());
}

} // namespace
} // namespace turtle::datatype
