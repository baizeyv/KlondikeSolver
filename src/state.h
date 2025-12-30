//
// Created by baizeyv on 12/26/2025.
//

#ifndef KLONDIKESOLVER_STATE_H
#define KLONDIKESOLVER_STATE_H
#include "card.h"
#include "history_item.h"

#include <vector>

#include "poker.h"

class state {
	/**
	 * * -9999代表还没有进行估值,其他值为真正的估值
	 */
	int valuation = -9999;

public:
	/**
	 * * 右上角的牌堆
	 */
	std::vector<card *> waste_cards{};

	/**
	 * * 每一列的隐藏的牌
	 */
	std::vector<std::vector<card *> > hidden_tableau_cards{};

	/**
	 * * 每一列的可见的牌
	 */
	std::vector<std::vector<card *> > visible_tableau_cards{};

	/**
	 * * 左上角已经收集的牌 (size: 4)
	 */
	std::vector<std::vector<card *> > foundation_cards{};

	/**
	 * * 历史记录
	 */
	std::vector<history_item> history{};

	/**
	 * * 右上角的牌堆区域翻到第几张牌了
	 * # -1 代表还没有翻牌,可以点击翻牌到deck_cards[0]
	 */
	int deck_index = -1;

	/**
	 * * 上一步的状态
	 */
	const state *previous;

	/**
	 * * 使用这个 constructor method 相当于生成的是 root_state
	 * @param pkr poker 指定的一副牌
	 */
	explicit state(poker *pkr);

	/**
	 * * 这个constructor用于走步骤的时候创建新的状态
	 * @param previous_state
	 */
	explicit state(const state *previous_state);

	[[nodiscard]]
	std::string to_string() const;

	/**
	 * * 是否完成了 (所有隐藏牌都翻开了)
	 * @return
	 */
	[[nodiscard]]
	bool is_completed() const;

	/**
	 * * 找到所有可以移动到的新的状态
	 * @return
	 */
	[[nodiscard]]
	std::vector<state *> find_movable() const;

	/**
	 * * 移动牌
	 * @param from from column index
	 * @param count 移动的牌的数量
	 * @param to to column index
	 */
	void move_card(int from, int count, int to);

	/**
	 * * 计算及获取一次估值
	 * @return
	 */
	int get_valuation();

	[[nodiscard]]
	std::string to_serialized() const;

	/**
	 * * destructor
	 */
	~state();

private:
	/**
	 * * 计算已经翻开的牌的数量的加权值
	 * # importance: 1.
	 * @return
	 */
	int calculate_revealed_value() const;

	/**
	 * * 计算衡量当前局面允许多少 "合法且有意义的移动" 的加权值
	 * @return
	 */
	int calculate_mobility_value() const;

	/**
	 * * 计算空列数的加权值
	 * @return
	 */
	int calculate_empty_column_value() const;

	/**
	 * * 计算 waste 牌堆的状态质量
	 * @return
	 */
	int calculate_waste_playable_value() const;

	/**
	 * * 隐藏牌的字符串显示
	 * @param row 第几行
	 * @param max 隐藏牌最大数量
	 * @return
	 */
	[[nodiscard]]
	std::string hidden_string(int row, int max) const;

	[[nodiscard]]
	std::string floor_hidden_string(int row) const;

	[[nodiscard]]
	std::string visible_string(int row, int max) const;

	[[nodiscard]]
	std::string floor_visible_string(int row) const;

	[[nodiscard]]
	std::string deck_string() const;

	[[nodiscard]]
	std::string collected_string() const;
};

#endif // KLONDIKESOLVER_STATE_H
