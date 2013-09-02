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
	void setWidget(CRUIWidget * widget) { _data[0]->setWidget(widget); }
	void changeAction(int newAction) { _data[0]->_action = newAction; }
public:
	int count() const { return _data.length(); }
	const CRUIMotionEventItem * operator[] (int index) const { return index >= 0 && index<_data.length() ? _data[index] : NULL; }
	const CRUIMotionEventItem * get(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index] : NULL; }
	int getX(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getX() : 0; }
	int getY(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getY() : 0; }
	int getStartX(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getStartX() : 0; }
	int getStartY(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getStartY() : 0; }
	int getAction(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getAction() : 0; }
    lUInt64 getEventTimestamp(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getEventTimestamp() : 0; }
    lUInt64 getDownEventTimestamp(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getDownEventTimestamp() : 0; }
    lUInt64 getDownDuration(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getDownDuration() : 0; }
	lUInt64 getPointerId(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getPointerId() : 0; }
	CRUIWidget * getWidget(int index = 0) const { return index >= 0 && index<_data.length() ? _data[index]->getWidget() : 0; }
	/// find pointer index by pointer id, returns -1 if not found
	int findPointerId(int pointerId);
};

class CRUIEventManager {
protected:
	CRUIWidget * _rootWidget;
	CRUIMotionEventItem * _lastTouchEvent;
	bool dispatchTouchEvent(CRUIWidget * widget, CRUIMotionEvent * event);
public:
	CRUIEventManager();
	void setRootWidget(CRUIWidget * rootWidget) { _rootWidget = rootWidget; }
	bool dispatchTouchEvent(CRUIMotionEvent * event);
};

class CRUIOnTouchEventListener {
public:
	virtual bool onTouch(CRUIWidget * widget, const CRUIMotionEvent * event) = 0;
	virtual ~CRUIOnTouchEventListener() {}
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



#endif /* CRUIEVENT_H_ */
