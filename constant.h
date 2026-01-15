//
// Created by baizeyv on 12/29/2025.
//

#ifndef KLONDIKESOLVER_CONST_H
#define KLONDIKESOLVER_CONST_H
#include <cstdint>
#include <iostream>
#include <string>

#ifndef  TALON_SIZE
#define  TALON_SIZE 24
#endif

namespace kld {
	constexpr int MAX_ROUNDS = 15;
	constexpr int MAX_MOVES = 255;
	constexpr uint8_t MAX_CARD = 52;
	constexpr uint8_t MAX_RANK = 13;
	constexpr uint8_t MAX_SUIT = 4;

	// # 牌堆布局常量
	constexpr int TOTAL_FOUNDATIONS = 4;
	constexpr int TOTAL_TABLEAUS = 7;
	constexpr int PILE_STOCK = 0;
	constexpr int PILE_WASTE = 1;
	constexpr int PILE_FOUNDATION_START = 2;
	constexpr int PILE_FOUNDATION_END = PILE_FOUNDATION_START + TOTAL_FOUNDATIONS - 1;
	constexpr int PILE_TABLEAU_START = PILE_FOUNDATION_END + 1;
	constexpr int PILE_TABLEAU_END = PILE_TABLEAU_START + TOTAL_TABLEAUS - 1;
	constexpr int PILE_SIZE = TOTAL_FOUNDATIONS + TOTAL_TABLEAUS + 2; // # 总共13个pile

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
