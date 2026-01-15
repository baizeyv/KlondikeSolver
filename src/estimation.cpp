//
// Created by baizeyv on 1/13/2026.
//

#include "estimation.h"

#include <cstdint>

uint8_t estimation::total() const {
	const uint16_t sum = static_cast<uint16_t>(current) + static_cast<uint16_t>(remaining);
	return (sum > 255) ? 255 : static_cast<uint8_t>(sum);
}

bool estimation::operator==(const estimation &other) const {
	return current == other.current && remaining == other.remaining;
}
