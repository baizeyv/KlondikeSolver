//
// Created by baizeyv on 12/26/2025.
//

#include "poker.h"

#include <algorithm>

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

solver poker::call() const {
	state st;

	int idx = 0;
	for (int i = 0; i < 7; ++ i) {
		vector<card> vec;
		for (int j = 0; j <= i; ++ j) {
			vec.push_back(cards[idx++]);
		}
		st.tableaus[i] = tableau(vec, 1);
	}

	for (size_t i = idx; i < cards.size(); ++ i) {
		st.stock.push_back(cards[idx++]);
	}

	std::ranges::reverse(st.stock);

	solver s;
	s.setup(st);
	return s;
}
