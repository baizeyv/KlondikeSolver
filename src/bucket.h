//
// Created by baizeyv on 1/13/2026.
//

#ifndef KLONDIKESOLVER_BUCKET_H
#define KLONDIKESOLVER_BUCKET_H


#include "estimation.h"
#include "state_key.h"

#pragma pack(push, 1)

struct bucket {
	/**
	 * * 棋盘状态的hash value
	 */
	state_key key;

	/**
	 * * 对应的评估数据
	 */
	estimation value;

	[[nodiscard]]
	bool is_empty() const;
};
#pragma pack(pop)


#endif //KLONDIKESOLVER_BUCKET_H