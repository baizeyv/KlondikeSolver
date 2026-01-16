//
// Created by baizeyv on 1/15/2026.
//

#ifndef KLONDIKESOLVER_HINT_H
#define KLONDIKESOLVER_HINT_H

#include <array>

#include "motion.h"
#include "pile.h"
#include "solver.h"
#include "../constant.h"
#include "../meow.h"


/**
 * * dfs 提示
 */
class hint {
	solver *slr;

	/**
	 * (1-13)
	 */
	array<int, kld::TOTAL_FOUNDATIONS> foundation_top_rank;

	array<int, kld::TOTAL_FOUNDATIONS> foundation_stack_id;

	array<int, kld::TOTAL_FOUNDATIONS> foundation_safe_rank;

	int first_empty_tableau_stack_id;

	array<pile*, kld::TOTAL_TABLEAUS> tableau_sorted_by_hidden_count;

	// # 各种分类的移动列表
	vector<motion> t2f_safe;
	vector<motion> t2f_flip;
	vector<motion> t2f;

	vector<motion> w2f_safe;
	vector<motion> w2f;

	vector<motion> t2t_flip;
	vector<motion> t2t_empty_t2t_flip;
	vector<motion> t2t_empty;
	vector<motion> t2t_t2f_flip;
	vector<motion> t2t_t2f;

	vector<motion> w2t_t2t_flip;
	vector<motion> w2t;

	vector<motion> f2t_t2t_flip;
	vector<motion> f2t_w2t;

	vector<motion> all;
	vector<motion> auto_moves;

	bool cached;

public:
	explicit hint(solver *_slr);

	~hint();

	/**
	 * * 获取一个提示的移动
	 * @return
	 */
	motion get() const;

private:
	/**
	 * * 更新 foundation 中每种花色当前已叠放的最高点数 (真正的点数,没有牌是0,有了之后是1-13)
	 */
	void update_foundation_top_rank();

	/**
	 * * 建立suit与piles index 之间的映射关系
	 */
	void update_foundation_pile_id();

	/**
	 * * 安全收纳评估
	 */
	void update_foundation_safe_rank();

	/**
	 * * tableau 到 foundation, 利用之前计算的foundation_top_rank和foundation_safe_rank来寻找并分类所有可能的移动
	 * # 安全移动 > 能翻牌的移动 > 普通移动
	 */
	void update_tableau_to_foundation();

	/**
	 * * waste 到 foundation, 这个对比上边的方法不需要考虑翻牌的情况了
	 */
	void update_waste_to_foundation();

	/**
	 * * 在7个tableau中寻找第一个出现的空位,并记录其id
	 */
	void update_first_empty_tableau_pile_id();

	/**
	 * * 根据隐藏牌的数量,对7个tableau进行降序排列
	 */
	void sort_tableau_by_hidden_count();

	/**
	 * * tableau to tableau 负责扫描tableau之间所有可能的移动,并根据移动的战略价值(如能否翻开新牌,是否暴露了foundation所需要的牌)对操作进行精细分类
	 */
	void update_tableau_to_tableau();

	/**
	 * * waste to tableau 判断waste顶部的牌是否可以移动到7个tableau中的某一个
	 */
	void update_waste_to_tableau();

	/**
	 * * foundation to tableau 将foundation已经收集的牌拿出来当"垫脚石"
	 */
	void update_foundation_to_tableau();

	/**
	 * * 按照从高到低的权重顺序合并到all的主列表中
	 */
	void merge();

	/**
	 * * 检测上一步,防止死循环
	 */
	void review_last_move();

	/**
	 * * 去重于填充,将经过优先级排序并处理过死循环风险的all列表内容,最终筛选进执行列表auto中
	 */
	void update_auto();

	/**
	 * * 根据起始堆栈id和移动卡牌数量,在列表中查找匹配渡轮拆散指令
	 * @param list 待搜索的移动指令列表
	 * @param from 起始堆栈id
	 * @param count 移动的卡牌数量
	 * @return 匹配的移动
	 */
	static motion get(const vector<motion>& list, int from, int count);

	/**
	 * * 判断列表中是否存在符合条件的移动指令
	 * @param list
	 * @param from
	 * @param count
	 * @return
	 */
	static bool contains(vector<motion>& list, int from, int count);
};


#endif //KLONDIKESOLVER_HINT_H
