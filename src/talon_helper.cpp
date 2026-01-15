//
// Created by baizeyv on 1/13/2026.
//

#include "talon_helper.h"

talon_helper::talon_helper() {
	stock_waste.fill(card_ext::UNKNOWN);
	cards_drawn.fill(0);
	stock_used.fill(false);
}

size_t talon_helper::calculate(size_t draw_count, const pile &waste_pile, const pile &stock_pile) {
	size_t size = 0;
	stock_used.fill(false);

	// 1. 检查当前废牌堆顶(waste top)
	// # 这是玩家目前唯一可以直接移动的牌
	size_t waste_size = waste_pile.size;
	if (waste_size > 0) {
		stock_waste[size] = waste_pile.peek_top_unchecked();
		cards_drawn[size] = 0; // 0步,因为它已经在废牌堆顶
	}

	// 2. 检查发排队中等待翻开的牌
	// # 根据 draw_count 计算出每翻一次会露出的那张牌
	int32_t stock_size = static_cast<int32_t>(stock_pile.size);
	int32_t position = stock_size - static_cast<int32_t>(draw_count);

	// # 如果剩下的牌不足一个 draw_count, 则只能翻到最后一张
	if (position < 0) {
		position = (stock_size > 0) ? 0 : -1;
	}

	int32_t i = position;
	while (i >= 0) {
		size_t i_usize = static_cast<size_t>(i);
		stock_waste[size] = stock_pile.get(i_usize);
		// # 计算需要的翻牌动作次数
		cards_drawn[size] = stock_size - i;
		stock_used[i_usize] = true;
		size += 1;
		i -= static_cast<int32_t>(draw_count);
	}

	// 3. 检查需要"重新洗牌(redeal)"后才能看到的牌
	// # 也就是当前已经在废牌堆里,但压在下面的牌
	int32_t amount_to_draw = stock_size + 1;
	int32_t waste_size_index = static_cast<int32_t>(waste_size) - 1;

	int32_t position_waste = static_cast<int32_t>(draw_count) - 1;
	while (position_waste < waste_size_index) {
		size_t pos_usize = static_cast<size_t>(position_waste);
		stock_waste[size] = waste_pile.get(pos_usize);
		// 用负数标记：表示需要完成当前轮次并 Redeal
		cards_drawn[size] = -amount_to_draw - position_waste;
		size += 1;
		position_waste += static_cast<int32_t>(draw_count);
	}

	// 4. 处理极端情况：Redeal 之后在 Stock 中新产生的可触达点
	// # 仅在 draw_count > 1 时发生（因为 3 张一翻会导致牌的相对位置改变）
	if (position_waste > waste_size_index && waste_size_index >= 0) {
		amount_to_draw += stock_size + waste_size_index;
		position = stock_size - position_waste + waste_size_index;

		int32_t j = position;
		while (j > 0) {
			size_t j_usize = static_cast<size_t>(j);
			if (stock_used[j_usize]) {
				break;
			}
			stock_waste[size] = stock_pile.get(j_usize);
			cards_drawn[size] = j - amount_to_draw;
			size += 1;
			j -= static_cast<int32_t>(draw_count);
		}
	}

	return size;
}
