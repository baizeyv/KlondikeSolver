//
// Created by baizeyv on 1/20/2026.
//

#include "analyzer.h"

#include <utility>

#include "hint.h"
#include "poker.h"

analyzer::analyzer(string _level_seed) : level_seed(std::move(_level_seed)) {
}

void analyzer::solve() { {
		const poker ida_pkr(level_seed);
		solver slr = ida_pkr.call();
		// # ida* 搜索结果
		const solve_result ida_result = slr.solve(false);
		if (ida_result.is_solved()) {
			result = ida_result;
			return;
		}
	} {
		const poker a_pkr(level_seed);
		solver slr = a_pkr.call();
		// # a* 搜索结果
		const solve_result a_result = slr.solve(100000000, false, false);
		result = a_result;
	}
}

void analyzer::simulate_analysis() const {
	if (!result.is_solved())
		return;

	const poker a_pkr(level_seed);
	const solver slr = a_pkr.call();
	// # 初始状态 (最简单的就是从solver中来获取,不需要再次手动构建)
	state previous_state = slr.initial_state;

	// # 难度
	double difficulty = 0.0;

	// # 强制度 (这是"如果不这样走就完了"的量化) (mobility<=3是真正的危险区)
	double forcedness = 0.0;

	// # 低自由度持续时间的部分参数
	int compression_len = 0;
	double compression_score = 0.0;

	// # 资源消耗惩罚参数
	double resource_burn = 0.0;
	double stagnation = 0.0;

	// # 信息释放效率 (我们不看做了多少事,而看:每一步带来了多少新信息)
	double progress = 0.0;
	double efficiency = 0.0;

	int N = 1;

	for (int i = 0; i < result.actions.size(); ++i, ++ N) {
		// # 遍历解题路径
		auto act = result.actions[i];
		// # 根据两个参数来创建新的状态
		state next_state = make_next_state(previous_state, act);

		if (can_abort(previous_state, next_state)) {
			break;
		}

		// # 当前步骤的特征
		step_feature feature = make_step_feature(a_pkr, previous_state, next_state, act, result, i);

		// # 计算强制度
		forcedness += (feature.mobility == 1 ? 3.0 : feature.mobility == 2 ? 1.5 : feature.mobility == 3 ? 0.5 : 0.0);

		// # 计算低自由度持续时间
		if (feature.mobility <= 2) {
			compression_len ++;
			compression_score += compression_len;
		} else {
			compression_len = 0;
		}

		// # 计算资源消耗惩罚
		// # (消耗空列但没翻牌->假进展) (这一步在走"唯一解",但同时把未来堵死)
		if (feature.consume_empty && !feature.flip_card)
			resource_burn += 2.0;
		if (!feature.flip_card && feature.foundation_ready == 0)
			stagnation += 1.0;

		// # 计算信息释放效率
		progress += (feature.flip_card ? 2.0 : 0.0) + feature.foundation_ready * 0.5 - (feature.consume_empty ? 1.0 : 0.0);

		previous_state = next_state;
	}
	// ? 高难局往往efficiency很低
	efficiency = progress / N;

	difficulty = 1.5 * forcedness + 1.0 * compression_score + 1.2 * resource_burn + 1.0 * stagnation + 5.0 * (1.0 - efficiency);

	cout << " difficulty score: " << difficulty << endl;
}

void analyzer::output() const {
	cout << result.to_str() << endl;
}

int analyzer::calculate_mobility_count(const poker &pkr, const vector<action> &actions, const int index) {
	solver slr = pkr.call();
	for (int i = 0; i < actions.size(); ++i) {
		if (i > index)
			break;
		const auto [type, from_idx, to_idx, count] = actions[i];
		switch (type) {
			case action_type::waste_to_foundation: {
				auto _ = slr.move_now(kld::PILE_WASTE, to_idx + kld::PILE_FOUNDATION_START, 1);
			}
			break;
			case action_type::waste_to_tableau: {
				auto _ = slr.move_now(kld::PILE_WASTE, to_idx + kld::PILE_TABLEAU_START, 1);
			}
			break;
			case action_type::tableau_to_foundation: {
				auto _ = slr.move_now(from_idx + kld::PILE_TABLEAU_START, to_idx + kld::PILE_FOUNDATION_START,
				                      1);
			}
			break;
			case action_type::foundation_to_tableau: {
				auto _ = slr.move_now(from_idx + kld::PILE_FOUNDATION_START, to_idx + kld::PILE_TABLEAU_START,
				                      1);
			}
			break;
			case action_type::tableau_to_tableau: {
				auto _ = slr.move_now(from_idx + kld::PILE_TABLEAU_START, to_idx + kld::PILE_TABLEAU_START,
				                      count);
			}
			break;
			case action_type::draw:
			case action_type::redeal: {
				auto _ = slr.draw_now();
			}
			break;
		}
		if (index == actions.size() - 1)
			cout << slr.to_str() << endl;
	}
	slr.hint_kit->clear();
	slr.hint_kit->update();
	return slr.hint_kit->get_mobility();
}

state analyzer::make_next_state(const state &previous, const action &move) {
	// # 初始化新状态
	state new_state;
	new_state.copy_from(previous);

	switch (move.type) {
		case action_type::waste_to_foundation:
			new_state.move_waste_to_foundation(move.to_idx);
			break;
		case action_type::waste_to_tableau:
			new_state.move_waste_to_tableau(move.to_idx);
			break;
		case action_type::foundation_to_tableau:
			new_state.move_foundation_to_tableau(move.from_idx, move.to_idx);
			break;
		case action_type::tableau_to_foundation:
			new_state.move_tableau_to_foundation(move.from_idx, move.to_idx);
			break;
		case action_type::tableau_to_tableau:
			new_state.move_tableau_to_tableau(move.from_idx, move.to_idx, move.count);
			break;
		case action_type::draw:
		case action_type::redeal:
			new_state.draw();
			break;
	}
	return new_state;
}

bool analyzer::can_abort(const state &previous, const state &next) {
	const int old_face_down_count = previous.calculate_face_down_count();
	const int now_face_down_count = next.calculate_face_down_count();
	if (now_face_down_count == 0 && old_face_down_count > now_face_down_count)
		return true;
	return false;
}

step_feature analyzer::make_step_feature(const poker &pkr, const state &previous, const state &next, const action &move,
                                         const solve_result &sr, const int idx) {
	const auto sra = sr.actions;
	return {
		static_cast<uint8_t>(calculate_mobility_count(pkr, sra, idx)),
		static_cast<uint8_t>(next.calculate_empty_column_count()),
		static_cast<uint8_t>(next.calculate_face_down_count()),
		static_cast<uint8_t>(next.calculate_foundation_ready_count()),
		next.check_flip_card(previous, move),
		next.check_consume_empty(previous, move)
	};
}
