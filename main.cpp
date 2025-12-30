#include <iostream>

#include "constant.h"
#include "helper/cxxopts.h"
#include "mode/test_mode.h"
#include "src/solver.h"

int main(const int argc, char *argv[]) {
	system("chcp 65001 > nul"); // # 将terminal的编码设置为utf-8, > nul是为了不输出切换提示
	cxxopts::Options options(
		"klondike",
		"\n[Klondike Solitaire Solver]\n@author: baizeyv\n@contact:baizeyv@gmail.com\n@git: https://github.com/baizeyv/KlondikeSolver\n");
	options.add_options()("h,help", "show help information.")("t,test", "test");
	try {
		if (const auto result = options.parse(argc, argv); result.count("help")) {
			// todo:
			return 0;
		} else if (result.count("test")) {
			system("cls");
			kld::output_icon();
			const auto mode = new test_mode();
			mode->setup();
			mode->enter();
			delete mode;
			// todo:
		}
	} catch (const cxxopts::exceptions::exception e) {
		std::cerr << "error " << e.what() << std::endl;
	}
	return 0;
}
