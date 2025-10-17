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

    // --- 1. ��������ӱ��� ---
    Sprite* bgSprite = Sprite::create("res/card_general.png");
    if (!bgSprite) {
        CCLOG("Error: Failed to load card background 'card_general.png'");
        return false;
    }
    this->addChild(bgSprite);
    // ��ȡ����ʵ�ʳߴ磨�ؼ�������Ԫ��λ�û��ڴ˼��㣩
    Size bgSize = bgSprite->getContentSize();
    // ����Card�����ߴ��뱳��һ��
    this->setContentSize(bgSize);
    // ����ê����Ϊ���ģ������������м䣨���������뱳���ߴ���쵼��ƫ�ƣ�
    bgSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    bgSprite->setPosition(Vec2(bgSize.width / 2, bgSize.height / 2));

    // --- 2. �ж���ɫ����/�죩 ---
    std::string colorStr = (_suit == CardSuitType::CST_HEARTS || _suit == CardSuitType::CST_DIAMONDS)
        ? "red" : "black";

    // --- 3. ���Ͻ�С���֣����ڱ������ϽǶ�λ�� ---
    std::string smallNumPath = "res/number/small_" + colorStr + "_" + getFaceString(_face) + ".png";
    Sprite* smallNumSprite = Sprite::create(smallNumPath);
    if (smallNumSprite) {
        smallNumSprite->setAnchorPoint(Vec2(0, 1)); // ê�㣺���Ͻǣ�0,1��
        // λ�ã��������Ͻ�ƫ��10px����߿��е��������ȫ��ʾ���֣�
        smallNumSprite->setPosition(Vec2(10, bgSize.height - 10));
        this->addChild(smallNumSprite);
    }
    else {
        CCLOG("Warning: С���� %s ����ʧ��", smallNumPath.c_str());
    }

    // --- 4. ���Ͻǻ�ɫ�����ڱ������ϽǶ�λ�� ---
    std::string suitPath = "res/suits/" + getSuitString(_suit) + ".png";
    Sprite* suitSprite = Sprite::create(suitPath);
    if (suitSprite) {
        suitSprite->setAnchorPoint(Vec2(1, 1)); // ê�㣺���Ͻǣ�1,1��
        // λ�ã��������Ͻ�ƫ��10px
        suitSprite->setPosition(Vec2(bgSize.width - 10, bgSize.height - 10));
        this->addChild(suitSprite);
    }
    else {
        CCLOG("Warning: ��ɫ %s ����ʧ��", suitPath.c_str());
    }

    // --- 5. ���Ĵ����֣����ڱ������Ķ�λ�� ---
    std::string bigNumPath = "res/number/big_" + colorStr + "_" + getFaceString(_face) + ".png";
    Sprite* bigNumSprite = Sprite::create(bigNumPath);
    if (bigNumSprite) {
        bigNumSprite->setAnchorPoint(Vec2(0.5f, 0.5f)); // ê�㣺����
        // λ�ã�����������
        bigNumSprite->setPosition(Vec2(bgSize.width / 2, bgSize.height / 2));
        this->addChild(bigNumSprite);
    }
    else {
        CCLOG("Warning: ������ %s ����ʧ��", bigNumPath.c_str());
    }

    return true;
}

// ��������������ɫö��ת��Ϊ�ַ��� (�Ѹ�����ƥ���µĻ�ɫͼƬ��)
std::string Card::getSuitString(CardSuitType suit) {
    switch (suit) {
    case CardSuitType::CST_CLUBS:    return "club";
    case CardSuitType::CST_DIAMONDS: return "diamond";
    case CardSuitType::CST_HEARTS:   return "heart";
    case CardSuitType::CST_SPADES:   return "spade";
    default: return "";
    }
}

// ����������������ö��ת��Ϊ�ַ��� (�����޸�)
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