#include "Card.h"

using namespace cocos2d;

Card* Card::create(CardSuitType suit, CardFaceType face) {
    Card* card = new (std::nothrow) Card();
    if (card && card->init(suit, face)) {
        card->autorelease();
        return card;
    }
    CC_SAFE_DELETE(card);
    return nullptr;
}

bool Card::init(CardSuitType suit, CardFaceType face) {
    if (!Node::init()) {
        return false;
    }

    _suit = suit;
    _face = face;

    // --- 1. 创建并添加背景 ---
    Sprite* bgSprite = Sprite::create("res/card_general.png");
    if (!bgSprite) {
        CCLOG("Error: Failed to load card background 'card_general.png'");
        return false;
    }
    this->addChild(bgSprite);
    // 获取背景实际尺寸（关键：所有元素位置基于此计算）
    Size bgSize = bgSprite->getContentSize();
    // 设置Card容器尺寸与背景一致
    this->setContentSize(bgSize);
    // 背景锚点设为中心，放在容器正中间（避免容器与背景尺寸差异导致偏移）
    bgSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    bgSprite->setPosition(Vec2(bgSize.width / 2, bgSize.height / 2));

    // --- 2. 判断颜色（黑/红） ---
    std::string colorStr = (_suit == CardSuitType::CST_HEARTS || _suit == CardSuitType::CST_DIAMONDS)
        ? "red" : "black";

    // --- 3. 左上角小数字（基于背景左上角定位） ---
    std::string smallNumPath = "res/number/small_" + colorStr + "_" + getFaceString(_face) + ".png";
    Sprite* smallNumSprite = Sprite::create(smallNumPath);
    if (smallNumSprite) {
        smallNumSprite->setAnchorPoint(Vec2(0, 1)); // 锚点：左上角（0,1）
        // 位置：背景左上角偏移10px（与边框有点距离以完全显示数字）
        smallNumSprite->setPosition(Vec2(10, bgSize.height - 10));
        this->addChild(smallNumSprite);
    }
    else {
        CCLOG("Warning: 小数字 %s 加载失败", smallNumPath.c_str());
    }

    // --- 4. 右上角花色（基于背景右上角定位） ---
    std::string suitPath = "res/suits/" + getSuitString(_suit) + ".png";
    Sprite* suitSprite = Sprite::create(suitPath);
    if (suitSprite) {
        suitSprite->setAnchorPoint(Vec2(1, 1)); // 锚点：右上角（1,1）
        // 位置：背景右上角偏移10px
        suitSprite->setPosition(Vec2(bgSize.width - 10, bgSize.height - 10));
        this->addChild(suitSprite);
    }
    else {
        CCLOG("Warning: 花色 %s 加载失败", suitPath.c_str());
    }

    // --- 5. 中心大数字（基于背景中心定位） ---
    std::string bigNumPath = "res/number/big_" + colorStr + "_" + getFaceString(_face) + ".png";
    Sprite* bigNumSprite = Sprite::create(bigNumPath);
    if (bigNumSprite) {
        bigNumSprite->setAnchorPoint(Vec2(0.5f, 0.5f)); // 锚点：中心
        // 位置：背景正中心
        bigNumSprite->setPosition(Vec2(bgSize.width / 2, bgSize.height / 2));
        this->addChild(bigNumSprite);
    }
    else {
        CCLOG("Warning: 大数字 %s 加载失败", bigNumPath.c_str());
    }

    return true;
}

// 辅助函数：将花色枚举转换为字符串 (已更新以匹配新的花色图片名)
std::string Card::getSuitString(CardSuitType suit) {
    switch (suit) {
    case CardSuitType::CST_CLUBS:    return "club";
    case CardSuitType::CST_DIAMONDS: return "diamond";
    case CardSuitType::CST_HEARTS:   return "heart";
    case CardSuitType::CST_SPADES:   return "spade";
    default: return "";
    }
}

// 辅助函数：将点数枚举转换为字符串 (无需修改)
std::string Card::getFaceString(CardFaceType face) {
    switch (face) {
    case CardFaceType::CFT_ACE:   return "A";
    case CardFaceType::CFT_TWO:   return "2"; 
    case CardFaceType::CFT_THREE: return "3";
    case CardFaceType::CFT_FOUR:  return "4";
    case CardFaceType::CFT_FIVE:  return "5";
    case CardFaceType::CFT_SIX:   return "6";
    case CardFaceType::CFT_SEVEN: return "7";
    case CardFaceType::CFT_EIGHT: return "8";
    case CardFaceType::CFT_NINE:  return "9";
    case CardFaceType::CFT_TEN:   return "10";
    case CardFaceType::CFT_JACK:  return "J";
    case CardFaceType::CFT_QUEEN: return "Q";
    case CardFaceType::CFT_KING:  return "K";
    default: return "";
    }
}