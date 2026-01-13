//
// Created by baizeyv on 12/26/2025.
//

#include "state.h"

#include <algorithm>
#include <stdexcept>
#include "../helper/xxhash.h"

#include "../constant.h"

state::state(poker *pkr) : previous(nullptr) {
	for (int i = 0; i < 4; ++i) {
		vector<card *> collected_vec{};
		// # 初始化左上角的已收集的牌堆
		foundation_cards.emplace_back(collected_vec);
	}
	int idx = 0;
	for (int i = 0; i < 7; ++i) {
		vector<card *> hidden_vec{};
		hidden_tableau_cards.emplace_back(hidden_vec);
		vector<card *> visible_vec{};
		visible_tableau_cards.emplace_back(visible_vec);

		// # 一共7列
		for (int j = 0; j < i; ++j) {
			// # 隐藏的
			hidden_tableau_cards[i].push_back(&(pkr->cards[idx++]));
		}
		visible_tableau_cards[i].push_back(&(pkr->cards[idx++]));
	}
	for (size_t i = idx; i < pkr->cards.size(); ++i) {
		waste_cards.push_back(&(pkr->cards[idx++]));
	}
	deck_index = -1;
}

state::state(const state *previous_state) {
	pile_vec new_visible_cards;
	pile_vec new_hidden_cards;
	pile_vec new_collected_cards;
	const pile new_deck_cards(previous_state->waste_cards);
	const vector new_history(previous_state->history.begin(), previous_state->history.end());
	for (int i = 0; i < 7; ++i) {
		pile x(previous_state->visible_tableau_cards[i]);
		new_visible_cards.emplace_back(x);
		pile y(previous_state->hidden_tableau_cards[i]);
		new_hidden_cards.emplace_back(y);
	}
	for (int i = 0; i < 4; ++i) {
		pile x(previous_state->foundation_cards[i]);
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

string state::to_str() const {
	// # 最终的字符串结果
	string ret;
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

vector<state *> state::find_movable() const {
	vector<state *> result;
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
								const auto last = new_state->visible_tableau_cards[column][
									new_state->visible_tableau_cards[column].size() - 1];
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
	// ? 这个选项应该是在没有可以任何可以移动的情况下才添加 (maybe)
	// if (!waste_cards.empty()) {
	// 	auto ns = new state(this);
	// 	ns->move_card(8, 8, 8);
	// 	result.emplace_back(ns);
	// }
	// # 将点击右上角牌堆直接换为跳到最合适的deck_index,而不是每一步都去点
	auto func = [this, &result](const int i) {
		const auto cd = waste_cards[i];
		for (int x = 0; x < visible_tableau_cards.size(); ++x) {
			if (visible_tableau_cards[x].empty()) {
				// # 需要移动K
				if (cd->get_value() == 13) {
					// # 确定是K
					auto new_state = new state(this);
					new_state->move_card(10, i, x);
					result.emplace_back(new_state);
				}
			} else {
				if (visible_tableau_cards[x].back()->can_move_to_me(cd)) {
					auto new_state = new state(this);
					new_state->move_card(10, i, x);
					result.emplace_back(new_state);
				}
			}
		}
	};
	if (deck_index < 0) {
		for (int i = 0; i < waste_cards.size(); ++i) {
			func(i);
		}
	} else {
		for (int i = deck_index; i < waste_cards.size(); ++i) {
			func(i);
		}
		for (int i = 0; i < deck_index; ++i) {
			func(i);
		}
	}

	// # 添加从左上角移动牌到下边
	for (int i = 0; i < 7; ++i) {
		if (visible_tableau_cards[i].empty())
			continue;
		const auto vis_last_card = visible_tableau_cards[i].back();
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
			visible_tableau_cards[to].push_back(waste_cards.pop(deck_index));
			deck_index--;
		} else {
			// # deck_index 的< 0 d情况是不能从右上角取牌的
			throw runtime_error("to index error.");
		}
	} else if (count == 8 && to == 8) {
		// # N88 (N是0-6), 代表从当前状态收集from_index列的最后一张到左上角
		const int i = visible_tableau_cards[from][visible_tableau_cards[from].size() - 1]->get_suit();
		foundation_cards[i].push_back(visible_tableau_cards[from].pop_back());
		// # 判断是否为空了
		if (visible_tableau_cards[from].empty() && !hidden_tableau_cards[from].empty()) {
			// # 翻开隐藏的牌
			visible_tableau_cards[from].push_back(hidden_tableau_cards[from].pop_back());
		}
	} else if (from == 9 && count == 9 && to == 9) {
		// # 999代表从右上角移动到左上角
		if (deck_index >= 0) {
			const auto suit = waste_cards[deck_index]->get_suit();
			foundation_cards[suit].push_back(waste_cards.pop(deck_index));
			deck_index--;
		} else {
			throw runtime_error("index error.");
		}
	} else if (from == 9 && count != 9 && to != 9) {
		// # N9N 代表从左上角count(index:0,1,2,3)移动到下边指定的to_index
		// # 这里的count 代表的是from_index
		if (!foundation_cards[count].empty()) {
			visible_tableau_cards[to].push_back(foundation_cards[count].pop_back());
		} else {
			throw runtime_error("count index error.");
		}
	} else if (from == 10) {
		// # 这个情况直接将waste中的这个deck_index中放到对应的位置,deck_index也需要回退一个位置
		// # to_index 是从waste移动到的那一列的index
		// # count 代表的是这个新的 deck_index
		visible_tableau_cards[to].push_back(waste_cards.pop(count));
		deck_index = count - 1;
	} else {
		const size_t tmp_count = min<size_t>(count, visible_tableau_cards[from].size());
		visible_tableau_cards[to].push_back(visible_tableau_cards[from].pop_back(tmp_count));
		// # 来源列没有可见牌的时候要翻开来源列的隐藏的牌
		if (visible_tableau_cards[from].empty() && !hidden_tableau_cards[from].empty()) {
			visible_tableau_cards[from].push_back(hidden_tableau_cards[from].pop_back());
		}
	}
	// # 添加历史记录
	history_item hi{};
	hi.set_from(from);
	hi.set_to(to);
	hi.set_count(count);
	hi.set_collection(false); // todo: 这个不能永远是false,应该和上边的行为保持一致
	history.insert(history.begin(), hi);
}

int state::get_valuation() {
	if (valuation != -9999)
		return valuation;

	int value = 0; // todo:

	// # 1. 已经翻开的牌的数量
	// * revealed = 已经翻开的 tableau 牌的数量
	const int revealed = calculate_revealed_value();
	// todo:


	return valuation;
	// todo:
}

string state::to_serialized() const {
	if (history.size() > 0) {
		int f = history[0].get_from();
		int t = history[0].get_to();
		int c = history[0].get_count();
		int cc = history[0].get_collection();
	}
	string ret;
	for (int i = 0; i < 7; i++) {
		ret += "/";
		for (int x = 0; x < hidden_tableau_cards[i].size(); ++x) {
			ret += hidden_tableau_cards[i][x]->get_char();
		}
		for (int x = 0; x < visible_tableau_cards[i].size(); ++x) {
			ret += visible_tableau_cards[i][x]->get_char();
		}
	}
	ret += "*";
	for (int x = 0; x < waste_cards.size(); ++x) {
		ret += waste_cards[x]->get_char();
	}
	ret += to_string(deck_index);

	for (int i = 0; i < 4; ++i) {
		ret += "#";
		for (int x = 0; x < foundation_cards[i].size(); ++x) {
			ret += foundation_cards[i][x]->get_char();
		}
	}

	return ret;
}

state_key state::to_hash() const {
	const string ser = to_serialized();
	const XXH128_hash_t hash = XXH3_128bits(ser.data(), ser.size());
	return {hash.low64, hash.high64};
}

state::~state() {
	waste_cards.clear();
	hidden_tableau_cards.clear();
	visible_tableau_cards.clear();
	foundation_cards.clear();
	history.clear();
}

int state::calculate_revealed_value() const {
	// # 这里的 base_weight = 100 是一个权重值,可以进行调试, (100是W1, weight-1)

	int revealed_value = 0;
	for (int i = 0; i < hidden_tableau_cards.size(); ++i) {
		constexpr int base_weight = 100;

		// # 当前列中的隐藏牌的数量
		const int current_hidden_count_in_column = hidden_tableau_cards[i].size();
		const float dot_weight = current_hidden_count_in_column * 1.0f / 10;

		revealed_value += (1 + dot_weight) * base_weight * (i - current_hidden_count_in_column);
	}

	// # 1. 已经翻开的牌的数量
	return revealed_value;
}

int state::calculate_mobility_value() const {
	// # 2. 可移动性加权值计算

	// # 衡量当前局面允许多少"合法且有意义的移动"
	// todo:

	int value = 0;
	// todo:

	return value;
}

int state::calculate_empty_column_value() const {
	// # 3. tableau 结构质量 (比foundation更重要)
	int value = 0;

	// # 空列的数量
	int empty_column_count = 0;
	for (int i = 0; i < 7; ++i) {
		if (hidden_tableau_cards[i].empty() && visible_tableau_cards[i].empty()) {
			empty_column_count++;
		}
	}
	value += empty_column_count * 80; // # 这里的80是一个权重值,可以进行调试

	// todo: 添加长序列的 ( maybe )
	return value;
}

int state::calculate_waste_playable_value() const {
	int value = 0;
	// todo:
	return value;
}

string state::hidden_string(const int row, const int max) const {
	if (max == 0)
		return "";
	if (row == max - 1)
		return floor_hidden_string(row);
	return floor_hidden_string(row) + "\n" + hidden_string(row + 1, max);
}

string state::floor_hidden_string(const int row) const {
	string ret;
	for (auto &item: hidden_tableau_cards) {
		auto column = item;
		column.reverse();
		if (column.size() > row) {
			ret += column[column.size() - row - 1]->to_str();
		} else {
			ret += kld::empty_card;
		}
	}
	return ret;
}

string state::visible_string(const int row, const int max) const {
	if (max == 0)
		return "";
	if (row == max - 1)
		return floor_visible_string(row);
	return floor_visible_string(row) + "\n" + visible_string(row + 1, max);
}

string state::floor_visible_string(const int row) const {
	string ret;
	for (auto &item: visible_tableau_cards) {
		auto column = item;
		column.reverse();
		if (column.size() > row) {
			ret += column[column.size() - row - 1]->to_str();
		} else {
			ret += kld::empty_card;
		}
	}
	return ret;
}

string state::deck_string() const {
	string ret;
	ret += kld::empty_card;
	for (int x = 0; x < waste_cards.size(); ++x) {
		ret += waste_cards[x]->to_str();
	}
	ret += "\n";
	const int space_count = (deck_index + 1) * 5 + 2;
	for (int i = 0; i < space_count; ++i) {
		ret += " ";
	}
	ret += "\033[34m↑\033[0m";
	return ret;
}

string state::collected_string() const {
	string ret;
	for (auto &item: foundation_cards) {
		for (int x = 0; x < item.size(); ++x) {
			ret += item[x]->to_str();
		}
		if (!item.empty())
			ret += "\n";
	}
	return ret;
}
