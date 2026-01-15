//
// Created by baizeyv on 1/13/2026.
//

#include "card_ext.h"

#include "../constant.h"

const card_ext card_ext::UNKNOWN = {};

card_ext::card_ext() : id(kld::MAX_CARD), id2(0), suit(kld::MAX_SUIT), rank(kld::MAX_RANK), is_red(2), is_even(1), red_even(2),
                       order(0) {
}

card_ext::card_ext(const uint8_t _id, const uint8_t _id2, const uint8_t _suit, const uint8_t _rank,
                   const uint8_t _is_red, const uint8_t _is_even,
                   const uint8_t _red_even, const uint8_t _order) : id(_id), id2(_id2), suit(_suit), rank(_rank),
                                                                    is_red(_is_red), is_even(_is_even),
                                                                    red_even(_red_even),
                                                                    order(_order) {
}

card_ext card_ext::create_with_id(const uint8_t id) {
	if (id >= kld::MAX_CARD)
		return UNKNOWN;

	const uint8_t rank = id % kld::MAX_RANK; // # 提取点数(0-12)
	const uint8_t suit = id / kld::MAX_RANK; // # 提取花色(0-3)
	const uint8_t id2 = (rank << 2) | suit;
	const uint8_t is_red = suit & 1;
	const uint8_t is_even = rank & 1;
	const uint8_t red_even = is_red ^ is_even;
	const uint8_t order = suit >> 1;

	return card_ext(id, id2, suit, rank, is_red, is_even, red_even, order);
}

card_ext card_ext::creat_with_rank_suit(const uint8_t rank, const uint8_t suit) {
	return create_with_id((suit * kld::MAX_RANK) + rank);
}

bool card_ext::is_unknown() const {
	return id >= kld::MAX_CARD;
}

bool card_ext::is_king() const {
	return rank == kld::MAX_RANK - 1;
}
