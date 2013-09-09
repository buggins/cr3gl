/*
 * cr3tizen.h
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#ifndef CR3TIZEN_H_
#define CR3TIZEN_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FUi.h>
#include <FUiControls.h>
#include <crui.h>
#include <cruievent.h>

void LVInitCoolReaderTizen(const wchar_t * resourceDir, const wchar_t * dbDir);
void LVSetTizenLogger();

class CRUIEventAdapter : public Tizen::Ui::ITouchEventListener, public Tizen::Ui::IKeyEventListener {
	CRUIEventManager * _eventManager;
	LVPtrVector<CRUIMotionEventItem> _activePointers;
	int _keyModifiers;
	int _keyModifiersRight;
	void dispatchTouchEvent(const Tizen::Ui::TouchEventInfo &touchInfo);
	bool dispatchKeyEvent(KEY_EVENT_TYPE action, Tizen::Ui::KeyCode keyCode);
	int findPointer(lUInt64 id);
public:
	CRUIEventAdapter(CRUIEventManager * eventManager);
	// touch event listener
	virtual void  OnTouchCanceled (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo);
	virtual void  OnTouchFocusIn (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo);
	virtual void  OnTouchFocusOut (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo);
	virtual void  OnTouchMoved (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo);
	virtual void  OnTouchPressed (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo);
	virtual void  OnTouchReleased (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo);
	virtual void  OnKeyLongPressed (const Tizen::Ui::Control &source, Tizen::Ui::KeyCode keyCode);
	virtual void  OnKeyPressed (const Tizen::Ui::Control &source, Tizen::Ui::KeyCode keyCode);
	virtual void  OnKeyReleased (const Tizen::Ui::Control &source, Tizen::Ui::KeyCode keyCode);
};

#endif /* CR3TIZEN_H_ */
