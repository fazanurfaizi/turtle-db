#ifndef TURTLE_TYPE_DATA_TYPE_H
#define TURTLE_TYPE_DATA_TYPE_H

namespace turtle::datatype {

enum class DataType {
  INVALID = 0,
  BOOLEAN,
  INTEGER,
  TINYINT,
  SMALLINT,
  BIGINT,
  DECIMAL,
  VARCHAR,
  TIMESTAMP
};

}

#endif // !TURTLE_TYPE_DATA_TYPE_H
