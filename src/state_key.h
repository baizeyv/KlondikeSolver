//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_STATE_KEY_H
#define KLONDIKESOLVER_STATE_KEY_H
#include <cstdint>

struct state_key {
	uint64_t low;
	uint64_t high;

	bool operator==(const state_key & o) const noexcept {
		return low == o.low && high == o.high;
	}
};

struct state_key_hasher {
	size_t operator()(const state_key & k) const noexcept {
		// # 64位混合,足够了
		return static_cast<size_t>(k.low ^ (k.high * 0x9E3779B97F4A7C15ull));
	}
};

#endif //KLONDIKESOLVER_STATE_KEY_H