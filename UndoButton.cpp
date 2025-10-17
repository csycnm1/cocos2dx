#include "UndoButton.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

UndoButton* UndoButton::create() {
    UndoButton* btn = new (std::nothrow) UndoButton();
    if (btn && btn->init()) {  
        btn->autorelease();
        return btn;
    }
    CC_SAFE_DELETE(btn);
    return nullptr;
}

void UndoButton::setUndoCallback(const std::function<void()>& callback) {
    _undoCallback = callback;
    addClickEventListener([this](Ref* sender) {
        if (_undoCallback) {
            _undoCallback();
        }
        });
}