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

CRUIEventManager * CRUIEventManager::_instance = NULL;

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
    lUInt64 currentTs = GetCurrentTimeMillis();
    lUInt32 t0 = _track[0].ts;
    if (currentTs - t0 > 300)
        return lvPoint(0,0);
    for (int i = 0; i < _track.length() - 2; i++) {
        int dt = (int)(currentTs - _track[i].ts);
        //int dt = t0 - _track[i].ts;
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


CRUIWidget * CRUIEventManager::getFocusedWidget() {
    if (!_instance)
        return NULL;
    return _instance->_focusedWidget;
}

void CRUIEventManager::focusChanged(CRUIWidget * widget) {
    if (_focusedWidget == widget)
        return;
    if (_focusedWidget && _rootWidget && _rootWidget->isChild(_focusedWidget))
        _focusedWidget->onFocusChange(false);
    _focusedWidget = NULL;
    if (widget && _rootWidget && _rootWidget->isChild(widget)) {
        _focusedWidget = widget;
        widget->onFocusChange(true);
    }
}

void CRUIEventManager::showVirtualKeyboard(int mode, lString16 text, bool multiline) {
    if (_instance && _instance->_rootWidget)
        _instance->_rootWidget->showVirtualKeyboard(mode, text, multiline);
}

void CRUIEventManager::hideVirtualKeyboard() {
    if (_instance && _instance->_rootWidget)
        _instance->_rootWidget->hideVirtualKeyboard();
}

bool CRUIEventManager::isVirtualKeyboardShown() {
    if (_instance && _instance->_rootWidget)
        return _instance->_rootWidget->isVirtualKeyboardShown();
    return false;
}

bool CRUIEventManager::dispatchKey(CRUIKeyEvent * event) {
    if (_instance)
        return _instance->dispatchKeyEvent(event);
    return false;
}

void CRUIEventManager::requestScreenUpdate(bool force) {
    if (_instance && _instance->_rootWidget)
        _instance->_rootWidget->update(force);
}

void CRUIEventManager::dispatchFocusChange(CRUIWidget * widget) {
    if (!_instance)
        return;
    _instance->focusChanged(widget);
}

CRUIEventManager::~CRUIEventManager() {
    _instance = NULL;
}

CRUIEventManager::CRUIEventManager() : _rootWidget(NULL), _lastTouchEvent(NULL), _keyDownEvents(1024), _focusedWidget(NULL) {
    _instance = this;
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

CRUITimerItem::CRUITimerItem(lUInt32 _id, lUInt32 _intervalMillis, bool _repeat, CRUIWidget * _widget)
    : id(_id), widget(_widget)
{
    if (_repeat) {
        intervalMillis = _intervalMillis;
    } else {
        intervalMillis = 0;
    }
    nextTs = GetCurrentTimeMillis() + _intervalMillis;
}

void CRUITimerItem::update(lUInt32 _intervalMillis, bool _repeat, CRUIWidget * _widget) {
    widget = _widget;
    if (_repeat) {
        intervalMillis = _intervalMillis;
    } else {
        intervalMillis = 0;
    }
    nextTs = GetCurrentTimeMillis() + _intervalMillis;
}

bool CRUIEventManager::dispatchTimerEvent() {
    lUInt64 ts = GetCurrentTimeMillis();
    LVPtrVector<CRUITimerItem, false> reschedule;
    for (int i = _timers.length() - 1; i >= 0; i--) {
        CRUITimerItem * item = _timers[i];
        if (item->nextTs > ts)
            continue;
        _timers.remove(i);
        if (_rootWidget && _rootWidget->isChild(item->widget)) {
            if (!item->widget->onTimerEvent(item->id))
                item->intervalMillis = 0; // disable timer if handler returned false
            if (item->intervalMillis) {
                item->nextTs = ts + item->intervalMillis;
                reschedule.add(item);
            } else {
                delete item;
            }
        } else {
            // item with non-existing widget
            delete item;
        }
    }
    // place rescheduled events into queue, sorted
    for (int i = 0; i < reschedule.length(); i++) {
        updateTimerQueue(reschedule[i]);
    }
    if (_timers.length()) {
        // restart
        int interval = (int)(_timers[0]->nextTs - ts);
        if (interval < 0)
            interval = 0;
        startTimer(interval);
        return true;
    } else {
        concurrencyProvider->executeGui(NULL, 0);
        return false;
    }
}

class TimerHandler : public CRRunnable {
    CRUIEventManager * eventManager;
public:
    TimerHandler(CRUIEventManager * _eventManager) : eventManager(_eventManager) {}
    virtual void run() { eventManager->dispatchTimerEvent(); }
};

void CRUIEventManager::startTimer(lUInt32 interval) {
    concurrencyProvider->executeGui(new TimerHandler(this), interval);
}

void CRUIEventManager::updateTimerQueue(CRUITimerItem * item) {
    for (int i = 0; i < _timers.length(); i++) {
        if (_timers[i]->nextTs > item->nextTs) {
            _timers.insert(i, item);
            return;
        }
    }
    _timers.add(item);
}

void CRUIEventManager::updateTimerQueue(int index) {
    CRUITimerItem * item = _timers.remove(index);
    updateTimerQueue(item);
}

int CRUIEventManager::findTimer(lUInt32 timerId) {
    for (int i = 0; i < _timers.length(); i++) {
        if (_timers[i]->id == timerId)
            return i;
    }
    return -1;
}

void CRUIEventManager::setTimer(lUInt32 timerId, CRUIWidget * widget, lUInt32 interval, bool repeat) {
    if (!_instance)
        return;
    _instance->setTimerInternal(timerId, widget, interval, repeat);
}

void CRUIEventManager::setTimerInternal(lUInt32 timerId, CRUIWidget * widget, lUInt32 interval, bool repeat) {
    lUInt32 oldId = _timers.length() ? _timers[0]->id : 0;
    int index = findTimer(timerId);
    CRUITimerItem * timer = NULL;
    if (index >= 0) {
        // update properties of existing timer
        timer = _timers.remove(index);
        timer->update(interval, repeat, widget);
    } else {
        // create new timer
        timer = new CRUITimerItem(timerId, interval, repeat, widget);
    }
    updateTimerQueue(timer);
    if (_timers[0]->id != oldId) // changed
        startTimer((int)(_timers[0]->nextTs - GetCurrentTimeMillis()));
}

void CRUIEventManager::cancelTimer(lUInt32 timerId) {
    if (!_instance)
        return;
    _instance->cancelTimerInternal(timerId);
}

void CRUIEventManager::cancelTimerInternal(lUInt32 timerId) {
    int index = findTimer(timerId);
    if (index >= 0)
        delete _timers.remove(index);
    if (!_timers.length()) // cancel
        concurrencyProvider->executeGui(NULL, 0);
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

void CRUIEventManager::onSystemLanguageChanged() {
	_rootWidget->onSystemLanguageChanged();
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
		//CRLog::trace("Sending CANCEL event");
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
    	_rootWidget->update(false);
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
            	_rootWidget->update(false);
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
	_rootWidget->update(false);
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
    // focus support
    if (!widget && _focusedWidget && _rootWidget->isChild(_focusedWidget)) {
        if (dispatchKeyEvent(_focusedWidget, event))
            return true; // successfully dispatched to focused widget
    }
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


/// pass download result to window
void CRUIEventManager::dispatchDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream) {
    class DownloadMessage : public CRRunnable {
    public:
        CRUIMainWidget * _main;
        int _downloadTaskId;
        lString8 _url;
        int _result;
        lString8 _resultMessage;
        lString8 _mimeType;
        int _size;
        LVStreamRef _stream;
        DownloadMessage(CRUIMainWidget * main, int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream)
            : _main(main), _downloadTaskId(downloadTaskId), _url(url), _result(result), _resultMessage(resultMessage), _mimeType(mimeType), _size(size), _stream(stream)
        {

        }
        void run() {
            _main->onDownloadResult(_downloadTaskId, _url, _result, _resultMessage, _mimeType, _size, _stream);
        }

    };
    DownloadMessage * msg = new DownloadMessage(_rootWidget, downloadTaskId, url, result, resultMessage, mimeType, size, stream);
    concurrencyProvider->executeGui(msg);
}

/// download progress
void CRUIEventManager::dispatchDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded) {
    class ProgressMessage : public CRRunnable {
    public:
        CRUIMainWidget * _main;
        int _downloadTaskId;
        lString8 _url;
        int _result;
        lString8 _resultMessage;
        lString8 _mimeType;
        int _size;
        int _sizeDownloaded;
        ProgressMessage(CRUIMainWidget * main, int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded)
            : _main(main), _downloadTaskId(downloadTaskId), _url(url), _result(result), _resultMessage(resultMessage), _mimeType(mimeType), _size(size), _sizeDownloaded(sizeDownloaded)
        {

        }
        void run() {
            _main->onDownloadProgress(_downloadTaskId, _url, _result, _resultMessage, _mimeType, _size, _sizeDownloaded);
        }

    };
    ProgressMessage * msg = new ProgressMessage(_rootWidget, downloadTaskId, url, result, resultMessage, mimeType, size, sizeDownloaded);
    concurrencyProvider->executeGui(msg);
}


int CRUIHttpTaskManagerBase::generateTaskId() {
    CRGuard guard(_lock); CR_UNUSED(guard);
    return ++_nextTaskId;
}

CRUIHttpTaskManagerBase::CRUIHttpTaskManagerBase(CRUIEventManager * eventManager, int threadCount) : _eventManager(eventManager), _executor(), _activeTasks(100), _nextTaskId(1) {
    CR_UNUSED(threadCount);
    _lock = concurrencyProvider->createMutex();
}

/// override to create task of custom type
CRUIHttpTaskBase * CRUIHttpTaskManagerBase::createTask() {
    CRUIHttpTaskBase * task = new CRUIHttpTaskBase(this);
    return task;
}

/// returns 0 if not supported, task ID if download task is started
int CRUIHttpTaskManagerBase::openUrl(lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs) {
    int taskId = generateTaskId();
    CRUIHttpTaskBase * task = createTask();
    task->init(taskId, url, method, login, password, saveAs);
    CRGuard guard(_lock); CR_UNUSED(guard);
    _activeTasks.set(taskId, task);
    executeTask(task);
    return taskId;
}

/// cancel specified download task
void CRUIHttpTaskManagerBase::cancelDownload(int downloadTaskId) {
    CRGuard guard(_lock); CR_UNUSED(guard);
    CRUIHttpTaskBase * task = _activeTasks.get(downloadTaskId);
    if (task) {
        task->_cancelled = true;
        _activeTasks.remove(task->_downloadTaskId);
        //delete task;
    }
}

/// call to pass received data to output buffer
void CRUIHttpTaskBase::dataReceived(const lUInt8 * data, int len) {
    // TODO: accept data
    if (_stream.isNull()) {
        if (_saveAs.empty()) {
            // create memory stream
            _stream = LVCreateMemoryStream();
        } else {
            // create file stream
        }
    }
    CRLog::trace("dataReceived(%d)", len);
    if (!_stream.isNull()) {
        lvsize_t bytesWritten = 0;
        _stream->Write(data, len, &bytesWritten);
    }
}

void CRUIHttpTaskBase::run() {
    _taskManager->executeTask(this);
}

/// post task to queue
void CRUIHttpTaskManagerBase::postTask(CRUIHttpTaskBase * task) {
    _executor.execute(task);
}

/// override to do actual download synchronously - will be called in separate thread
void CRUIHttpTaskManagerBase::executeTask(CRUIHttpTaskBase * task) {
    task->doDownload();
}

void CRUIHttpTaskManagerBase::onTaskProgress(CRUIHttpTaskBase * task) {
    if (!task->_cancelled) {
        _eventManager->dispatchDownloadProgress(task->_downloadTaskId, task->_url, task->_result, task->_resultMessage, task->_mimeType, task->_size, task->_sizeDownloaded);
    }
}

void CRUIHttpTaskManagerBase::onTaskFinished(CRUIHttpTaskBase * task) {
    if (!task->_cancelled) {
        _eventManager->dispatchDownloadResult(task->_downloadTaskId, task->_url, task->_result, task->_resultMessage, task->_mimeType, task->_size, task->_stream);
    }
    CRGuard guard(_lock); CR_UNUSED(guard);
    // remove task from list
    _activeTasks.remove(task->_downloadTaskId);
}
