//
// Created by baizeyv on 12/29/2025.
//

#include "solver.h"

#include <iostream>

#include "../constant.h"
#include "../helper/helper.h"

solver::solver(const std::string &seed) : calc(0) {
	main_poker = new poker(seed);
	root_state = new state(main_poker);
}

void solver::call_dfs() {
	dfs(root_state, false);
	helper::trim_memory();
	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
}

void solver::call_step_dfs() {
	dfs(root_state, true);
	helper::trim_memory();
}

void solver::dfs(const state *root, const bool step_mode) {
	if (calc % 1000000 == 0) {
		// # 每1000000次尝试就释放一次物理内存,防止垃圾机子爆内存
		helper::trim_memory();
	}

	if (step_mode) {
		while (next_step == 0) {
			if (abort_step == 1) {
				break;
			}
		}
		next_step = 0;
		std::cout << std::endl << root->to_string() << std::endl;
	}

	calc++;
	all_serialized_states.insert(root->to_serialized());

	std::vector<state *> states; {
		const auto movable_state = root->find_movable();
		for (const auto &item: movable_state) {
			if (!state_serialized_exists(item)) {
				states.emplace_back(item);
			} else {
				delete item;
			}
		}
	}
	for (size_t i = 0; i < states.size(); ++i) {
		for (size_t x = i; x < states.size(); ++x) {
			all_serialized_states.insert(states[x]->to_serialized());
		}
		if (states[i]->is_completed()) {
			// # 完成了,可以进行动画收牌了
			if (step_mode) {
				std::cout << states[i]->to_string() << std::endl;
			}
			std::cout << "COMPLETED!" << std::endl;
			// todo: delete
			// delete states[i];
			continue;
		}
		dfs(states[i], step_mode);
		// delete states[i];
	}
	// todo:
}

bool solver::state_serialized_exists(const state *new_state) const {
	const auto str = new_state->to_serialized();
	return all_serialized_states.contains(str);
}
