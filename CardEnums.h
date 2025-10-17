#pragma once
#ifndef CARD_ENUMS_H
#define CARD_ENUMS_H

/**
 * @brief 定义卡牌的花色
 */
enum class CardSuitType
{
    CST_CLUBS,    // 梅花
    CST_DIAMONDS, // 方块
    CST_HEARTS,   // 红桃
    CST_SPADES,   // 黑桃
    CST_COUNT     // 花色总数，用于遍历或校验，本身不是一个有效的花色
};

/**
 * @brief 定义卡牌的点数
 */
enum class CardFaceType
{
    CFT_ACE,   // A
    CFT_TWO,   // 2
    CFT_THREE, // 3
    CFT_FOUR,  // 4
    CFT_FIVE,  // 5
    CFT_SIX,   // 6
    CFT_SEVEN, // 7
    CFT_EIGHT, // 8
    CFT_NINE,  // 9
    CFT_TEN,   // 10
    CFT_JACK,  // J
    CFT_QUEEN, // Q
    CFT_KING,  // K
    CFT_COUNT  // 点数总数，用于遍历或校验，本身不是一个有效的点数
};

#endif // CARD_ENUMS_H