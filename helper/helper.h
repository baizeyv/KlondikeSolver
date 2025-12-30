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

	static void trim_memory();
};

/**
 * * 获取指定 vector 在内存中的占用
 * @tparam T
 * @param vec
 * @return
 */
template<typename T>
size_t get_vector_memory(const std::vector<T> &vec) {
	return sizeof(T) * vec.capacity();
}

/**
 * * 获取指定二维 vector 在内存中的占用
 * @tparam T
 * @param nestedVec
 * @return
 */
template<typename T>
size_t get_nested_vector_memory(const std::vector<std::vector<T> > &nestedVec) {
	size_t total = get_vector_memory(nestedVec);
	for (const auto &inner: nestedVec) {
		total += get_vector_memory(inner);
	}
	return total;
}

#endif // KLONDIKESOLVER_HELPER_H
