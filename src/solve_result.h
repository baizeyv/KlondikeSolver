//
// Created by baizeyv on 1/15/2026.
//

#ifndef KLONDIKESOLVER_SOLVE_RESULT_H
#define KLONDIKESOLVER_SOLVE_RESULT_H
#include <chrono>

#include "action.h"

struct solve_result {
	/**
	 * * 是否最优解
	 */
	bool minimal = false;

	/**
	 * * 搜索过的状态数
	 */
	int32_t states = 0;

	/**
	 * * 耗时 (秒)
	 */
	std::chrono::duration<double> elapsed{};

	/**
	 * * 解路径
	 */
	std::vector<action> actions;

	string to_str() const;
};

#endif //KLONDIKESOLVER_SOLVE_RESULT_H