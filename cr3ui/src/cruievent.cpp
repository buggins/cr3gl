/*
 * cruievent.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruievent.h"
#include "cruiwidget.h"

using namespace CRUI;


int CRUIMotionEvent::findPointerId(int pointerId) {
	for (int i=0; i<_data.length(); i++)
		if (_data[i]->getPointerId() == pointerId)
			return i;
	return -1;
}

CRUIMotionEventItem::CRUIMotionEventItem(const CRUIMotionEventItem * previous, lUInt64 pointerId, int action, int x, int y, lUInt64 ts)
: _pointerId(pointerId),
  _action(action),
  _x(x),
  _y(y),
  _startX(previous ? previous->getStartX() : x),
  _startY(previous ? previous->getStartY() : y),
  _ts(ts),
  _downTs(previous ? previous->getDownEventTimestamp() : ts),
  _isOutside(previous ? previous->isOutside() : false),
  _widget(previous ? previous->getWidget() : NULL)
{
	//
}


CRUIEventManager::CRUIEventManager() : _rootWidget(NULL), _lastTouchEvent(NULL) {

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
	if (!event->getWidget()) { // if not not assigned on widget
		for (int i=0; i<widget->getChildCount(); i++) {
			CRUIWidget * child = widget->getChild(i);
			if (dispatchTouchEvent(child, event)) {
				if (action == ACTION_DOWN) {
					//CRLog::trace("setting widget on DOWN");
					if (!event->getWidget())
						event->setWidget(child);
				}
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
	bool res = widget->onTouchEvent(event);
	if (res && action == ACTION_FOCUS_OUT) // if FOCUS_OUT returned true - stop tracking movements outside
		event->_data[0]->_isOutside = true;
	if (pointInside)
		event->_data[0]->_isOutside = false;
	return res;
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


