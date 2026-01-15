//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_MOTION_H
#define KLONDIKESOLVER_MOTION_H
#include <cstdint>
#include <tuple>


// # 成员按1字节对齐
#pragma pack(push, 1)
/**
 * ! 移动操作
 */
struct motion {
	/**
	 * * 低4位:from, 高4位:to
	 */
	uint8_t value1;
	/**
	 * * 低7位:count, 最高位:flip(是否翻牌)
	 */
	uint8_t value2;

	[[nodiscard]]
	bool is_null() const;

	[[nodiscard]]
	uint8_t from() const;

	[[nodiscard]]
	uint8_t to() const;

	[[nodiscard]]
	uint8_t count() const;

	[[nodiscard]]
	bool flip() const;

	motion();

	motion(uint8_t from, uint8_t to, uint8_t count, bool flip);

	[[nodiscard]]
	std::tuple<size_t, size_t, size_t, bool> values() const;
};
#pragma pack(pop)


#endif //KLONDIKESOLVER_MOTION_H
