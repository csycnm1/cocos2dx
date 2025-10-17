#ifndef CARD_H
#define CARD_H

#include "cocos2d.h"
#include "CardEnums.h"

// Card 现在继承自 Node，使其成为一个容器
class Card : public cocos2d::Node {
public:
    // 创建卡牌的静态方法，参数不变
    static Card* create(CardSuitType suit, CardFaceType face);

    // 重写 init 方法
    bool init(CardSuitType suit, CardFaceType face);

    // 获取卡牌属性的方法不变
    CardSuitType getSuit() const { return _suit; }
    CardFaceType getFace() const { return _face; }

	// 新增：获取花色字符串（如 "hearts", "spades"）
    static std::string getSuitString(CardSuitType suit);
    // 新增：获取点数字符串（如 "A", "2", ..., "K"
    static std::string getFaceString(CardFaceType face);

private:
    CardSuitType _suit;
    CardFaceType _face;
};

#endif // CARD_H