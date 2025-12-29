//
// Created by baizeyv on 12/26/2025.
//

#include "state.h"

#include <algorithm>
#include <stdexcept>

#include "../constant.h"

state::state(poker *pkr) : previous(nullptr) {
	for (int i = 0; i < 4; ++i) {
		std::vector<card *> collected_vec{};
		// # 初始化左上角的已收集的牌堆
		collected_cards.emplace_back(collected_vec);
	}
	int idx = 0;
	for (int i = 0; i < 7; ++i) {
		std::vector<card *> hidden_vec{};
		hidden_cards.emplace_back(hidden_vec);
		std::vector<card *> visible_vec{};
		visible_cards.emplace_back(visible_vec);

		// # 一共7列
		for (int j = 0; j < i; ++j) {
			// # 隐藏的
			hidden_cards[i].emplace_back(&(pkr->cards[idx++]));
		}
		visible_cards[i].emplace_back(&(pkr->cards[idx++]));
	}
	for (size_t i = idx; i < pkr->cards.size(); ++i) {
		deck_cards.emplace_back(&(pkr->cards[idx++]));
	}
	deck_index = -1;
}

state::state(const state *previous_state) {
	std::vector<std::vector<card *> > new_visible_cards;
	std::vector<std::vector<card *> > new_hidden_cards;
	std::vector<std::vector<card *> > new_collected_cards;
	const std::vector new_deck_cards(previous_state->deck_cards.begin(), previous_state->deck_cards.end());
	const std::vector new_history(previous_state->history.begin(), previous_state->history.end());
	for (int i = 0; i < 7; ++i) {
		std::vector x(previous_state->visible_cards[i].begin(), previous_state->visible_cards[i].end());
		new_visible_cards.emplace_back(x);
		std::vector y(previous_state->hidden_cards[i].begin(), previous_state->hidden_cards[i].end());
		new_hidden_cards.emplace_back(y);
	}
	for (int i = 0; i < 4; ++i) {
		std::vector x(previous_state->collected_cards[i].begin(), previous_state->collected_cards[i].end());
		new_collected_cards.emplace_back(x);
	}
	visible_cards = new_visible_cards;
	hidden_cards = new_hidden_cards;
	deck_cards = new_deck_cards;
	history = new_history;
	previous = previous_state;
	deck_index = previous_state->deck_index;
}

std::string state::to_string() const {
	// # 最终的字符串结果
	std::string ret;
	// # 最多隐藏牌列的隐藏牌的数量
	int max_hidden = 0;
	for (auto &item: hidden_cards) {
		if (!item.empty() && item.size() > max_hidden) {
			max_hidden = item.size();
		}
	}
	// # 最多显示牌列的显示牌的数量
	int max_visible = 0;
	for (auto &item: visible_cards) {
		if (!item.empty() && item.size() > max_visible) {
			max_visible = item.size();
		}
	}
	ret += hidden_string(0, max_hidden);
	ret += "\n\n";
	ret += visible_string(0, max_visible);
	ret += "\n\n";
	ret += deck_string();
	ret += "\n\n";
	ret += collected_string();
	return ret;
}

std::vector<state *> state::find_movable() {
	std::vector<state *> result;
	for (int i = 0; i < 7; i++) {
		if (visible_cards[i].empty()) {
			// # 其他列的K可以移动到当前这里
			for (int column = 0; column < 7; ++column) {
				if (column == i)
					continue;
				if (visible_cards[column][0]->get_value() == 13) {
					// # 可见牌是K
					auto new_state = new state(this);
					// # 从column列移动所有可见的牌到i列 (因为是移动K的,所以是移动全部)
					new_state->move_card(column, visible_cards[column].size(), i);
					result.emplace_back(new_state);
				}
			}
		} else {
			// # 开始查找其他列可以移动到这里的部分

			// # 最下边的这张牌
			const auto card_ptr = visible_cards[i][visible_cards[i].size() - 1];
			for (int column = 0; column < 7; ++column) {
				if (column == i)
					continue;
				int cnt = 0;
				for (int idx = visible_cards[column].size(); idx >= 0; --idx) {
					cnt++;
					if (card_ptr->can_move_to_me(visible_cards[column][idx])) {
						// todo: 这里需要dfs剪枝 maybe
						auto new_state = new state(this);
						new_state->move_card(column, cnt, i);
						result.emplace_back(new_state);
						break;
					}
				}
			}
		}
	}

	// # 添加从右上角选牌下来
	if (deck_index >= 0 && !deck_cards.empty()) {
		for (int i = 0; i < visible_cards.size(); ++i) {
			if (visible_cards[i].empty())
				continue;
			const auto cd = visible_cards[i][visible_cards[i].size() - 1];
			if (cd->can_move_to_me(deck_cards[deck_index])) {
				auto new_state = new state(this);
				new_state->move_card(8, 8, i);
				result.emplace_back(new_state);
			}
		}
	}

	// # 添加收集到左上角
	for (size_t i = 0; i < collected_cards.size(); ++i) {
		// # 已经收集了的一列的最后一张牌
		const auto cd = collected_cards[i][collected_cards[i].size() - 1];
		for (int j = 0; j < 7; ++j) {
			if (visible_cards[j].empty())
				continue;
			const auto last_card = visible_cards[j][visible_cards[j].size() - 1];
			if (cd->get_suit() == last_card->get_suit() && cd->get_value() + 1 == last_card->get_value()) {
				// # 可以添加了
				auto new_state = new state(this);
				new_state->move_card(j, 8, 8);
				result.emplace_back(new_state);
			}
		}
	}

	// # 添加点击右上角牌堆
	// todo: 这个选项应该是在没有可以任何可以移动的情况下才添加
	if (!deck_cards.empty()) {
		auto ns = new state(this);
		ns->move_card(8, 8, 8);
		result.emplace_back(ns);
	}

	// # 添加从左上角移动牌到下边
	// todo:
	return result;
}

void state::move_card(const int from, const int count, const int to) {
	if (from == 8 && count == 8 && to == 8) {
		// # 888代表点击右上角,相当于移动deck_index指针
		deck_index++;
		if (deck_index >= deck_cards.size()) {
			deck_index = -1;
		}
	} else if (from == 8 && count == 8) {
		// # 88N (N是0-6), 代表从右上角取一张牌到to_index这一列的最后
		if (deck_index >= 0) {
			visible_cards[to].emplace_back(deck_cards[deck_index]);
			deck_cards.erase(deck_cards.begin() + deck_index);
			deck_index--;
		} else {
			// # deck_index 的< 0 d情况是不能从右上角取牌的
			throw std::runtime_error("to index error.");
		}
	} else if (count == 8 && to == 8) {
		// # N88 (N是0-6), 代表从当前状态收集from_index列的最后一张到左上角
		const int i = visible_cards[from][visible_cards[from].size() - 1]->get_suit();
		collected_cards[i].emplace_back(visible_cards[from][visible_cards[from].size() - 1]);
		visible_cards[from].erase(visible_cards[from].end() - 1, visible_cards[from].end());
		// # 判断是否为空了
		if (visible_cards[from].empty() && !hidden_cards[from].empty()) {
			// # 翻开隐藏的牌
			visible_cards[from].emplace_back(hidden_cards[from][hidden_cards[from].size() - 1]);
			hidden_cards[from].erase(hidden_cards[from].end() - 1, hidden_cards[from].end());
		}
	} else {
		const size_t tmp_count = std::min<size_t>(count, visible_cards[from].size());
		const size_t from_total_count = visible_cards[from].size();
		for (size_t i = from_total_count - tmp_count; i < from_total_count; ++i) {
			visible_cards[to].emplace_back(visible_cards[from][i]);
		}
		visible_cards[from].erase(visible_cards[from].begin() + from_total_count - tmp_count,
		                          visible_cards[from].end());
		// # 来源列没有可见牌的时候要翻开来源列的隐藏的牌
		if (visible_cards[from].empty() && !hidden_cards[from].empty()) {
			visible_cards[from].insert(visible_cards[from].end(), hidden_cards[from].end() - 1,
			                           hidden_cards[from].end());
			hidden_cards[from].erase(hidden_cards[from].end() - 1, hidden_cards[from].end());
		}
	}
	// # 添加历史记录
	history_item hi{};
	hi.set_from(from);
	hi.set_to(to);
	hi.set_count(count);
	hi.set_collection(false);
	history.insert(history.begin(), hi);
}

std::string state::to_serialized() const {
	std::string ret;
	for (int i = 0; i < 7; i++) {
		ret += "/";
		for (const auto j: hidden_cards[i]) {
			ret += j->get_char();
		}
		for (const auto j: visible_cards[i]) {
			ret += j->get_char();
		}
	}
	ret += "*";
	for (const auto deck_card: deck_cards) {
		ret += deck_card->get_char();
	}
	ret += std::to_string(deck_index);

	for (int i = 0; i < 4; ++i) {
		ret += "#";
		for (const auto j: collected_cards[i]) {
			ret += j->get_char();
		}
	}

	return ret;
}

state::~state() {
	deck_cards.clear();
	hidden_cards.clear();
	visible_cards.clear();
	collected_cards.clear();
	history.clear();
}

std::string state::hidden_string(const int row, const int max) const {
	if (max == 0)
		return "";
	if (row == max - 1)
		return floor_hidden_string(row);
	return floor_hidden_string(row) + "\n" + hidden_string(row + 1, max);
}

std::string state::floor_hidden_string(const int row) const {
	std::string ret;
	for (auto &item: hidden_cards) {
		auto column = item;
		std::ranges::reverse(column);
		if (column.size() > row) {
			ret += column[column.size() - row - 1]->to_string();
		} else {
			ret += kld::empty_card;
		}
	}
	return ret;
}

std::string state::visible_string(const int row, const int max) const {
	if (max == 0)
		return "";
	if (row == max - 1)
		return floor_visible_string(row);
	return floor_visible_string(row) + "\n" + visible_string(row + 1, max);
}

std::string state::floor_visible_string(const int row) const {
	std::string ret;
	for (auto &item: visible_cards) {
		auto column = item;
		std::ranges::reverse(column);
		if (column.size() > row) {
			ret += column[column.size() - row - 1]->to_string();
		} else {
			ret += kld::empty_card;
		}
	}
	return ret;
}

std::string state::deck_string() const {
	std::string ret;
	ret += kld::empty_card;
	for (auto &item: deck_cards) {
		ret += item->to_string();
	}
	ret += "\n";
	const int space_count = (deck_index + 1) * 5 + 2;
	for (int i = 0; i < space_count; ++i) {
		ret += " ";
	}
	ret += "^";
	return ret;
}

std::string state::collected_string() const {
	std::string ret;
	for (auto &item: collected_cards) {
		for (auto &cd: item) {
			ret += cd->to_string();
		}
		if (!item.empty())
			ret += "\n";
	}
	return ret;
}
