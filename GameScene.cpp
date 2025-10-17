#include "GameScene.h"
#include "UndoButton.h"
#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

using namespace cocos2d;
using namespace cocos2d::ui;
using namespace rapidjson;

// �������ݽṹ�������ļ������ã�
struct CardData {
    int cardFace;
    int cardSuit;
    Vec2 position;
};

Scene* GameScene::createScene() {
    return GameScene::create();
}

// ��¼�ƶ���ʷ��������������˵�״̬��
void GameScene::recordMoveHistory(
    Card* movedCard, Node* movedOrigParent, Vec2 movedOrigPos, bool movedOrigVisible,
    Card* replacedCard, Node* replacedOrigParent, Vec2 replacedOrigPos, bool replacedOrigVisible
) {
    CardMoveHistory history;
    // ��¼���ƶ�����״̬
    history.movedCard = movedCard;
    history.movedCardOrigParent = movedOrigParent;
    history.movedCardOrigPos = movedOrigPos;
    history.movedCardOrigVisible = movedOrigVisible;
    // ��¼���滻����״̬
    history.replacedCard = replacedCard;
    history.replacedCardOrigParent = replacedOrigParent;
    history.replacedCardOrigPos = replacedOrigPos;
    history.replacedCardOrigVisible = replacedOrigVisible;
    // ������ʷջ
    _moveHistory.push_back(history);
}

// ���˲���������ԭ����״̬����������
void GameScene::undoMove() {
    if (_moveHistory.empty()) {
        CCLOG("�޿��û��˼�¼");
        return;
    }
    // ��ȡ���һ���ƶ���¼
    CardMoveHistory lastMove = _moveHistory.back();
    _moveHistory.pop_back();

    // -------------------------- 1. ���˱��ƶ��Ŀ��� --------------------------
    if (lastMove.movedCard) {
        lastMove.movedCard->retain(); // ��ֹ�ͷ�
        lastMove.movedCard->stopAllActions(); // ֹͣ��ǰ����
        lastMove.movedCard->removeFromParent();
        // ��ԭ��ԭʼ���ڵ��λ�ã����򶯻���
        lastMove.movedCardOrigParent->addChild(lastMove.movedCard);
        lastMove.movedCard->setVisible(lastMove.movedCardOrigVisible);
        lastMove.movedCard->runAction(
            MoveTo::create(0.3f, lastMove.movedCardOrigPos) // �����ƶ�����
        );
        lastMove.movedCard->release();
    }

    // -------------------------- 2. ���˱��滻�Ŀ��� --------------------------
    if (lastMove.replacedCard) {
        lastMove.replacedCard->retain(); // ��ֹ�ͷ�
        lastMove.replacedCard->stopAllActions(); // ֹͣ��ǰ����
        lastMove.replacedCard->removeFromParent();
        // ��ԭ��ԭʼ���ڵ��λ�ã����򶯻���
        lastMove.replacedCardOrigParent->addChild(lastMove.replacedCard);
        lastMove.replacedCard->setVisible(lastMove.replacedCardOrigVisible);
        lastMove.replacedCard->runAction(
            MoveTo::create(0.3f, lastMove.replacedCardOrigPos) // �����ƶ�����
        );
        lastMove.replacedCard->release();
        // ��ԭ�������Ҳ࿨������
        _lastStackCard = lastMove.replacedCard;
    }
}

// �����������ж������������Ƿ��������Ƹ���
bool isCardCovered(Card* targetCard, Layout* mainCardArea) {
    if (!targetCard || !mainCardArea) return false;

    // 1. ��ȡĿ�꿨�Ƶ���������ϵ���Σ������ж��ص���
    Rect targetWorldRect = targetCard->getBoundingBox();
    // 2. ��ȡĿ�꿨�Ƶ�Z��㼶���㼶�͵Ŀ��ܱ����ǣ�
    int targetZOrder = targetCard->getLocalZOrder();

    // 3. ���������������������ƣ�����Ƿ���ڡ��ص���Zֵ���ߡ��Ŀ���
    const Vector<Node*>& children = mainCardArea->getChildren();
    for (auto& child : children) {
        // ���˷�Card���͵��ӽڵ㣨�������У�
        Card* otherCard = dynamic_cast<Card*>(child);
        if (!otherCard || otherCard == targetCard) continue;

        // �����ɼ��Ŀ��ƣ����صĿ��Ʋ������ڵ��жϣ�
        if (!otherCard->isVisible()) continue;

        // �����������Ƶ���������ϵ����
        Rect otherWorldRect = otherCard->getBoundingBox();
        // ���������ص�������������Zֵ���� �� Ŀ�꿨�Ʊ�����
        if (targetWorldRect.intersectsRect(otherWorldRect) && otherCard->getLocalZOrder() > targetZOrder) {
            CCLOG("���ƣ�%s�������ǣ���ֹ�ƶ�", targetCard->getFaceString(targetCard->getFace()).c_str());
            return true;
        }
    }

    // ���ص����ص�����Zֵ���� �� δ������
    return false;
}

// ������ʼ���������߼���
bool GameScene::init() {
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // -------------------------- 1. ���������������������� --------------------------
    Layout* mainCardArea = Layout::create();
    mainCardArea->setContentSize(Size(1080, 1500)); // �̶��ߴ�
    mainCardArea->setBackGroundColor(Color3B(170, 121, 66)); // �غ�ɫ
    mainCardArea->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    mainCardArea->setAnchorPoint(Vec2(0.5f, 1.0f)); // ��������ê��
    mainCardArea->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height));
    this->addChild(mainCardArea);

    // -------------------------- 2. �������������������� --------------------------
    Layout* deckArea = Layout::create();
    deckArea->setContentSize(Size(1080, 580)); // �̶��ߴ�
    deckArea->setBackGroundColor(Color3B(148, 33, 146)); // ��ɫ
    deckArea->setBackGroundColorType(Layout::BackGroundColorType::SOLID);
    deckArea->setAnchorPoint(Vec2(0.5f, 0.0f)); // �ײ�����ê��
    deckArea->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y));
    this->addChild(deckArea);

    // �����������������֣�����ͨ�ƣ����ң�������+���˰�ť��
    Layout* leftStackArea = Layout::create(); // �����ͨ����
    leftStackArea->setContentSize(Size(deckArea->getContentSize().width / 2, deckArea->getContentSize().height));
    leftStackArea->setPosition(Vec2(0, 0));
    deckArea->addChild(leftStackArea);

    Layout* rightStackArea = Layout::create(); // �Ҳඥ����+��ť��
    rightStackArea->setContentSize(Size(deckArea->getContentSize().width / 2, deckArea->getContentSize().height));
    rightStackArea->setPosition(Vec2(deckArea->getContentSize().width / 2, 0));
    deckArea->addChild(rightStackArea);

    // -------------------------- 3. ���������ļ������ؿ��ƣ� --------------------------
    std::string jsonPath = FileUtils::getInstance()->fullPathForFilename("game_config/game_test_config.json");
    std::string jsonContent = FileUtils::getInstance()->getStringFromFile(jsonPath);
    if (jsonContent.empty()) {
        CCLOG("���������ļ������ڻ�Ϊ�գ�");
        return true;
    }

    Document doc;
    doc.Parse(jsonContent.c_str());
    if (doc.HasParseError()) {
        CCLOG("����JSON����ʧ�ܣ������룺%d", doc.GetParseError());
        return true;
    }

    // -------------------------- 4. �����������������ƣ� --------------------------
    if (doc.HasMember("Playfield") && doc["Playfield"].IsArray()) {
        const rapidjson::Value& playfieldArr = doc["Playfield"];
        // ������˳������Z��㼶������صĿ���Zֵ���ߣ���ʾ���Ϸ���
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
                // �ؼ�������Z��㼶������صĿ���Zֵ������ȷ�����ǹ�ϵ��ȷ��
                mainCardArea->addChild(card, zOrder++);

                // -------------------------- �����Ƶ���¼��������ڵ��жϣ� --------------------------
                auto touchListener = EventListenerTouchOneByOne::create();
                touchListener->onTouchBegan = [this, card, mainCardArea, rightStackArea](Touch* touch, Event* event) {
                    // ��һ�����жϿ����Ƿ��������Ƹ��ǣ����������ֹ�ƶ���
                    if (isCardCovered(card, mainCardArea)) {
                        return false; // ����Ӧ���
                    }

                    // �ڶ�����������Ƿ��ڵ�ǰ���Ʒ�Χ��
                    Vec2 touchPos = touch->getLocation();
                    Vec2 cardLocalPos = card->convertToNodeSpace(touchPos);
                    Rect cardRect = Rect(0, 0, card->getContentSize().width, card->getContentSize().height);
                    if (!cardRect.containsPoint(cardLocalPos) || !_lastStackCard) {
                        return false;
                    }

                    // ��������У�������1����K��A�����������
                    CardFaceType clickedFace = card->getFace();
                    CardFaceType topFace = _lastStackCard->getFace();
                    bool isMatch = false;

                    // ���������K(12)��A(0)��1
                    if ((clickedFace == CardFaceType::CFT_KING && topFace == CardFaceType::CFT_ACE) ||
                        (clickedFace == CardFaceType::CFT_ACE && topFace == CardFaceType::CFT_KING)) {
                        isMatch = true;
                    }
                    // ��ͨ�����������1
                    else {
                        int faceDiff = abs(static_cast<int>(clickedFace) - static_cast<int>(topFace));
                        isMatch = (faceDiff == 1);
                    }

                    // ���Ĳ���ƥ��ɹ���ִ���滻�߼�
                    if (isMatch) {
                        // ��¼���ƶ����ƣ������ƣ���ԭʼ״̬
                        Node* movedOrigParent = card->getParent();
                        Vec2 movedOrigPos = card->getPosition();
                        bool movedOrigVisible = card->isVisible();

                        // ��¼���滻���ƣ������������ƣ���ԭʼ״̬
                        Node* replacedOrigParent = _lastStackCard->getParent();
                        Vec2 replacedOrigPos = _lastStackCard->getPosition();
                        bool replacedOrigVisible = _lastStackCard->isVisible();

                        // ������ʷ��¼�����ڻ��ˣ�
                        recordMoveHistory(
                            card, movedOrigParent, movedOrigPos, movedOrigVisible,
                            _lastStackCard, replacedOrigParent, replacedOrigPos, replacedOrigVisible
                        );

                        // ִ���滻������ԭ�����ƣ��ƶ������Ƶ�����λ��
                        _lastStackCard->setVisible(false); // ���ض���ɾ��
                        card->retain();
                        card->removeFromParent();
                        rightStackArea->addChild(card);
                        card->runAction(MoveTo::create(0.3f, replacedOrigPos)); // �ƶ�����
                        card->release();

                        // ���¶���������������
                        _lastStackCard = card;
                    }

                    return true;
                    };
                _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, card);
            }
        }
    }

    // -------------------------- 5. ���ض����������ƣ� --------------------------
    if (doc.HasMember("Stack") && doc["Stack"].IsArray()) {
        const rapidjson::Value& stackArr = doc["Stack"];
        rapidjson::SizeType totalCardCount = stackArr.Size();
        if (totalCardCount == 0) return true;

        // 5.1 ���������ͨ�ƣ������һ���⣩
        rapidjson::SizeType leftCardCount = (totalCardCount > 1) ? totalCardCount - 1 : 0;
        float leftAreaWidth = leftStackArea->getContentSize().width;
        float leftCardInterval = (leftAreaWidth - 280) / (leftCardCount > 0 ? leftCardCount : 1); // �߾�280

        for (rapidjson::SizeType i = 0; i < leftCardCount; ++i) {
            const rapidjson::Value& cardData = stackArr[i];
            int face = cardData["CardFace"].GetInt();
            int suit = cardData["CardSuit"].GetInt();
            float x = 140 + i * leftCardInterval + leftCardInterval / 2; // ��߾�140
            float y = leftStackArea->getContentSize().height / 2;

            Card* card = Card::create(static_cast<CardSuitType>(suit), static_cast<CardFaceType>(face));
            if (card) {
                card->setAnchorPoint(Vec2(0.5f, 0.5f));
                card->setPosition(Vec2(x, y));
                leftStackArea->addChild(card);

                // -------------------------- �����������¼�������ơ������ƣ� --------------------------
                auto touchListener = EventListenerTouchOneByOne::create();
                touchListener->onTouchBegan = [this, card, leftStackArea, rightStackArea](Touch* touch, Event* event) {
                    // 1. ������Ƿ��ڵ�ǰ������
                    Vec2 touchPos = touch->getLocation();
                    Vec2 cardLocalPos = card->convertToNodeSpace(touchPos);
                    Rect cardRect = Rect(0, 0, card->getContentSize().width, card->getContentSize().height);
                    if (!cardRect.containsPoint(cardLocalPos)) {
                        return false;
                    }

                    // 2. ��¼���ƶ����ƣ�������ƣ���ԭʼ״̬
                    Node* movedOrigParent = card->getParent();
                    Vec2 movedOrigPos = card->getPosition();
                    bool movedOrigVisible = card->isVisible();

                    // 3. ��¼���滻���ƣ�ԭ�����ƣ���ԭʼ״̬
                    Node* replacedOrigParent = _lastStackCard->getParent();
                    Vec2 replacedOrigPos = _lastStackCard->getPosition();
                    bool replacedOrigVisible = _lastStackCard->isVisible();

                    // 4. ������ʷ��¼�����ڻ��ˣ�
                    recordMoveHistory(
                        card, movedOrigParent, movedOrigPos, movedOrigVisible,
                        _lastStackCard, replacedOrigParent, replacedOrigPos, replacedOrigVisible
                    );

                    // 5. ִ�з����滻������ԭ�����ƣ��ƶ�������Ƶ�����λ��
                    _lastStackCard->setVisible(false); // ���ض���ɾ��
                    card->retain();
                    card->removeFromParent();
                    rightStackArea->addChild(card);
                    card->runAction(MoveTo::create(0.3f, replacedOrigPos)); // �ƶ�����
                    card->release();

                    // 6. ���¶���������������
                    _lastStackCard = card;

                    return true;
                    };
                _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, card);
            }
        }

        // 5.2 �����Ҳඥ���ƣ���ʼ�����ƣ�
        const rapidjson::Value& lastCardData = stackArr[totalCardCount - 1];
        int lastFace = lastCardData["CardFace"].GetInt();
        int lastSuit = lastCardData["CardSuit"].GetInt();
        _lastStackCard = Card::create(static_cast<CardSuitType>(lastSuit), static_cast<CardFaceType>(lastFace));
        if (_lastStackCard) {
            _lastStackCard->setAnchorPoint(Vec2(0.5f, 0.5f));
            _lastStackCard->setPosition(Vec2(rightStackArea->getContentSize().width / 4, rightStackArea->getContentSize().height / 2));
            rightStackArea->addChild(_lastStackCard);
        }

        // -------------------------- 6. ��ӻ��˰�ť --------------------------
        UndoButton* undoBtn = UndoButton::create();
        if (undoBtn) {
            undoBtn->setAnchorPoint(Vec2(0.5f, 0.5f));
            undoBtn->setPosition(Vec2(rightStackArea->getContentSize().width * 3 / 4, rightStackArea->getContentSize().height / 2));
            // ��ť��ʽ�������֣�
            undoBtn->setTitleFontName("fonts/arial.ttf");
            undoBtn->setTitleText("back");
            undoBtn->setTitleFontSize(50);
            undoBtn->setTitleColor(Color3B::WHITE);
            undoBtn->setContentSize(Size(120, 60));
            // �󶨻��˻ص�
            undoBtn->setUndoCallback([this]() {
                CCLOG("ִ�л��˲���");
                this->undoMove();
                });
            rightStackArea->addChild(undoBtn);
        }
    }

    return true;
}