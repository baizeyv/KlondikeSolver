//
// Created by baizeyv on 1/13/2026.
//

#include "bucket.h"

bool bucket::is_empty() const {
	return key.low == 0 && key.high == 0; // 对应 u64::MAX
}
