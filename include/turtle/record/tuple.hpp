#pragma once

#include <vector>
#include <cstdint>
#include "turtle/catalog/schema.hpp"
#include "turtle/common/record_id.hpp"
#include "turtle/datatype/value.hpp"

namespace turtle::record {

class Tuple {
public:
  Tuple() = default;

  // Construct a tuple from a list of values and the schema
  Tuple(std::vector<datatype::Value> values, const catalog::Schema *schema);

  Tuple(const Tuple &other);

  Tuple &operator=(const Tuple &other);

  ~Tuple();

  /**
   * Serializes the tuple data into the given storage buffer.
   * @param storage The destination buffer (must be at least storage_size())
   */
  void serialize_to(char *storage) const;

  /**
   * Deserializes the tuple from a storage buffer.
   * @param storage The source buffer
   * @param size The size of the tuple data
   */
  void deserialize_from(const char *storage, uint32_t size);

  /**
   * Retrieves the value of a specific column.
   * @param schema The schema of the table this tuple belongs to
   * @param column_idx The index of the column to retrieve
   * @return The value of the column
   */
  datatype::Value value(const catalog::Schema *schema, uint32_t column_idx) const;

  inline uint32_t storage_size() const { return storage_size_; }
  inline RecordId record_id() const { return record_id_; }
  inline void set_recod_id(RecordId rid) { record_id_ = rid; }
  inline char *data() const { return data_; }

  auto to_string(const catalog::Schema *schema) const -> std::string;

private:
  bool allocated_{false};
  uint32_t storage_size_{0};
  char *data_{nullptr};
  RecordId record_id_{};
};

}
