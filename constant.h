//
// Created by baizeyv on 12/29/2025.
//

#ifndef KLONDIKESOLVER_CONST_H
#define KLONDIKESOLVER_CONST_H
#include <atomic>
#include <iostream>
#include <string>

namespace kld {
	inline std::string empty_card = "\033[30m:::: \033[0m";

	static void output_icon() {
		std::cout
				<< "██ ▄█▀ ▄▄     ▄▄▄  ▄▄  ▄▄ ▄▄▄▄  ▄▄ ▄▄ ▄▄ ▄▄▄▄▄ " << std::endl
				<< "████   ██    ██▀██ ███▄██ ██▀██ ██ ██▄█▀ ██▄▄  " << std::endl
				<< "██ ▀█▄ ██▄▄▄ ▀███▀ ██ ▀██ ████▀ ██ ██ ██ ██▄▄▄ " << std::endl
				<< "\033[34m	 Welcome to Klondike Solver!\033[0m" << std::endl;
	}
}

#endif //KLONDIKESOLVER_CONST_H
