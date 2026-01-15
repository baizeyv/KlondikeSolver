//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_STATE_MAP_H
#define KLONDIKESOLVER_STATE_MAP_H
#include <optional>
#include <vector>

#include "bucket.h"
#include "../meow.h"


/**
 * * 用于A*搜索的状态的去重和剪枝
 */
class state_map {
private:
	size_t capacity;

	std::vector<bucket> buckets;
public:
	explicit state_map(size_t cap);

	/**
	 * * 获取指定key的评估值
	 * * 使用 线性探测法(linear probing) 处理hash-clash
	 * * 返回 pair<评估值指针, 桶索引>
	 * @param key
	 * @return
	 */
	[[nodiscard]]
	optional<pair<const estimation*, size_t>> get(const state_key& key) const;

	/**
	 * * 插入新状态
	 * @param key
	 * @param value
	 */
	void insert(state_key& key, estimation value);

	/**
	 * * 提供修改评估值的接口
	 * @param index
	 * @return
	 */
	estimation& estimation_mut(size_t index);
};


#endif //KLONDIKESOLVER_STATE_MAP_H