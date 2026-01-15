//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_MOTION_NODE_H
#define KLONDIKESOLVER_MOTION_NODE_H


#include <cstdint>
#include <array>
#include <vector>

#include "motion.h"
#include "../constant.h"

#pragma pack(push, 1)
struct motion_node {

	uint32_t parent; // # 父节点在 node_storage 数组中的索引

	motion mov; // # 当前步骤执行的移动操作

	/**
	 * * 将从根节点到当前节点的完整移动路径提取到destination数组中
	 * @param destination  用于接受移动指令的数组
	 * @param nodes 全局的节点存储池 (A*中的 node_storage)
	 * @return 返回路径的总步数
	 */
	size_t copy_path(std::array<motion, kld::MAX_MOVES>& destination, const std::vector<motion_node>& nodes) const;

};
#pragma pack(pop)


#endif //KLONDIKESOLVER_MOTION_NODE_H