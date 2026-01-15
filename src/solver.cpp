//
// Created by baizeyv on 1/13/2026.
//

#include "solver.h"

#include <algorithm>
#include <chrono>
#include <queue>
#include <unordered_map>

#include "action.h"
#include "../helper/xxhash.h"
#include "motion_index.h"

solver::solver() : draw_count(1), foundation_score(0), initial_foundation_score(0),
                   foundation_minimum(0),
                   moves_total(0), round_count(1), last_move(motion()) {
	// # 初始化花色到foundation的映射表
	// # 这里将其填充为4,标识当前还没有任何花色分配到回收站
	suits_to_foundations.fill(kld::MAX_SUIT);
	initial_piles.fill(pile());
	piles.fill(pile());
	moves.fill(motion());
}

void solver::setup(const state &st) {
	uint8_t foundation_score_temp = 0;
	uint8_t foundation_slots = 0; // # 位掩码,记录哪些foundation已被占用

	// # 初始化花色到foundation索引的映射表,填充默认值(255)
	ranges::fill(this->suits_to_foundations, static_cast<size_t>(kld::MAX_SUIT));

	// # 1. 同步stock
	{
		auto &pile = this->initial_piles[kld::PILE_STOCK];
		pile.reset();
		for (const auto &cd: st.stock) {
			pile.push_card(card_ext::create_with_id(cd.get_id()));
		}
	}

	// # 2. 同步waste
	{
		auto &pile = this->initial_piles[kld::PILE_WASTE];
		pile.reset();
		for (const auto &cd: st.waste) {
			pile.push_card(card_ext::create_with_id(cd.get_id()));
		}
	}

	// # 3. 同步foundation
	for (size_t i = 0; i < kld::TOTAL_FOUNDATIONS; ++i) {
		auto &pile = this->initial_piles[kld::PILE_FOUNDATION_START + i];
		pile.reset();

		if (st.foundations[i].has_value()) {
			const auto &cd = st.foundations[i].value();
			uint8_t suit = cd.get_suit();
			uint8_t rank = cd.get_value() - 1;

			foundation_score_temp += (rank + 1);

			// # foundation内部需要还原被压住的牌(A,2,3...current)
			for (uint8_t j = 0; j <= rank; ++j) {
				pile.push_card(card_ext::creat_with_rank_suit(j, suit));
			}

			// # 建立花色映射: 当前花色固定对应第i个foundation
			this->suits_to_foundations[suit] = kld::PILE_FOUNDATION_START + i;
			foundation_slots |= (1 << i); // # 标记该槽位已占用
		}
	}

	// # 4. 为没有牌的花色分配剩余的foundation槽位
	for (uint8_t i = 0; i < kld::MAX_SUIT; ++i) {
		if (this->suits_to_foundations[i] == static_cast<size_t>(kld::MAX_SUIT)) {
			for (size_t j = 0; j < kld::TOTAL_FOUNDATIONS; ++j) {
				if (!(foundation_slots & (1 << j))) {
					this->suits_to_foundations[i] = kld::PILE_FOUNDATION_START + j;
					foundation_slots |= (1 << j);
					break;
				}
			}
		}
	}

	// # 5. 同步tableau
	for (size_t i = 0; i < kld::TOTAL_TABLEAUS; ++i) {
		auto &pile = this->initial_piles[kld::PILE_TABLEAU_START + i];
		pile.reset();

		for (const auto &cd: st.tableaus[i].cards) {
			pile.push_card(card_ext::create_with_id(cd.get_id()));
		}
		// # 设置正面朝上的牌数
		pile.set_face_up_count(st.tableaus[i].face_up_count);
	}

	// # 6. 保存快照并执行重置
	this->initial_state = st;
	this->initial_foundation_score = foundation_score_temp;

	// # 将当前运行状态恢复到initial_state定义的状态
	this->reset();
}

solve_result solver::solve(const uint32_t max_nodes, const bool minimal, const bool step_mode) {
	// # 1. 初始化校验
	if (!this->initial_state.is_valid()) {
		throw runtime_error("invalid initial poker state");
	}
	// # 优先队列(open set),按启发式分值从小到大排列
	std::priority_queue<motion_index> open;

	// # 状态记录表(closed set),用于剪枝,记录已经访问过的状态及其实际代价
	state_map closed(max_nodes);

	// # 节点池,用于回溯路径,使用预分配空间避免频繁内存申请
	vector<motion_node> node_storage(max_nodes + 1);

	uint32_t node_count = 0;
	uint8_t max_foundation_score = 0;
	vector<motion> possible_moves;
	array<motion, kld::MAX_MOVES> moves_history;

	// # 2. 初始状态入队
	const estimation init_est = {0, this->minimum_moves_remaining(false)};
	auto k = this->get_state();
	closed.insert(k, init_est);

	// # node_storage[0] 作为根节点
	node_storage[node_count] = motion_node();
	open.push(motion_index(node_count, 0, init_est));
	node_count++;

	// # 当前已经找到的最优步骤(默认255)
	uint8_t best_solution_move_count = 255;
	std::optional<uint32_t> solution_node_index = std::nullopt;
	const auto start_time = std::chrono::steady_clock::now();

	// # 3. 主循环
	while (!open.empty()) {
		if (node_count >= max_nodes)
			break;

		// # 弹出当前最优节点
		motion_index current_node = open.top();
		open.pop();

		// # 剪枝: 如果当前路径成本已经超过已经找到的最优解,跳过
		if (current_node.est.total() >= best_solution_move_count)
			continue;

		// # 核心步骤: 重构棋盘状态
		// # 这里的逻辑是: 为了节省内存,node_storage 只存最后一步和父节点索引
		// # 我们需要通过回溯把棋盘恢复到当前节点对应的样子
		uint8_t moves_to_make = node_storage[current_node.index].copy_path(moves_history, node_storage); // todo:
		this->reset();
		for (int i = moves_to_make - 1; i >= 0; --i) {
			this->make_move(moves_history[i]);
		}

		// # 步骤模式
		if (step_mode) {
			while (next_step == 0) {
				if (abort_step == 1) {
					break;
				}
			}
			next_step = 0;
			std::cout << "============================================" << std::endl;
			std::cout << std::endl << to_str() << std::endl;
		}

		// # 4. 生成当前状态下的所有合法移动
		possible_moves.clear();
		this->compute_possible_moves(possible_moves);

		for (const auto &mov: possible_moves) {
			// # 计算此步产生的实际代价(有些移动包含自动执行的后续步骤)
			const uint8_t add_moves = this->calculate_additional_moves(mov);
			this->make_move(mov);
			estimation new_est = {
				static_cast<uint8_t>(current_node.est.current + add_moves),
				this->minimum_moves_remaining(this->round_count == kld::MAX_ROUNDS)
			};

			// # 5. 状态检查与剪枝
			if (new_est.total() < best_solution_move_count && this->round_count <= kld::MAX_ROUNDS) {
				auto key = this->get_state();
				bool should_skip = false;

				auto r = closed.get(key);
				if (r.has_value()) {
					const auto [fst, snd] = *r;
					if (fst->total() > new_est.total()) {
						closed.estimation_mut(snd) = new_est;
					} else {
						should_skip = true;
					}
				} else {
					closed.insert(key, new_est);
				}

				if (!should_skip) {
					// # 记录新节点
					node_storage[node_count] = motion_node{current_node.index, mov};
					bool solved = (this->foundation_score == 52);
					if (this->foundation_score > max_foundation_score || solved) {
						solution_node_index = node_count;
						max_foundation_score = this->foundation_score;
					}

					if (solved) {
						best_solution_move_count = new_est.total();
						node_count++;
						if (!minimal) {
							while (!open.empty())
								open.pop();
							goto found_solution;
						}
					} else {
						// # 计算 A* 启发式分值 f(n) = g(n) + h(n)
						// # 这里还加入了一些权重微调 (如round_count)
						const int heuristic =
								(new_est.total() << 1) + add_moves + (52 - this->foundation_score)
								+ (this->round_count << 1);
						open.push(motion_index{node_count, static_cast<int16_t>(heuristic), new_est});
						node_count++;
					}
				}
			}

			if (step_mode) {
				std::cout << std::endl << to_str() << std::endl;
				std::cout << std::to_string(new_est.total()) << std::endl;
				std::cout << "#######################################" << std::endl;
			}

			// # 回溯棋盘以便尝试下一个possible_move
			this->undo_move();
			if (node_count >= max_nodes)
				break;
		}
	}

found_solution:
	// # 6. 结果导出
	if (!solution_node_index.has_value() || max_foundation_score < 52) {
		throw runtime_error("no solution found.");
	}

	// # 最终将棋盘状态置为解出的终局
	this->reset();
	const uint8_t final_moves = node_storage[*solution_node_index].copy_path(moves_history, node_storage);
	for (int i = final_moves - 1; i >= 0; --i) {
		this->make_move(moves_history[i]);
	}

	const auto end_time = std::chrono::steady_clock::now();

	return {
		minimal && node_count < max_nodes, static_cast<int32_t>(node_count), end_time - start_time, export_actions()
	};
}

state_key solver::get_state() const {
	string res;
	for (const auto &item: piles) {
		res += "/";
		res += std::to_string(item.face_up_count());
		for (size_t i = 0; i < item.size; ++i) {
			const uint8_t id = item.cards[i].id;
			if (id < 26) {
				const char c = 'a' + id;
				res += c;
			} else {
				const char c = 'A' - 26 + id;
				res += c;
			}
		}
	}
	const XXH128_hash_t hash = XXH3_128bits(res.data(), res.size());
	return {hash.low64, hash.high64};
}

uint8_t solver::minimum_moves_remaining(const bool is_last_round) const {
	const pile &waste_pile = this->piles[kld::PILE_WASTE];
	const size_t waste_size = waste_pile.size;
	const size_t stock_size = this->piles[kld::PILE_STOCK].size;
	const size_t d_count = this->draw_count;

	// # 基础步数计算
	// # stock_size: 发牌堆剩下的牌每张都要移动出来 (1步骤)
	// # stock_size.div_ceil: 翻开这些牌所需的点击次数
	// # waste_size: 废牌堆里现有的牌每张都要移走 (1步)
	size_t num = stock_size + (stock_size + d_count - 1) / d_count + waste_size;

	// # 记录每个花色再当前扫描堆中见到的最小点数
	array<uint8_t, kld::MAX_SUIT> mins;

	// # 1. 处理waste的阻塞开销
	if (d_count == 1 || is_last_round) {
		mins.fill(255); // # 初始化为最大值
		for (size_t i = 0; i < waste_size; ++i) {
			card_ext card = waste_pile.get(i);
			uint8_t suit_idx = card.suit;
			if (card.rank < mins[suit_idx]) {
				mins[suit_idx] = card.rank;
			} else {
				// # 如果一张点数大的牌压在点数小的牌上面
				// # 意味着这张牌必须先移动到tableau 才能腾出下面的牌
				num += 1;
			}
		}
	}

	// # 2. 处理7个tableau的阻塞开销
	for (size_t i = kld::PILE_TABLEAU_START; i <= kld::PILE_TABLEAU_END; ++i) {
		mins.fill(255);
		const pile &pile = this->piles[i];
		num += pile.size; // # 基础步数: 每张牌一如foundation算1步

		// # 计算第一张正面拍的索引
		const size_t first_face_up_idx = pile.size - pile.face_up_count();

		for (size_t j = 0; j < pile.size; ++j) {
			card_ext card = pile.get(j);
			uint8_t suit_idx = card.suit;

			// # 检查逻辑: 如果较大的牌挡住了较小的牌
			if (card.rank < mins[suit_idx]) {
				// # 如果这张牌是背面朝上的 (j < first), 更新该花色的最小值
				if (j < first_face_up_idx) {
					mins[suit_idx] = card.rank;
				}
			} else {
				// # 阻塞: 必须有额外的一步将这张牌移开
				num += 1;
				// # 如果这张牌已经是正面朝上的,那么它造成的阻塞逻辑到此为止
				if (j >= first_face_up_idx) {
					break;
				}
			}
		}
	}
	return static_cast<uint8_t>(num);
}

uint8_t solver::calculate_additional_moves(motion mov) const {
	// # 基础移动(把牌放好)记1步
	uint8_t count = 1;
	const uint8_t mov_count = static_cast<uint8_t>(mov.count());

	// # 如果是从 wast 移动牌, 且涉及到翻牌操作
	if (mov.from() == static_cast<uint8_t>(kld::PILE_WASTE) && mov_count != 0) {
		uint8_t d_count = this->draw_count; // 1 或 3
		if (!mov.flip()) {
			// # 场景A: 普通翻牌
			// # 步数 = 向上取整(需要翻出的牌数/每次翻牌张数)
			count += (mov_count + d_count - 1) / d_count;
		} else {
			// # 场景B: 涉及重新洗牌 (redeal)
			// # 这意味着需要先把当前stock翻完,执行一次redeal,再从头翻出剩下的牌
			uint8_t stock_size = static_cast<uint8_t>(this->piles[kld::PILE_STOCK].size);

			// # 1. 翻完当前剩余stock的步数
			count += (stock_size + d_count - 1) / d_count;

			// # 2. redeal后,继续翻出目标牌所需的步数
			// ! 注意: 此时原waste已经变成新的stock
			count += (mov_count - stock_size + d_count - 1) / d_count;
		}
	}
	return count;
}

void solver::compute_possible_moves(possible_moves &pm) {
	// # 1. 更新安全收牌水位线 (foundation_minimum)
	// # 逻辑: 找到4个foundation中最小的长度, + 1 作为安全阈值
	size_t min_size = this->piles[kld::PILE_FOUNDATION_START].size;
	for (size_t i = kld::PILE_FOUNDATION_START + 1; i <= kld::PILE_FOUNDATION_END; ++i) {
		if (this->piles[i].size < min_size) {
			min_size = this->piles[i].size;
		}
	}

	// # foundation_minimum 用于后续子函数判断无脑收牌是否安全
	this->foundation_minimum = static_cast<uint8_t>(min_size) + 1;

	// # 2. 尝试执行"基于上一步动作"的强制剪枝逻辑
	// # 如果返回true, 说明发现了一个必须要做的动作,直接跳过后续搜索
	if (this->compute_with_last_move(pm)) {
		return;
	}

	// # 3. 尝试从 tableau 寻找动作
	// # 如果发现了符合安全阈值的收牌动作,回清空列表并只保留这个动作,返回true
	if (this->compute_move_from_tableau(pm)) {
		return;
	}

	// # 4. 尝试从 waste/stock 寻找动作
	// # 同样包含安全收牌的快速返回逻辑
	if (this->compute_move_from_waste(pm)) {
		return;
	}

	// # 5. 最后考虑从foundation拿回tableau的低优先级动作
	// # 这个方法永远返回false,因为它不会触发确定性剪枝
	this->compute_move_from_foundation(pm);
}

bool solver::compute_with_last_move(possible_moves &pm) const {
	// # 1. 获取上一个动作的属性
	auto [move_from, move_to, move_count, move_flip] = this->last_move.values();

	// # 2. 检查特定条件:
	// # - 上一步是从一个牌阵移动到另一个牌阵
	// # - 并且上一步没有触发翻牌 (说明移动的是一叠已经翻开的牌的一部分)
	if (move_from >= kld::PILE_TABLEAU_START && move_from <= kld::PILE_TABLEAU_END && move_to >= kld::PILE_TABLEAU_START
	    && move_to <=
	    kld::PILE_TABLEAU_END && !move_flip) {
		const pile &src_pile = this->piles[move_from];
		// # 如果源牌堆还有牌,检查新露出的那张牌
		if (src_pile.size > 0) {
			const card_ext src_top_card = src_pile.peek_top_unchecked();
			// # 3. 核心逻辑: 如果新露出的牌能进回收站,则强制生成这个动作
			const optional<uint8_t> foundation_idx = this->can_move_to_foundation(src_top_card);
			if (foundation_idx.has_value()) {
				// # 判断执行此动作是否需要翻开新牌
				// # 条件: 牌堆里还有多余一张牌,且当前可见牌只有这一张
				const bool will_flip = (src_pile.size > 1 && src_pile.face_up_count() == 1);

				pm.emplace_back(static_cast<uint8_t>(move_from), *foundation_idx, 1, will_flip);
				return true; // # 返回true标识发现了一个高优先级动作
			}
		}
	}
	return false;
}

bool solver::compute_move_from_waste(possible_moves &pm) {
	uint8_t d_count = this->draw_count; // 1或3

	// # 1. 调用 talon_helper 计算当前 stock/waste 序列中所有可触达的牌
	const size_t talon_count = this->tl_helper.calculate(d_count, this->piles[kld::PILE_WASTE],
	                                                     this->piles[kld::PILE_STOCK]);
	for (size_t idx = 0; idx < talon_count; ++idx) {
		card_ext talon_card = this->tl_helper.stock_waste[idx];
		int32_t cards_to_draw = this->tl_helper.cards_drawn[idx];

		// # 如果 cards_drawn 是负数, 说明需要一次 redeal
		bool flip = cards_to_draw < 0;
		if (flip) {
			cards_to_draw = -cards_to_draw;
		}

		// # 逻辑A: 检查是否能进入 foundation
		optional<uint8_t> foundation_idx = this->can_move_to_foundation(talon_card);
		if (foundation_idx.has_value()) {
			pm.emplace_back(static_cast<uint8_t>(kld::PILE_WASTE), *foundation_idx,
			                static_cast<uint8_t>(cards_to_draw),
			                flip);

			// # 安全收派剪枝: 如果这张牌的点数小于等于 foundation_minimum,
			// # 说明收掉它绝对安全,不会导致死局
			if (talon_card.rank <= this->foundation_minimum) {
				if (d_count > 1) {
					// # 翻3张模式下,由于哪掉一张牌回改变后续序列,所以步立即返回,继续找
					continue;
				}
				// # 翻1张模式下,如果能直接拿(0步)或列表只有这一个动作,直接锁定
				if (cards_to_draw == 0 || pm.size() == 1) {
					return true;
				}
				break;
			}
		}

		// # 逻辑B: 检查是否能够移到牌阵 (tableau)
		for (uint8_t t_idx = kld::PILE_TABLEAU_START; t_idx <= kld::PILE_TABLEAU_END; ++t_idx) {
			card_ext tableau_top_card = this->piles[t_idx].peek_top();

			// # 规则: 点数小1且颜色不同(K不能通过此逻辑移动,因为它只能去空位)
			if (static_cast<int32_t>(tableau_top_card.rank) - static_cast<int32_t>(talon_card.rank) == 1 && talon_card.
			    is_red !=
			    tableau_top_card.is_red) {
				pm.emplace_back(static_cast<uint8_t>(kld::PILE_WASTE), t_idx, static_cast<uint8_t>(cards_to_draw),
				                flip);

				// # 如果是K移动到了空位(或者满足条件的tableau),不再尝试其他位置
				if (talon_card.is_king()) {
					break;
				}
			}
		}
	}
	return false;
}

bool solver::compute_move_from_foundation(possible_moves &pm) const {
	// # 遍历4个foundation
	for (uint8_t f_idx = kld::PILE_FOUNDATION_START; f_idx <= kld::PILE_FOUNDATION_END; ++f_idx) {
		const pile &f_pile = this->piles[f_idx];

		// # 剪枝逻辑: 如果foundation里的牌点数 <= foundation_minimum, 则绝对不拿回来
		// # 因为这些牌是安全的,拿回来之后增加步数,不会吧创造更多可能
		if (f_pile.size <= static_cast<size_t>(this->foundation_minimum))
			continue;

		// # 获取foundation顶部的牌
		card_ext f_card = f_pile.peek_top_unchecked();

		// # 遍历7个 tableau
		for (uint8_t t_idx = kld::PILE_TABLEAU_START; t_idx <= kld::PILE_TABLEAU_END; ++t_idx) {
			const card_ext t_top_card = this->piles[t_idx].peek_top();

			// # 规则: tableau顶部的牌点数比foundation的牌大1,且颜色不同
			if (static_cast<int32_t>(t_top_card.rank) - static_cast<int32_t>(f_card.rank) == 1 && t_top_card.is_red !=
			    f_card.is_red) {
				// # 创建动作: 从foundation(f_idx)到tableau(t_idx),数量1,不涉及翻牌
				pm.emplace_back(f_idx, t_idx, 1, false);

				// # 如果是 K (索然逻辑上K不会进foundation后再回tableau,但这里保持逻辑完整)
				if (f_card.is_king())
					break;
			}
		}
	}
	return false;
}

bool solver::compute_move_from_tableau(possible_moves &pm) {
	vector<uint8_t> none_empty_tableaus;
	int empty_tableaus_count = 0;

	// # 1. 预扫描: 区分空堆和非空堆
	for (uint8_t idx = kld::PILE_TABLEAU_START; idx <= kld::PILE_TABLEAU_END; ++idx) {
		if (this->piles[idx].size > 0) {
			none_empty_tableaus.push_back(idx);
		} else {
			empty_tableaus_count += 1;
		}
	}

	// # 2. 遍历每一个非空牌tableau作为"源堆"(from)
	for (const uint8_t src_idx: none_empty_tableaus) {
		pile &src_pile = this->piles[src_idx];
		size_t src_pile_size = src_pile.size;

		// ! --- 逻辑A: 检查顶牌是否能进入foundation ---
		card_ext src_top_card = src_pile.peek_top_unchecked();
		optional<uint8_t> foundation_idx = this->can_move_to_foundation(src_top_card);
		if (foundation_idx.has_value()) {
			// # 是否需要翻开下方的牌
			bool will_flip = (src_pile_size > 1 && src_pile.face_up_count() == 1);
			motion mov(src_idx, *foundation_idx, 1, will_flip);

			// # 确定性剪枝: 如果这张牌点数足够小,直接执行,清空其他可能
			if (src_top_card.rank <= this->foundation_minimum) {
				pm.clear();
				pm.push_back(mov);
				return true;
			} else {
				pm.push_back(mov);
			}
		}

		// ! --- 逻辑B: 检查tableau之间的移动 (移动整叠或部分) ---
		card_ext src_first_face_up_card = src_pile.peek_first_face_up_unchecked();
		// # 计算当前有多少张牌是正面朝上的
		int32_t src_face_up_count = static_cast<int32_t>(src_first_face_up_card.rank) - static_cast<int32_t>(
			                            src_top_card.rank) + 1;

		// # 标记 K 是否已经尝试移入空列,避免重复生成到不同空位的动作
		bool king_moved = !src_first_face_up_card.is_king();
		for (uint8_t dest_idx = kld::PILE_TABLEAU_START; dest_idx <= kld::PILE_TABLEAU_END; ++dest_idx) {
			if (src_idx == dest_idx)
				continue;
			pile &dest_pile = this->piles[dest_idx];

			// # 场景1: 目标堆为空 (只能放K)
			if (dest_pile.size == 0) {
				// # 如果这叠牌的最底部是K,且移动它可以翻开下方的背面牌
				if (!king_moved && static_cast<int32_t>(src_pile_size) != src_face_up_count) {
					pm.emplace_back(src_idx, dest_idx, (uint8_t) src_face_up_count, true);
					king_moved = true; // # 一个K移动到一个空位即可,不需要尝试所有空位
				}
				continue;
			}

			// # 场景2: 目标堆不为空
			card_ext dest_top_card = dest_pile.peek_top_unchecked();

			// # 基础检查: 颜色交替且点数递减
			// # 优化技巧: 利用red_even属性快速判断颜色和奇偶匹配
			if (static_cast<int32_t>(dest_top_card.rank) - static_cast<int32_t>(src_first_face_up_card.rank) > 1
			    || src_top_card.red_even != dest_top_card.red_even
			    || src_top_card.rank >= dest_top_card.rank) {
				continue;
			}

			// # 计算需要从源堆移动多少张牌
			const int32_t src_moved_count = static_cast<int32_t>(dest_top_card.rank) - static_cast<int32_t>(src_top_card
				                                .rank);
			// # 核心逻辑：判断此移动是否有价值
			// ! 1. 移动整叠正面朝上的牌，且能翻开新牌（或没有空位时平衡牌堆）
			// ! 2. 移动部分牌（Splitting），条件是移动后露出的那张牌能立即进回收站
			if ((src_moved_count == src_face_up_count && (
				     src_moved_count != static_cast<int32_t>(src_pile_size) || empty_tableaus_count == 0))
			    || (src_moved_count < src_face_up_count &&
			        this->can_move_to_foundation(
				        src_pile.peek_nth_from_top_unchecked(static_cast<size_t>(src_moved_count))).
			        has_value())) {
				const bool flip_after_move = (src_pile_size > static_cast<size_t>(src_moved_count) && src_moved_count ==
				                              src_face_up_count);
				pm.emplace_back(src_idx, dest_idx, static_cast<uint8_t>(src_moved_count), flip_after_move);
			}
		}
	}

	return false;
}

void solver::make_move(const motion mov) {
	// # 1. 记录动作历史
	this->moves[this->moves_total] = mov;
	this->moves_total += 1;
	this->last_move = mov;

	// # 解析动作原始数据: 来源、去向、数量、是否翻牌
	auto [move_from, move_to, move_count, move_flip] = mov.values();

	// # 2. 特殊逻辑: 处理发牌堆(stock)和废牌堆(waste)的交互
	if (move_from == kld::PILE_WASTE && move_count != 0) {
		if (!move_flip) {
			// # 普通发牌: 将n张牌从stock翻转移动到waste
			this->piles[kld::PILE_STOCK].move_n_cards_reversed_to(this->piles[kld::PILE_WASTE], move_count);
		} else {
			// # 重新发牌(redeal)逻辑
			this->round_count += 1;
			const int32_t size = static_cast<int32_t>(this->piles[kld::PILE_STOCK].size)
			                     + static_cast<int32_t>(this->piles[kld::PILE_WASTE].size)
			                     - static_cast<int32_t>(move_count);
			if (size >= 1) {
				// # 将waste全体翻回stock
				this->piles[kld::PILE_WASTE].move_n_cards_reversed_to(this->piles[kld::PILE_STOCK],
				                                                      static_cast<size_t>(size));
			} else {
				// # 特殊边缘情况: stock已经干了
				this->piles[kld::PILE_STOCK].move_n_cards_reversed_to(this->piles[kld::PILE_WASTE],
				                                                      static_cast<size_t>(-size));
			}
		}
	}

	// # 3. 通用逻辑: 移动卡牌
	if (move_from == kld::PILE_WASTE || move_count == 1) {
		// # 单张移动 (或者从废牌堆移动)
		this->piles[move_from].pop_card_to(this->piles[move_to]);

		// # 更新foundation分数
		if (move_to >= kld::PILE_FOUNDATION_START && move_to <= kld::PILE_FOUNDATION_END) {
			this->foundation_score += 1;
		} else if (move_from >= kld::PILE_FOUNDATION_START && move_from <= kld::PILE_FOUNDATION_END) {
			// # 极其罕见: 从foundation把牌拿回tableau
			this->foundation_score -= 1;
		}
	} else {
		// # 多张移动; 通常发生再 tableau 之间 (移动一串排好的牌)
		this->piles[move_from].move_n_cards_to(this->piles[move_to], move_count);
	}

	// # 4. 翻牌逻辑: 如果移动走牌后,原牌堆顶部是背面朝上的,将其翻开
	if (move_flip && (move_from >= kld::PILE_TABLEAU_START && move_from <= kld::PILE_TABLEAU_END)) {
		this->piles[move_from].set_face_up_count(1);
	}
}

void solver::undo_move() {
	// # 1. 回退步数统计
	this->moves_total -= 1;

	// # 获取刚刚执行的那个动作
	motion mov = this->moves[this->moves_total];

	// # 恢复 last_move 为再上一个动作
	this->last_move = (this->moves_total > 0) ? this->moves[this->moves_total - 1] : motion();

	// # 解析动作数据
	auto [move_from, move_to, move_count, move_flip] = mov.values();

	// # 2. 撤销通用卡牌移动 (注意: 方向与make_move相反,从to回到from)
	if (move_from == kld::PILE_WASTE || move_count == 1) {
		// # 单张回推
		this->piles[move_to].pop_card_to(this->piles[move_from]);

		// # 恢复foundation分数
		if (move_to >= kld::PILE_FOUNDATION_START && move_to <= kld::PILE_FOUNDATION_END) {
			this->foundation_score -= 1;
		} else if (move_from >= kld::PILE_FOUNDATION_START && move_from <= kld::PILE_FOUNDATION_END) {
			this->foundation_score += 1;
		}
	} else {
		// # 多张回推 (整块移动)
		this->piles[move_to].move_n_cards_to(this->piles[move_from], move_count);
	}

	// # 3. 撤销翻牌状态
	// # 如果 move_flip 为 true, 说明当时移走牌后翻开了下一张,现在要把它盖回去
	if (move_flip && (move_from >= kld::PILE_TABLEAU_START && move_from <= kld::PILE_TABLEAU_END)) {
		// # 将可见牌数设回当时的 move_count
		this->piles[move_from].set_face_up_count(move_count);
	}

	// # 4. 撤销发牌(stock/waste)的复杂交互
	if (move_from == kld::PILE_WASTE && move_count != 0) {
		if (!move_flip) {
			// # 撤销普通发牌: 从waste翻转移回stock
			this->piles[kld::PILE_WASTE].move_n_cards_reversed_to(this->piles[kld::PILE_STOCK], move_count);
		} else {
			// # 撤销重新发牌(redeal)
			this->round_count -= 1;
			int32_t size = static_cast<int32_t>(this->piles[kld::PILE_STOCK].size)
			               + static_cast<int32_t>(this->piles[kld::PILE_WASTE].size)
			               - static_cast<int32_t>(move_count);
			if (size >= 1) {
				// # 将 stock 中的牌反转移回 waste
				this->piles[kld::PILE_STOCK].move_n_cards_reversed_to(this->piles[kld::PILE_WASTE],
				                                                      static_cast<size_t>(size));
			} else {
				// # 边缘情况处理
				this->piles[kld::PILE_WASTE].move_n_cards_reversed_to(this->piles[kld::PILE_STOCK],
				                                                      static_cast<size_t>(-size));
			}
		}
	}
}

optional<uint8_t> solver::can_move_to_foundation(card_ext cd) const {
	// # 1. 安全检查
	if (cd.is_unknown()) {
		return std::nullopt;
	}

	// # 2. O(1) 查找该花色队医的foundation堆索引
	// ! 注意:这里使用了 suits_to_foundations 映射表
	const uint8_t idx = this->suits_to_foundations[cd.suit];

	// # 3. 核心逻辑: 判断点数是否匹配
	// # card.rank 是 0-12
	// # foundation.size 恰好代表了下一张需要的点数
	if (this->piles[idx].size == static_cast<size_t>(cd.rank)) {
		return idx;
	}
	return std::nullopt;
}

void solver::reset() {
	// # 1. 恢复当前牌堆状态 (使用 std::array 的拷贝赋值)
	this->piles = this->initial_piles;
	// # 2. 重置基础分数和统计数据
	this->foundation_score = this->initial_foundation_score;
	// # 3. 重置收牌阈值 (Foundation Minimum)
	// 这是自动收牌逻辑的判断标准，重置为 0 表示重新开始评估
	this->foundation_minimum = 0;
	// # 4. 重置步数统计和发牌轮数
	this->moves_total = 0;
	this->round_count = 1;
	// # 5. 清除上一步记录
	this->last_move = motion();
}

vector<action> solver::export_actions() const {
	vector<action> actions;
	// # 初始化模拟状态
	size_t stock_size = this->initial_piles[kld::PILE_STOCK].size;
	size_t waste_size = this->initial_piles[kld::PILE_WASTE].size;
	constexpr size_t d_count = 1; // # 每次翻牌的数量

	// # 克隆初始棋盘进行模拟演练
	state st = this->initial_state;

	for (size_t i = 0; i < this->moves_total; ++i) {
		const motion &mov = this->moves[i];

		// # 获取移动的具体参数(src,dest,count,是否设置redeal/flip)
		const uint8_t move_from = mov.from();
		const uint8_t move_to = mov.to();
		const uint8_t move_count = mov.count();
		const bool move_flip = mov.flip();

		// # 情况A: 从waste移出
		if (move_from == kld::PILE_WASTE) {
			if (!move_flip) {
				// # 普通翻牌: 计算需要Draw的次数(向上取整)
				size_t times = (move_count + d_count - 1) / d_count;
				for (size_t j = 0; j < times; ++j) {
					actions.push_back(action::draw());
					st.draw();
				}
				stock_size -= move_count;
				waste_size += move_count;
			} else {
				// # 复杂翻牌: 涉及到redeal
				if (stock_size == 0) {
					actions.push_back(action::redeal());
					st.draw();
				}

				// # 第一阶段: 抽干当前stock
				const size_t times = (stock_size + d_count - 1) / d_count;
				for (size_t j = 0; j < times; ++j) {
					actions.push_back(action::draw());
					st.draw();
					if (st.need_redeal()) {
						// # 如果棋盘判定需要redeal
						actions.push_back(action::redeal());
						st.draw();
					}
				}

				// # 第二阶段: 洗牌后继续抽到目标牌
				size_t remaining_draw = (move_count - stock_size + d_count - 1) / d_count;
				for (size_t j = 0; j < remaining_draw; ++j) {
					actions.push_back(action::draw());
					st.draw();
				}

				// # 更新模拟计算器
				int32_t diff = static_cast<int32_t>(stock_size) + static_cast<int32_t>(waste_size) - static_cast<
					               int32_t>(move_count);
				waste_size = static_cast<size_t>(static_cast<int32_t>(waste_size) - diff);
				stock_size = static_cast<size_t>(static_cast<int32_t>(stock_size) + diff);
			}
			// # 最终把那张牌拿走
			waste_size -= 1;

			// # 确定落点: 是去foundation还是tableau
			if (move_to >= kld::PILE_FOUNDATION_START && move_to <= kld::PILE_FOUNDATION_END) {
				const size_t idx = move_to - kld::PILE_FOUNDATION_START;
				actions.push_back(action::waste2foundation(idx));
				st.move_waste_to_foundation(idx);
			} else if (move_to >= kld::PILE_TABLEAU_START && move_to <= kld::PILE_TABLEAU_END) {
				size_t idx = move_to - kld::PILE_TABLEAU_START;
				actions.push_back(action::waste2tableau(idx));
				st.move_waste_to_tableau(idx);
			}
		} else if (move_from >= kld::PILE_TABLEAU_START && move_from <= kld::PILE_TABLEAU_END) {
			// # 情况B: 从tableau移出
			const size_t from_idx = move_from - kld::PILE_TABLEAU_START;
			if (move_to >= kld::PILE_FOUNDATION_START && move_to <= kld::PILE_FOUNDATION_END) {
				const size_t to_idx = move_to - kld::PILE_FOUNDATION_START;
				actions.push_back(action::tableau2foundation(from_idx, to_idx));
				st.move_tableau_to_foundation(from_idx, to_idx);
			} else if (move_to >= kld::PILE_TABLEAU_START && move_to <= kld::PILE_TABLEAU_END) {
				const size_t to_idx = move_to - kld::PILE_TABLEAU_START;
				actions.push_back(action::tableau2tableau(from_idx, to_idx, move_count));
				st.move_tableau_to_tableau(from_idx, to_idx, move_count);
			}
		} else if (move_from >= kld::PILE_FOUNDATION_START && move_from <= kld::PILE_FOUNDATION_END) {
			// # 情况C: 从foundation移动到tableau
			const size_t from_idx = move_from - kld::PILE_FOUNDATION_START;
			if (move_to >= kld::PILE_TABLEAU_START && move_to <= kld::PILE_TABLEAU_END) {
				const size_t to_idx = move_to - kld::PILE_TABLEAU_START;
				actions.push_back(action::foundation2tableau(from_idx, to_idx));
				st.move_foundation_to_tableau(from_idx, to_idx);
			}
		}
	}
	return actions;
}

string solver::to_str() const {
	// # 最终的字符串结果
	string ret;
	// # 最多隐藏牌列的隐藏牌的数量
	int max_hidden = 0;

	for (int i = kld::PILE_TABLEAU_START; i <= kld::PILE_TABLEAU_END; ++i) {
		auto &item = this->piles[i];
		if (item.size > 0) {
			const uint8_t val = *item.first;
			if (val > max_hidden) {
				max_hidden = val;
			}
		}
	}

	// # 最多显示牌列的显示牌的数量
	int max_visible = 0;
	for (int i = kld::PILE_TABLEAU_START; i <= kld::PILE_TABLEAU_END; ++i) {
		auto &item = this->piles[i];
		if (item.size > 0) {
			const uint8_t val = item.size - *item.first;
			if (val > max_visible) {
				max_visible = val;
			}
		}
	}

	ret += hidden_string(0, max_hidden);
	ret += "\n\n";
	ret += visible_string(0, max_visible);
	ret += "\n\n";
	ret += deck_string();
	ret += "\n\n";
	ret += collected_string();
	ret += "----------------------------------";
	return ret;
}

string solver::hidden_string(const int row, const int max) const {
	if (max == 0)
		return "";
	if (row == max - 1)
		return floor_hidden_string(row);
	return floor_hidden_string(row) + "\n" + hidden_string(row + 1, max);
}

string solver::floor_hidden_string(const int row) const {
	string ret;
	for (int i = kld::PILE_TABLEAU_START; i <= kld::PILE_TABLEAU_END; ++i) {
		auto &item = this->piles[i];
		vector<card> v{};
		if (item.size > 0) {
			for (int j = 0; j < *item.first; ++j) {
				const auto ext = &item.cards[j];
				v.emplace_back(ext->rank + 1, ext->suit);
			}
		}
		std::ranges::reverse(v);
		if (v.size() > row) {
			ret += v[v.size() - row - 1].to_str();
		} else {
			ret += kld::empty_card;
		}
	}
	return ret;
}

string solver::visible_string(const int row, const int max) const {
	if (max == 0)
		return "";
	if (row == max - 1)
		return floor_visible_string(row);
	return floor_visible_string(row) + "\n" + visible_string(row + 1, max);
}

string solver::floor_visible_string(const int row) const {
	string ret;
	for (int i = kld::PILE_TABLEAU_START; i <= kld::PILE_TABLEAU_END; ++i) {
		auto &item = this->piles[i];
		vector<card> v{};
		if (item.size > 0) {
			for (int j = *item.first; j < item.size; ++j) {
				const auto ext = &item.cards[j];
				v.emplace_back(ext->rank + 1, ext->suit);
			}
		}
		std::ranges::reverse(v);
		if (v.size() > row) {
			ret += v[v.size() - row - 1].to_str();
		} else {
			ret += kld::empty_card;
		}
	}
	return ret;
}

string solver::deck_string() const {
	string ret;

	ret += kld::empty_card;
	const auto waste = this->piles[kld::PILE_WASTE];
	for (int x = 0; x < waste.size; ++x) {
		const auto ext = waste.get(x);
		card cd(ext.rank + 1, ext.suit);
		ret += cd.to_str();
	}
	ret += "\n";
	ret += kld::empty_card;
	const auto stock = this->piles[kld::PILE_STOCK];
	for (int x = stock.size - 1; x >= 0; --x) {
		const auto ext = stock.get(x);
		card cd(ext.rank + 1, ext.suit);
		ret += cd.to_str();
	}

	return ret;
}

string solver::collected_string() const {
	string ret;
	for (int i = kld::PILE_FOUNDATION_START; i <= kld::PILE_FOUNDATION_END; ++i) {
		auto &item = this->piles[i];
		for (int x = 0; x < item.size; ++x) {
			const auto ext = item.get(x);
			card cd(ext.rank + 1, ext.suit);
			ret += cd.to_str();
		}

		if (item.size > 0)
			ret += "\n";
	}
	return ret;
}
