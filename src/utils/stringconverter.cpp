#include "stringconverter.h"

#include <string>
#include <utility>

std::pair<std::string, std::string> split_at(const std::string &input, const std::string &delimiter) {
  std::size_t pos = input.find(delimiter);
  if (pos == std::string::npos)
    return {input, ""};
  return {
      input.substr(0, pos),
      input.substr(pos + delimiter.length())
  };
}
