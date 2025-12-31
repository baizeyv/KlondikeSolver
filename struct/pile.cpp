//
// Created by baizeyv on 12/31/2025.
//

#include "pile.h"

#include <algorithm>


pile::pile() {
}

pile::pile(std::vector<card *> vec) : vec(std::move(vec)) {
}

card * pile::back() const {
	return vec.back();
}

card *pile::pop_back() {
	card *ptr = vec.back(); // # 取出最后一张牌的指针
	vec.pop_back(); // # 弹出最后一个元素
	return ptr;
}

pile pile::pop_back(const int n) {
	// # 最后n张牌的数组
	const std::vector last_n(vec.end() - n, vec.end());
	vec.erase(vec.end() - n, vec.end()); // # 擦除最后的n张牌
	return pile(last_n);
}

card *pile::pop(const int index) {
	card *ptr = vec[index];
	vec.erase(vec.begin() + index);
	return ptr;
}

void pile::push_back(card *card) {
	vec.emplace_back(card);
}

void pile::push_back(pile pile) {
	vec.insert(vec.end(), pile.vec.begin(), pile.vec.end());
	pile.vec.clear();
}

bool pile::empty() const {
	return vec.empty();
}

int pile::size() const {
	return vec.size();
}

void pile::reverse() {
	std::ranges::reverse(vec);
}

void pile::clear() {
	vec.clear();
}

const card *pile::operator[](const size_t index) const {
	return vec[index];
}
