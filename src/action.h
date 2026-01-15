//
// Created by baizeyv on 1/14/2026.
//

#ifndef KLONDIKESOLVER_ACTION_H
#define KLONDIKESOLVER_ACTION_H
#include "action_type.h"

#include "state.h"


struct action {
	/**
	 * * 动作类型
	 */
	action_type type;

	/**
	 * * 用于 tableau 或 foundation 的 from 索引
	 */
	size_t from_idx;

	/**
	 * * 目标索引
	 */
	size_t to_idx;

	/**
	 * * 仅用于 tableau_to_tableau 的卡牌数量
	 */
	size_t count;

	[[nodiscard]]
	bool is_redeal() const;

	/**
	 * * 批量格式化动画
	 * @param actions 动作集合
	 * @return
	 */
	static std::string format_actions(const std::vector<action>& actions);

	/**
	 * * 点击发牌
	 * @return
	 */
	static action draw();

	/**
	 * * 重新洗牌
	 * @return
	 */
	static action redeal();

	/**
	 * * waste to foundation
	 * @return foundation index
	 */
	static action waste2foundation(size_t f_idx);

	/**
	 * * waste to tableau
	 * @param t_idx tableau index
	 * @return
	 */
	static action waste2tableau(size_t t_idx);

	/**
	 * * tableau to foundation
	 * @param t_idx tableau index
	 * @param f_idx foundation index
	 * @return
	 */
	static action tableau2foundation(size_t t_idx, size_t f_idx);

	/**
	 * * foundation to tableau
	 * @param f_idx foundation index
	 * @param t_idx tableau index
	 * @return
	 */
	static action foundation2tableau(size_t f_idx, size_t t_idx);

	/**
	 * * tableau to tableau
	 * @param f_idx tableau from index
	 * @param t_idx tableau to index
	 * @param n card count
	 * @return
	 */
	static action tableau2tableau(size_t f_idx, size_t t_idx, size_t n);
};

void apply_action(state& pk, const action& act);


#endif //KLONDIKESOLVER_ACTION_H