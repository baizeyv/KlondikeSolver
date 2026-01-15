//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_ESTIMATION_H
#define KLONDIKESOLVER_ESTIMATION_H
#include <cstdint>


/**
 * * 估算方法 f(n) = g(n) + h(n)
 * * g(n) -> 从起点到当前节点的实际代价 (步数)
 * * h(n) -> 启发式估算到终点的代价
 */
struct estimation {
	/**
	 * * g(n):从起点到当前节点的实际代价 (步数)
	 */
	uint8_t current;

	/**
	 * * h(n):启发式估算到终点的代价
	 */
	uint8_t remaining;

	uint8_t total() const;

	bool operator==(const estimation & other) const;
};


#endif //KLONDIKESOLVER_ESTIMATION_H