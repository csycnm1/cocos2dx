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
    // ��д init ������ȷ����ȷ��ʼ�� Button ����
    virtual bool init() override {
        if (!Button::init()) {  // ��ʼ������
            return false;
        }
        // �������������ð�ťĬ����ʽ
        return true;
    }
private:
	std::function<void()> _undoCallback;
};

#endif