//
// Created by baizeyv on 1/20/2026.
//

#ifndef KLONDIKESOLVER_ANALYZER_H
#define KLONDIKESOLVER_ANALYZER_H

#include "poker.h"
#include "solve_result.h"
#include "step_feature.h"
#include "../csv_struct/export_data.hpp"

/**
 * * difficulty analyzer (难度分析器)
 */
class analyzer {
	/**
	 * * 关卡种子字符串
	 */
	string level_seed;

	/**
	 * * 解决结果
	 */
	solve_result result;

	/**
	 * * 是否是使用auto_move方法解决的题
	 */
	bool is_auto_move;

public:

	/**
	 * * constructor
	 * @param _level_seed 关卡种子
	 */
	explicit analyzer(string _level_seed);

	/**
	 * * 先 IDA*(DFS) 再 A*(BFS) 搜索解决方案
	 */
	void solve();

	/**
	 * * 模拟关卡及分析
	 */
	export_data simulate_analysis(bool output_content) const;

	/**
	 * * 输出
	 */
	void output() const;

private:

	/**
	 * * 计算当前局面的合法move数量
	 * @return
	 */
	static int calculate_mobility_count(const poker &pkr, const vector<action> &actions, int index);

	/**
	 * * 根据上一个状态以及移动模式来创建之后的状态
	 * @param previous 移动前的状态
	 * @param move 移动操作
	 * @return 移动后的状态
	 */
	static state make_next_state(const state& previous, const action& move);

	/**
	 * * 是否可以打断难度计算公式 (没有隐藏牌了)
	 * @param previous 上一个状态
	 * @param next 下一个状态
	 * @return
	 */
	static bool can_abort(const state& previous, const state& next);

	/**
	 * * 根据上一步下一步以及移动操作来创建当前步骤的状态特征
	 * @param pkr
	 * @param previous 上一个旧状态
	 * @param next 新状态
	 * @param move 移动操作
	 * @param sr
	 * @param idx
	 * @return 当前步骤的状态特征
	 */
	static step_feature make_step_feature(const poker& pkr, const state& previous, const state& next, const action& move, const solve_result& sr, int idx);
};


#endif //KLONDIKESOLVER_ANALYZER_H
