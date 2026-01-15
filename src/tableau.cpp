//
// Created by baizeyv on 1/14/2026.
//

#include "tableau.h"

tableau::tableau() : face_up_count(0) {
}

tableau::tableau(vector<card> init_cards, const size_t face_up) : cards(std::move(init_cards)), face_up_count(face_up) {
}

bool tableau::is_empty() const {
	return cards.empty();
}

size_t tableau::size() const {
	return cards.size();
}

const card * tableau::peek_top() const {
	if (cards.empty())
		return nullptr;
	return &cards.back();
}

card tableau::pop_unchecked() {
	if (cards.empty()) {
		return card::UNKNOWN;
	}
	const card cd = cards.back();
	cards.pop_back();

	if (cards.empty()) {
		face_up_count = 0;
	} else {
		// # 这里的逻辑是确保如果下面还有牌，哪怕移走了所有正面牌，也会自动露出一张（需翻开）
		face_up_count = (face_up_count > 1) ? (face_up_count - 1) : 1;
	}
	return cd;
}

vector<card> tableau::drain_unchecked(size_t count) {
	const size_t n = cards.size();
	if (count > n)
		count = n;

	// # 获取最后 count 张牌
	vector<card> result(cards.end() - count, cards.end());

	// # 从原容器中删除
	cards.erase(cards.end() - count, cards.end());

	if (cards.empty()) {
		face_up_count = 0;
	} else {
		// # 更新正面牌数量
		const size_t remaining_face_up = (face_up_count > count) ? (face_up_count - count) : 1;
		face_up_count = remaining_face_up;
	}
	return result;
}

void tableau::push(const card card) {
	cards.push_back(card);
	face_up_count += 1;
}
