/*
 * cr3tizen.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#include "tizenx.h"
#include "cr3tizen.h"
#include "lvstring.h"
#include "glfont.h"
#include "gldrawbuf.h"
#include "crui.h"
#include <sys/time.h>

using namespace CRUI;

class CRTizenLogger : public CRLog
{
protected:
    virtual void log( const char * level, const char * msg, va_list args )
    {
    	char buf[4096];
    	vsnprintf(buf, 4095, msg, args);
        if (!strcmp("ERROR", level))
        	AppLogExceptionInternal("", 0, "%s", buf);
        else if (!strcmp("INFO", level))
        	AppLogInternal("", 0, "%s", buf);
        else
        	AppLogDebugInternal("", 0, "%s", buf);
    }
public:
    CRTizenLogger()
    {
        static const char * log_level_names[] = {
        "FATAL",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "TRACE",
        };
        info( "Started logging. Level=%s", log_level_names[getLogLevel()] );
    }

    virtual ~CRTizenLogger() {
    }
};

void LVSetTizenLogger() {
	CRLog::setLogger(new CRTizenLogger());
}

void LVInitCoolReaderTizen(const wchar_t * resourceDir) {
	LVSetTizenLogger();
	CRLog::info("Starting CoolReader");
	CRLog::setLogLevel(CRLog::LL_TRACE);
	InitFontManager(lString8());
	LVInitGLFontManager(fontMan);
	fontMan->RegisterFont(lString8("/usr/share/fonts/TizenSansMeduim.ttf"));
	fontMan->RegisterFont(lString8("/usr/share/fonts/TizenSansRegular.ttf"));
	fontMan->RegisterFont(lString8("/usr/share/fallback_fonts/TizenSansFallback.ttf"));
	fontMan->SetFallbackFontFace(lString8("Tizen Sans Fallback"));
	lString8Collection dirs;
	//dirs.add(UnicodeToUtf8(resourceDir));
	dirs.add(UnicodeToUtf8(resourceDir) + "screen-density-xhigh");
	LVCreateResourceResolver(dirs);
	LVGLCreateImageCache();

	currentTheme = new CRUITheme(lString8("BLACK"));
	currentTheme->setTextColor(0x000000);
	currentTheme->setFontForSize(CRUI::FONT_SIZE_XSMALL, fontMan->GetFont(16, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setFontForSize(CRUI::FONT_SIZE_SMALL, fontMan->GetFont(20, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setFontForSize(CRUI::FONT_SIZE_MEDIUM, fontMan->GetFont(26, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setFontForSize(CRUI::FONT_SIZE_LARGE, fontMan->GetFont(34, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setFontForSize(CRUI::FONT_SIZE_XLARGE, fontMan->GetFont(44, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setListDelimiterVertical(resourceResolver->getIcon("divider_light_v3.png"));
	CRUIStyle * buttonStyle = currentTheme->addSubstyle(lString8("BUTTON"));
	//keyboard_key_feedback_background.9
	buttonStyle->setBackground("btn_default_normal.9")->setFontSize(FONT_SIZE_LARGE);
	//buttonStyle->setBackground("keyboard_key_feedback_background.9")->setFontSize(FONT_SIZE_LARGE)->setPadding(10);
	//buttonStyle->setBackground("btn_default_normal.9")->setFontSize(FONT_SIZE_LARGE)->setPadding(10);
	buttonStyle->addSubstyle(STATE_PRESSED, STATE_PRESSED)->setBackground("btn_default_pressed.9");
	buttonStyle->addSubstyle(STATE_FOCUSED, STATE_FOCUSED)->setBackground("btn_default_selected.9");
	buttonStyle->addSubstyle(STATE_DISABLED, STATE_DISABLED)->setTextColor(0x80000000);
}



CRUIEventAdapter::CRUIEventAdapter(CRUIEventManager * eventManager) : _eventManager(eventManager)
{

}

using namespace Tizen::Ui;

int CRUIEventAdapter::findPointer(lUInt64 id) {
	for (int i=0; i<_activePointers.length(); i++)
		if (_activePointers[i]->getPointerId() == id)
			return i;
	return -1;
}

lUInt64 GetCurrentTimeMillis() {
#if defined(LINUX) || defined(ANDROID) || defined(_LINUX)
	timeval ts;
	gettimeofday(&ts, NULL);
	return ts.tv_sec * (lUInt64)1000 + ts.tv_usec / 1000;
#else
	#error * You should define GetCurrentTimeMillis() *
#endif
}

void CRUIEventAdapter::dispatchTouchEvent(const Tizen::Ui::TouchEventInfo &touchInfo)
{
	int x = touchInfo.GetCurrentPosition().x;
	int y = touchInfo.GetCurrentPosition().y;
	unsigned long pointId = touchInfo.GetPointId();
//	int startX = touchInfo.GetStartPosition().x;
//	int starty = touchInfo.GetStartPosition().y;
	int status = touchInfo.GetTouchStatus();
	int action = 0;
	switch (status) {
	case TOUCH_PRESSED: //The touch pressed event type
		action = ACTION_DOWN; break;
	case TOUCH_LONG_PRESSED: //The touch long pressed event type
		//action = ACTION_DOWN; ignore
		break;
	case TOUCH_RELEASED: //The touch released event type
		action = ACTION_UP; break;
	case TOUCH_MOVED: //The touch moved event type
		action = ACTION_MOVE; break;
	case TOUCH_DOUBLE_PRESSED: //The touch double pressed event type
		//action = ACTION_DOWN; // ignore
		break;
	case TOUCH_FOCUS_IN: //The touch focus-in event type
		action = ACTION_FOCUS_IN; break;
	case TOUCH_FOCUS_OUT: //The touch focus-out event type
		action = ACTION_FOCUS_OUT; break;
	case TOUCH_CANCELED: //The touch canceled event type
		action = ACTION_CANCEL; break;
	}
	if (action) {
		int index = findPointer(pointId);
		CRUIMotionEventItem * lastItem = index >= 0 ? _activePointers[index] : NULL;
		bool isLast = (action == ACTION_CANCEL || action == ACTION_UP);
		bool isFirst = (action == ACTION_DOWN);
		if (!lastItem && !isFirst) {
			CRLog::warn("Ignoring unexpected touch event %d with id%lld", action, pointId);
			return;
		}
		lUInt64 ts = GetCurrentTimeMillis();
		CRUIMotionEventItem * item = new CRUIMotionEventItem(lastItem, pointId, action, x, y, ts);
		if (index >= 0) {
			if (!isLast)
				_activePointers.set(index, item);
			else
				_activePointers.remove(index);
		} else {
			if (!isLast)
				_activePointers.add(item);
		}
		CRUIMotionEvent * event = new CRUIMotionEvent();
		event->addEvent(item);
		for (int i=0; i<_activePointers.length(); i++) {
			if (_activePointers[i] != item)
				event->addEvent(_activePointers[i]);
		}
		_eventManager->dispatchTouchEvent(event);
		delete event;
	}
}

void  CRUIEventAdapter::OnTouchCanceled (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchFocusIn (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchFocusOut (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchMoved (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchPressed (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchReleased (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

