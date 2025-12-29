//
// Created by baizeyv on 12/26/2025.
//

#ifndef KLONDIKESOLVER_STATE_H
#define KLONDIKESOLVER_STATE_H
#include "card.h"
#include "history_item.h"

#include <vector>

class state {
public:
  /**
   * * 右上角的牌堆
   */
  std::vector<card *> deck_cards;

  /**
   * * 每一列的隐藏的牌
   */
  std::vector<std::vector<card *>> hidden_cards;

  /**
   * * 每一列的可见的牌
   */
  std::vector<std::vector<card *>> visible_cards;

  /**
   * * 历史记录
   */
  std::vector<history_item> history;

  /**
   * * 上一步的状态
   */
  const state * previous;

  // todo:
};

#endif // KLONDIKESOLVER_STATE_H
