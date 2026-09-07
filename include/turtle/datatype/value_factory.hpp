#pragma once

#include "turtle/common/exceptions.hpp"
#include "turtle/datatype/abstract_pool.hpp"
#include "turtle/datatype/value.hpp"
#include <algorithm>

namespace turtle::datatype {

class ValueFactory {
public:
  static inline auto clone(const Value &src, __attribute__((__unused__))
                                             AbstractPool *dataPool = nullptr)
      -> Value {
    return src.copy();
  }

  static inline auto get_tiny_int_value(int8_t value) -> Value {
    return {DataType::TINYINT, value};
  }

  static inline auto get_small_int_value(int16_t value) -> Value {
    return {DataType::SMALLINT, value};
  }

  static inline auto get_integer_value(int32_t value) -> Value {
    return {DataType::INTEGER, value};
  }

  static inline auto get_big_int_value(int64_t value) -> Value {
    return {DataType::BIGINT, value};
  }

  static inline auto get_timestamp_value(int64_t value) -> Value {
    return {DataType::TIMESTAMP, value};
  }

  static inline auto get_decimal_value(double value) -> Value {
    return {DataType::DECIMAL, value};
  }

  static inline auto get_boolean_value(CmpBool value) -> Value {
    return {DataType::BOOLEAN, value == CmpBool::CmpNull
                                   ? TURTLE_BOOLEAN_NULL
                                   : static_cast<int8_t>(value)};
  }

  static inline auto get_boolean_value(bool value) -> Value {
    return {DataType::BOOLEAN, static_cast<int8_t>(value)};
  }

  static inline auto get_boolean_value(int8_t value) -> Value {
    return {DataType::BOOLEAN, value};
  }

  static inline auto get_varchar_value(const char *value, bool manage_data,
                                       __attribute__((__unused__))
                                       AbstractPool *pool = nullptr) -> Value {
    auto len = static_cast<uint32_t>(value == nullptr ? 0U : strlen(value) + 1);
    return get_varchar_value(value, len, manage_data);
  }

  static inline auto get_varchar_value(const char *value, uint32_t len,
                                       bool manage_data,
                                       __attribute__((__unused__))
                                       AbstractPool *pool = nullptr) -> Value {
    return {DataType::VARCHAR, value, len, manage_data};
  }

  static inline auto get_varchar_value(const std::string &value,
                                       __attribute__((__unused__))
                                       AbstractPool *pool = nullptr) -> Value {
    return {DataType::VARCHAR, value};
  }

  static inline auto get_null_value_by_type(DataType type_id) -> Value {
    Value ret_value;
    switch (type_id) {
    case DataType::BOOLEAN:
      ret_value = get_boolean_value(TURTLE_BOOLEAN_NULL);
      break;
    case DataType::TINYINT:
      ret_value = get_tiny_int_value(TURTLE_INT8_NULL);
      break;
    case DataType::SMALLINT:
      ret_value = get_small_int_value(TURTLE_INT16_NULL);
      break;
    case DataType::INTEGER:
      ret_value = get_integer_value(TURTLE_INT32_NULL);
      break;
    case DataType::BIGINT:
      ret_value = get_big_int_value(TURTLE_INT64_NULL);
      break;
    case DataType::DECIMAL:
      ret_value = get_decimal_value(TURTLE_DECIMAL_NULL);
      break;
    case DataType::VARCHAR:
      ret_value = get_varchar_value(nullptr, false, nullptr);
      break;
    default: {
      throw Exception(ExceptionType::UNKNOWN_TYPE,
                      "Attempting to create invalid null type");
    }
    }
    return ret_value;
  }

  static inline auto get_zero_value_by_type(DataType type_id) -> Value {
    std::string zero_string("0");

    switch (type_id) {
    case DataType::BOOLEAN:
      return get_boolean_value(false);
    case DataType::TINYINT:
      return get_tiny_int_value(0);
    case DataType::SMALLINT:
      return get_small_int_value(0);
    case DataType::INTEGER:
      return get_integer_value(0);
    case DataType::BIGINT:
      return get_big_int_value(0);
    case DataType::DECIMAL:
      return get_decimal_value(static_cast<double>(0));
    case DataType::VARCHAR:
      return get_varchar_value(zero_string);
    default:
      break;
    }
    throw Exception(ExceptionType::UNKNOWN_TYPE,
                    "Unknown type for get_zero_value_by_type");
  }

  static inline auto cast_as_big_int(const Value &value) -> Value {
    if (Type::get_instance(DataType::BIGINT)
            ->is_coercable_from(value.data_type())) {
      if (value.is_null()) {
        return ValueFactory::get_big_int_value(
            static_cast<int64_t>(TURTLE_INT64_NULL));
      }
      switch (value.data_type()) {
      case DataType::TINYINT:
        return ValueFactory::get_big_int_value(
            static_cast<int64_t>(value.get_as<int8_t>()));
      case DataType::SMALLINT:
        return ValueFactory::get_big_int_value(
            static_cast<int64_t>(value.get_as<int16_t>()));
      case DataType::INTEGER:
        return ValueFactory::get_big_int_value(
            static_cast<int64_t>(value.get_as<int32_t>()));
      case DataType::BIGINT:
        return ValueFactory::get_big_int_value(value.get_as<int64_t>());
      case DataType::DECIMAL: {
        if (value.get_as<double>() > static_cast<double>(TURTLE_INT64_MAX) ||
            value.get_as<double>() < static_cast<double>(TURTLE_INT64_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_big_int_value(
            static_cast<int64_t>(value.get_as<double>()));
      }
      case DataType::VARCHAR: {
        std::string str = value.to_string();
        int64_t bigint = 0;
        try {
          bigint = stoll(str);
        } catch (std::out_of_range &e) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        } catch (std::invalid_argument &e) {
          throw Exception("Invalid input syntax for bigint: \'" + str + "\'");
        }
        if (bigint > TURTLE_INT64_MAX || bigint < TURTLE_INT64_MIN) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_big_int_value(bigint);
      }
      default:
        break;
      }
    }
    throw Exception(Type::get_instance(value.data_type())->to_string(value) +
                    " is not coercable to BIGINT.");
  }

  static inline auto cast_as_integer(const Value &value) -> Value {
    if (Type::get_instance(DataType::INTEGER)
            ->is_coercable_from(value.data_type())) {
      if (value.is_null()) {
        return ValueFactory::get_integer_value(TURTLE_INT32_NULL);
      }
      switch (value.data_type()) {
      case DataType::TINYINT:
        return ValueFactory::get_integer_value(
            static_cast<int32_t>(value.get_as<int8_t>()));
      case DataType::SMALLINT:
        return ValueFactory::get_integer_value(
            static_cast<int32_t>(value.get_as<int16_t>()));
      case DataType::INTEGER:
        return ValueFactory::get_integer_value(value.get_as<int32_t>());
      case DataType::BIGINT: {
        if (value.get_as<int64_t>() > static_cast<int64_t>(TURTLE_INT32_MAX) ||
            value.get_as<int64_t>() < static_cast<int64_t>(TURTLE_INT32_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_integer_value(
            static_cast<int32_t>(value.get_as<int64_t>()));
      }
      case DataType::DECIMAL: {
        if (value.get_as<double>() > static_cast<double>(TURTLE_INT32_MAX) ||
            value.get_as<double>() < static_cast<double>(TURTLE_INT32_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_integer_value(
            static_cast<int32_t>(value.get_as<double>()));
      }
      case DataType::VARCHAR: {
        std::string str = value.to_string();
        int32_t integer = 0;
        try {
          integer = stoi(str);
        } catch (std::out_of_range &e) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        } catch (std::invalid_argument &e) {
          throw Exception("Invalid input syntax for integer: \'" + str + "\'");
        }

        if (integer > TURTLE_INT32_MAX || integer < TURTLE_INT32_MIN) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_integer_value(integer);
      }
      default:
        break;
      }
    }
    throw Exception(Type::get_instance(value.data_type())->to_string(value) +
                    " is not coercable to INTEGER.");
  }

  static inline auto cast_as_small_int(const Value &value) -> Value {
    if (Type::get_instance(DataType::SMALLINT)
            ->is_coercable_from(value.data_type())) {
      if (value.is_null()) {
        return ValueFactory::get_small_int_value(
            static_cast<int16_t>(TURTLE_INT16_NULL));
      }
      switch (value.data_type()) {
      case DataType::TINYINT:
        return ValueFactory::get_small_int_value(
            static_cast<int16_t>(value.get_as<int8_t>()));
      case DataType::SMALLINT:
        return ValueFactory::get_small_int_value(value.get_as<int16_t>());
      case DataType::INTEGER: {
        if (value.get_as<int32_t>() > static_cast<int32_t>(TURTLE_INT16_MAX) ||
            value.get_as<int32_t>() < static_cast<int32_t>(TURTLE_INT16_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_small_int_value(
            static_cast<int16_t>(value.get_as<int32_t>()));
      }
      case DataType::BIGINT: {
        if (value.get_as<int64_t>() > static_cast<int64_t>(TURTLE_INT16_MAX) ||
            value.get_as<int64_t>() < static_cast<int64_t>(TURTLE_INT16_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_small_int_value(
            static_cast<int16_t>(value.get_as<int64_t>()));
      }
      case DataType::DECIMAL: {
        if (value.get_as<double>() > static_cast<double>(TURTLE_INT16_MAX) ||
            value.get_as<double>() < static_cast<double>(TURTLE_INT16_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_small_int_value(
            static_cast<int16_t>(value.get_as<double>()));
      }
      case DataType::VARCHAR: {
        std::string str = value.to_string();
        int16_t smallint = 0;
        try {
          smallint = static_cast<int16_t>(stoi(str));
        } catch (std::out_of_range &e) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        } catch (std::invalid_argument &e) {
          throw Exception("Invalid input syntax for smallint: \'" + str + "\'");
        }
        if (smallint < TURTLE_INT16_MIN) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_small_int_value(smallint);
      }
      default:
        break;
      }
    }
    throw Exception(Type::get_instance(value.data_type())->to_string(value) +
                    " is not coercable to SMALLINT.");
  }

  static inline auto cast_as_tiny_int(const Value &value) -> Value {
    if (Type::get_instance(DataType::TINYINT)
            ->is_coercable_from(value.data_type())) {
      if (value.is_null()) {
        return ValueFactory::get_tiny_int_value(TURTLE_INT8_NULL);
      }
      switch (value.data_type()) {
      case DataType::TINYINT:
        return ValueFactory::get_tiny_int_value(value.get_as<int8_t>());
      case DataType::SMALLINT: {
        if (value.get_as<int16_t>() > static_cast<int16_t>(TURTLE_INT8_MAX) ||
            value.get_as<int16_t>() < static_cast<int16_t>(TURTLE_INT8_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_tiny_int_value(
            static_cast<int8_t>(value.get_as<int16_t>()));
      }
      case DataType::INTEGER: {
        if (value.get_as<int32_t>() > static_cast<int32_t>(TURTLE_INT8_MAX) ||
            value.get_as<int32_t>() < static_cast<int32_t>(TURTLE_INT8_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_tiny_int_value(
            static_cast<int8_t>(value.get_as<int32_t>()));
      }
      case DataType::BIGINT: {
        if (value.get_as<int64_t>() > static_cast<int64_t>(TURTLE_INT8_MAX) ||
            value.get_as<int64_t>() < static_cast<int64_t>(TURTLE_INT8_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_tiny_int_value(
            static_cast<int8_t>(value.get_as<int64_t>()));
      }
      case DataType::DECIMAL: {
        if (value.get_as<double>() > static_cast<double>(TURTLE_INT8_MAX) ||
            value.get_as<double>() < static_cast<double>(TURTLE_INT8_MIN)) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_tiny_int_value(
            static_cast<int8_t>(value.get_as<double>()));
      }
      case DataType::VARCHAR: {
        std::string str = value.to_string();
        int8_t tinyint = 0;
        try {
          tinyint = static_cast<int8_t>(stoi(str));
        } catch (std::out_of_range &e) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        } catch (std::invalid_argument &e) {
          throw Exception("Invalid input syntax for tinyint: \'" + str + "\'");
        }
        if (tinyint < TURTLE_INT8_MIN) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_tiny_int_value(tinyint);
      }
      default:
        break;
      }
    }
    throw Exception(Type::get_instance(value.data_type())->to_string(value) +
                    " is not coercable to TINYINT.");
  }

  static inline auto cast_as_decimal(const Value &value) -> Value {
    if (Type::get_instance(DataType::DECIMAL)
            ->is_coercable_from(value.data_type())) {
      if (value.is_null()) {
        return ValueFactory::get_decimal_value(
            static_cast<double>(TURTLE_DECIMAL_NULL));
      }
      switch (value.data_type()) {
      case DataType::TINYINT:
        return ValueFactory::get_decimal_value(
            static_cast<double>(value.get_as<int8_t>()));
      case DataType::SMALLINT:
        return ValueFactory::get_decimal_value(
            static_cast<double>(value.get_as<int16_t>()));
      case DataType::INTEGER:
        return ValueFactory::get_decimal_value(
            static_cast<double>(value.get_as<int32_t>()));
      case DataType::BIGINT:
        return ValueFactory::get_decimal_value(
            static_cast<double>(value.get_as<int64_t>()));
      case DataType::DECIMAL:
        return ValueFactory::get_decimal_value(value.get_as<double>());
      case DataType::VARCHAR: {
        std::string str = value.to_string();
        double res = 0;
        try {
          res = stod(str);
        } catch (std::out_of_range &e) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        } catch (std::invalid_argument &e) {
          throw Exception("Invalid input syntax for decimal: \'" + str + "\'");
        }
        if (res > TURTLE_DECIMAL_MAX || res < TURTLE_DECIMAL_MIN) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Numeric value out of range.");
        }
        return ValueFactory::get_decimal_value(res);
      }
      default:
        break;
      }
    }
    throw Exception(Type::get_instance(value.data_type())->to_string(value) +
                    " is not coercable to DECIMAL.");
  }

  static inline auto cast_as_varchar(const Value &value) -> Value {
    if (Type::get_instance(DataType::VARCHAR)
            ->is_coercable_from(value.data_type())) {
      if (value.is_null()) {
        return ValueFactory::get_varchar_value(nullptr, false);
      }
      switch (value.data_type()) {
      case DataType::BOOLEAN:
      case DataType::TINYINT:
      case DataType::SMALLINT:
      case DataType::INTEGER:
      case DataType::BIGINT:
      case DataType::DECIMAL:
      case DataType::VARCHAR:
        return ValueFactory::get_varchar_value(value.to_string().c_str(),
                                               false);
      default:
        break;
      }
    }
    throw Exception(Type::get_instance(value.data_type())->to_string(value) +
                    " is not coercable to VARCHAR.");
  }

  static inline auto cast_as_timestamp(const Value &value) -> Value {
    if (Type::get_instance(DataType::TIMESTAMP)
            ->is_coercable_from(value.data_type())) {
      if (value.is_null()) {
        return ValueFactory::get_timestamp_value(TURTLE_TIMESTAMP_NULL);
      }
      switch (value.data_type()) {
      case DataType::TIMESTAMP:
        return ValueFactory::get_timestamp_value(value.get_as<uint64_t>());
      case DataType::VARCHAR: {
        std::string str = value.to_string();
        if (str.length() == 22) {
          str = str.substr(0, 19) + ".000000" + str.substr(19, 3);
        }
        if (str.length() != 29) {
          throw Exception("Timestamp format error.");
        }
        if (str[10] != ' ' || str[4] != '-' || str[7] != '-' ||
            str[13] != ':' || str[16] != ':' || str[19] != '.' ||
            (str[26] != '+' && str[26] != '-')) {
          throw Exception("Timestamp format error.");
        }
        bool is_digit[29] = {
            true,  true, true, true,  false, true, true,  false, true, true,
            false, true, true, false, true,  true, false, true,  true, false,
            true,  true, true, true,  true,  true, false, true,  true};
        for (int i = 0; i < 29; i++) {
          if (is_digit[i]) {
            if (str[i] < '0' || str[i] > '9') {
              throw Exception("Timestamp format error.");
            }
          }
        }
        int tz = 0;
        uint32_t year = 0;
        uint32_t month = 0;
        uint32_t day = 0;
        uint32_t hour = 0;
        uint32_t min = 0;
        uint32_t sec = 0;
        uint32_t micro = 0;
        uint64_t res = 0;
        if (sscanf(str.c_str(), "%4u-%2u-%2u %2u:%2u:%2u.%6u%3d", &year, &month,
                   &day, &hour, &min, &sec, &micro, &tz) != 8) {
          throw Exception("Timestamp format error.");
        }
        if (year > 9999 || month > 12 || day > 31 || hour > 23 || min > 59 ||
            sec > 59 || micro > 999999 || day == 0 || month == 0) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Timestamp value out of range.");
        }
        uint32_t max_day[13] = {0,  31, 28, 31, 30, 31, 30,
                                31, 31, 30, 31, 30, 31};
        uint32_t max_day_lunar[13] = {0,  31, 29, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
          if (day > max_day_lunar[month]) {
            throw Exception(ExceptionType::OUT_OF_RANGE,
                            "Timestamp value out of range.");
          }
        } else if (day > max_day[month]) {
          throw Exception(ExceptionType::OUT_OF_RANGE,
                          "Timestamp value out of range.");
        }
        uint32_t timezone = tz + 12;
        if (tz > 26) {
          throw Exception("Timestamp format error.");
        }
        res += month;
        res *= 32;
        res += day;
        res *= 27;
        res += timezone;
        res *= 10000;
        res += year;
        res *= 100000;
        res += hour * 3600 + min * 60 + sec;
        res *= 1000000;
        res += micro;
        return ValueFactory::get_timestamp_value(res);
      }
      default:
        break;
      }
    }
    throw Exception(Type::get_instance(value.data_type())->to_string(value) +
                    " is not coercable to TIMESTAMP.");
  }

  static inline auto cast_as_boolean(const Value &value) -> Value {
    if (Type::get_instance(DataType::BOOLEAN)
            ->is_coercable_from(value.data_type())) {
      if (value.is_null()) {
        return ValueFactory::get_boolean_value(TURTLE_BOOLEAN_NULL);
      }
      switch (value.data_type()) {
      case DataType::BOOLEAN:
        return ValueFactory::get_boolean_value(value.get_as<int8_t>());
      case DataType::VARCHAR: {
        std::string str = value.to_string();
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        if (str == "true" || str == "1" || str == "t") {
          return ValueFactory::get_boolean_value(true);
        }
        if (str == "false" || str == "0" || str == "f") {
          return ValueFactory::get_boolean_value(false);
        }
        throw Exception("Boolean value format error.");
      }
      default:
        break;
      }
    }
    throw Exception(Type::get_instance(value.data_type())->to_string(value) +
                    " is not coercable to BOOLEAN.");
  }
};

} // namespace turtle::datatype
