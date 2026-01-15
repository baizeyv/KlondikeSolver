//
// Created by baizeyv on 1/13/2026.
//

#include "state_map.h"

#include <stdexcept>

state_map::state_map(const size_t cap) : capacity(cap) {
	buckets.reserve(cap);
}

optional<pair<const estimation *, size_t>> state_map::get(const state_key& key) const {
	uint64_t h = key.low;
	h ^= key.high + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);

	size_t index = static_cast<size_t>(h % capacity);

	for (size_t i = 0; i < capacity; ++ i) {
		const bucket& bkt = buckets[index];
		if (bkt.is_empty()) {
			// # 遇到空桶,说明key不存在
			return std::nullopt;
		}
		if (bkt.key == key) {
			return make_pair(&bkt.value, index);
		}
		index = (index + 1) % capacity; // # 线性探测下一个未知
	}
	return std::nullopt;
}

void state_map::insert(state_key& key, estimation value) {
	uint64_t h = key.low;
	h ^= key.high + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);

	size_t index = static_cast<size_t>(h % capacity);
	for (size_t i = 0; i < capacity; ++ i) {
		bucket& bkt = buckets[index];
		if (bkt.is_empty()) {
			bkt.key = key;
			bkt.value = value;
			return;
		}
		index = (index + 1) % capacity;
	}
	throw runtime_error("state_map full");
}

estimation & state_map::estimation_mut(const size_t index) {
	return buckets[index].value;
}
