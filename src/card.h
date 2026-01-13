//
// Created by baizeyv on 12/26/2025.
//

#ifndef KLONDIKESOLVER_CARD_H
#define KLONDIKESOLVER_CARD_H
#include <cstdint>
#include <string>

/**
 * * 方片 A-B-C-D-E-F-G-H-I-J-K-L-M <br/>
 * * 黑桃 N-O-P-Q-R-S-T-U-V-W-X-Y-Z <br/>
 * * 梅花 a-b-c-d-e-f-g-h-i-j-k-l-m <br/>
 * * 红桃 n-o-p-q-r-s-t-u-v-w-x-y-z <br/>
 */
class card {
	/**
	 * * 牌面值
	 */
	uint8_t value: 4;
	/**
	 * * 牌面花色 <br/>
	 * * 0方片 1黑桃 2梅花 3红桃
	 */
	uint8_t suit: 2;

	/**
	 * * 原本的字符值
	 */
	char original_char;

public:
	explicit card(char c);

	/**
	 * * 判断指定牌是否可以移动到当前这张牌的的下边
	 * @param cd
	 * @return
	 */
	bool can_move_to_me(const card* cd) const;

	[[nodiscard]]
	uint8_t get_value() const;

	[[nodiscard]]
	uint8_t get_suit() const;

	[[nodiscard]]
	char get_char() const;

	[[nodiscard]]
	std::string to_str() const;

private:
	/**
	 * * 获取花色字符串
	 * @return
	 */
	[[nodiscard]]
	std::string get_suit_string() const;

	/**
	 * * 获取牌面值字符串
	 * @return
	 */
	[[nodiscard]]
	std::string get_value_string() const;
};

#endif // KLONDIKESOLVER_CARD_H
