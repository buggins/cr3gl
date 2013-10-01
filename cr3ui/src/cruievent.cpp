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
  _widget(previous ? previous->getWidget() : NULL)
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
    if (widget->getVisibility() == VISIBLE && widget->onTouchEventPreProcess(event)) {
        if (action == ACTION_DOWN) {
            //CRLog::trace("setting widget on DOWN");
            if (!event->getWidget())
                event->setWidget(widget);
        }
        updateScreen();
        return true;
    }
	if (!event->getWidget()) { // if not not assigned on widget
		for (int i=0; i<widget->getChildCount(); i++) {
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
	_rootWidget->update();
}

bool CRUIEventManager::dispatchTouchEvent(CRUIMotionEvent * event) {
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
		return dispatchTouchEvent(widget, event);
	}
	if (event->getAction() != ACTION_DOWN) { // skip non tracked event - only DOWN allowed
		CRLog::trace("Skipping non-down event without widget");
		return false;
	}
	//CRLog::trace("No widget: dispatching using tree");
	return dispatchTouchEvent(_rootWidget, event);
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
        for (int i=0; i<widget->getChildCount(); i++) {
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

