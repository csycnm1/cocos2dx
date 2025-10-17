#pragma once
#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__

#include "Card.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <vector>

// 完整记录单次移动的所有状态（支持回退）
struct CardMoveHistory {
    Card* movedCard;                  // 被移动的卡牌（主牌区/手牌区卡牌）
    cocos2d::Node* movedCardOrigParent; // 被移动卡牌的原始父节点
    cocos2d::Vec2 movedCardOrigPos;   // 被移动卡牌的原始位置
    bool movedCardOrigVisible;        // 被移动卡牌的原始可见性

    Card* replacedCard;               // 被替换的卡牌（堆牌区右侧卡牌）
    cocos2d::Node* replacedCardOrigParent; // 被替换卡牌的原始父节点
    cocos2d::Vec2 replacedCardOrigPos;   // 被替换卡牌的原始位置
    bool replacedCardOrigVisible;      // 被替换卡牌的原始可见性
};

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(GameScene);

    // 记录移动历史（含被替换卡牌状态）
    void recordMoveHistory(
        Card* movedCard, cocos2d::Node* movedOrigParent, cocos2d::Vec2 movedOrigPos, bool movedOrigVisible,
        Card* replacedCard, cocos2d::Node* replacedOrigParent, cocos2d::Vec2 replacedOrigPos, bool replacedOrigVisible
    );
    // 执行回退操作（反向动画+状态还原）
    void undoMove();

private:
    Card* _lastStackCard; // 堆牌区右侧当前显示的卡牌（顶部牌）
    std::vector<CardMoveHistory> _moveHistory; // 移动历史栈（支持连续回退）
};

#endif // !__GAMESCENE_H__