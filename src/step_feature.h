//
// Created by baizeyv on 1/20/2026.
//

#ifndef KLONDIKESOLVER_STEP_FEATURE_H
#define KLONDIKESOLVER_STEP_FEATURE_H
#include <cstdint>


/**
 * * 解题路径上的每一步的状态特征
 */
struct step_feature {
	/**
	 * * 当前合法的move数量 (走完当前这步后的那个状态的可移动数量)
	 */
	uint8_t mobility;

	/**
	 * * 走完这步后的空列数量
	 */
	uint8_t empty_cols;

	/**
	 * * 走完这步后所有列中 face-down 总数
	 */
	uint8_t face_down;

	/**
	 * * 走完这步后,可立即推进foundation的数量
	 */
	uint8_t foundation_ready;

	/**
	 * * 当前步骤是否产生了翻牌行为
	 */
	bool flip_card;

	/**
	 * * 当前步骤是否产生了消耗空列的行为
	 */
	bool consume_empty;

private:
	/**
	 * * 计算强制度 (这是"如果不这么走就完了"的量化) (mobility<=3是真正的危险区)
	 * @return
	 */
	int calculate_forcedness() const;

	/**
	 * * 计算资源消耗惩罚
	 * # 消耗空列但没翻牌 -> 假进展
	 * # 这一步在走"唯一解",但同时把未来堵死
	 * @return
	 */
	int calculate_resource_burn() const;

	// todo:

};


#endif //KLONDIKESOLVER_STEP_FEATURE_H