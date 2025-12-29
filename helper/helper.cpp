//
// Created by baizeyv on 12/26/2025.
//

#include "helper.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

std::vector<std::string> helper::split(const std::string &str,
                                       const std::string &delimiter) {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = 0;

  while ((end = str.find(delimiter, start)) != std::string::npos) {
    result.push_back(str.substr(start, end - start));
    start = end + delimiter.length();
  }

  result.push_back(str.substr(start)); // 最后一个部分
  return result;
}

void helper::ltrim(std::string &s) {
  if (s.empty())
    return;
  s.erase(s.begin(), std::ranges::find_if(s, [](const unsigned char ch) {
            return !std::isspace(ch);
          }));
}

void helper::rtrim(std::string &s) {
  if (s.empty())
    return;
  const auto it = std::find_if(
      s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); });
  s.erase(it.base(), s.end());
}

void helper::trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

std::string helper::read_file(const std::string &path) {
  const std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Failed to open file " + path);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::vector<std::string> helper::read_file_line(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Failed to open file " + path);
  }
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(file, line)) {
    lines.push_back(line);
  }
  file.close();
  return lines;
}

std::string helper::get_current_timestamp_millis() {
  const int64_t timestamp =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  return std::to_string(timestamp);
}
