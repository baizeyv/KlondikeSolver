//
// Created by baizeyv on 1/20/2026.
//

#ifndef KLONDIKESOLVER_HARD1_JSON_DATA_H
#define KLONDIKESOLVER_HARD1_JSON_DATA_H
#include <fstream>
#include <iostream>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

struct level_json_data {
	int id;

	std::string card_code;

	int layer;

	float win_rate;

	int score_25;

	int score_50;

	int score_75;

	float win_rate_stand;
};

inline std::string convert_card_code(const std::string &original) {
	if (original.size() == 59)
		return original;
	if (original.empty())
		return "";
	std::string ret;

	int idx = 0;

	for (int i = 1; i <= 7; ++i) {
		for (int x = 0; x < i; ++x) {
			ret += original[idx++];
		}
		ret += "#";
	}

	for (int i = idx; i < original.length(); ++i) {
		ret += original[i];
	}
	return ret;
}

inline std::vector<level_json_data> load_hard1_from_file(const std::string &file_path) {
	std::vector<level_json_data> levels;
	std::ifstream file(file_path);

	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << file_path << std::endl;
		return levels;
	}

	try {
		json j;
		file >> j;

		if (j.is_array()) {
			for (const auto &item: j) {
				level_json_data data;
				// # 映射 JSON 字段到结构体
				// # 使用 .value() 可以提供默认值,放置字段缺失时崩溃
				data.id = item.value("ID", 0);
				data.card_code = convert_card_code(item.value("CardCode", ""));
				data.layer = item.value("Layer", 0);
				data.win_rate = item.value("WinRate", 0.0);
				data.score_25 = item.value("Score25", 0);
				data.score_50 = item.value("Score50", 0);
				data.score_75 = item.value("Score75", 0);
				data.win_rate_stand = item.value("WinRateStand", 0.0);

				levels.push_back(data);
			}
		}
	} catch (const json::parse_error &e) {
		std::cerr << "Parse Error: " << e.what() << std::endl;
	}
	return levels;
}

inline std::vector<level_json_data> load_classic_from_file(const std::string &file_path) {
	std::vector<level_json_data> levels;
	std::ifstream file(file_path);

	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << file_path << std::endl;
		return levels;
	}

	try {
		json j;
		file >> j;

		for (auto &el: j.items()) {
			json item = el.value();
			level_json_data data;

			// # 映射 JSON 字段到结构体
			// # 使用 .value() 可以提供默认值,放置字段缺失时崩溃
			data.id = item.value("ID", 0);
			data.card_code = convert_card_code(item.value("CardCode", ""));
			data.layer = item.value("Layer", 0);
			data.win_rate = item.value("WinRate", 0.0);
			data.score_25 = item.value("Score25", 0);
			data.score_50 = item.value("Score50", 0);
			data.score_75 = item.value("Score75", 0);
			data.win_rate_stand = item.value("WinRateStand", 0.0);

			levels.push_back(data);
		}
	} catch (const json::parse_error &e) {
		std::cerr << "Parse Error: " << e.what() << std::endl;
	}
	return levels;
}

#endif //KLONDIKESOLVER_HARD1_JSON_DATA_H
