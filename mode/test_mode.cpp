//
// Created by baizeyv on 12/29/2025.
//

#include "test_mode.h"
#include "../src/poker.h"

#include <memory>
#include <thread>

#include "../constant.h"
#include "../io/csv_writer.h"
#include "../io/json_reader.h"
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


	commands->insert(std::make_pair("exp", []() {
		json_reader jr;
		jr.setup();


		auto func = [](const vector<export_data> &csv, const std::string &filename) {
			csv_writer cw;
			cw.export_csv<export_data>(csv, "D:/develop/KlondikeSolver/" + filename + ".csv");
		};
		for (const auto &[fst, snd]: jr.classic_data) {
			for (const auto &item: snd) {
				analyzer ana(item.card_code);
				cout << to_string(item.id) << " : " << item.card_code << endl;
				ana.solve();
				auto dt = ana.simulate_analysis(false);
				dt.id = item.id;
				dt.seed = item.card_code;

				vector<export_data> hard1_csv;
				hard1_csv.push_back(dt);

				func(hard1_csv, "export_classic" + std::to_string(fst));
			}
		}
		for (const auto &item: jr.point_five_data) {
			analyzer ana(item.card_code);
			cout << to_string(item.id) << " : " << item.card_code << endl;
			ana.solve();
			auto dt = ana.simulate_analysis(false);
			dt.id = item.id;
			dt.seed = item.card_code;

			vector<export_data> hard1_csv;
			hard1_csv.push_back(dt);

			func(hard1_csv, "point_five");
		}
		for (const auto &item: jr.hard1_data) {
			analyzer ana(item.card_code);
			cout << to_string(item.id) << " : " << item.card_code << endl;
			ana.solve();
			auto dt = ana.simulate_analysis(false);
			dt.id = item.id;
			dt.seed = item.card_code;

			vector<export_data> hard1_csv;
			hard1_csv.push_back(dt);

			func(hard1_csv, "export_hard1");
		}
	}));
	// commands->insert(std::make_pair("arst", []() {
	// 	json_reader jr;
	// 	jr.setup();
	// }));
	commands->insert(std::make_pair("oo", [this]() {
		test_thread = std::make_unique<std::thread>(std::thread([]() {
			analyzer ana("r#GJ#OkS#XxUW#hNpza#tyTRsE#fmHVqCi#cZQDPuInALBvdjwYlKMgeobF");
			ana.solve();
			ana.output();
			ana.simulate_analysis(true);
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
