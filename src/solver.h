//
// Created by baizeyv on 12/29/2025.
//

#ifndef KLONDIKESOLVER_SOLVER_H
#define KLONDIKESOLVER_SOLVER_H
#include <unordered_set>

#include "state.h"


class solver {
	/**
	 * * 所有尝试过的状态的指针hash_set
	 */
	std::unordered_set<std::string> all_serialized_states{};

	/**
	 * * 求解计数
	 */
	int calc;

public:
	/**
	 * * 当前这副牌
	 */
	poker *main_poker;

	/**
	 * * 根状态 (初始状态)
	 */
	state *root_state;

	explicit solver(const std::string &seed);

	void call_dfs();

	void call_step_dfs();

private:
	void dfs(const state *root, const bool step_mode);

	/**
	 * * 判断指定状态是否已经跑过了
	 * @param new_state
	 * @return
	 */
	bool state_serialized_exists(const state *new_state) const;

public:
	uint8_t next_step : 1 = 0;
	uint8_t abort_step : 1 = 0;
};


#endif //KLONDIKESOLVER_SOLVER_H
