//
// Created by baizeyv on 1/15/2026.
//
#include "solve_result.h"

string solve_result::to_str() const {
	if (actions.empty()) {
		string s;
		s += "\n duration: ";
		s += std::to_string(elapsed.count());
		s += "\n { unsolved }";
		return s;
	}
	string ret;
	ret += "\nminimal: ";
	ret += std::to_string(minimal);
	ret += " / states: ";
	ret += std::to_string(states);
	ret += " / duration: ";
	ret += std::to_string(elapsed.count());
	ret += "s\n";
	ret += "history:\n";
	ret += action::format_actions(actions);
	return ret;
}

bool solve_result::is_solved() const {
	return !actions.empty();
}
