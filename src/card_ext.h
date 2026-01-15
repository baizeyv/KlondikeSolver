//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_CARD_EXT_H
#define KLONDIKESOLVER_CARD_EXT_H
#include <cstdint>


/**
 * * 牌的额外计算信息
 */
struct card_ext {
	/**
	 * * 原始ID (0-51)
	 */
	uint8_t id;

	/**
	 * * 排序优化ID (rank << 2 | suit)
	 */
	uint8_t id2;

	/**
	 * * 花色 (0:方块,1:梅花,2:红桃,3:黑桃)
	 */
	uint8_t suit;

	/**
	 * * 点数(0-12)
	 */
	uint8_t rank;

	/**
	 * * 是否是红色 (1:红色,0:黑色)
	 */
	uint8_t is_red;

	/**
	 * *  是否偶数 (1:偶数,0:奇数)
	 */
	uint8_t is_even;

	/**
	 * * 核心优化字段 (is_red ^ is_even)
	 * * 红-偶 和 黑-奇 是一样的
	 * # 用途：解算器通过判断 cardA.red_even == cardB.red_even 就能快速筛选出哪些牌可能可以互相叠放，这比逐个判断颜色和点数要快得多。
	 */
	uint8_t red_even;

	/**
	 * * 核心辅助排序字段 (suit >> 1)
	 * # 用途：这通常用于将花色归类（比如将 0,1 分为一组，2,3 分为一组）。在某些变体规则或特定的状态压缩算法中，用来区分大小花色组。
	 */
	uint8_t order;

	static const card_ext UNKNOWN;

	card_ext();

	card_ext(uint8_t _id, uint8_t _id2, uint8_t _suit, uint8_t _rank,
			uint8_t _is_red, uint8_t _is_even, uint8_t _red_even, uint8_t _order);

	/**
	 * * 通过指定id创建新的card_ext
	 * @param id
	 * @return
	 */
	static card_ext create_with_id(uint8_t id);

	/**
	 * * 通过指定value和suit来创建card_ext
	 * @param rank 0-12
	 * @param suit 0-3
	 * @return
	 */
	static card_ext creat_with_rank_suit(uint8_t rank, uint8_t suit);

	/**
	 * * 判断是否是未知牌
	 * @return
	 */
	bool is_unknown() const;

	/**
	 * * 判断是否是 K
	 * @return
	 */
	bool is_king() const;

	bool operator==(const card_ext &c) const {
		return id == c.id;
	};
};


#endif //KLONDIKESOLVER_CARD_EXT_H
