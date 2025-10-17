#pragma once
#ifndef __UNDO_BUTTON_H__
#define __UNDO_BUTTON_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;

class UndoButton : public cocos2d::ui::Button {
public:
	static UndoButton* create();
	void setUndoCallback(const std::function<void()>& callback);
protected:
	UndoButton() {}
	virtual ~UndoButton() override {}
    // 重写 init 方法，确保正确初始化 Button 基类
    virtual bool init() override {
        if (!Button::init()) {  // 初始化父类
            return false;
        }
        // 可以在这里设置按钮默认样式
        return true;
    }
private:
	std::function<void()> _undoCallback;
};

#endif