//
// Created by baizeyv on 1/14/2026.
//

#ifndef KLONDIKESOLVER_TABLEAU_H
#define KLONDIKESOLVER_TABLEAU_H

#include <vector>
#include "../meow.h"
#include "card.h"


struct tableau {

	/**
	 * * 存储当前列的所有牌
	 */
	vector<card> cards;

	/**
	 * * 正面朝上的卡牌数量
	 */
	size_t face_up_count;

	/**
	 * * default constructor
	 */
	tableau();

	/**
	 * * constructor
	 * @param init_cards
	 * @param face_up
	 */
	tableau(vector<card> init_cards, size_t face_up);

	/**
	 * * 检查是否为空
	 * @return
	 */
	[[nodiscard]]
	bool is_empty() const;

	/**
	 * * 获取总长度
	 * @return
	 */
	[[nodiscard]]
	size_t size() const;

	/**
	 * * 查看最顶部的牌 (不移除)
	 * @return 获取到的牌 (不可操作)
	 */
	[[nodiscard]]
	const card* peek_top() const;

	/**
	 * * 弹出一张牌 (无检查版本)
	 * * logic: 如果移除后为空,正面数归0;否则正面数减1,但保持至少是1(除非是空)
	 * @return
	 */
	[[nodiscard]]
	card pop_unchecked();

	/**
	 * * 批量移除顶部的牌
	 * @param count 移除的数量
	 * @return 移除的卡牌集合
	 */
	[[nodiscard]]
	vector<card> drain_unchecked(size_t count);

	/**
	 * * 压入一张牌
	 * @param card
	 */
	void push(card card);

};


#endif //KLONDIKESOLVER_TABLEAU_H