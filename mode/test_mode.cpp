//
// Created by baizeyv on 12/29/2025.
//

#include "test_mode.h"

#include <memory>
#include <thread>

#include "../constant.h"
#include "../src/poker.h"
#include "../src/solver.h"
#include "../src/state.h"

test_mode::test_mode() : is_input(true), step_solver(nullptr) {
}

test_mode::~test_mode() {
	delete arg_commands;
	delete commands;
	delete step_solver;
}

void test_mode::setup() {
	arg_commands = new std::map<std::string, std::function<void(const std::string &)> >;
	commands = new std::map<std::string, std::function<void()> >;

	commands->insert(std::make_pair("qq", []() {
		// solver slr("J#CQ#Zji#HGcY#qEWra#MuPwgv#eVzkdyK#tBNRLDXAloTOhmnxfFUsSpIb");
		solver slr("K#GN#rRc#dQhy#xjUsn#iSwLqA#uTMXoDa#mPbzkeOYtIBfClVJvWgFHEpZ");
		slr.call_dfs();
	}));
	commands->insert(std::make_pair("ww", [this]() {
		test_thread = std::make_unique<std::thread>(std::thread([this]() {
			step_solver = new solver("J#CQ#Zji#HGcY#qEWra#MuPwgv#eVzkdyK#tBNRLDXAloTOhmnxfFUsSpIb");
			step_solver->call_step_dfs();
		}));
	}));
	commands->insert(std::make_pair("ss", [this]() {
		if (step_solver != nullptr) {
			step_solver->next_step = 1;
		}
	}));

	commands->insert(std::make_pair("clear", []() {
		system("cls");
		kld::output_icon();
	}));
}

bool test_mode::input() {
	return is_input;
}
