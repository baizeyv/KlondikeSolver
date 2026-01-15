//
// Created by baizeyv on 1/13/2026.
//

#include "pile.h"

#include <algorithm>

pile::pile() : size(0), first(std::nullopt) {
	// # 用未知牌来填充整个array
	cards.fill(card_ext::UNKNOWN);
}

void pile::reset() {
	size = 0;
	first = std::nullopt;
	cards.fill(card_ext::UNKNOWN);
}

void pile::set_face_up_count(const size_t count) {
	first = static_cast<uint8_t>(size - count);
}

void pile::push_card(const card_ext &cd) {
	cards[size] = cd;
	size++;
}

void pile::pop_card_to(pile &p) {
	size--;
	p.push_card(cards[size]);
}

void pile::move_n_cards_to(pile &to, const size_t count) {
	const size_t from_idx = size - count;
	const size_t to_idx = to.size;
	for (size_t i = 0; i < count; ++i) {
		to.cards[to_idx + i] = cards[from_idx + i];
	}
	size -= count;
	to.size += count;
}

void pile::move_n_cards_reversed_to(pile &to, const size_t count) {
	const size_t from_idx = size - count;
	const size_t to_idx = to.size;

	for (size_t i = 0; i < count; ++ i) {
		to.cards[to_idx + i] = cards[from_idx + i];
	}
	// # 翻转目标牌堆新加入的这部分牌
	std::reverse(to.cards.begin() + to_idx, to.cards.begin() + to_idx + count);

	size -= count;
	to.size += count;
}

card_ext pile::get(const size_t index) const {
	return cards[index];
}

card_ext pile::peek_top() const {
	if (size > 0)
		return cards[size - 1];
	return card_ext::UNKNOWN;
}

card_ext pile::peek_top_unchecked() const {
	return cards[size - 1];
}

card_ext pile::peek_first_face_up() const {
	if (size > 0 && first.has_value())
		return cards[first.value()];
	return card_ext::UNKNOWN;
}

card_ext pile::peek_first_face_up_unchecked() const {
	return cards[first.value()];
}

card_ext pile::peek_nth_from_top_unchecked(size_t offset) const {
	return cards[size - offset - 1];
}

size_t pile::face_up_count() const {
	if (first.has_value()) {
		return size - first.value();
	}
	return 0;
}

bool pile::operator==(const pile &other) const {
	if (size != other.size || first != other.first)
		return false;
	for (size_t i = 0; i < size; ++ i) {
		if (cards[i].id != other.cards[i].id)
			return false;
	}
	return true;
}
