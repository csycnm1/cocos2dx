#ifndef CARD_H
#define CARD_H

#include "cocos2d.h"
#include "CardEnums.h"

// Card ���ڼ̳��� Node��ʹ���Ϊһ������
class Card : public cocos2d::Node {
public:
    // �������Ƶľ�̬��������������
    static Card* create(CardSuitType suit, CardFaceType face);

    // ��д init ����
    bool init(CardSuitType suit, CardFaceType face);

    // ��ȡ�������Եķ�������
    CardSuitType getSuit() const { return _suit; }
    CardFaceType getFace() const { return _face; }

	// ��������ȡ��ɫ�ַ������� "hearts", "spades"��
    static std::string getSuitString(CardSuitType suit);
    // ��������ȡ�����ַ������� "A", "2", ..., "K"
    static std::string getFaceString(CardFaceType face);

private:
    CardSuitType _suit;
    CardFaceType _face;
};

#endif // CARD_H