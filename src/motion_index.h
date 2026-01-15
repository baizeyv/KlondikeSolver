//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_MOTION_INDEX_H
#define KLONDIKESOLVER_MOTION_INDEX_H
#include <cstdint>

#include "estimation.h"

struct motion_index {
	/**
	 * * 指向 node_storage 数组中 motion_node 的索引
	 */
	uint32_t index;

	/**
	 * 启发式搜索的排序优先级 (得分越低优先级越高)
	 */
	int16_t priority;

	/**
	 * 详细的步数评估信息
	 */
	estimation est;

	bool operator>(const motion_index &other) const {
		return this->priority > other.priority;
	}

	/**
	 * * 在 MoveIndex 中，我们的目标是让 priority 值越小 的节点越先被处理（因为在 A* 算法中，得分越低代表离目标越近，优先级越高）。
	 * * C++ 默认行为：std::priority_queue 默认是一个 最大堆 (Max-Heap)。它总是把 “最大” 的元素放在堆顶。它通过调用 operator< 来判断谁更大。
	 * * 我们的需求：我们需要一个 最小堆 (Min-Heap)，即把 priority 最小的元素放在堆顶。
	 * @param other
	 * @return
	 */
	bool operator<(const motion_index &other) const {
		// # priority 越小,优先级越高
		return this->priority > other.priority;
	}
};

#endif //KLONDIKESOLVER_MOTION_INDEX_H
