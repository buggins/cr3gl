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
#include "crui.h"

void LVInitCoolReaderTizen(const wchar_t * resourceDir);
void LVSetTizenLogger();

class CRUIEventAdapter : public Tizen::Ui::ITouchEventListener {
	CRUIEventManager * _eventManager;
	LVPtrVector<CRUIMotionEventItem> _activePointers;
	void dispatchTouchEvent(const Tizen::Ui::TouchEventInfo &touchInfo);
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
};

#endif /* CR3TIZEN_H_ */
