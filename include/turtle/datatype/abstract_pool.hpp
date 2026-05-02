#pragma once

#include <cstdlib>

namespace turtle::datatype {

class AbstractPool {
public:
  virtual ~AbstractPool() = default;

  /**
   * @brief Allocate a contiguous block of memory of the given size
   * @param size The size (in bytes) of memory to allocate
   * @return A non-null pointer if allocation is successful. A null pointer if
   * allocation fails.
   *
   * TODO: Provide good error codes for failure cases
   */
  virtual auto allocate(size_t size) -> void * = 0;

  /**
   * @brief Returns the provided chunk of memory back into the pool
   */
  virtual void free(void *ptr) = 0;
};

} // namespace TURTLE::TYPE
