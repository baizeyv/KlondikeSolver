//
// Created by baizeyv on 1/15/2026.
//

#include "hint.h"

hint::hint(solver *_slr) : slr(_slr), foundation_top_rank(), foundation_stack_id(), foundation_safe_rank(),
                           first_empty_tableau_stack_id(0),
                           cached(false) {
}

hint::~hint() {
	t2f_safe.clear();
	t2f_flip.clear();
	t2f.clear();
	w2f_safe.clear();
	w2f.clear();
	t2t_flip.clear();
	t2t_empty_t2t_flip.clear();
	t2t_empty.clear();
	t2t_t2f_flip.clear();
	t2t_t2f.clear();
	w2t_t2t_flip.clear();
	w2t.clear();
	f2t_t2t_flip.clear();
	f2t_w2t.clear();
	all.clear();
	auto_moves.clear();
}

motion hint::get() const {
	return all.size() > 0 ? all[0] : motion{};
}

void hint::update_foundation_top_rank() {
	// # 1. 重置所有花色的最高点数为0 (表示还没有牌)
	this->foundation_top_rank.fill(0);

	// # 2. 遍历4个foundation
	for (int id = kld::PILE_FOUNDATION_START; id <= kld::PILE_FOUNDATION_END; ++id) {
		// # 从 solver 的实例指针中获取到对应的牌堆
		const pile *pile = &slr->piles[id];

		// # 获取牌堆顶的卡牌 (即该花色目前收纳的最大值)
		const auto cd = pile->peek_top();
		if (!cd.is_unknown()) {
			this->foundation_top_rank[cd.suit] = cd.rank + 1;
		}
	}
}

void hint::update_foundation_pile_id() {
	// # 1. 初始化: 将所有花色对应的 piles index 设置为 -1 (表示未分配)
	this->foundation_stack_id.fill(-1);

	for (int i = kld::PILE_FOUNDATION_START; i <= kld::PILE_FOUNDATION_END; ++i) {
		this->foundation_stack_id[i - kld::PILE_FOUNDATION_START] = i;
	}
}

void hint::update_foundation_safe_rank() {
	// # 1. 计算红色牌的安全线: 取决于黑色牌中较小的那个进度 +1
	int red_safe_rank = std::min(this->foundation_top_rank[1], this->foundation_top_rank[3]) + 1;

	// # 强制最小值. 通常A总是可以安全收纳的,所以安全线至少从2开始
	if (red_safe_rank < 2) {
		red_safe_rank = 2;
	}
	// # 2. 计算黑色牌的安全线: 取决于红色牌中较小的那个进度 +1
	int black_safe_rank = std::min(this->foundation_top_rank[0], this->foundation_top_rank[2]) + 1;
	if (black_safe_rank < 2) {
		black_safe_rank = 2;
	}

	// # 3. 将计算结果映射回每种花色
	// ! 注意: 黑色花色的安全线参考红色的进度,反之亦然
	this->foundation_safe_rank[0] = red_safe_rank;
	this->foundation_safe_rank[1] = black_safe_rank;
	this->foundation_safe_rank[2] = red_safe_rank;
	this->foundation_safe_rank[3] = black_safe_rank;
}

void hint::update_tableau_to_foundation() {
	// # 1. 遍历7个tableau
	for (int from_index = kld::PILE_TABLEAU_START; from_index <= kld::PILE_TABLEAU_END; ++from_index) {
		pile *pile = &slr->piles[from_index];
		if (pile->size <= 0)
			continue;

		// # 2. 获取顶层的牌的信息
		auto cd = pile->peek_top();
		if (!cd.is_unknown()) {
			// # 有效牌
			const int rank = cd.rank + 1;
			const int suit = cd.suit;

			// # 3. 检查是否符合放入foundation的规则
			// * 规则: 该花色已收纳的最大点数必须是当前牌点数-1
			if (this->foundation_top_rank[suit] == rank - 1) {
				// # 4. 特殊逻辑: 检查这一步是否能翻开一张新的背面牌
				// * 条件: 当前牌堆正面牌只有一张,且总牌数大于1 (说明下面压着背面牌)
				const bool can_flip_hidden_card = (pile->face_up_count() == 1 && pile->size > 1);

				motion move(from_index, this->foundation_stack_id[suit], 1, can_flip_hidden_card);

				// # 5. 根据优先级将移动执行存入不同的列表
				if (rank <= this->foundation_safe_rank[suit]) {
					// # 最高优先级: 安全移动(收进去不会导致死局)
					this->t2f_safe.push_back(move);
				} else if (can_flip_hidden_card) {
					// # 次高优先级: 虽然不完全安全,但能翻开一张新的背面牌,增加游戏进度
					this->t2f_flip.push_back(move);
				} else {
					// # 普通优先级: 可以收,但有风险或收益不高
					this->t2f.push_back(move);
				}
			}
		}
	}
}

void hint::update_waste_to_foundation() {
	// # 1. 获取废牌堆
	const pile *waste_pile = &slr->piles[kld::PILE_WASTE];
	const pile *stock_pile = &slr->piles[kld::PILE_STOCK];

	if (stock_pile->size <= 0 && waste_pile->size <= 0) {
		// # stock 和 waste中都没有牌,直接返回
		return;
	}

	// # 获取废牌堆最顶上的那张牌

	// todo:
}

void hint::update_first_empty_tableau_pile_id() {
	// # 1. 初始化,默认没有空位(-1)
	this->first_empty_tableau_stack_id = -1;

	// # 2. 顺序遍历7个tableau
	for (int id = kld::PILE_TABLEAU_START; id <= kld::PILE_TABLEAU_END; ++id) {
		// # 3. 获取对应id的堆栈并检查牌的数量
		const pile *pile = &slr->piles[id];
		if (pile->size <= 0) {
			// # 4. 找到第一个数量为0的堆栈, 记录其id
			this->first_empty_tableau_stack_id = id;

			// # 5. 找到一个后立即退出循环 (不需要知道后续还有多少空位)
			break;
		}
	}

	// todo:
}

void hint::sort_tableau_by_hidden_count() {
	// # 1. 将7个tableau引用填充到缓存数组中
	auto arr = this->tableau_sorted_by_hidden_count;
	for (int id = kld::PILE_TABLEAU_START, index = 0; id <= kld::PILE_TABLEAU_END; id++, index++) {
		arr[index] = &this->slr->piles[id];
	}
	// # 2. 使用冒泡排序 (bubble sort) 进行排序
	// * 排序标准: 隐藏牌数量 hidden_count
	// * 排序顺序: 降序 (descending order)
	for (int i = 0; i < kld::TOTAL_TABLEAUS; ++i) {
		for (int j = 1; j < kld::TOTAL_TABLEAUS - i; ++j) {
			const auto a = arr[j - 1];
			const auto b = arr[j];

			// # 如果前一个pile的隐藏牌比后一个少,则交换位置
			// * 结果是:隐藏牌最多的pile排在数组最前面
			const auto a_size = a->size - a->face_up_count();
			const auto b_size = b->size - b->face_up_count();
			if (a_size < b_size) {
				arr[j - 1] = b;
				arr[j] = a;
			}
		}
	}
	this->tableau_sorted_by_hidden_count = arr;
}

void hint::update_tableau_to_tableau() {
	// # 1. 检查是否存在被压住的K
	// # 如果某个牌堆有隐藏牌,且隐藏牌正上方压着的第一张可见牌是K
	bool has_king_block = false;
	for (int id = kld::PILE_TABLEAU_START; id <= kld::PILE_TABLEAU_END; ++id) {
		const pile *pile = &this->slr->piles[id];
		int hidden_count = pile->size - pile->face_up_count();
		auto cd = pile->peek_first_face_up();
		if (hidden_count > 0 && !cd.is_unknown() && cd.rank == 12) {
			// # 有K
			has_king_block = true;
			break;
		}
	}

	// # 2. 遍历经过排序的tableau (优先处理隐藏牌多的)
	for (int i = 0; i < kld::TOTAL_TABLEAUS; ++i) {
		pile *from = this->tableau_sorted_by_hidden_count[i];
		if (from->size <= 0)
			continue;

		// int bottom_index = from->size - *from->first;
		// int bottom_card
		// int bottom_rank

		for (int to_id = kld::PILE_TABLEAU_START; to_id <= kld::PILE_TABLEAU_END; ++to_id) {
			// todo:
		}
	}

	// todo:
}

void hint::update_waste_to_tableau() {
	// todo:
}

void hint::update_foundation_to_tableau() {
	// todo:
}

void hint::merge() {
	// # 在合并之前,童虫需要清空主列表all
	this->all.clear();

	// * --- 优先级1: 绝对安全的收纳 (tableau to foundation safety) (waste to foundation safety) ---
	this->all.insert(all.end(), t2f_safe.begin(), t2f_safe.end());
	this->all.insert(all.end(), w2f_safe.begin(), w2f_safe.end());

	// * --- 优先级2: 能翻开牌背面的直接移动 (direct flip) ---
	this->all.insert(all.end(), t2f_flip.begin(), t2f_flip.end());
	this->all.insert(all.end(), t2t_flip.begin(), t2t_flip.end());

	// * --- 优先级3: 为了翻牌而进行的辅助收纳 (indrect flip) ---
	this->all.insert(all.end(), t2t_t2f_flip.begin(), t2t_t2f_flip.end());

	// * --- 优先级4: 涉及空位或waste的翻牌连锁 (advanced flip) ---
	this->all.insert(all.end(), t2t_empty_t2t_flip.begin(), t2t_empty_t2t_flip.end());
	this->all.insert(all.end(), w2t_t2t_flip.begin(), w2t_t2t_flip.end());
	this->all.insert(all.end(), f2t_t2t_flip.begin(), f2t_t2t_flip.end()); // # 即使从foundation拿牌,只要能翻牌也排在这里

	// * --- 优先级5: 普通foundation (normal foundation) ---
	this->all.insert(all.end(), t2f.begin(), t2f.end());
	this->all.insert(all.end(), w2f.begin(), w2f.end());

	// * --- 优先级6: 普通的连锁辅助移动 (chain moves) ---
	this->all.insert(all.end(), t2t_t2f.begin(), t2t_t2f.end());

	// * --- 优先级7: 普通的waste移动 (normal waste to tableau) ---
	this->all.insert(all.end(), w2t.begin(), w2t.end());

	// * --- 优先级8: 普通的空位填补 (tableau to empty)
	this->all.insert(all.end(), t2t_empty.begin(), t2t_empty.end());

	// * --- 优先级9: 最低优先级: 回撤foundation以救活牌局
	this->all.insert(all.end(), f2t_w2t.begin(), f2t_w2t.end());
}

void hint::review_last_move() {
	// todo:
}

void hint::update_auto() {
	// # 遍历所有已排序并筛选出的候选移动 (all列表)
	for (int i = 0, n = this->all.size(); i < n; ++i) {
		auto move = this->all[i];

		// # 检查auto列表中是否已经存在来自同一个起始堆栈且移动牌数相同的指定
		// # 这是为了防止在同一帧内对同一个牌堆下达多个冲突的移动指令

		// todo:
	}
	// todo:
}

motion hint::get(const vector<motion> &list, const int from, const int count) {
	for (auto move: list) {
		if (move.from() == from && move.count() == count) {
			return move;
		}
	}
	return {};
}

bool hint::contains(vector<motion> &list, const int from, const int count) {
	return !get(list, from, count).is_null();
}
