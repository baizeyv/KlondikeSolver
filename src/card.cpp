//
// Created by baizeyv on 12/26/2025.
//

#include "card.h"

#include <stdexcept>

card::card(const char c) {
	original_char = c;
	if (c >= 'A' && c <= 'M') {
		// # 方片
		value = c - 'A' + 1;
		suit = 0;
	} else if (c >= 'N' && c <= 'Z') {
		// # 黑桃
		value = c - 'N' + 1;
		suit = 1;
	} else if (c >= 'a' && c <= 'm') {
		// # 梅花
		value = c - 'a' + 1;
		suit = 2;
	} else if (c >= 'n' && c <= 'z') {
		// # 红桃
		value = c - 'n' + 1;
		suit = 3;
	} else {
		throw std::runtime_error("card char error!");
	}
}

bool card::can_move_to_me(const card *cd) const {
	if (cd->get_value() + 1 == value) {
		if (suit == 0 || suit == 3) {
			if (const auto other_suit = cd->get_suit(); other_suit != 1 && other_suit != 2) {
				return false;
			}
		} else if (suit == 1 || suit == 2) {
			if (const auto other_suit = cd->get_suit(); other_suit != 0 && other_suit != 3) {
				return false;
			}
		}
	} else {
		return false;
	}
	return true;
}

uint8_t card::get_value() const {
	return value;
}

uint8_t card::get_suit() const {
	return suit;
}

char card::get_char() const {
	return original_char;
}

std::string card::to_str() const {
	std::string ret;
	ret += "\033[";
	if (suit == 0 || suit == 3) {
		ret += "31m";
	} else {
		ret += "32m";
	}
	return ret + get_suit_string() + get_value_string() + "\033[0m";
}

std::string card::get_suit_string() const {
	switch (suit) {
		case 0:
			return "D"; // Diamonds 方片
		case 1:
			return "S"; // Spades 黑桃
		case 2:
			return "C"; // Clubs  梅花
		case 3:
			return "H"; // Heart 红桃
		default:
			throw std::runtime_error("error on `get_suit_string()` -> undefined suit.");
	}
}

std::string card::get_value_string() const {
	if (value == 10)
		return "[X] ";
	if (value == 11)
		return "[J] ";
	if (value == 12)
		return "[Q] ";
	if (value == 13)
		return "[K] ";
	return "[" + std::to_string(value) + "] ";
}
