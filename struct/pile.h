//
// Created by baizeyv on 12/31/2025.
//

#ifndef KLONDIKESOLVER_PILE_H
#define KLONDIKESOLVER_PILE_H
#include <vector>

#include "../src/card.h"

// ! 这个结构并不安全,在使用任何方法前都需要确定不越界

class pile {
	/**
	 * * 牌堆中的牌数组
	 */
	std::vector<card *> vec {};

public:
	/**
	 * * no-arguments constructor
	 */
	pile();

	explicit pile(std::vector<card *> vec);

	/**
	 * * 获取最后一张牌
	 * @return
	 */
	card* back() const;

	/**
	 * * 弹出最后一张牌
	 * @return
	 */
	card* pop_back();

	/**
	 * * 弹出最后n张牌
	 * @param n
	 * @return
	 */
	pile pop_back(int n);

	/**
	 * * 弹出指定索引的一张牌
	 * @param index
	 * @return
	 */
	card* pop(int index);

	/**
	 * * 将一张牌放入最后
	 * @param card
	 */
	void push_back(card* card);

	/**
	 * * 将一组牌堆放入最后
	 * @param pile
	 */
	void push_back(pile pile);

	/**
	 * * 判断牌堆是否为空
	 * @return
	 */
	bool empty() const;

	/**
	 * * 获取当前这个牌堆的牌数量
	 * @return
	 */
	int size() const;

	/**
	 * * 翻转牌堆
	 */
	void reverse();

	/**
	 * * 清空牌堆
	 */
	void clear();

	/**
	 * * 只读的索引器
	 * @param index
	 * @return
	 */
	const card* operator[](size_t index) const;
};

/**
 * * 牌堆数组
 */
using pile_vec = std::vector<pile>;

#endif //KLONDIKESOLVER_PILE_H
