//
// Created by baizeyv on 12/29/2025.
//

#ifndef KLONDIKESOLVER_TEST_MODE_H
#define KLONDIKESOLVER_TEST_MODE_H
#include <memory>
#include <thread>

#include "base_mode.h"
#include "../src/solver.h"


class test_mode final : public base_mode{

	bool is_input;

	std::unique_ptr<std::thread> test_thread;

	solver* step_solver;

public:
	test_mode();

	~test_mode() override;

	void setup() override;

	bool input() override;
};


#endif //KLONDIKESOLVER_TEST_MODE_H