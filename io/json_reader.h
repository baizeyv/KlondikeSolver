//
// Created by baizeyv on 1/20/2026.
//

#ifndef KLONDIKESOLVER_JSON_READER_H
#define KLONDIKESOLVER_JSON_READER_H
#include <vector>

#include "../json_struct/level_json_data.h"


class json_reader {
public:
	/**
	 * * 读取到的 hard1 的数据
	 */
	std::vector<level_json_data> hard1_data{};

	/**
	 * * 读取到的 classic 的数据 (key: 1-10)
	 */
	std::unordered_map<int, std::vector<level_json_data> > classic_data{};

	/**
	 * * 读取到的 point_five 的数据
	 */
	std::vector<level_json_data> point_five_data{};

	void setup();

private:
	void read_hard1();

	void read_classic();

	void read_point_five();
};


#endif //KLONDIKESOLVER_JSON_READER_H
