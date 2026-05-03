#pragma once
#include <exception>

namespace turtle {

class SyntaxErrorException : public std::exception {};

class NoDatabaseSelectedException : public std::exception {};

class DatabaseNotExistException : public std::exception {};

class DatabaseAlreadyExistsException : public std::exception {};

class TableNotExistException : public std::exception {};

class TableAlreadyExistsException : public std::exception {};

class IndexAlreadyExistsException : public std::exception {};

class IndexNotExistException : public std::exception {};

class OneIndexEachTableException : public std::exception {};

class BPlusTreeException : public std::exception {};

class IndexMustBeCreatedOnPKException : public std::exception {};

class PrimaryKeyConflictException : public std::exception {};

enum class ExceptionType {
  /** Invalid exception type.*/
  INVALID = 0,
  /** Value out of range. */
  OUT_OF_RANGE = 1,
  /** Conversion/casting error. */
  CONVERSION = 2,
  /** Unknown type in the type subsystem. */
  UNKNOWN_TYPE = 3,
  /** Decimal-related errors. */
  DECIMAL = 4,
  /** Type mismatch. */
  MISMATCH_TYPE = 5,
  /** Division by 0. */
  DIVIDE_BY_ZERO = 6,
  /** Incompatible type. */
  INCOMPATIBLE_TYPE = 8,
  /** Out of memory error */
  OUT_OF_MEMORY = 9,
  /** Method not implemented. */
  NOT_IMPLEMENTED = 11,
  /** Execution exception. */
  EXECUTION = 12,
};

} // namespace turtle
