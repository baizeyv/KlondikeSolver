//
// Created by baizeyv on 1/14/2026.
//

#ifndef KLONDIKESOLVER_POKER_HA
#define KLONDIKESOLVER_POKER_HA

#include <array>
#include <optional>

#include "tableau.h"
#include "../constant.h"
#include "../meow.h"
#include "card.h"

struct action;


/**
 * * 牌局状态结构体
 */
struct state {
	/**
	 * * 牌堆
	 */
	vector<card> stock{};

	/**
	 * * 废牌堆
	 */
	vector<card> waste{};

	/**
	 * * 4个foundation
	 */
	array<optional<card>, kld::TOTAL_FOUNDATIONS> foundations{};

	/**
	 * * 7个牌阵列
	 */
	array<tableau, kld::TOTAL_TABLEAUS> tableaus{};

	/**
	 * * constructor
	 */
	state();

	/**
	 * * 计算foundation的总得分
	 * * logic: 累加4个foundation堆顶卡片的点数
	 * @return
	 */
	[[nodiscard]]
	uint8_t foundation_score() const;

	/**
	 * * 校验当前棋盘状态是否合法
	 * * 检查项:
	 * # 1. 没有任何一张牌是UNKNOWN
	 * # 2. 52张牌不重不漏,每张牌再棋盘上只出现一次
	 * @return
	 */
	[[nodiscard]]
	bool is_valid() const;

	/**
	 * * 判断是否需要 redeal
	 * @return
	 */
	[[nodiscard]]
	bool need_redeal() const;

	/**
	 * * 执行发牌 (点击stock堆)的操作
	 * * 处理翻牌逻辑以及重新发牌逻辑
	 */
	void draw();

	/**
	 * * 将waste顶部的牌移动到foundation
	 * @param idx foundation的索引(0-3)
	 */
	void move_waste_to_foundation(size_t idx);

	/**
	 * * 将waste顶部的牌移动到指定的tableau
	 * @param idx 目标tableau列的索引(0-6)
	 */
	void move_waste_to_tableau(size_t idx);

	/**
	 * * 将tableau顶部的牌移动到foundation
	 * @param tableau_idx 源tableau索引(0-6)
	 * @param foundation_idx 目标foundation索引(0-3)
	 */
	void move_tableau_to_foundation(size_t tableau_idx, size_t foundation_idx);

	/**
	 * * 将多张牌从一个tableau移动到另一个tableau
	 * @param from_idx 源tableau索引(0-6)
	 * @param to_idx 目标tableau索引(0-6)
	 * @param count 移动的卡牌数量
	 */
	void move_tableau_to_tableau(size_t from_idx, size_t to_idx, size_t count);

	/**
	 * * 将牌从foundation放回tableau
	 * @param foundation_idx 源foundation索引(0-3)
	 * @param tableau_idx 目标tableau索引(0-6)
	 */
	void move_foundation_to_tableau(size_t foundation_idx, size_t tableau_idx);

	/**
	 * * 将另一个poker的状态复制到当前对象
	 * @param pk source poker object
	 */
	void copy_from(const state &pk);

	/* * ======================================================================== */

	/**
	 * * 计算当前局面的空列数
	 * @return
	 */
	[[nodiscard]]
	int calculate_empty_column_count() const;

	/**
	 * * 计算当前局面所有列中face-down总数
	 * @return
	 */
	[[nodiscard]]
	int calculate_face_down_count() const;

	/**
	 * * 计算当前局面中可立刻推进foundation的数量
	 * @return
	 */
	[[nodiscard]]
	int calculate_foundation_ready_count() const;

	/**
	 * * 检测执行某步骤是否产生了翻牌行为
	 * @param previous 移动前的状态
	 * @param act 移动操作
	 * @return
	 */
	[[nodiscard]]
	bool check_flip_card(const state &previous, const action& act) const;

	/**
	 * * 检测执行某步骤是否产生了消耗空列的行为
	 * @param previous 移动前的状态
	 * @param act 移动操作
	 * @return
	 */
	[[nodiscard]]
	bool check_consume_empty(const state &previous, const action& act) const;

	string to_str() const;

private:
	/**
	 * * 是否可以将指定牌推进foundation中
	 * @param cd 指定card
	 * @return
	 */
	bool can_move_to_foundation(const card& cd) const;
};


#endif //KLONDIKESOLVER_POKER_HA
