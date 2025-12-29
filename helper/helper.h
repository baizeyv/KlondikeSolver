//
// Created by baizeyv on 12/26/2025.
//

#ifndef KLONDIKESOLVER_HELPER_H
#define KLONDIKESOLVER_HELPER_H
#include <string>
#include <vector>

class helper {
public:
  static std::vector<std::string> split(const std::string &str,
                                        const std::string &delimiter);

  static void ltrim(std::string &s);

  static void rtrim(std::string &s);

  static void trim(std::string &s);

  static std::string read_file(const std::string &path);

  static std::vector<std::string> read_file_line(const std::string &path);

  static std::string get_current_timestamp_millis();
};

#endif // KLONDIKESOLVER_HELPER_H
