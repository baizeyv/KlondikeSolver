//
// Created by baizeyv on 1/14/2026.
//

#include "action.h"

#include <iomanip>
#include <sstream>

bool action::is_redeal() const {
	return type == action_type::redeal;
}

std::string action::format_actions(const std::vector<action> &actions) {
	std::vector<std::string> list;

	size_t i = 0;
	while (i < actions.size()) {
		const action &act = actions[i];
		// # 1. 合并连续的draw
		if (act.type == action_type::draw) {
			size_t count = 1;
			while (i + count < actions.size() && actions[i + count].type == action_type::draw) {
				++count;
			}
			list.emplace_back("/D:" + std::to_string(count));

			i += count;
			continue;
		}

		// # 2. 其他动作逐个格式化
		switch (act.type) {
			case action_type::waste_to_foundation:
				list.push_back(
					"/W-F" + std::to_string(act.to_idx) + "-" + std::to_string(
						act.count));
				break;
			case action_type::waste_to_tableau:
				list.push_back(
					"/W-T" + std::to_string(act.to_idx) + "-" + std::to_string(
						act.count));
				break;
			case action_type::tableau_to_foundation:
				list.push_back(
					"/T" + std::to_string(act.from_idx) + "-F" + std::to_string(act.to_idx) + "-" + std::to_string(
						act.count));
				break;
			case action_type::foundation_to_tableau:
				list.push_back(
					"/F" + std::to_string(act.from_idx) + "-T" + std::to_string(act.to_idx) + "-" + std::to_string(
						act.count));
				break;
			case action_type::tableau_to_tableau:
				list.push_back(
					"/T" + std::to_string(act.from_idx) + "-T" + std::to_string(act.to_idx) + "-" + std::to_string(
						act.count));
				break;
			case action_type::redeal:
				list.emplace_back("/R");
				break;
			default:
				break;
		}
		++i;
	}

	// # 3. 计算列宽 (用于对齐)
	size_t column_width = 0;
	for (const auto& s : list) {
		column_width = std::max(column_width, s.size());
	}
	column_width += 1;

	// # 4. 每行10个,左对齐输出
	std::ostringstream out;
	for (size_t xx = 0; xx < list.size(); xx += 10) {
		const size_t end = std::min(xx + 10, list.size());
		for (size_t j = xx; j < end; ++ j) {
			out << std::left << std::setw(static_cast<int>(column_width)) << list[j];
		}
		out << '\n';
	}
	return out.str();
}

action action::draw() {
	return {action_type::draw, 0, 0, 0};
}

action action::redeal() {
	return {action_type::redeal, 0, 0, 0};
}

action action::waste2foundation(const size_t f_idx) {
	return {action_type::waste_to_foundation, 0, f_idx, 1};
}

action action::waste2tableau(const size_t t_idx) {
	return {action_type::waste_to_tableau, 0, t_idx, 1};
}

action action::tableau2foundation(const size_t t_idx, const size_t f_idx) {
	return {action_type::tableau_to_foundation, t_idx, f_idx, 1};
}

action action::foundation2tableau(const size_t f_idx, const size_t t_idx) {
	return {action_type::foundation_to_tableau, f_idx, t_idx, 1};
}

action action::tableau2tableau(const size_t f_idx, const size_t t_idx, const size_t n) {
	return {action_type::tableau_to_tableau, f_idx, t_idx, n};
}

void apply_action(state &pk, const action &act) {
	switch (act.type) {
		case action_type::waste_to_foundation:
			pk.move_waste_to_foundation(act.to_idx);
			break;
		case action_type::waste_to_tableau:
			pk.move_waste_to_tableau(act.to_idx);
			break;
		case action_type::tableau_to_foundation:
			pk.move_tableau_to_foundation(act.from_idx, act.to_idx);
			break;
		case action_type::foundation_to_tableau:
			pk.move_foundation_to_tableau(act.from_idx, act.to_idx);
			break;
		case action_type::tableau_to_tableau:
			pk.move_tableau_to_tableau(act.from_idx, act.to_idx, act.count);
			break;
		case action_type::draw:
		case action_type::redeal:
			pk.draw();
			break;
	}
}
