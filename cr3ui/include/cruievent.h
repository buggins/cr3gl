/*
 * cruievent.h
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#ifndef CRUIEVENT_H_
#define CRUIEVENT_H_

#include <crengine.h>
#include "cruitheme.h"

class CRUIWidget;

namespace CRUI {
	enum {
		ACTION_DOWN = 1,
		ACTION_MOVE,
		ACTION_UP,
		ACTION_FOCUS_OUT,
		ACTION_FOCUS_IN,
		ACTION_CANCEL,
	};
};

class CRUIMotionEventItem {
	friend class CRUIEventManager;
	friend class CRUIEventAdapter;
	friend class CRUIMotionEvent;
	lUInt64 _pointerId;
	int _action;
	int _x;
	int _y;
	int _startX;
	int _startY;
	lUInt64 _ts;
	lUInt64 _downTs;
	bool _isOutside;
	CRUIWidget * _widget;

    struct TrackItem {
        lInt16 x;
        lInt16 y;
        lUInt32 ts; // millis, relative to downTs
        TrackItem(lInt16 _x, lInt16 _y, lUInt32 _ts) : x(_x), y(_y), ts(_ts) {}
        TrackItem() : x(0), y(0), ts(0) {}
    };
    LVArray<TrackItem> _track;

    void setWidget(CRUIWidget * widget) { _widget = widget; }
public:

    lvPoint getSpeed(int maxtime = 500);

	CRUIMotionEventItem(const CRUIMotionEventItem * previous, lUInt64 pointerId, int action, int x, int y, lUInt64 ts);
	int getX() const { return _x; }
	int getY() const { return _y; }
	int getStartX() const { return _startX; }
	int getStartY() const { return _startY; }
	lUInt64 getEventTimestamp() const { return _ts; }
	lUInt64 getDownEventTimestamp() const { return _downTs; }
	lUInt64 getDownDuration() const { return _ts - _downTs; }
	int getAction() const { return _action; }
	lUInt64 getPointerId() const { return _pointerId; }
	CRUIWidget * getWidget() const { return _widget; }
	bool isOutside() const { return _isOutside; }
};

class CRUIMotionEvent {
	friend class CRUIEventManager;
	friend class CRUIEventAdapter;
	LVPtrVector<CRUIMotionEventItem, false> _data;
	void addEvent(CRUIMotionEventItem * event) { _data.add(event); }
	void changeAction(int newAction) { _data[0]->_action = newAction; }
public:
    // use with caution
    void setWidget(CRUIWidget * widget) { _data[0]->setWidget(widget); }
    int count() const { return _data.length(); }
	const CRUIMotionEventItem * operator[] (int index) const { return index >= 0 && index<_data.length() ? _data[index] : NULL; }
	const CRUIMotionEventItem * get(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index] : NULL; }
	int getX(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getX() : 0; }
	int getY(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getY() : 0; }
	int getStartX(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getStartX() : 0; }
	int getStartY(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getStartY() : 0; }
	int getAction(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getAction() : 0; }
    lvPoint getSpeed(int maxtime = 500, int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getSpeed(maxtime) : lvPoint(0,0); }
    lUInt64 getEventTimestamp(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getEventTimestamp() : 0; }
    lUInt64 getDownEventTimestamp(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getDownEventTimestamp() : 0; }
    lUInt64 getDownDuration(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getDownDuration() : 0; }
	lUInt64 getPointerId(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getPointerId() : 0; }
	CRUIWidget * getWidget(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getWidget() : 0; }
	/// find pointer index by pointer id, returns -1 if not found
	int findPointerId(lUInt64 pointerId);
};

enum KEY_EVENT_TYPE {
    KEY_ACTION_PRESS,
    KEY_ACTION_RELEASE,
    KEY_ACTION_UNKNOWN
};

enum CR_KEY_MODIFIER {
    CR_KEY_MODIFIER_NONE =    0x00000000,
    CR_KEY_MODIFIER_SHIFT =   0x10000000,
    CR_KEY_MODIFIER_CONTROL = 0x20000000,
    CR_KEY_MODIFIER_ALT =     0x40000000,
    CR_KEY_MODIFIER_META =    0x80000000,
    CR_KEY_MODIFIER_KEYPAD =  0x08000000
};

#define CR_KEY_SPACE ' '
#define CR_KEY_0 '0'
#define CR_KEY_1 '1'
#define CR_KEY_2 '2'
#define CR_KEY_3 '0'
#define CR_KEY_4 '4'
#define CR_KEY_5 '5'
#define CR_KEY_6 '6'
#define CR_KEY_7 '7'
#define CR_KEY_8 '8'
#define CR_KEY_9 '9'

#define CR_KEY_A 'A'
#define CR_KEY_B 'B'
#define CR_KEY_C 'C'
#define CR_KEY_D 'D'
#define CR_KEY_E 'E'
#define CR_KEY_F 'F'
#define CR_KEY_G 'G'
#define CR_KEY_H 'H'
#define CR_KEY_I 'I'
#define CR_KEY_J 'J'
#define CR_KEY_K 'K'
#define CR_KEY_L 'L'
#define CR_KEY_M 'M'
#define CR_KEY_N 'N'
#define CR_KEY_O 'O'
#define CR_KEY_P 'P'
#define CR_KEY_Q 'Q'
#define CR_KEY_R 'R'
#define CR_KEY_S 'S'
#define CR_KEY_T 'T'
#define CR_KEY_U 'U'
#define CR_KEY_V 'V'
#define CR_KEY_W 'W'
#define CR_KEY_X 'X'
#define CR_KEY_Y 'Y'
#define CR_KEY_Z 'Z'

#define CR_KEY_SHIFT 0x101
#define CR_KEY_CONTROL 0x102
#define CR_KEY_ALT 0x103
#define CR_KEY_META 0x104

#define CR_KEY_HOME 0x110
#define CR_KEY_END 0x111
#define CR_KEY_PGUP 0x112
#define CR_KEY_PGDOWN 0x113
#define CR_KEY_LEFT 0x114
#define CR_KEY_RIGHT 0x115
#define CR_KEY_UP 0x116
#define CR_KEY_DOWN 0x117

#define CR_KEY_ESC 0x118

#define CR_KEY_DELETE 0x119
#define CR_KEY_BACKSPACE 0x11A

#define CR_KEY_RETURN 0x120

#define CR_KEY_BACK 0x130
#define CR_KEY_MENU 0x131


class CRUIKeyEvent {
    KEY_EVENT_TYPE _type;
    int _key;
    bool _autorepeat;
    int _count;
    int _modifiers;
    lUInt64  _ts;
    lUInt64  _downTs;
    CRUIWidget * _widget;
public:
    KEY_EVENT_TYPE getType() const { return _type; }
    int key() const { return _key; }
    int count() const { return _count; }
    bool isAutorepeat() const { return _autorepeat; }
    int modifiers() const { return _modifiers; }
    lUInt64 getEventTimestamp() const { return _ts; }
    lUInt64 getDownEventTimestamp() const { return _downTs; }
    lUInt64 getDownDuration() const { return _ts - _downTs; }
    CRUIWidget * getWidget() { return _widget; }
    void setWidget(CRUIWidget * widget) { _widget = widget; }
    void setDownEvent(const CRUIKeyEvent * v) {
        _downTs = v->_ts;
        _widget = v->_widget;
    }

    CRUIKeyEvent(const CRUIKeyEvent & v) : _type(v._type), _key(v._key), _autorepeat(v._autorepeat), _count(v._count), _modifiers(v._modifiers), _ts(v._ts), _downTs(v._downTs), _widget(v._widget) {}
    CRUIKeyEvent(KEY_EVENT_TYPE type, int key, bool autorepeat, int count, int modifiers) : _type(type), _key(key), _autorepeat(autorepeat), _count(count), _modifiers(modifiers), _widget(NULL) {
        _ts = GetCurrentTimeMillis();
        _downTs = _ts;
    }
    ~CRUIKeyEvent() {}
};

class CRUIMainWidget;
class CRUIEventManager {
protected:
	CRUIMainWidget * _rootWidget;
	CRUIMotionEventItem * _lastTouchEvent;
    LVHashTable<lUInt32, CRUIKeyEvent*> _keyDownEvents;
	bool dispatchTouchEvent(CRUIWidget * widget, CRUIMotionEvent * event);
    bool dispatchKeyEvent(CRUIWidget * widget, CRUIKeyEvent * event);
    void updateScreen();
public:
	CRUIEventManager();
	void setRootWidget(CRUIMainWidget * rootWidget) { _rootWidget = rootWidget; }
	bool dispatchTouchEvent(CRUIMotionEvent * event);
    bool dispatchKeyEvent(CRUIKeyEvent * event);
};

class CRUIOnTouchEventListener {
public:
	virtual bool onTouch(CRUIWidget * widget, const CRUIMotionEvent * event) = 0;
	virtual ~CRUIOnTouchEventListener() {}
};

class CRUIOnKeyEventListener {
public:
    virtual bool onKey(CRUIWidget * widget, const CRUIKeyEvent * event) = 0;
    virtual ~CRUIOnKeyEventListener() {}
};

class CRUIOnClickListener {
public:
	virtual bool onClick(CRUIWidget * widget) = 0;
	virtual ~CRUIOnClickListener() {}
};

class CRUIOnLongClickListener {
public:
	virtual bool onLongClick(CRUIWidget * widget) = 0;
	virtual ~CRUIOnLongClickListener() {}
};

class CRUIListWidget;
class CRUIOnListItemClickListener {
public:
	virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex) = 0;
	virtual ~CRUIOnListItemClickListener() {}
};

class CRUIOnListItemLongClickListener {
public:
	virtual bool onListItemLongClick(CRUIListWidget * widget, int itemIndex) = 0;
	virtual ~CRUIOnListItemLongClickListener() {}
};

class CRUISliderWidget;
class CRUIOnScrollPosCallback {
public:
    virtual bool onScrollPosChange(CRUISliderWidget * widget, int pos, bool manual) = 0;
    virtual ~CRUIOnScrollPosCallback() {}
};

#endif /* CRUIEVENT_H_ */
