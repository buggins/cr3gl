/*
 * cruievent.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruievent.h"
#include "cruiwidget.h"
#include "cruimain.h"

using namespace CRUI;


int CRUIMotionEvent::findPointerId(lUInt64 pointerId) {
	for (int i=0; i<_data.length(); i++)
		if (_data[i]->getPointerId() == pointerId)
			return i;
	return -1;
}

/// set cancelled event flag to all pointers
void CRUIMotionEvent::cancelAllPointers() const {
	for (int i = 0; i < _data.length(); i++) {
		_data[i]->cancel();
	}
}

/// returns true if cancel is requested for any of pointers
bool CRUIMotionEvent::isCancelRequested() const {
	for (int i = 0; i < _data.length(); i++) {
		if (_data[i]->isCancelRequested())
			return true;
	}
	return false;
}

/// create cancel processing event for cancelled pointers
CRUIMotionEvent * CRUIMotionEvent::createCancelEvent() const {
	if (!isCancelRequested())
		return NULL;
	CRUIMotionEvent * res = new CRUIMotionEvent();
	for (int i = 0; i <_data.length(); i++) {
		if (_data[i]->isCancelRequested()) {
			_data[i]->_cancelled = true;
			_data[i]->_cancelRequested = false;
			_data[i]->_action = ACTION_CANCEL;
			res->_data.add(_data[i]);
		}
	}
	return res;
}

int CRUIMotionEvent::getPinchDx() const {
	if (count() != 2)
		return 0;
	int dx1 = myAbs(getStartX(0) - getStartX(1));
	int dx2 = myAbs(getX(0) - getX(1));
	return dx2 - dx1;
}

int CRUIMotionEvent::getPinchDy() const {
	if (count() != 2)
		return 0;
	int dy1 = myAbs(getStartY(0) - getStartY(1));
	int dy2 = myAbs(getY(0) - getY(1));
	return dy2 - dy1;
}

int CRUIMotionEvent::getAvgStartX() const {
	int s = 0;
	for (int i = 0; i < _data.length(); i++)
		s += _data[i]->getStartX();
	return s / _data.length();
}

int CRUIMotionEvent::getAvgStartY() const {
	int s = 0;
	for (int i = 0; i < _data.length(); i++)
		s += _data[i]->getStartY();
	return s / _data.length();
}

int CRUIMotionEvent::getAvgX() const {
	int s = 0;
	for (int i = 0; i < _data.length(); i++)
		s += _data[i]->getX();
	return s / _data.length();
}

int CRUIMotionEvent::getAvgY() const {
	int s = 0;
	for (int i = 0; i < _data.length(); i++)
		s += _data[i]->getY();
	return s / _data.length();
}

/// returns average delta X for multitouch event
int CRUIMotionEvent::getAvgDeltaX() const {
	int s = 0;
	for (int i = 0; i < _data.length(); i++)
		s += _data[i]->getDeltaX();
	return s / _data.length();
}
/// returns average delta Y for multitouch event
int CRUIMotionEvent::getAvgDeltaY() const {
	int s = 0;
	for (int i = 0; i < _data.length(); i++)
		s += _data[i]->getDeltaY();
	return s / _data.length();
}

void CRUIMotionEvent::setWidget(CRUIWidget * widget) {
	for (int i = 0; i < _data.length(); i++) {
		_data[i]->setWidget(widget);
	}
}

#define MAX_TRACK_MILLIS 2000

CRUIMotionEventItem::CRUIMotionEventItem(const CRUIMotionEventItem * previous, lUInt64 pointerId, int action, int x, int y, lUInt64 ts)
: _pointerId(pointerId),
  _action(action),
  _x(x),
  _y(y),
  _startX(previous ? previous->getStartX() : x),
  _startY(previous ? previous->getStartY() : y),
  _ts(ts),
  _downTs(action == ACTION_DOWN ? ts : (previous ? previous->getDownEventTimestamp() : ts)),
  _isOutside(previous ? previous->isOutside() : false),
  _widget(previous ? previous->getWidget() : NULL),
  _cancelRequested(previous ? previous->isCancelRequested() : false),
  _cancelled(previous ? previous->isCancelled() : false)
{
	//
    //CRLog::trace("created event ts=%lld downts=%lld", _ts, _downTs);

    if (previous)
        _track.reserve(previous->_track.length() + 1);
    lUInt32 trackTs = (lUInt32)(ts - _downTs);
    lUInt32 minTs = trackTs - MAX_TRACK_MILLIS;
    TrackItem t(x, y, trackTs);
    _track.add(t);
    if (previous) {
        for (int i = 0; i<previous->_track.length(); i++) {
            if (previous->_track[i].ts < minTs)
                _track.add(previous->_track[i]);
        }
    }
}

lvPoint CRUIMotionEventItem::getSpeed(int maxtime) {
    if (_track.length() < 2)
        return lvPoint(0,0);
    int vx0 = _track[0].x - _track[1].x;
    int vy0 = _track[0].y - _track[1].y;
    lInt64 svx = 0;
    lInt64 svy = 0;
    lInt64 sw = 0;
    lUInt32 t0 = _track[0].ts;
    for (int i = 0; i < _track.length() - 2; i++) {
        int dt = t0 - _track[i].ts;
        if (dt > maxtime)
            break;
        int t = _track[i].ts - _track[i + 1].ts;
        int dx = _track[i].x - _track[i + 1].x;
        int dy = _track[i].y - _track[i + 1].y;
        int vx = t > 0 ? dx * 1000 / t : 0;
        int vy = t > 0 ? dy * 1000 / t : 0;
        int p = vx * vx0 + vy * vy0;
        if (p < 0)
            break;
        lInt64 w = ((lInt64)t) * 1000000/(1000 + (dt * 1000 / maxtime) * 10);
        svx += vx * w;
        svy += vy * w;
        sw += w;
    }
    if (sw <= 0)
        return lvPoint(0, 0);
    return lvPoint((int)(svx / sw), (int)(svy / sw));
}


CRUIEventManager::CRUIEventManager() : _rootWidget(NULL), _lastTouchEvent(NULL), _keyDownEvents(1024) {

}

bool CRUIEventManager::dispatchTouchEvent(CRUIWidget * widget, CRUIMotionEvent * event) {
	if (!widget)
		return false; // invalid widget
	if (!event->count()) // invalid event
		return false;
	bool pointInside = widget->isPointInside(event->getX(), event->getY());
	if (!pointInside && !event->getWidget())
		return false;
    int action = event->getAction();
    if (widget->getVisibility() == VISIBLE && _rootWidget->onTouchEventPreProcess(event)) {
        if (action == ACTION_DOWN) {
            //CRLog::trace("setting widget on DOWN");
            if (!event->getWidget())
                event->setWidget(widget);
        }
        updateScreen();
        return true;
    }
	if (!event->getWidget()) { // if not not assigned on widget
        for (int i=widget->getChildCount() - 1; i >= 0; i--) {
			CRUIWidget * child = widget->getChild(i);
            if (child->getVisibility() == VISIBLE && dispatchTouchEvent(child, event)) {
				if (action == ACTION_DOWN) {
					//CRLog::trace("setting widget on DOWN");
					if (!event->getWidget())
						event->setWidget(child);
				}
		        updateScreen();
				return true;
			}
		}
	}
	bool oldIsOutside = event->get()->isOutside();
	//CRLog::trace("Old pointInside=%s  new pointInside=%s", (!oldIsOutside ? "yes" : "no"), (pointInside ? "yes" : "no"));
	if (action == ACTION_UP && !pointInside) {
		// if UP is outside of control - change to CANCEL
		event->changeAction(ACTION_CANCEL);
		action = ACTION_CANCEL;
	}
	if (oldIsOutside != !pointInside) {
		// in/out
		if (!pointInside && action == ACTION_MOVE) {// moving outside - already sent FOCUS_OUT, now just ignore
			// converting to FOCUS_OUT
			event->changeAction(ACTION_FOCUS_OUT);
			action = ACTION_FOCUS_OUT;
		}
		if (pointInside && action == ACTION_MOVE) {
			/// point is back in - send FOCUS IN
			event->changeAction(ACTION_FOCUS_IN);
			action = ACTION_FOCUS_IN;
		}
	} else {
		if (!pointInside && action == ACTION_MOVE) { // moving outside - already sent FOCUS_OUT with response true, no tracking, now just ignore
			//CRLog::trace("MOVE outside of bounds - ignoring");
			return false;
		}
	}
	//CRLog::trace("calling widget->onTouchEvent");
    bool res = widget->getVisibility() == VISIBLE && widget->onTouchEvent(event);
	if (res && action == ACTION_FOCUS_OUT) // if FOCUS_OUT returned true - stop tracking movements outside
		event->_data[0]->_isOutside = true;
	if (pointInside)
		event->_data[0]->_isOutside = false;
    if (res && action == ACTION_DOWN) {
        //CRLog::trace("setting widget on DOWN");
        if (!event->getWidget())
            event->setWidget(widget);
    }
    if (res)
    	updateScreen();
    return res;
}

void CRUIEventManager::updateScreen() {
    _rootWidget->update(false);
}

bool CRUIEventManager::interceptTouchEvent(const CRUIMotionEvent * event, CRUIWidget * widget) {
    //(const_cast<CRUIMotionEvent *>(event))->setWidget(widget);
	if (!_rootWidget || !_rootWidget->isChild(widget))
		return false;
	// send fake cancel event to all widgets already tracking this event
	LVPtrVector<CRUIMotionEventItem> itemsToCancel;
	for (int i = 0; i < event->count(); i++) {
		if (event->getWidget(i) && event->getWidget(i) != widget && !event->get(i)->isCancelled() && !event->get(i)->isCancelled()) {
			const CRUIMotionEventItem * olditem = event->get(i);
			itemsToCancel.add(new CRUIMotionEventItem(olditem, olditem->getPointerId(), ACTION_CANCEL, olditem->getX(), olditem->getY(), olditem->getEventTimestamp()));
		}
	}
	if (itemsToCancel.length()) {
		CRLog::trace("Event items to cancel on intercept: %d", itemsToCancel.length());
		CRUIMotionEvent * cancelEvent = new CRUIMotionEvent();
		for (int i = 0; i < itemsToCancel.length(); i++) {
			cancelEvent->addEvent(itemsToCancel[i]);
		}
		dispatchTouchEvent(cancelEvent);
		delete cancelEvent;
	}
	// override widget in event
	for (int i = 0; i < event->count(); i++) {
	    (const_cast<CRUIMotionEventItem *>(event->get(i)))->setWidget(widget);
	}
	return true;
}

void CRUIEventManager::setRootWidget(CRUIMainWidget * rootWidget) {
	_rootWidget = rootWidget;
	_rootWidget->setEventManager(this);
}

bool CRUIEventManager::dispatchTouchEvent(CRUIMotionEvent * event) {
	if (_rootWidget == NULL) {
		CRLog::error("Cannot dispatch touch event: no root widget");
		return false;
	}
	//CRLog::trace("Touch event %d (%d,%d) %s", event->getAction(), event->getX(), event->getY(), (event->getWidget() ? "[widget]" : ""));
	bool res = false;
	CRUIWidget * widget = event->getWidget();
	if (widget) {
		// event is tracked by widget
		if (!_rootWidget->isChild(widget)) {
			CRLog::trace("Widget is not a child of root - skipping event");
			res = false;
		} else {
			//CRLog::trace("Dispatching event directly to widget");
			res = dispatchTouchEvent(widget, event);
		}
	} else if (event->getAction() != ACTION_DOWN) { // skip non tracked event - only DOWN allowed
		CRLog::trace("Skipping non-down event %d without widget", event->getAction());
		res = false;
	} else {
		//CRLog::trace("No widget: dispatching using tree");
		res = dispatchTouchEvent(_rootWidget, event);
	}
	if (event->isCancelRequested()) {
		CRLog::trace("Sending CANCEL event");
		CRUIMotionEvent * cancelEvent = event->createCancelEvent();
		dispatchTouchEvent(cancelEvent);
		delete cancelEvent;
	}
	_rootWidget->update(false);
	return res;
}

bool CRUIEventManager::dispatchKeyEvent(CRUIWidget * widget, CRUIKeyEvent * event) {
    if (!widget)
        return false; // invalid widget
    KEY_EVENT_TYPE action = event->getType();
    if (widget->onKeyEventPreProcess(event)) {
        if (action == KEY_ACTION_PRESS) {
            //CRLog::trace("setting widget on DOWN");
            if (!event->getWidget())
                event->setWidget(widget);
        }
        return true;
    }
    if (!event->getWidget()) { // if not not assigned on widget
        for (int i = widget->getChildCount() - 1; i >= 0; i--) {
            CRUIWidget * child = widget->getChild(i);
            if (dispatchKeyEvent(child, event)) {
                if (action == KEY_ACTION_PRESS) {
                    //CRLog::trace("setting widget on DOWN");
                    if (!event->getWidget())
                        event->setWidget(child);
                }
                return true;
            }
        }
    }

    bool res = widget->onKeyEvent(event);
    if (res && action == KEY_ACTION_PRESS) {
        //CRLog::trace("setting widget on DOWN");
        if (!event->getWidget())
            event->setWidget(widget);
    }
    return res;
}

bool CRUIEventManager::dispatchKeyEvent(CRUIKeyEvent * event) {
    CRUIKeyEvent * downevent = _keyDownEvents.get(event->key());
    if (event->getType() == KEY_ACTION_RELEASE) {
        if (downevent) {
            _keyDownEvents.remove(event->key());
            event->setDownEvent(downevent);
            delete downevent;
            downevent = NULL;
        }
    } else {
        if (!event->isAutorepeat()) {
            if (downevent) {
                _keyDownEvents.remove(event->key());
                delete downevent;
            }
            downevent = new CRUIKeyEvent(*event);
            _keyDownEvents.set(event->key(), downevent);
        } else {
            if (downevent) {
                event->setDownEvent(downevent);
                downevent = NULL;
            }
        }
    }
    if (_rootWidget == NULL) {
        CRLog::error("Cannot dispatch touch event: no root widget");
        return false;
    }

    //CRLog::trace("Touch event %d (%d,%d) %s", event->getAction(), event->getX(), event->getY(), (event->getWidget() ? "[widget]" : ""));
    CRUIWidget * widget = event->getWidget();
    if (widget) {
        // event is tracked by widget
        if (!_rootWidget->isChild(widget)) {
            CRLog::trace("Widget is not a child of root - skipping event");
            return false;
        }
        //CRLog::trace("Dispatching event directly to widget");
        return dispatchKeyEvent(widget, event);
    }
    if (event->getType() != KEY_ACTION_PRESS) { // skip non tracked event - only DOWN allowed
        CRLog::trace("Skipping non-down key event without widget");
        return false;
    }
    //CRLog::trace("No widget: dispatching using tree");
    bool res = dispatchKeyEvent(_rootWidget, event);
    if (downevent) {
        downevent->setWidget(event->getWidget());
    }
    return res;
}

