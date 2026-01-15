//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_TALON_HELPER_H
#define KLONDIKESOLVER_TALON_HELPER_H
#include <array>

#include "card_ext.h"
#include "pile.h"
#include "../meow.h"


struct talon_helper {
	/**
	 * * 存储当前可以触达的所有卡牌
	 */
	array<card_ext, TALON_SIZE> stock_waste;

	/**
	 * * 存储触达该牌需要执行的"翻牌步数"(用于启发式步数估计)
	 * # 正数代表直接翻牌,负数代表需要重新洗牌(redeal)后翻牌
	 */
	array<int32_t, TALON_SIZE> cards_drawn;

	/**
	 * * 内部标记: 哪些牌已经被记录过,防止重复
	 */
	array<bool, TALON_SIZE> stock_used;

	/**
	 * * constructor
	 */
	talon_helper();

	/**
	 * * 核心计算方法: 计算当前发牌系统下所有可用的牌
	 * # Stock (发牌堆)：游戏开始时，剩下的没有分配到牌阵中的牌。它们背面朝上。玩家点击这里来寻找新牌。
	 * # Waste (废牌堆)：当玩家点击 Stock 时，翻开的牌会进入 Waste。Waste 最顶端的那张牌是 “激活” 状态，玩家可以将其移动到 Foundation（回收站）或 Tableau（牌阵）。
	 * # Draw Count (翻牌张数)：
	 *   翻 1 张 (Draw 1)：每次点击 Stock，只有 1 张牌进入 Waste。这种模式下，Stock 中的所有牌最终都能被看到。
	 *   翻 3 张 (Draw 3)：每次点击 Stock，会连翻 3 张到 Waste，但你只能拿最上面那张。这会导致某些牌被压在下面，必须拿掉顶牌或重新洗牌才能看到。
	 * @param draw_count 翻牌规则(通常为1或3)
	 * @param waste_pile 废牌堆
	 * @param stock_pile 发牌堆
	 * @return 返回可触达卡牌的总数
	 */
	size_t calculate(size_t draw_count, const pile &waste_pile, const pile &stock_pile);
};


#endif //KLONDIKESOLVER_TALON_HELPER_H
