//
// Created by baizeyv on 12/26/2025.
//

#ifndef KLONDIKESOLVER_POKER_Hz
#define KLONDIKESOLVER_POKER_Hz
#include <string>
#include <vector>

#include "card.h"
#include "solver.h"

class poker {
	/**
	 * * level seed
	 * # (for example: W#wg#diA#Kxof#RYItC#vhFQSk#MTuOmNn#XpZJyBlLEsaDzcbjGrUqHVPe)
	 */
	std::string seed;

public:

	/**
	 * * 牌堆
	 */
	std::vector<card> cards{};

	explicit poker(const std::string &seed);

	[[nodiscard]]
	solver call() const;
};

#endif // KLONDIKESOLVER_POKER_Hz
