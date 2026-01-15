//
// Created by baizeyv on 1/13/2026.
//

#include "motion.h"

bool motion::is_null() const {
	return value1 == 0;
}

uint8_t motion::from() const {
	return value1 & 0x0F;
}

uint8_t motion::to() const {
	return value1 >> 4;
}

uint8_t motion::count() const {
	return value2 & 0x7F;
}

bool motion::flip() const {
	return (value2 & 0x80) != 0;
}

motion::motion() : value1(0), value2(0) {
}

motion::motion(const uint8_t from, const uint8_t to, const uint8_t count, const bool flip) {
	// value1: 低 4 位存 from (0-15)，高 4 位存 to (0-15)
	value1 = (from & 0x0F) | (to << 4);
	// value2: 低 7 位存 count (0-127)，最高位存 flip 标志
	value2 = (count & 0x7F) | (flip ? 0x80 : 0x00);
}

std::tuple<size_t, size_t, size_t, bool> motion::values() const {
	return {
		static_cast<size_t>(from()),
		static_cast<size_t>(to()),
		count(),
		flip()
	};
}
