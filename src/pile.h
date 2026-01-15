//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_PILE_H2
#define KLONDIKESOLVER_PILE_H2

#include <array>
#include <cstdint>
#include <optional>
#include "../constant.h"

#include "card_ext.h"


struct pile {
	/**
	 * * 当前牌堆中的卡牌数量
	 */
	size_t size;

	/**
	 * * 第一张"翻开"的牌在cards数组中的索引
	 */
	std::optional<uint8_t> first;

	/**
	 * * 固定大小的存储空间
	 */
	std::array<card_ext, TALON_SIZE> cards;

	pile();

	/**
	 * * 重置牌堆
	 */
	void reset();

	/**
	 * * 设置翻开牌的数量 (用于初始化牌真阵列)
	 * @param count
	 */
	void set_face_up_count(size_t count);

	/**
	 * * 第一张牌放入牌顶
	 * @param cd
	 */
	void push_card(const card_ext &cd);

	/**
	 * * 将堆顶牌弹出到另一个牌堆
	 * @param p
	 */
	void pop_card_to(pile &p);

	/**
	 * * 移动n张牌(顺序不变,用于 tableau 之间的移动)
	 * @param to
	 * @param count
	 */
	void move_n_cards_to(pile &to, size_t count);

	/**
	 * * 移动n张牌并翻转 (用于模拟从stock翻牌到waste)
	 * @param to
	 * @param count
	 */
	void move_n_cards_reversed_to(pile &to, size_t count);

	/**
	 * * 获取指定索引的牌
	 * @param index
	 * @return
	 */
	[[nodiscard]]
	card_ext get(size_t index) const;

	/**
	 * * 查看堆顶牌
	 * @return
	 */
	[[nodiscard]]
	card_ext peek_top() const;

	/**
	 * * 查看堆顶牌 (不检查size,用于已知非空的情况)
	 * @return
	 */
	[[nodiscard]]
	card_ext peek_top_unchecked() const;

	/**
	 * * 获取第一张翻开的牌
	 * @return
	 */
	[[nodiscard]]
	card_ext peek_first_face_up() const;

	/**
	 * * 获取第一张翻开的牌 (不进行检查)
	 * @return
	 */
	[[nodiscard]]
	card_ext peek_first_face_up_unchecked() const;

	/**
	 * * 查看从顶部往下数第offset张牌
	 * @param offset
	 * @return
	 */
	[[nodiscard]]
	card_ext peek_nth_from_top_unchecked(size_t offset) const;

	/**
	 * * 获取当前翻开牌的总数
	 * @return
	 */
	[[nodiscard]]
	size_t face_up_count() const;

	bool operator==(const pile &other) const;
};


#endif //KLONDIKESOLVER_PILE_H2
