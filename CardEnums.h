#pragma once
#ifndef CARD_ENUMS_H
#define CARD_ENUMS_H

/**
 * @brief ���忨�ƵĻ�ɫ
 */
enum class CardSuitType
{
    CST_CLUBS,    // ÷��
    CST_DIAMONDS, // ����
    CST_HEARTS,   // ����
    CST_SPADES,   // ����
    CST_COUNT     // ��ɫ���������ڱ�����У�飬������һ����Ч�Ļ�ɫ
};

/**
 * @brief ���忨�Ƶĵ���
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
    CFT_COUNT  // �������������ڱ�����У�飬������һ����Ч�ĵ���
};

#endif // CARD_ENUMS_H