//
// Created by baizeyv on 12/26/2025.
//

#ifndef KLONDIKESOLVER_CARD_H
#define KLONDIKESOLVER_CARD_H
#include <cstdint>

class card {
public:
  uint8_t value : 4;
  /**
   * * 牌面花色
   */
  uint8_t suit : 2;
};

#endif // KLONDIKESOLVER_CARD_H
