//
// Created by baizeyv on 1/14/2026.
//

#include "state.h"

#include <algorithm>

#include "poker.h"

state::state() {
	foundations.fill(std::nullopt);
}

uint8_t state::foundation_score() const {
	uint8_t total_score = 0;

	// # 遍历4个foundation
	for (const auto &f_card_opt: this->foundations) {
		// # 如果该位置有牌
		if (f_card_opt.has_value()) {
			total_score += (f_card_opt->get_value());
		} else {
			total_score += 0;
		}
	}
	return total_score;
}

bool state::is_valid() const {
	// # 记录每张牌是否见过, 共52张牌
	array<bool, 52> seen;
	seen.fill(false);
	size_t count = 0;

	// # 定义一个内部lambda辅助函数用于检查卡牌数组
	auto check_cards = [&](const vector<card> &cards) -> bool {
		for (const auto &cd: cards) {
			if (cd.is_unknown())
				return false;
			size_t id = static_cast<size_t>(cd.get_id());
			if (id >= 52 || seen[id]) {
				// # 重复出现的牌或非法ID
				return false;
			}
			seen[id] = true;
			count += 1;
		}
		return true;
	};

	// # 1. 检查stock
	if (!check_cards(this->stock))
		return false;
	// # 2. 检查waste
	if (!check_cards(this->waste))
		return false;
	// # 3. 检查foundation
	// ! 注意: foundation只存了顶牌,但逻辑上它包含了从A到该顶牌的所有牌
	for (const auto &f_card_opt: this->foundations) {
		if (!f_card_opt.has_value())
			continue;

		// # 还原foundation中"下面"被压住的所有牌
		vector<card> f_stack;
		const uint8_t current_suit = f_card_opt->get_suit();
		for (uint8_t r = 0; r < f_card_opt->get_value(); ++r) {
			f_stack.push_back(card(r + 1, current_suit));
		}
		if (!check_cards(f_stack))
			return false;
	}

	// # 4. 检查7个tableau
	for (const auto &tab: this->tableaus) {
		// # 直接传入存储在 tableau 结构体中的cards vector
		if (!check_cards(tab.cards))
			return false;
	}

	// # 5. 最终确认:是否恰好见到了52张不同的牌
	return count == 52;
}

bool state::need_redeal() const {
	return stock.empty() && !waste.empty();
}

void state::draw() {
	const size_t stock_len = this->stock.size();
	if (stock_len == 0) {
		// # 重新洗牌redeal逻辑
		// # 如果发牌堆空了,且waste不为空,则将waste整体倒回stock
		if (!this->waste.empty()) {
			std::reverse(this->waste.begin(), this->waste.end());
			// # 将waste的所有元素移动/添加到stock
			this->stock.insert(this->stock.end(), this->waste.begin(), this->waste.end());
			// # 清空waste
			this->waste.clear();
		}
	} else {
		// # 翻牌draw逻辑 (draw_count)
		const size_t d_count = 1;
		// # 实际能翻的数量取决于剩余牌数
		const size_t num = std::min(d_count, stock_len);

		// # 1. 确定起始迭代器
		const auto start_it = this->stock.end() - num;
		// # 2. 将这部分牌反转并存入临时容器或直接处理
		vector<card> drawn_cards(start_it, this->stock.end());
		ranges::reverse(drawn_cards);
		// # 3. 压入废牌堆
		this->waste.insert(this->waste.end(), drawn_cards.begin(), drawn_cards.end());
		// # 4. 从原发牌堆移除
		this->stock.erase(start_it, this->stock.end());
	}
}

void state::move_waste_to_foundation(const size_t idx) {
	// # 1. 从waste弹出顶部的牌
	if (this->waste.empty()) {
		return;
	}
	card card = this->waste.back();
	this->waste.pop_back();

	// # 2. 更新foundation对应位置的顶牌
	this->foundations[idx] = card;
}

void state::move_waste_to_tableau(const size_t idx) {
	// # 1. 从waste弹出顶部的牌
	// # 如果waste为空,则不执行任何操作
	if (this->waste.empty())
		return;

	const card card = this->waste.back();
	this->waste.pop_back();

	// # 2. 将牌压入目标的 tableau
	this->tableaus[idx].push(card);
}

void state::move_tableau_to_foundation(const size_t tableau_idx, const size_t foundation_idx) {
	// # 1. 从指定的tableau列中弹出一张牌
	card card = this->tableaus[tableau_idx].pop_unchecked();
	// # 2. 将弹出的牌放入对应的foundation位置
	this->foundations[foundation_idx] = card;
}

void state::move_tableau_to_tableau(const size_t from_idx, const size_t to_idx, const size_t count) {
	// # 1. 从源tableau中抽取指定数量的牌
	vector<card> cards = this->tableaus[from_idx].drain_unchecked(count);
	// # 2. 更新目标tableau的正面牌计数
	// # 移动过去的牌在目标列上依然是正面朝上的
	this->tableaus[to_idx].face_up_count += cards.size();
	// # 3. 将卡牌序列追加到目标tableau的末尾
	this->tableaus[to_idx].cards.insert(this->tableaus[to_idx].cards.end(), cards.begin(), cards.end());
}

void state::move_foundation_to_tableau(size_t foundation_idx, size_t tableau_idx) {
	// # 1. 获取foundation顶部的牌
	if (!this->foundations[foundation_idx].has_value())
		return;
	const card cd = this->foundations[foundation_idx].value();
	const uint8_t r = cd.get_value();
	// # 2. 更新foundation状态
	// # 如果拿走的是A,foundation变空
	// # 否则,foundation的顶牌变为-1那张牌
	if (r == 1) {
		this->foundations[foundation_idx] = std::nullopt;
	} else {
		// # 创建一张点数小1级,花色相同的牌作为新的顶牌
		this->foundations[foundation_idx] = card(r - 1, cd.get_suit());
	}
	// # 3. 将拿出的牌压入目标tableau
	this->tableaus[tableau_idx].push(cd);
}

void state::copy_from(const state &pk) {
	// # 1. 赋值stock和waste
	this->stock = pk.stock;
	this->waste = pk.waste;
	// # 2. 赋值foundation
	this->foundations = pk.foundations;
	// # 3. 赋值7个tableau
	for (size_t i = 0; i < kld::TOTAL_TABLEAUS; ++i) {
		this->tableaus[i] = pk.tableaus[i];
	}
}

int state::calculate_empty_column_count() const {
	int count = 0;
	for (const auto &tab: tableaus) {
		count += tab.is_empty() ? 1 : 0;
	}
	return count;
}

int state::calculate_face_down_count() const {
	int count = 0;
	for (const auto &tab: tableaus) {
		count += tab.size() - tab.face_up_count;
	}
	return count;
}

int state::calculate_foundation_ready_count() const {
	int count = 0;
	if (!waste.empty()) {
		// # waste亮着的第一张牌(在vector中是最后一张)
		const card cd = this->waste.back();
		if (can_move_to_foundation(cd)) {
			// # 可以推进foundation (waste->foundation)
			count++;
		}
	}
	for (const auto &tab: tableaus) {
		if (tab.is_empty())
			continue;
		if (tab.face_up_count > 0) {
			const card cd = tab.cards[tab.size() - 1];
			if (can_move_to_foundation(cd)) {
				// # 可以推进foundation (tableau->foundation)
				count++;
			}
		}
	}
	return count;
}

bool state::check_flip_card(const state &previous, const action &act) const {
	switch (act.type) {
		case action_type::tableau_to_tableau:
		case action_type::tableau_to_foundation: {
			const auto old_tab = previous.tableaus[act.from_idx];
			const auto now_tab = tableaus[act.from_idx];
			// # 移动之前的背面牌数量
			const int old_face_down_count = old_tab.size() - old_tab.face_up_count;
			// # 移动之后的背面牌数量
			const int now_face_down_count = now_tab.size() - now_tab.face_up_count;
			return old_face_down_count - now_face_down_count == 1;
		}
		case action_type::waste_to_foundation:
		case action_type::waste_to_tableau:
		case action_type::foundation_to_tableau:
		case action_type::draw:
		case action_type::redeal:
			return false;
	}
	return false;
}

bool state::check_consume_empty(const state &previous, const action &act) const {
	switch (act.type) {
		case action_type::tableau_to_foundation:
		case action_type::tableau_to_tableau: {
			const auto old_tab = previous.tableaus[act.to_idx];
			const auto now_tab = tableaus[act.to_idx];
			// # 移动前为空,移动后不为空代表消耗了空列
			return old_tab.is_empty() && !now_tab.is_empty();
		}
		case action_type::waste_to_foundation:
		case action_type::waste_to_tableau:
		case action_type::foundation_to_tableau:
		case action_type::draw:
		case action_type::redeal:
			return false;
	}
	return false;
}

string state::to_str() const {
	string ret;
	ret += "STOCK: ";
	for (const auto &item: stock) {
		ret += item.to_str();
	}
	ret += "\nWASTE: ";
	for (const auto &item: waste) {
		ret += item.to_str();
	}
	ret += "\nFOUNDATION: ";
	for (const auto &item: foundations) {
		if (item.has_value()) {
			ret += item.value().to_str();
		} else {
			ret += kld::empty_card;
		}
	}
	ret += "\nTABLEAU: \n";
	for (const auto &tab: tableaus) {
		for (const auto &item: tab.cards) {
			ret += item.to_str();
		}
		ret += "{" + to_string(tab.face_up_count);
		ret += "}\n";
	}
	ret += "\n";
	return ret;
}

bool state::can_move_to_foundation(const card &cd) const {
	if (cd.get_value() == 1)
		return true;
	const auto suit = cd.get_suit();
	const auto now = foundations[suit];
	if (!now.has_value()) {
		return false;
	}
	const auto n = now.value();
	if (n.get_value() + 1 == cd.get_value())
		return true;
	return false;
}
