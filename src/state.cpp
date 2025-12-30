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
		foundation_cards.emplace_back(collected_vec);
	}
	int idx = 0;
	for (int i = 0; i < 7; ++i) {
		std::vector<card *> hidden_vec{};
		hidden_tableau_cards.emplace_back(hidden_vec);
		std::vector<card *> visible_vec{};
		visible_tableau_cards.emplace_back(visible_vec);

		// # 一共7列
		for (int j = 0; j < i; ++j) {
			// # 隐藏的
			hidden_tableau_cards[i].emplace_back(&(pkr->cards[idx++]));
		}
		visible_tableau_cards[i].emplace_back(&(pkr->cards[idx++]));
	}
	for (size_t i = idx; i < pkr->cards.size(); ++i) {
		waste_cards.emplace_back(&(pkr->cards[idx++]));
	}
	deck_index = -1;
}

state::state(const state *previous_state) {
	std::vector<std::vector<card *> > new_visible_cards;
	std::vector<std::vector<card *> > new_hidden_cards;
	std::vector<std::vector<card *> > new_collected_cards;
	const std::vector new_deck_cards(previous_state->waste_cards.begin(), previous_state->waste_cards.end());
	const std::vector new_history(previous_state->history.begin(), previous_state->history.end());
	for (int i = 0; i < 7; ++i) {
		std::vector x(previous_state->visible_tableau_cards[i].begin(), previous_state->visible_tableau_cards[i].end());
		new_visible_cards.emplace_back(x);
		std::vector y(previous_state->hidden_tableau_cards[i].begin(), previous_state->hidden_tableau_cards[i].end());
		new_hidden_cards.emplace_back(y);
	}
	for (int i = 0; i < 4; ++i) {
		std::vector x(previous_state->foundation_cards[i].begin(), previous_state->foundation_cards[i].end());
		new_collected_cards.emplace_back(x);
	}
	visible_tableau_cards = new_visible_cards;
	hidden_tableau_cards = new_hidden_cards;
	waste_cards = new_deck_cards;
	foundation_cards = new_collected_cards;
	history = new_history;
	previous = previous_state;
	deck_index = previous_state->deck_index;
}

std::string state::to_string() const {
	// # 最终的字符串结果
	std::string ret;
	// # 最多隐藏牌列的隐藏牌的数量
	int max_hidden = 0;
	for (auto &item: hidden_tableau_cards) {
		if (!item.empty() && item.size() > max_hidden) {
			max_hidden = item.size();
		}
	}
	// # 最多显示牌列的显示牌的数量
	int max_visible = 0;
	for (auto &item: visible_tableau_cards) {
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
	ret += "----------------------------------";
	return ret;
}

bool state::is_completed() const {
	for (size_t i = 0; i < hidden_tableau_cards.size(); ++i) {
		if (!hidden_tableau_cards[i].empty())
			return false;
	}
	return true;
}

std::vector<state *> state::find_movable() const {
	std::vector<state *> result;
	for (int i = 0; i < 7; i++) {
		if (visible_tableau_cards[i].empty()) {
			// # 其他列的K可以移动到当前这里
			for (int column = 0; column < 7; ++column) {
				if (column == i || visible_tableau_cards[column].empty())
					continue;
				if (visible_tableau_cards[column][0]->get_value() == 13) {
					// # 可见牌是K
					auto new_state = new state(this);
					// # 从column列移动所有可见的牌到i列 (因为是移动K的,所以是移动全部)
					new_state->move_card(column, visible_tableau_cards[column].size(), i);
					if (visible_tableau_cards[i].empty() && hidden_tableau_cards[i].empty()) {
						// # 要移动到的是空列
						if (hidden_tableau_cards[column].empty()) {
							// # 当前来源列没有隐藏的牌了,这个时候就不移动了
							delete new_state;
						} else {
							result.emplace_back(new_state);
						}
					} else {
						result.emplace_back(new_state);
					}
				}
			}
		} else {
			// # 开始查找其他列可以移动到这里的部分

			// # 最下边的这张牌
			const auto card_ptr = visible_tableau_cards[i][visible_tableau_cards[i].size() - 1];
			for (int column = 0; column < 7; ++column) {
				if (column == i)
					continue;
				int cnt = 0;
				for (int idx = visible_tableau_cards[column].size() - 1; idx >= 0; --idx) {
					cnt++;
					if (card_ptr->can_move_to_me(visible_tableau_cards[column][idx])) {
						// todo: 这里需要dfs剪枝 maybe
						auto new_state = new state(this);
						const int old_from_count = visible_tableau_cards[column].size();
						new_state->move_card(column, cnt, i);
						const int new_to_count = new_state->visible_tableau_cards[i].size();
						if (new_to_count <= old_from_count) {
							// # 这个情况是移动后,最长序列变短了
							// * 判断是不是可以收牌,否则就不需要移动这一步
							if (!new_state->visible_tableau_cards[column].empty()) {
								const auto last = new_state->visible_tableau_cards[column][new_state->visible_tableau_cards[column].size() - 1];
								const auto suit = last->get_suit();
								const auto val = last->get_value();
								if (foundation_cards[suit].size() == val - 1) {
									// # 可以放到左上角
									result.emplace_back(new_state);
								} else {
									delete new_state;
								}
							} else {
								delete new_state;
							}
						} else {
							result.emplace_back(new_state);
						}
						break;
					}
				}
			}
		}
	}

	// # 添加收集到左上角
	for (size_t i = 0; i < foundation_cards.size(); ++i) {
		// # 已经收集了的一列的最后一张牌
		if (foundation_cards[i].empty()) {
			for (int j = 0; j < 7; ++j) {
				if (visible_tableau_cards[j].empty())
					continue;
				const auto last_card = visible_tableau_cards[j][visible_tableau_cards[j].size() - 1];
				if (i == last_card->get_suit() && 1 == last_card->get_value()) {
					// # 可以添加了
					auto new_state = new state(this);
					new_state->move_card(j, 8, 8);
					result.emplace_back(new_state);
				}
			}
		} else {
			const auto cd = foundation_cards[i][foundation_cards[i].size() - 1];
			for (int j = 0; j < 7; ++j) {
				if (visible_tableau_cards[j].empty())
					continue;
				const auto last_card = visible_tableau_cards[j][visible_tableau_cards[j].size() - 1];
				if (cd->get_suit() == last_card->get_suit() && cd->get_value() + 1 == last_card->get_value()) {
					// # 可以添加了
					auto new_state = new state(this);
					new_state->move_card(j, 8, 8);
					result.emplace_back(new_state);
				}
			}
		}
	}

	// # 添加从右上角选牌下来
	if (deck_index >= 0 && !waste_cards.empty()) {
		for (int i = 0; i < visible_tableau_cards.size(); ++i) {
			if (visible_tableau_cards[i].empty()) {
				if (waste_cards[deck_index]->get_value() == 13) {
					// # K
					auto new_state = new state(this);
					new_state->move_card(8, 8, i);
					result.emplace_back(new_state);
				}
				continue;
			}
			const auto cd = visible_tableau_cards[i][visible_tableau_cards[i].size() - 1];
			if (cd->can_move_to_me(waste_cards[deck_index])) {
				auto new_state = new state(this);
				new_state->move_card(8, 8, i);
				result.emplace_back(new_state);
			}
		}
	}

	// # 添加从右上角移动到左上角
	if (deck_index >= 0) {
		const auto suit = waste_cards[deck_index]->get_suit();
		const auto val = waste_cards[deck_index]->get_value();
		if (val == 1) {
			if (foundation_cards[suit].empty()) {
				auto new_state = new state(this);
				new_state->move_card(9, 9, 9);
				result.emplace_back(new_state);
			}
		} else {
			if (!foundation_cards[suit].empty()) {
				const auto left_val = foundation_cards[suit][foundation_cards[suit].size() - 1]->get_value();
				if (left_val + 1 == val) {
					auto new_state = new state(this);
					new_state->move_card(9, 9, 9);
					result.emplace_back(new_state);
				}
			}
		}
	}

	// # 添加点击右上角牌堆
	// todo: 这个选项应该是在没有可以任何可以移动的情况下才添加 (maybe)
	if (!waste_cards.empty()) {
		auto ns = new state(this);
		ns->move_card(8, 8, 8);
		result.emplace_back(ns);
	}

	// # 添加从左上角移动牌到下边
	for (int i = 0; i < 7; ++i) {
		if (visible_tableau_cards[i].empty())
			continue;
		const auto vis_last_card = visible_tableau_cards[i][visible_tableau_cards[i].size() - 1];
		if (vis_last_card->get_value() > 2) {
			const auto suit = vis_last_card->get_suit();
			if (suit == 0 || suit == 3) {
				// # 红的
				// * 需要从左上角取黑的
				if (foundation_cards[1].size() == vis_last_card->get_value() - 1) {
					// # 可以取出,但还不知道是不是需要剪枝
					if (foundation_cards[0].size() < vis_last_card->get_value() - 2
					    || foundation_cards[3].size() < vis_last_card->get_value() - 2) {
						// # 需要取出
						auto new_state = new state(this);
						new_state->move_card(9, 1, i);
						result.emplace_back(new_state);
					}
				}
				if (foundation_cards[2].size() == vis_last_card->get_value() - 1) {
					// # 可以取出,但还不知道是不是需要剪枝
					if (foundation_cards[0].size() < vis_last_card->get_value() - 2
					    || foundation_cards[3].size() < vis_last_card->get_value() - 2) {
						// # 需要取出
						auto new_state = new state(this);
						new_state->move_card(9, 2, i);
						result.emplace_back(new_state);
					}
				}
			} else {
				// # 黑的
				// * 需要从左上角取红的
				if (foundation_cards[0].size() == vis_last_card->get_value() - 1) {
					// # 可以取出,但还不知道是不是需要剪枝
					if (foundation_cards[1].size() < vis_last_card->get_value() - 2
					    || foundation_cards[2].size() < vis_last_card->get_value() - 2) {
						// # 需要取出
						auto new_state = new state(this);
						new_state->move_card(9, 0, i);
						result.emplace_back(new_state);
					}
				}
				if (foundation_cards[3].size() == vis_last_card->get_value() - 1) {
					// # 可以取出,但还不知道是不是需要剪枝
					if (foundation_cards[1].size() < vis_last_card->get_value() - 2
					    || foundation_cards[2].size() < vis_last_card->get_value() - 2) {
						// # 需要取出
						auto new_state = new state(this);
						new_state->move_card(9, 3, i);
						result.emplace_back(new_state);
					}
				}
			}
		}
	}
	return result;
}

void state::move_card(const int from, const int count, const int to) {
	if (from == 8 && count == 8 && to == 8) {
		// # 888代表点击右上角,相当于移动deck_index指针
		deck_index++;
		if (deck_index >= waste_cards.size()) {
			deck_index = -1;
		}
	} else if (from == 8 && count == 8) {
		// # 88N (N是0-6), 代表从右上角取一张牌到to_index这一列的最后
		if (deck_index >= 0) {
			visible_tableau_cards[to].emplace_back(waste_cards[deck_index]);
			waste_cards.erase(waste_cards.begin() + deck_index);
			deck_index--;
		} else {
			// # deck_index 的< 0 d情况是不能从右上角取牌的
			throw std::runtime_error("to index error.");
		}
	} else if (count == 8 && to == 8) {
		// # N88 (N是0-6), 代表从当前状态收集from_index列的最后一张到左上角
		const int i = visible_tableau_cards[from][visible_tableau_cards[from].size() - 1]->get_suit();
		foundation_cards[i].emplace_back(visible_tableau_cards[from][visible_tableau_cards[from].size() - 1]);
		visible_tableau_cards[from].erase(visible_tableau_cards[from].end() - 1, visible_tableau_cards[from].end());
		// # 判断是否为空了
		if (visible_tableau_cards[from].empty() && !hidden_tableau_cards[from].empty()) {
			// # 翻开隐藏的牌
			visible_tableau_cards[from].emplace_back(hidden_tableau_cards[from][hidden_tableau_cards[from].size() - 1]);
			hidden_tableau_cards[from].erase(hidden_tableau_cards[from].end() - 1, hidden_tableau_cards[from].end());
		}
	} else if (from == 9 && count == 9 && to == 9) {
		// # 999代表从右上角移动到左上角
		if (deck_index >= 0) {
			const auto suit = waste_cards[deck_index]->get_suit();
			foundation_cards[suit].emplace_back(waste_cards[deck_index]);
			waste_cards.erase(waste_cards.begin() + deck_index);
			deck_index--;
		} else {
			throw std::runtime_error("index error.");
		}
	} else if (from == 9 && count != 9 && to != 9) {
		// # N9N 代表从左上角count(index:0,1,2,3)移动到下边指定的to_index
		// # 这里的count 代表的是from_index
		if (!foundation_cards[count].empty()) {
			visible_tableau_cards[to].emplace_back(foundation_cards[count][foundation_cards[count].size() - 1]);
			foundation_cards[count].erase(foundation_cards[count].begin() + foundation_cards[count].size() - 1);
		} else {
			throw std::runtime_error("count index error.");
		}
	} else {
		const size_t tmp_count = std::min<size_t>(count, visible_tableau_cards[from].size());
		const size_t from_total_count = visible_tableau_cards[from].size();
		for (size_t i = from_total_count - tmp_count; i < from_total_count; ++i) {
			visible_tableau_cards[to].emplace_back(visible_tableau_cards[from][i]);
		}
		visible_tableau_cards[from].erase(visible_tableau_cards[from].begin() + from_total_count - tmp_count,
		                          visible_tableau_cards[from].end());
		// # 来源列没有可见牌的时候要翻开来源列的隐藏的牌
		if (visible_tableau_cards[from].empty() && !hidden_tableau_cards[from].empty()) {
			visible_tableau_cards[from].insert(visible_tableau_cards[from].end(), hidden_tableau_cards[from].end() - 1,
			                           hidden_tableau_cards[from].end());
			hidden_tableau_cards[from].erase(hidden_tableau_cards[from].end() - 1, hidden_tableau_cards[from].end());
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

int state::get_valuation() {
	if (valuation != -9999)
		return valuation;

	int value = 0; // todo:
	for (size_t i = 0; i < hidden_tableau_cards.size(); ++ i) {
		// # 没有翻开的牌值: -10, -9, -8, -7, -6, -5
		// # 未翻开牌减分机制
		int num = 10;
		for (auto &_ : hidden_tableau_cards[i]) {
			value -= num;
			num --;
		}
		int tmp = value;
		if (!visible_tableau_cards[i].empty()) {
			// # i_index列的可见的牌的数量
			const int visible_count = visible_tableau_cards[i].size();
		}
	}
	return valuation;
	// todo:
}

std::string state::to_serialized() const {
	std::string ret;
	for (int i = 0; i < 7; i++) {
		ret += "/";
		for (const auto j: hidden_tableau_cards[i]) {
			ret += j->get_char();
		}
		for (const auto j: visible_tableau_cards[i]) {
			ret += j->get_char();
		}
	}
	ret += "*";
	for (const auto deck_card: waste_cards) {
		ret += deck_card->get_char();
	}
	ret += std::to_string(deck_index);

	for (int i = 0; i < 4; ++i) {
		ret += "#";
		for (const auto j: foundation_cards[i]) {
			ret += j->get_char();
		}
	}

	return ret;
}

state::~state() {
	waste_cards.clear();
	hidden_tableau_cards.clear();
	visible_tableau_cards.clear();
	foundation_cards.clear();
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
	for (auto &item: hidden_tableau_cards) {
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
	for (auto &item: visible_tableau_cards) {
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
	for (auto &item: waste_cards) {
		ret += item->to_string();
	}
	ret += "\n";
	const int space_count = (deck_index + 1) * 5 + 2;
	for (int i = 0; i < space_count; ++i) {
		ret += " ";
	}
	ret += "\033[34m↑\033[0m";
	return ret;
}

std::string state::collected_string() const {
	std::string ret;
	for (auto &item: foundation_cards) {
		for (auto &cd: item) {
			ret += cd->to_string();
		}
		if (!item.empty())
			ret += "\n";
	}
	return ret;
}
