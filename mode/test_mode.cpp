//
// Created by baizeyv on 12/29/2025.
//

#include "test_mode.h"
#include "../src/poker.h"

#include <memory>
#include <thread>

#include "../constant.h"
#include "../src/analyzer.h"

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
	commands->insert(std::make_pair("oo", [this]() {
		test_thread = std::make_unique<std::thread>(std::thread([this]() {
			analyzer ana("w#pQ#ZJI#utCY#drWeA#zhPjGi#EVmKDlx#goNRyqXnLbTOHMakFsUfScvB");
			ana.solve();
			ana.output();
			ana.simulate_analysis();
		}));
	}));
	commands->insert(std::make_pair("qq", []() {
		const poker pkr("L#mn#ild#txRW#bgoYB#fuSFVK#hrUGIcX#wyqajNCADPsZHkTpvQMzEeJO");
		auto slr = pkr.call();
		cout << slr.to_str() << endl;
		const auto res = slr.solve(2200000, false, false);
		cout << res.to_str() << endl;
	}));
	commands->insert(std::make_pair("ww", [this]() {
		test_thread = std::make_unique<std::thread>(std::thread([this]() {
			// step_solver = new solver("J#CQ#Zji#HGcY#qEWra#MuPwgv#eVzkdyK#tBNRLDXAloTOhmnxfFUsSpIb");
			// step_solver->call_step_dfs();
			const poker pkr("K#GN#rRc#dQhy#xjUsn#iSwLqA#uTMXoDa#mPbzkeOYtIBfClVJvWgFHEpZ");
			auto s = pkr.call();
			step_solver = &s;
			step_solver->solve(1000000, false, true);
		}));
	}));
	commands->insert(std::make_pair("ff", [this]() {
		test_thread = std::make_unique<std::thread>(std::thread([this]() {
			const poker pkr("L#mn#ild#txRW#bgoYB#fuSFVK#hrUGIcX#wyqajNCADPsZHkTpvQMzEeJO");
			auto s = pkr.call();
			step_solver = &s;
			const auto res = step_solver->solve(false);
			cout << res.to_str() << endl;
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
