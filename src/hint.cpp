//
// Created by baizeyv on 1/15/2026.
//

#include "hint.h"
#include "solver.h"

hint::hint(solver *_slr) : slr(_slr), foundation_top_rank(), foundation_stack_id(), foundation_safe_rank(),
                           first_empty_tableau_stack_id(0), cached(false) {
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
	return !all.empty() ? all[0] : motion{};
}

void hint::update() {
	if (cached)
		return;
	update_foundation_top_rank();
	update_foundation_pile_id();
	update_foundation_safe_rank();
	update_tableau_to_foundation();
	update_waste_to_foundation();
	update_first_empty_tableau_pile_id();
	sort_tableau_by_hidden_count();
	update_tableau_to_tableau();
	update_waste_to_tableau();
	update_foundation_to_tableau();
	merge();
	review_last_move();
	update_auto();
	cached = true;
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
		const pile *pile = &slr->piles[from_index];
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

	if (waste_pile->size <= 0) {
		// # waste中都没有牌,直接返回
		return;
	}

	// # 2. 获取废牌堆最顶上的那张牌
	const card_ext cd = waste_pile->peek_top();
	const int rank = cd.rank + 1;
	const int suit = cd.suit;
	if (foundation_top_rank[suit] == rank - 1) {
		const motion mv(kld::PILE_WASTE, foundation_stack_id[suit], 1, false);
		if (rank <= foundation_safe_rank[suit]) {
			w2f_safe.push_back(mv);
		} else {
			w2f.push_back(mv);
		}
	}
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
}

void hint::sort_tableau_by_hidden_count() {
	// # 1. 将7个tableau引用填充到缓存数组中
	auto arr = this->tableau_sorted_by_hidden_count;
	for (int id = kld::PILE_TABLEAU_START, index = 0; id <= kld::PILE_TABLEAU_END; id++, index++) {
		arr[index] = id;
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
			const auto a_size = slr->piles[a].size - slr->piles[a].face_up_count();
			const auto b_size = slr->piles[b].size - slr->piles[b].face_up_count();
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
		const auto hidden_count = pile->size - pile->face_up_count();
		auto cd = pile->peek_first_face_up();
		if (hidden_count > 0 && !cd.is_unknown() && cd.rank == 12) {
			// # 有K
			has_king_block = true;
			break;
		}
	}

	// # 2. 遍历经过排序的tableau (优先处理隐藏牌多的)
	for (int i = 0; i < kld::TOTAL_TABLEAUS; ++i) {
		const pile *from = &slr->piles[this->tableau_sorted_by_hidden_count[i]];
		if (from->size <= 0)
			continue;

		int from_id = this->tableau_sorted_by_hidden_count[i];
		// * 第一张正面牌的索引
		const int bottom_index = from->first.has_value() ? *from->first : 0;
		// * 第一张正面牌
		auto bottom_card = from->peek_first_face_up();
		const int bottom_rank = bottom_card.rank + 1;

		for (int to_id = kld::PILE_TABLEAU_START; to_id <= kld::PILE_TABLEAU_END; ++to_id) {
			if (from_id == to_id)
				continue;
			const pile *to = &slr->piles[to_id];

			if (to->size > 0) {
				const auto top_card = to->peek_top();
				const int top_rank = top_card.rank + 1;

				// # 情况A: 移动整组正面牌 (bottom rank 正好能接在 top rank 后面)
				if (bottom_rank == top_rank - 1) {
					// # 检查花色是否红黑交替,且该移动不是已经确定的"安全收纳"移动

					if (diff_color(bottom_card, top_card) && !contains(t2f_safe, from_id,
					                                                   static_cast<int>(from->face_up_count()))) {
						if (bottom_index > 0) {
							// # 优先级高: 能翻开背面牌
							t2t_flip.emplace_back(from_id, to_id, from->face_up_count(), true);
						} else if (has_king_block && this->first_empty_tableau_stack_id == -1) {
							// # 优先级中: 没有空位放K,且存在被压住的K时的一种特殊策略分类
							t2t_empty_t2t_flip.emplace_back(from_id, to_id, from->face_up_count(), false);
						} else {
							// # 优先级低: 普通的移动
							t2t_empty.emplace_back(from_id, to_id, from->face_up_count(), false);
						}
					}
				} else if (bottom_rank >= top_rank) {
					// # 情况B: 拆分正面牌组(只有一部分能接上去)
					const int move_rank = top_rank - 1;
					const int move_index = bottom_index + (bottom_rank - move_rank);
					if (move_index >= from->size)
						continue;
					auto move_card = from->get(move_index);
					const auto move_count = from->size - move_index;

					if (diff_color(move_card, top_card) && !contains(t2f_safe, from_id, static_cast<int>(move_count))) {
						// # 移动后露出的那张牌
						const auto next_card = from->get(move_index - 1);
						// * 关键决策: 如果移走这几张牌,露出的那张牌正好可以放进 foundation
						if (this->foundation_top_rank[next_card.suit] == next_card.rank) {
							if (move_index == bottom_index + 1 && bottom_index > 0) {
								t2t_t2f_flip.emplace_back(from_id, to_id, move_count, false);
							} else {
								t2t_t2f.emplace_back(from_id, to_id, move_count, false);
							}
						}
					}
				}
			} else {
				// # 情况C: 目标位置是空位

				// # 只有K可以移动到空位
				if (bottom_rank == 13 && to_id == this->first_empty_tableau_stack_id && !contains(
					    t2f_safe, from_id, static_cast<int>(from->face_up_count()))) {
					if (bottom_index > 0) {
						// # 移动K以翻开下面的牌
						t2t_flip.emplace_back(from_id, to_id, from->face_up_count(), true);
					}
				}
			}
		}
	}
}

void hint::update_waste_to_tableau() {
	// # 1. 获取 waste 废牌堆
	const pile *from_pile = &this->slr->piles[kld::PILE_WASTE];
	if (from_pile->size <= 0)
		return;

	// # 2. 优先级检查: 如果这张牌已经计划好要进入foundation(w2f_safe),则不考虑移动到牌堆
	// # 这是为了防止AI浪费这张可以安全回收的牌
	if (contains(w2f_safe, kld::PILE_WASTE, 1))
		return;

	const card_ext move_card = from_pile->peek_top();
	const int move_rank = move_card.rank + 1;

	// # 3. 遍历7个 tableau 寻找落脚点
	for (int to_id = kld::PILE_TABLEAU_START; to_id <= kld::PILE_TABLEAU_END; to_id++) {
		const pile *to_pile = &slr->piles[to_id];
		if (to_pile->size > 0) {
			// # 情况A: 目标牌堆不为空,检查是否符合"红黑交替,点数小1"的规则
			const card_ext top_card = to_pile->peek_top();
			if (move_rank == top_card.rank && diff_color(move_card, top_card)) {
				// ? 预测: 如果把废牌堆这张牌放过去,是否能带动手牌堆的其他移动并翻开背面牌?
				if (check_next_step_tableau_to_tableau_flip(to_id, move_card)) {
					this->w2t_t2t_flip.emplace_back(kld::PILE_WASTE, to_id, 1, false);
				} else {
					this->w2t.emplace_back(kld::PILE_WASTE, to_id, 1, false);
				}
			}
		} else {
			// # 情况B: 目标牌堆为空,只有K能进,且优先放入记录的第一个空位
			if (move_rank == 13 && to_id == this->first_empty_tableau_stack_id) {
				// # 同样进行预测: 移动K到空位后,是否能诱发后续的翻牌操作
				if (check_next_step_tableau_to_tableau_flip(to_id, move_card)) {
					w2t_t2t_flip.emplace_back(kld::PILE_WASTE, to_id, 1, false);
				} else {
					w2t.emplace_back(kld::PILE_WASTE, to_id, 1, false);
				}
			}
		}
	}
}

void hint::update_foundation_to_tableau() {
	const pile *waste = &slr->piles[kld::PILE_WASTE];
	bool check_f2t_w2t = false; // # 这个只有在draw_count==3的时候才生效

	// ? maybe: 从solver中获取真正的值
	const int draw_count = 1; // 1或3

	// # 1. 判定是否需要开启"回撤以解救废牌"的逻辑
	if (draw_count == 1) {
		// # 发牌发1张的模式通常不需要复杂的 foundation to tableau 的逻辑
		check_f2t_w2t = false;
	} else if (waste->size == 0) {
		check_f2t_w2t = false;
	} else {
		// # 检查waste当前的顶牌是否已经有路可走了
		bool has_w2tf = contains(w2f_safe, kld::PILE_WASTE, 1)
		                || contains(w2f, kld::PILE_WASTE, 1)
		                || contains(w2t_t2t_flip, kld::PILE_WASTE, 1)
		                || contains(w2t, kld::PILE_WASTE, 1);
		// # 如果waste顶牌无路可去(!has_w2tf),则开启回撤逻辑,看看能不能从foundation拿张牌回来接应它
		check_f2t_w2t = !has_w2tf;
	}

	// # 2. 遍历4个foundation
	for (int from_id = kld::PILE_FOUNDATION_START; from_id <= kld::PILE_FOUNDATION_END; ++from_id) {
		const pile *from_stack = &this->slr->piles[from_id];
		if (from_stack->size <= 0)
			continue;
		auto move_card = from_stack->peek_top();
		const int move_rank = move_card.rank + 1;

		// # 3. 遍历7个tableau寻找回撤点
		for (int to_id = kld::PILE_TABLEAU_START; to_id <= kld::PILE_TABLEAU_END; ++to_id) {
			const pile *to_stack = &slr->piles[to_id];
			// # 情况A: 回撤到有牌的牌堆
			if (to_stack->size > 0) {
				auto top_card = to_stack->peek_top();
				if (move_rank == top_card.rank && diff_color(move_card, top_card)) {
					// # 优先级1: 回撤后能立即带动牌堆间的翻牌 (t2t_flip)
					if (check_next_step_tableau_to_tableau_flip(to_id, move_card)) {
						f2t_t2t_flip.emplace_back(from_id, to_id, 1, false);
					} else if (check_f2t_w2t) {
						// # 优先级2: 回撤后能接应waste里的牌(w2t)
						const card_ext waste_card = waste->peek_top();
						if (waste_card.rank + 1 == move_card.rank && diff_color(waste_card, move_card)) {
							f2t_w2t.emplace_back(from_id, to_id, 1, false);
						}
					}
				}
			} else if (move_rank == 13 && to_id == this->first_empty_tableau_stack_id) {
				// # 情况B: 回撤K到空位
				if (check_next_step_tableau_to_tableau_flip(to_id, move_card)) {
					this->f2t_t2t_flip.emplace_back(from_id, to_id, 1, false);
				} else if (check_f2t_w2t) {
					const card_ext waste_card = waste->peek_top();
					if (waste_card.rank + 1 == move_card.rank && diff_color(waste_card, move_card)) {
						this->f2t_w2t.emplace_back(from_id, to_id, 1, false);
					}
				}
			}
		}
	}
}

void hint::merge() {
	// # 在合并之前,需要清空主列表all
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
	// # 1. 获取刚刚执行过的最后一步
	const motion *last_move = &this->slr->last_move;
	if (last_move == nullptr)
		return;

	// # 2. 构造 "逆操作" 的特征
	// ? 如果上一步是 from->to, 那么逆操作就是 to->from
	const int reverse_from = last_move->to();
	const int reverse_to = last_move->from();
	const int reverse_count = last_move->count();

	// # 3. 在所有候选移动 (all列表) 中寻找这个逆操作
	for (int i = 0, n = this->all.size(); i < n; ++i) {
		motion move = this->all[i];

		// # 检查当前候选移动是否正好是上一步的撤销动作

		if (move.from() == reverse_from && move.to() == reverse_to && move.count() == reverse_count) {
			// # 4. 发现死循环风险!
			// # 将该移动从当前位置移除
			this->all.erase(all.begin() + i);

			// # 将其重新添加到列表的最末尾 (赋予它最低的优先级)
			// # 这样只有在完全没有其他任何路可走时,AI才会考虑搬回来
			this->all.push_back(move);

			// # 找到一个逆操作即可退出循环
			break;
		}
	}
}

void hint::update_auto() {
	// # 遍历所有已排序并筛选出的候选移动 (all列表)
	for (auto move: this->all) {
		// # 检查auto列表中是否已经存在来自同一个起始堆栈且移动牌数相同的指定
		// # 这是为了防止在同一帧内对同一个牌堆下达多个冲突的移动指令

		if (!contains(auto_moves, move.from(), move.count())) {
			auto_moves.push_back(move);
		}
	}
}

bool hint::check_next_step_tableau_to_tableau_flip(const int to_id, const card_ext top_card) const {
	const int top_rank = top_card.rank + 1;

	// # 遍历7个牌堆,寻找是否有一叠牌正等着这张top_card作为垫脚石
	for (int from_id = kld::PILE_TABLEAU_START; from_id <= kld::PILE_TABLEAU_END; ++from_id) {
		// # 不能自己搬给自己
		if (from_id != to_id) {
			const pile *from = &this->slr->piles[from_id];
			if (from->first.has_value() && *from->first > 0) {
				const int bottom_index = *from->first;
				// # 只有当该牌堆有隐藏牌时,这种搬运才有意义(因为目标是flip)
				auto bottom_card = from->get(bottom_index); // 第一张正面牌

				// # 检查这叠牌是否能接在刚刚放入的top_card后面
				if (bottom_card.rank + 1 == top_rank - 1 && diff_color(bottom_card, top_card)) {
					const auto count = from->face_up_count();

					// # 关键过滤: 如果这个移动本身已经在其他高优先级列表中 (比如已经算好能够翻牌了),就不重复计算它的"预测收益"了
					if (!contains(t2t_flip, from_id, static_cast<int>(count))
					    && !contains(t2f_safe, from_id, static_cast<int>(count))
					    && !contains(t2f_flip, from_id, static_cast<int>(count))) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

void hint::clear() {
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
	cached = false;
}

motion hint::get(const vector<motion> &list, const int from, const int count) {
	for (auto move: list) {
		if (move.from() == from && move.count() == count) {
			return move;
		}
	}
	return {};
}

bool hint::contains(const vector<motion> &list, const int from, const int count) {
	return !get(list, from, count).is_null();
}

bool hint::diff_color(const card_ext &a, const card_ext &b) {
	const bool flag1 = (a.suit == 0 || a.suit == 2) && (b.suit == 1 || b.suit == 3);
	const bool flag2 = (a.suit == 1 || a.suit == 3) && (b.suit == 0 || b.suit == 2);
	return flag1 || flag2;
}
