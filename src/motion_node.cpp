//
// Created by baizeyv on 1/13/2026.
//

#include "motion_node.h"

size_t motion_node::copy_path(std::array<motion, kld::MAX_MOVES>& destination, const std::vector<motion_node>& nodes) const {
	// # 当前移动操作为空
	if (this->mov.is_null())
		return 0;

	// # 由于是单项链表(指向父节点),我们需要先回溯
	size_t index = 0;
	const motion_node* current = this;

	// # 循环到根节点 (根节点的mov通常是null或者parent执行自己/特定值)
	while (!current->mov.is_null()) {
		destination[index++] = current->mov;

		// # 如果parent为0且不是第0个节点,说明到达逻辑终点
		if (current->parent == 0 && index > 0) {
			// # 检查第0个节点是否也有有效移动
			if (!nodes[0].mov.is_null() && current != &nodes[0]) {
				destination[index++] = nodes[0].mov;
			}
			break;
		}
		current = &nodes[current->parent];
	}
	// ! 注意: 提取出来的路径是倒序的 (从当前状态到初始状态)
	// ! 从外部调用的,通常需要 std::reverse 来使其变为正序
	return index;
}
