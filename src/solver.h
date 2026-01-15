//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_SOLVER_H
#define KLONDIKESOLVER_SOLVER_H

#include <memory>

#include "motion_node.h"
#include "state.h"
#include "state_map.h"
#include "talon_helper.h"
#include "../constant.h"
#include "../meow.h"
#include "pile.h"
#include "solve_result.h"
#include "state_key.h"

using possible_moves = vector<motion>;

class solver {
public:
	// ! --- 核心游戏状态 ---

	/**
	 * * 当前棋盘的所有牌堆
	 */
	array<pile, kld::PILE_SIZE> piles;

	/**
	 * * 初始棋盘备份 (用于重置)
	 */
	array<pile, kld::PILE_SIZE> initial_piles;

	/**
	 * * 关键映射表: suits_to_foundations[suit_id] = foundation_pile_index
	 * * suit_id: 0,1,2,3
	 * * value: 对应piles数组中的索引
	 */
	array<uint8_t, kld::TOTAL_FOUNDATIONS> suits_to_foundations{};

	/**
	 * * 搜索路径与动作记录
	 */
	array<motion, kld::MAX_MOVES> moves;

	/**
	 * * stock to wast 的翻牌张数 (1或3)
	 */
	uint8_t draw_count;

	/**
	 * * 当前回收站的总牌数(0-52)
	 */
	uint8_t foundation_score;

	/**
	 * * 初始foundation分数备份 (用于重置)
	 */
	uint8_t initial_foundation_score;

	/**
	 * * 自动收牌的阈值
	 */
	uint8_t foundation_minimum;

	/**
	 * * 当前已执行的步数
	 */
	uint16_t moves_total;

	/**
	 * * 当前发牌堆循环轮数
	 */
	uint8_t round_count;

	/**
	 * * 上一次执行的操作 (用于剪枝)
	 */
	motion last_move;

	/**
	 * * 初始状态
	 */
	state initial_state;

	// ! --- 搜索赋值组件 ---

	/**
	 * * 发牌逻辑预判器
	 */
	talon_helper tl_helper;

	/**
	 * * 使用unique_ptr管理大内存,防止栈溢出
	 * * 状态去重哈希表
	 */
	unique_ptr<state_map> states;

	/**
	 * * constructor
	 */
	solver();

	/**
	 * * solver 装载器, 设置初始棋盘状态
	 * * 将外部的state数据同步到solver内部的piles结构中,并初始化映射表
	 * @param st
	 */
	void setup(const state& st);

	/**
	 * * A* 搜索主函数
	 * @param max_nodes 最大搜索节点数,防止内存溢出或时间过长
	 * @param minimal 是否寻找最优解 (为true时找到解不停止,继续搜更短的路径)
	 */
	solve_result solve(uint32_t max_nodes, bool minimal, bool step_mode);

	/**
	 * * 获取当前局面的hash value, 用于状态判重
	 * # 它的任务是将当前棋盘上所有会影响后续决策的信息，压缩成一个 32 字节的 state 数组，然后通过哈希算法生成一个 u64
	 * @return
	 */
	[[nodiscard]]
	state_key get_state() const;

	/**
	 * * 计算当前局面举例胜利至少还需要多少步 (对应A*算法中的h(n))
	 * # 它估计的剩余步数必须小于或等于实际完成游戏所需的步数。这段代码通过计算所有未收取的卡牌及其 “阻塞开销” 来提供一个乐观的估计。
	 * @param is_last_round
	 * @return
	 */
	[[nodiscard]]
	uint8_t minimum_moves_remaining(bool is_last_round) const;

	/**
	 * * 计算一个宏观动作包含的实例操作步骤
	 * # 解算器会将 “翻牌 3 次 + 移动卡牌” 压缩成一个宏观动作。但在计算 A* 算法的 $g(n)$（已花费代价）时，我们不能只把它计作 1 步，而必须还原出玩家实际点击鼠标的次数。
	 * @param mov 宏观动作
	 * @return 实际点击次数
	 */
	[[nodiscard]]
	uint8_t calculate_additional_moves(motion mov) const;

	/**
	 * * 生成当前状态下所有可能的合法动作
	 * @param pm 用于接收动作列表的容器
	 */
	void compute_possible_moves(possible_moves &pm);

	/**
	 * * 基于上一个动作进行特定剪枝或强制移动检测
	 * @param pm 候选动作列表
	 * @return 是否找到了推荐的强制移动
	 */
	bool compute_with_last_move(possible_moves &pm) const;

	/**
	 * * 计算从废牌堆 (及通过翻牌可触达的牌) 开始的所有合法移动
	 * @param pm 动作候选列表
	 * @return 是否发现了'必须立即执行'的动作
	 */
	bool compute_move_from_waste(possible_moves &pm);

	/**
	 * * 计算从foundation移回tableau的所有合法移动
	 * @param pm 动作候选列表
	 * @return 是否返回false (因为这种动作几乎不可能是强制性的/最优的)
	 */
	bool compute_move_from_foundation(possible_moves &pm) const;

	/**
	 * * 处理tableau的所有合法移动逻辑
	 * @param pm 动作候选列表
	 * @return 是否发现了'必须立即执行'的确定性动作
	 */
	bool compute_move_from_tableau(possible_moves &pm);

	/**
	 * * 执行一个动作并更新游戏状态
	 * @param mov
	 */
	void make_move(motion mov);

	/**
	 * * 撤销上一个动作,恢复棋盘状态
	 */
	void undo_move();

	/**
	 * * 判断卡牌可以进入哪个foundation堆
	 * @param cd
	 * @return
	 */
	optional<uint8_t> can_move_to_foundation(card_ext cd) const;

	/**
	 * * 将 solver 恢复到初始状态
	 */
	void reset();

	/**
	 * * 将解题器的移动序列转换为具体的操作序列
	 * @return
	 */
	vector<action> export_actions() const;

	// * ---------------------------------------------------------

	[[nodiscard]]
	string to_str() const;

	[[nodiscard]]
	string hidden_string(int row, int max) const;

	[[nodiscard]]
	string floor_hidden_string(int row) const;

	[[nodiscard]]
	string visible_string(int row, int max) const;

	[[nodiscard]]
	string floor_visible_string(int row) const;

	[[nodiscard]]
	string deck_string() const;

	[[nodiscard]]
	string collected_string() const;

	uint8_t next_step : 1 = 0;
	uint8_t abort_step : 1 = 0;
};


#endif //KLONDIKESOLVER_SOLVER_H
