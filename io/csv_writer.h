//
// Created by baizeyv on 1/20/2026.
//

#ifndef KLONDIKESOLVER_CSV_WRITER_H
#define KLONDIKESOLVER_CSV_WRITER_H
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "interface_csv_export.h"


class csv_writer {
public:
	template <typename T>
	void export_csv(const std::vector<T>& data_list, const std::string& file_path) {
		// # 静态assert, 确保T继承自指定接口
		static_assert(std::is_base_of_v<interface_csv_export, T>, "T must derive from interface_csv_export");

		std::ios_base::openmode mode = std::ios::out;
		mode |= std::ios::app;

		// # 检查文件是否存在
		std::ifstream checkFile(file_path);
		bool file_exists = checkFile.good();
		checkFile.close();

		std::ofstream file(file_path, mode);
		if (!file.is_open()) {
			std::cerr << "failed to open file: " << file_path << std::endl;
			return;
		}
		// # 写入utf-8 bom 放置excel乱码
		if (!file_exists)
			file << static_cast<char>(0xEF) << static_cast<char>(0xBB) << static_cast<char>(0xBF);

		if (!data_list.empty()) {
			// # 1. 写入表头
			if (!file_exists)
				file << data_list[0].to_csv_header() << "\n";

			// # 2. 写入数据行
			for (const auto& item : data_list) {
				file << item.to_csv_content() << "\n";
			}
		}
		file.close();
		std::cout << "successfully exported to " << file_path << std::endl;
	}
private:
	/**
	 * * 一个更健壮的字符串格式函数
	 * @param text
	 * @return
	 */
	static std::string escape_csv(const std::string &text);
};


#endif //KLONDIKESOLVER_CSV_WRITER_H
