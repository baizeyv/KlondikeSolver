//
// Created by baizeyv on 12/26/2025.
//

#include "poker.h"

#include "../helper/helper.h"

poker::poker(const std::string &seed) : seed(seed) {
	// # 分割字符串后的数组
	const std::vector<std::string> arr = helper::split(seed, "#");
	for (const auto & i : arr) {
		for (const char j : i) {
			card cd(j);
			cards.emplace_back(cd);
		}
	}
}
