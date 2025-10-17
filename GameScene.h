#pragma once
#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "Card.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>

// ������¼�����ƶ�������״̬��֧�ֻ��ˣ�
struct CardMoveHistory {
    Card* movedCard;                  // ���ƶ��Ŀ��ƣ�������/���������ƣ�
    cocos2d::Node* movedCardOrigParent; // ���ƶ����Ƶ�ԭʼ���ڵ�
    cocos2d::Vec2 movedCardOrigPos;   // ���ƶ����Ƶ�ԭʼλ��
    bool movedCardOrigVisible;        // ���ƶ����Ƶ�ԭʼ�ɼ���

    Card* replacedCard;               // ���滻�Ŀ��ƣ��������Ҳ࿨�ƣ�
    cocos2d::Node* replacedCardOrigParent; // ���滻���Ƶ�ԭʼ���ڵ�
    cocos2d::Vec2 replacedCardOrigPos;   // ���滻���Ƶ�ԭʼλ��
    bool replacedCardOrigVisible;      // ���滻���Ƶ�ԭʼ�ɼ���
};

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(GameScene);

    // ��¼�ƶ���ʷ�������滻����״̬��
    void recordMoveHistory(
        Card* movedCard, cocos2d::Node* movedOrigParent, cocos2d::Vec2 movedOrigPos, bool movedOrigVisible,
        Card* replacedCard, cocos2d::Node* replacedOrigParent, cocos2d::Vec2 replacedOrigPos, bool replacedOrigVisible
    );
    // ִ�л��˲��������򶯻�+״̬��ԭ��
    void undoMove();

private:
    Card* _lastStackCard; // �������Ҳ൱ǰ��ʾ�Ŀ��ƣ������ƣ�
    std::vector<CardMoveHistory> _moveHistory; // �ƶ���ʷջ��֧���������ˣ�
};

#endif // !__GAMESCENE_H__