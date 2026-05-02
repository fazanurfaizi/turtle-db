#pragma once

#include <fstream>
#include <string>

namespace turtle::util {

template <typename T> void write_binary(std::ofstream &ofs, const T &value) {
  ofs.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <typename T> auto read_binary(std::ifstream &ifs, T &value) -> bool {
  ifs.read(reinterpret_cast<char *>(&value), sizeof(T));
  return ifs.good();
}

void write_string(std::ofstream &ofs, const std::string &str);

auto read_string(std::ifstream &ifs) -> std::string;

} // namespace Turtle::Util

