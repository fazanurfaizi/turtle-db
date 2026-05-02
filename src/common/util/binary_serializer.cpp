#include "turtle/common/util/binary_serializer.hpp"
#include <ios>

namespace turtle::util {

void write_string(std::ofstream &ofs, const std::string &str) {
  size_t len = str.length();
  write_binary(ofs, len);
  ofs.write(str.c_str(), static_cast<std::streamsize>(len));
}

auto read_string(std::ifstream &ifs) -> std::string {
  size_t len;
  if (!read_binary(ifs, len)) {
    return "";
  }
  std::string str(len, '\0');
  ifs.read(&str[0], static_cast<std::streamsize>(len));
  return str;
}

} // namespace Turtle::Util
