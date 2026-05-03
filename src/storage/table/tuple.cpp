#include "turtle/storage/table/tuple.hpp"
#include "turtle/datatype/value.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>

namespace turtle::storage::table {

Tuple::Tuple(const catalog::ColumnSchema *schema,
             std::vector<datatype::Value> values) {
  assert(values.size() == schema->get_columns().size());

  uint32_t column_count = values.size();
  uint32_t null_bitmap_size = (column_count + 7) / 8;

  this->storage_size_ = null_bitmap_size;

  for (const auto &val : values) {
    this->storage_size_ += val.storage_size();
  }

  // Allocate memory
  this->data_ = new char[this->storage_size_];
  this->allocated_ = true;
  std::memset(this->data_, 0, this->storage_size_);

  // Serialize Null Bitmap & Values
  char *bitmap_ptr = this->data_;
  char *data_ptr = this->data_ + null_bitmap_size;

  for (uint32_t i = 0; i < column_count; ++i) {
    if (values[i].is_null()) {
      // set bit i to 1
      bitmap_ptr[i / 8] |= (1 << (i % 8));
    } else {
      // Serialize value
      values[i].SerializeTo(data_ptr);
      data_ptr += values[i].storage_size();
    }
  }
}

Tuple::Tuple(const Tuple &other) {
  if (other.allocated_) {
    this->allocated_ = true;
    this->storage_size_ = other.storage_size_;
    this->record_id_ = other.record_id_;
    this->data_ = new char[this->storage_size_];
    std::memcpy(this->data_, other.data_, this->storage_size_);
  } else {
    this->allocated_ = false;
    this->storage_size_ = 0;
    this->data_ = nullptr;
  }
}

Tuple &Tuple::operator=(const Tuple &other) {
  if (this == &other) {
    return *this;
  }
  if (this->allocated_) {
    delete[] this->data_;
  }

  if (other.allocated_) {
    this->allocated_ = true;
    this->storage_size_ = other.storage_size_;
    this->record_id_ = other.record_id_;
    this->data_ = new char[this->storage_size_];
    std::memcpy(this->data_, other.data_, this->storage_size_);
  } else {
    this->allocated_ = false;
    this->storage_size_ = 0;
    this->data_ = nullptr;
  }
  return *this;
}

Tuple::~Tuple() {
  if (this->allocated_) {
    delete[] this->data_;
  }
}

void Tuple::serialize_to(char *storage) const {
  std::memcpy(storage, this->data_, this->storage_size_);
}

void Tuple::deserialize_from(const char *storage, uint32_t size) {
  if (this->allocated_) {
    delete[] this->data_;
  }

  this->storage_size_ = size;
  this->data_ = new char[this->storage_size_];
  memcpy(this->data_, storage, this->storage_size_);
  this->allocated_ = true;
}

datatype::Value Tuple::value(const catalog::ColumnSchema *schema,
                             uint32_t column_idx) const {
  // We need column definition from Schema to determine types
  // const auto &columns = schema->
  return datatype::Value();
}

} // namespace turtle::storage::table
