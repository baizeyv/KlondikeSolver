//
// Created by baizeyv on 1/20/2026.
//

#include "json_reader.h"

#define JSON_PATH_PREFIX R"(D:/develop/KlondikeSolver/json_dic/)"

void json_reader::setup() {
	read_hard1();
	read_classic();
	read_point_five();
}

void json_reader::read_hard1() {
	hard1_data = load_hard1_from_file(JSON_PATH_PREFIX + std::string("hard1.json"));
}

void json_reader::read_classic() {
	classic_data.clear();
	for (int i = 1; i <= 10; ++ i) {
		auto vec = load_classic_from_file(JSON_PATH_PREFIX + std::string("classic_" + std::to_string(i) + ".json"));
		classic_data.emplace(i, vec);
	}
}

void json_reader::read_point_five() {
	point_five_data = load_classic_from_file(JSON_PATH_PREFIX + std::string("point_five.json"));
}
