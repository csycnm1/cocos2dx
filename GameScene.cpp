#include "GameScene.h"
#include "UndoButton.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

using namespace cocos2d;
using namespace cocos2d::ui;
using namespace rapidjson;

// 卡牌数据结构（配置文件解析用）
struct CardData {
    int cardFace;
    int cardSuit;
    Vec2 position;
};

Scene* GameScene::createScene() {
    return GameScene::create();
}

// 记录移动历史（保存所有需回退的状态）
void GameScene::recordMoveHistory(
    Card* movedCard, Node* movedOrigParent, Vec2 movedOrigPos, bool movedOrigVisible,
    Card* replacedCard, Node* replacedOrigParent, Vec2 replacedOrigPos, bool replacedOrigVisible
) {
    CardMoveHistory history;
    // 记录被移动卡牌状态
    history.movedCard = movedCard;
    history.movedCardOrigParent = movedOrigParent;
    history.movedCardOrigPos = movedOrigPos;
    history.movedCardOrigVisible = movedOrigVisible;
    // 记录被替换卡牌状态
    history.replacedCard = replacedCard;
    history.replacedCardOrigParent = replacedOrigParent;
    history.replacedCardOrigPos = replacedOrigPos;
    history.replacedCardOrigVisible = replacedOrigVisible;
    // 加入历史栈
    _moveHistory.push_back(history);
}

// 回退操作：反向还原所有状态（含动画）
void GameScene::undoMove() {
    if (_moveHistory.empty()) {
        CCLOG("无可用回退记录");
        return;
    }
    // 获取最后一次移动记录
    CardMoveHistory lastMove = _moveHistory.back();
    _moveHistory.pop_back();

    // -------------------------- 1. 回退被移动的卡牌 --------------------------
    if (lastMove.movedCard) {
        lastMove.movedCard->retain(); // 防止释放
        lastMove.movedCard->stopAllActions(); // 停止当前动画
        lastMove.movedCard->removeFromParent();
        // 还原到原始父节点和位置（反向动画）
        lastMove.movedCardOrigParent->addChild(lastMove.movedCard);
        lastMove.movedCard->setVisible(lastMove.movedCardOrigVisible);
        lastMove.movedCard->runAction(
            MoveTo::create(0.3f, lastMove.movedCardOrigPos) // 反向移动动画
        );
        lastMove.movedCard->release();
    }

    // -------------------------- 2. 回退被替换的卡牌 --------------------------
    if (lastMove.replacedCard) {
        lastMove.replacedCard->retain(); // 防止释放
        lastMove.replacedCard->stopAllActions(); // 停止当前动画
        lastMove.replacedCard->removeFromParent();
        // 还原到原始父节点和位置（反向动画）
        lastMove.replacedCardOrigParent->addChild(lastMove.replacedCard);
        lastMove.replacedCard->setVisible(lastMove.replacedCardOrigVisible);
        lastMove.replacedCard->runAction(
            MoveTo::create(0.3f, lastMove.replacedCardOrigPos) // 反向移动动画
        );
        lastMove.replacedCard->release();
        // 还原堆牌区右侧卡牌引用
        _lastStackCard = lastMove.replacedCard;
    }
}

// 辅助函数：判断主牌区卡牌是否被其他卡牌覆盖
bool isCardCovered(Card* targetCard, Layout* mainCardArea) {
    if (!targetCard || !mainCardArea) return false;

    // 1. 获取目标卡牌的世界坐标系矩形（用于判断重叠）
    Rect targetWorldRect = targetCard->getBoundingBox();
    // 2. 获取目标卡牌的Z轴层级（层级低的可能被覆盖）
    int targetZOrder = targetCard->getLocalZOrder();

    // 3. 遍历主牌区所有其他卡牌，检查是否存在“重叠且Z值更高”的卡牌
    const Vector<Node*>& children = mainCardArea->getChildren();
    for (auto& child : children) {
        // 过滤非Card类型的子节点（避免误判）
        Card* otherCard = dynamic_cast<Card*>(child);
        if (!otherCard || otherCard == targetCard) continue;

        // 仅检查可见的卡牌（隐藏的卡牌不参与遮挡判断）
        if (!otherCard->isVisible()) continue;

        // 计算其他卡牌的世界坐标系矩形
        Rect otherWorldRect = otherCard->getBoundingBox();
        // 若两卡牌重叠，且其他卡牌Z值更高 → 目标卡牌被覆盖
        if (targetWorldRect.intersectsRect(otherWorldRect) && otherCard->getLocalZOrder() > targetZOrder) {
            CCLOG("卡牌（%s）被覆盖，禁止移动", targetCard->getFaceString(targetCard->getFace()).c_str());
            return true;
        }
    }

    // 无重叠或重叠卡牌Z值更低 → 未被覆盖
    return false;
}

// 场景初始化（核心逻辑）
bool GameScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // -------------------------- 1. 创建主牌区（桌面牌区） --------------------------
    Layout* mainCardArea = Layout::create();
    mainCardArea->setContentSize(Size(1080, 1500)); // 固定尺寸
    mainCardArea->setBackGroundColor(Color3B(170, 121, 66)); // 棕褐色
    mainCardArea->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    mainCardArea->setAnchorPoint(Vec2(0.5f, 1.0f)); // 顶部中心锚点
    mainCardArea->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height));
    this->addChild(mainCardArea);

    // -------------------------- 2. 创建堆牌区（手牌区） --------------------------
    Layout* deckArea = Layout::create();
    deckArea->setContentSize(Size(1080, 580)); // 固定尺寸
    deckArea->setBackGroundColor(Color3B(148, 33, 146)); // 紫色
    deckArea->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    deckArea->setAnchorPoint(Vec2(0.5f, 0.0f)); // 底部中心锚点
    deckArea->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y));
    this->addChild(deckArea);

    // 堆牌区分左右两部分：左（普通牌）、右（顶部牌+回退按钮）
    Layout* leftStackArea = Layout::create(); // 左侧普通牌区
    leftStackArea->setContentSize(Size(deckArea->getContentSize().width / 2, deckArea->getContentSize().height));
    leftStackArea->setPosition(Vec2(0, 0));
    deckArea->addChild(leftStackArea);

    Layout* rightStackArea = Layout::create(); // 右侧顶部牌+按钮区
    rightStackArea->setContentSize(Size(deckArea->getContentSize().width / 2, deckArea->getContentSize().height));
    rightStackArea->setPosition(Vec2(deckArea->getContentSize().width / 2, 0));
    deckArea->addChild(rightStackArea);

    // -------------------------- 3. 解析配置文件（加载卡牌） --------------------------
    std::string jsonPath = FileUtils::getInstance()->fullPathForFilename("game_config/game_test_config.json");
    std::string jsonContent = FileUtils::getInstance()->getStringFromFile(jsonPath);
    if (jsonContent.empty()) {
        CCLOG("错误：配置文件不存在或为空！");
        return true;
    }

    Document doc;
    doc.Parse(jsonContent.c_str());
    if (doc.HasParseError()) {
        CCLOG("错误：JSON解析失败，错误码：%d", doc.GetParseError());
        return true;
    }

    // -------------------------- 4. 加载主牌区（桌面牌） --------------------------
    if (doc.HasMember("Playfield") && doc["Playfield"].IsArray()) {
        const rapidjson::Value& playfieldArr = doc["Playfield"];
        // 按加载顺序设置Z轴层级（后加载的卡牌Z值更高，显示在上方）
        int zOrder = 1;

        for (rapidjson::SizeType i = 0; i < playfieldArr.Size(); ++i) {
            const rapidjson::Value& cardData = playfieldArr[i];
            int face = cardData["CardFace"].GetInt();
            int suit = cardData["CardSuit"].GetInt();
            float x = cardData["Position"]["x"].GetFloat();
            float y = cardData["Position"]["y"].GetFloat();

            Card* card = Card::create(static_cast<CardSuitType>(suit), static_cast<CardFaceType>(face));
            if (card) {
                card->setAnchorPoint(Vec2(0.5f, 0.5f));
                card->setPosition(Vec2(x, y));
                // 关键：设置Z轴层级（后加载的卡牌Z值递增，确保覆盖关系正确）
                mainCardArea->addChild(card, zOrder++);

                // -------------------------- 桌面牌点击事件（新增遮挡判断） --------------------------
                auto touchListener = EventListenerTouchOneByOne::create();
                touchListener->onTouchBegan = [this, card, mainCardArea, rightStackArea](Touch* touch, Event* event) {
                    // 第一步：判断卡牌是否被其他卡牌覆盖（被覆盖则禁止移动）
                    if (isCardCovered(card, mainCardArea)) {
                        return false; // 不响应点击
                    }

                    // 第二步：检测点击是否在当前卡牌范围内
                    Vec2 touchPos = touch->getLocation();
                    Vec2 cardLocalPos = card->convertToNodeSpace(touchPos);
                    Rect cardRect = Rect(0, 0, card->getContentSize().width, card->getContentSize().height);
                    if (!cardRect.containsPoint(cardLocalPos) || !_lastStackCard) {
                        return false;
                    }

                    // 第三步：校验点数差1（含K与A的特殊情况）
                    CardFaceType clickedFace = card->getFace();
                    CardFaceType topFace = _lastStackCard->getFace();
                    bool isMatch = false;

                    // 特殊情况：K(12)与A(0)差1
                    if ((clickedFace == CardFaceType::CFT_KING && topFace == CardFaceType::CFT_ACE) ||
                        (clickedFace == CardFaceType::CFT_ACE && topFace == CardFaceType::CFT_KING)) {
                        isMatch = true;
                    }
                    // 普通情况：点数差1
                    else {
                        int faceDiff = abs(static_cast<int>(clickedFace) - static_cast<int>(topFace));
                        isMatch = (faceDiff == 1);
                    }

                    // 第四步：匹配成功，执行替换逻辑
                    if (isMatch) {
                        // 记录被移动卡牌（桌面牌）的原始状态
                        Node* movedOrigParent = card->getParent();
                        Vec2 movedOrigPos = card->getPosition();
                        bool movedOrigVisible = card->isVisible();

                        // 记录被替换卡牌（堆牌区顶部牌）的原始状态
                        Node* replacedOrigParent = _lastStackCard->getParent();
                        Vec2 replacedOrigPos = _lastStackCard->getPosition();
                        bool replacedOrigVisible = _lastStackCard->isVisible();

                        // 保存历史记录（用于回退）
                        recordMoveHistory(
                            card, movedOrigParent, movedOrigPos, movedOrigVisible,
                            _lastStackCard, replacedOrigParent, replacedOrigPos, replacedOrigVisible
                        );

                        // 执行替换：隐藏原顶部牌，移动桌面牌到顶部位置
                        _lastStackCard->setVisible(false); // 隐藏而非删除
                        card->retain();
                        card->removeFromParent();
                        rightStackArea->addChild(card);
                        card->runAction(MoveTo::create(0.3f, replacedOrigPos)); // 移动动画
                        card->release();

                        // 更新堆牌区顶部牌引用
                        _lastStackCard = card;
                    }

                    return true;
                    };
                _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, card);
            }
        }
    }

    // -------------------------- 5. 加载堆牌区（手牌） --------------------------
    if (doc.HasMember("Stack") && doc["Stack"].IsArray()) {
        const rapidjson::Value& stackArr = doc["Stack"];
        rapidjson::SizeType totalCardCount = stackArr.Size();
        if (totalCardCount == 0) return true;

        // 5.1 加载左侧普通牌（除最后一张外）
        rapidjson::SizeType leftCardCount = (totalCardCount > 1) ? totalCardCount - 1 : 0;
        float leftAreaWidth = leftStackArea->getContentSize().width;
        float leftCardInterval = (leftAreaWidth - 280) / (leftCardCount > 0 ? leftCardCount : 1); // 边距280

        for (rapidjson::SizeType i = 0; i < leftCardCount; ++i) {
            const rapidjson::Value& cardData = stackArr[i];
            int face = cardData["CardFace"].GetInt();
            int suit = cardData["CardSuit"].GetInt();
            float x = 140 + i * leftCardInterval + leftCardInterval / 2; // 左边距140
            float y = leftStackArea->getContentSize().height / 2;

            Card* card = Card::create(static_cast<CardSuitType>(suit), static_cast<CardFaceType>(face));
            if (card) {
                card->setAnchorPoint(Vec2(0.5f, 0.5f));
                card->setPosition(Vec2(x, y));
                leftStackArea->addChild(card);

                // -------------------------- 手牌区翻牌事件（左侧牌→顶部牌） --------------------------
                auto touchListener = EventListenerTouchOneByOne::create();
                touchListener->onTouchBegan = [this, card, leftStackArea, rightStackArea](Touch* touch, Event* event) {
                    // 1. 检测点击是否在当前卡牌上
                    Vec2 touchPos = touch->getLocation();
                    Vec2 cardLocalPos = card->convertToNodeSpace(touchPos);
                    Rect cardRect = Rect(0, 0, card->getContentSize().width, card->getContentSize().height);
                    if (!cardRect.containsPoint(cardLocalPos)) {
                        return false;
                    }

                    // 2. 记录被移动卡牌（左侧手牌）的原始状态
                    Node* movedOrigParent = card->getParent();
                    Vec2 movedOrigPos = card->getPosition();
                    bool movedOrigVisible = card->isVisible();

                    // 3. 记录被替换卡牌（原顶部牌）的原始状态
                    Node* replacedOrigParent = _lastStackCard->getParent();
                    Vec2 replacedOrigPos = _lastStackCard->getPosition();
                    bool replacedOrigVisible = _lastStackCard->isVisible();

                    // 4. 保存历史记录（用于回退）
                    recordMoveHistory(
                        card, movedOrigParent, movedOrigPos, movedOrigVisible,
                        _lastStackCard, replacedOrigParent, replacedOrigPos, replacedOrigVisible
                    );

                    // 5. 执行翻牌替换：隐藏原顶部牌，移动左侧手牌到顶部位置
                    _lastStackCard->setVisible(false); // 隐藏而非删除
                    card->retain();
                    card->removeFromParent();
                    rightStackArea->addChild(card);
                    card->runAction(MoveTo::create(0.3f, replacedOrigPos)); // 移动动画
                    card->release();

                    // 6. 更新堆牌区顶部牌引用
                    _lastStackCard = card;

                    return true;
                    };
                _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, card);
            }
        }

        // 5.2 加载右侧顶部牌（初始顶部牌）
        const rapidjson::Value& lastCardData = stackArr[totalCardCount - 1];
        int lastFace = lastCardData["CardFace"].GetInt();
        int lastSuit = lastCardData["CardSuit"].GetInt();
        _lastStackCard = Card::create(static_cast<CardSuitType>(lastSuit), static_cast<CardFaceType>(lastFace));
        if (_lastStackCard) {
            _lastStackCard->setAnchorPoint(Vec2(0.5f, 0.5f));
            _lastStackCard->setPosition(Vec2(rightStackArea->getContentSize().width / 4, rightStackArea->getContentSize().height / 2));
            rightStackArea->addChild(_lastStackCard);
        }

        // -------------------------- 6. 添加回退按钮 --------------------------
        UndoButton* undoBtn = UndoButton::create();
        if (undoBtn) {
            undoBtn->setAnchorPoint(Vec2(0.5f, 0.5f));
            undoBtn->setPosition(Vec2(rightStackArea->getContentSize().width * 3 / 4, rightStackArea->getContentSize().height / 2));
            // 按钮样式（仅文字）
            undoBtn->setTitleFontName("fonts/arial.ttf");
            undoBtn->setTitleText("back");
            undoBtn->setTitleFontSize(50);
            undoBtn->setTitleColor(Color3B::WHITE);
            undoBtn->setContentSize(Size(120, 60));
            // 绑定回退回调
            undoBtn->setUndoCallback([this]() {
                CCLOG("执行回退操作");
                this->undoMove();
                });
            rightStackArea->addChild(undoBtn);
        }
    }

    return true;
}