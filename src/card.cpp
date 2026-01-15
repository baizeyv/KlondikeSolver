//
// Created by baizeyv on 12/26/2025.
//

#include "card.h"

#include <stdexcept>

const card card::UNKNOWN = {};

card::card() : value(0), suit(0), original_char('0') {
}

card::card(const uint8_t v, const uint8_t s) : value(v), suit(s) {
	if (suit == 0) {
		// # 方片
		original_char = 'A' - 1 + v;
	} else if (suit == 1) {
		// # 梅花
		original_char = 'a' - 1 + v;
	} else if (suit == 2) {
		// # 红桃
		original_char = 'n' - 1 + v;
	} else if (suit == 3) {
		// # 黑桃
		original_char = 'N' - 1 + v;
	} else {
		throw std::runtime_error("card char error!");
	}
}

card::card(const char c) {
	original_char = c;
	if (c >= 'A' && c <= 'M') {
		// # 方片
		value = c - 'A' + 1;
		suit = 0;
	} else if (c >= 'N' && c <= 'Z') {
		// # 黑桃
		value = c - 'N' + 1;
		suit = 3;
	} else if (c >= 'a' && c <= 'm') {
		// # 梅花
		value = c - 'a' + 1;
		suit = 1;
	} else if (c >= 'n' && c <= 'z') {
		// # 红桃
		value = c - 'n' + 1;
		suit = 2;
	} else {
		throw std::runtime_error("card char error!");
	}
}

bool card::can_move_to_me(const card *cd) const {
	if (cd->get_value() + 1 == value) {
		if (suit == 0 || suit == 2) {
			if (const auto other_suit = cd->get_suit(); other_suit != 1 && other_suit != 3) {
				return false;
			}
		} else if (suit == 1 || suit == 3) {
			if (const auto other_suit = cd->get_suit(); other_suit != 0 && other_suit != 2) {
				return false;
			}
		}
	} else {
		return false;
	}
	return true;
}

bool card::is_king() const {
	return get_value() == 13;
}

uint8_t card::get_id() const {
	// * 方片 0  1  2  3  4  5  6  7  8  9  10 11 12
	// * 梅花 13 14 15 16 17 18 19 20 21 22 23 24 25
	// * 红桃 26 27 28 29 30 31 32 33 34 35 36 37 38
	// * 黑桃 39 40 41 42 43 44 45 46 47 48 49 50 51
	return (suit + 1) * 13 - 1 - (13 - get_value());
}

uint8_t card::get_value() const {
	return value;
}

uint8_t card::get_suit() const {
	return suit;
}

bool card::is_unknown() const {
	return value == 0;
}

char card::get_char() const {
	return original_char;
}

std::string card::to_str() const {
	std::string ret;
	ret += "\033[";
	if (suit == 0 || suit == 2) {
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
		case 3:
			return "S"; // Spades 黑桃
		case 1:
			return "C"; // Clubs  梅花
		case 2:
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
