#include "cruimain.h"

using namespace CRUI;

#include "gldrawbuf.h"
#include "crcoverpages.h"

#define WINDOW_ANIMATION_DELAY 250
#define SLOW_OPERATION_POPUP_DELAY 150
#define SLOW_OPERATION_POPUP_DIMMING_DURATION 1000
#define SLOW_OPERATION_DIM_COLOR 0x80404040

void applyThemeChange(CRUIWidget * widget) {
    if (!widget)
        return;
    widget->onThemeChanged();
    for (int i = 0; i < widget->getChildCount(); i++)
        applyThemeChange(widget->getChild(i));
}

void CRUIMainWidget::onThemeChanged()
{
    if (!_history.length()) {
        _home = new CRUIHomeWidget(this);
        _read = new CRUIReadWidget(this);
        _history.add(new HomeItem(this, _home));
        return;
    } else {
        applyThemeChange(_home);
        applyThemeChange(_read);
    }

    for (int i = 0; i < _history.length(); i++) {
        CRUIWidget * widget = _history[i]->getWidget();
        applyThemeChange(widget);
    }
    requestLayout();
}

/// returns true if widget is child of this
bool CRUIMainWidget::isChild(CRUIWidget * widget) {
    return widget == this || _history.currentWidget()->isChild(widget);
}

void CRUIMainWidget::showHome() {
    startAnimation(0, WINDOW_ANIMATION_DELAY);
}

void CRUIMainWidget::back() {
    if (_history.hasBack()) {
        startAnimation(_history.pos() - 1, WINDOW_ANIMATION_DELAY);
    }
}

class CoverpagesReadyCallback : public CRRunnable {
    CRUIMainWidget * _main;
    int _newpos;
public:
    CoverpagesReadyCallback(CRUIMainWidget * main, int newpos) : _main(main), _newpos(newpos) {}
    virtual void run() {
        _main->onAllCoverpagesReady(_newpos);
    }
};

void CRUIMainWidget::onAllCoverpagesReady(int newpos) {
    if (_history.next() && _history.next()->getMode() == MODE_FOLDER) {
        // animating move to folder view
        hideSlowOperationPopup();
        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
    }
}

void CRUIMainWidget::onDirectoryScanFinished(CRDirContentItem * item) {
    if (item->getDirType() == DIR_TYPE_RECENT) {
        // set recent book
        if (item->itemCount() > 0) {
            //_home->setLastBook(item->getItem(0));
        }
        update();
        return;
    }
    if (_pendingFolder == item->getPathName()) {
        item->sort(CRUI::BY_TITLE);
        int newpos = _history.findPosByMode(MODE_FOLDER, _pendingFolder);
        if (newpos < 0) {
            _history.setNext(new FolderItem(this, _pendingFolder));
            newpos = _history.pos() + 1;
        }
        CRLog::info("Directory %s is ready", item->getPathName().c_str());
        _pendingFolder.clear();
        CRUIFolderWidget * folder = static_cast<CRUIFolderWidget *>(_history[newpos]->getWidget());
        folder->measure(_pos.width(), _pos.height());
        folder->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
        if (folder && !folder->requestAllVisibleCoverpages()) {
            // initiate wait until all coverpages ready
            coverPageManager->setAllTasksFinishedCallback(new CoverpagesReadyCallback(this, newpos));
        } else {
            hideSlowOperationPopup();
            startAnimation(newpos, WINDOW_ANIMATION_DELAY);
        }
    } else {
        update();
    }
}

void CRUIMainWidget::onDocumentLoadFinished(lString8 pathname, bool success) {
    //
    CRLog::info("Document loaded: %s %s", pathname.c_str(), (success ? "successfully" : "with error"));

    BookDBBook * book = success ? bookDB->loadBook(pathname) : NULL;

    if (book) {
        CRFileItem * entry = dirCache->scanFile(pathname);
        //_home->setLastBook(entry);
        delete entry;
    } else {
        //_home->setLastBook(NULL);
    }
}

void CRUIMainWidget::onDocumentRenderFinished(lString8 pathname) {
    CRLog::info("Document rendered: %s", pathname.c_str());
    hideSlowOperationPopup();
    if (_history.current()->getMode() != MODE_READ) {
        int newpos = _history.findPosByMode(MODE_READ);
        if (newpos < 0) {
            _history.setNext(new ReadItem(this, _read));
            newpos = _history.pos() + 1;
        }
        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
    } else {
        update();
    }
}

void CRUIMainWidget::showSlowOperationPopup()
{
	if (_popup) {
		CRLog::error("showSlowOperationPopup() called twice");
		return;
	}
	CRLog::trace("CRUIMainWidget::showSlowOperationPopup()");
#if 0
    CRUITextWidget * pleaseWait = new CRUITextWidget(lString16("Please wait"));
    pleaseWait->setBackground(0xFFFFFF);
    pleaseWait->setPadding(PT_TO_PX(7));
    pleaseWait->setAlign(ALIGN_CENTER);
    pleaseWait->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
#else
    CRUISpinnerWidget * pleaseWait = new CRUISpinnerWidget("spinner_white_48", 360 + 180);
    //CRUISpinnerWidget * pleaseWait = new CRUISpinnerWidget("ic_menu_back");
    pleaseWait->setPadding(PT_TO_PX(7));
    pleaseWait->setAlign(ALIGN_CENTER);
    pleaseWait->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
#endif
    GLDrawBuf * buf = new GLDrawBuf(_pos.width(), _pos.height(), 32, true);
    buf->beforeDrawing();
    _history.currentWidget()->draw(buf);
    buf->afterDrawing();
    _popupBackground = buf;
    _popup = new CRUIPopupWindow(pleaseWait, SLOW_OPERATION_POPUP_DELAY, SLOW_OPERATION_POPUP_DIMMING_DURATION, SLOW_OPERATION_DIM_COLOR);
    _popup->measure(_pos.width(), _pos.height());
    _popup->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    update();
}

void CRUIMainWidget::hideSlowOperationPopup()
{
	if (!_popup) {
		CRLog::error("hideSlowOperationPopup() called twice");
		return;
	}
    if (_popup) {
        delete _popup;
        _popup = NULL;
    }
    if (_popupBackground) {
        delete _popupBackground;
        _popupBackground = NULL;
    }
}

void CRUIMainWidget::showFolder(lString8 folder, bool appendHistory) {
    CR_UNUSED(appendHistory);
   //if ((_currentFolder != folder && _pendingFolder != folder) || _mode != MODE_FOLDER) {
    _pendingFolder = folder;
    int newpos = _history.findPosByMode(MODE_FOLDER, folder);
    if (newpos < 0) {
        // create page now, to lock corresponding folder
        _history.setNext(new FolderItem(this, folder));
    }
    showSlowOperationPopup();
    CRLog::info("Starting background directory scan for %s", folder.c_str());
    dirCache->scan(folder);

//    int newpos = _history.findPosByMode(MODE_FOLDER, folder);
//    if (newpos < 0) {
//        showSlowOperationPopup();
//        _history.setNext(new FolderItem(this, folder));
//        //_popup->setBackground(0xC0000000); // dimming
//        CRLog::info("Starting background directory scan for %s", folder.c_str());
//        dirCache->scan(folder);
//    } else {
//        // found existing
//        // do nothing
//        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
//    }
}

void CRUIMainWidget::openBook(const CRFileItem * file) {
    if (!file)
        return;
    CRLog::debug("Opening book %s", file->getPathName().c_str());
    if (_animation.active) {
        CRLog::debug("Animation is active. Stopping.");
        stopAnimation();
    }
    _read->measure(_measuredWidth, _measuredHeight);
    _read->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    _read->openBook(file);
}

void CRUIMainWidget::runStartupTasksIfNeeded() {
    if (_initialized)
        return;
    _initialized = true;
    dirCache->setDefaultCallback(this);
    dirCache->scan(lString8(RECENT_DIR_TAG));
}

CRUIMainWidget::CRUIMainWidget()
: _home(NULL), _read(NULL)
, _popup(NULL), _popupBackground(NULL),    _screenUpdater(NULL)
, _platform(NULL), _lastAnimationTs(0), _initialized(false)
{
    onThemeChanged();
}

CRUIMainWidget::~CRUIMainWidget() {
    if (_home)
        delete _home;
    if (_read)
        delete _read;
}

void CRUIMainWidget::clearImageCaches() {
	CRLog::info("CRUIMainWidget::clearImageCaches()");
	_read->clearImageCaches();
	if (_popup)
		hideSlowOperationPopup();
}

int CRUIMainWidget::getChildCount() {
    int cnt = 0;
    if (_history.currentWidget())
        cnt++;
    if (_popup)
        cnt++;
    return cnt; //_currentWidget->getChildCount();
}

CRUIWidget * CRUIMainWidget::getChild(int index) {
    if (_popup && index == 0)
        return _popup;
    return _history.currentWidget();
    //return _currentWidget->getChild(index);
}

/// measure dimensions
void CRUIMainWidget::measure(int baseWidth, int baseHeight) {
    if (_popupBackground)
        return;
    _history.currentWidget()->measure(baseWidth, baseHeight);
    _measuredWidth = _history.currentWidget()->getMeasuredWidth();
    _measuredHeight = _history.currentWidget()->getMeasuredHeight();
    if (_popup)
        _popup->measure(baseWidth, baseHeight);
}

/// updates widget position based on specified rectangle
void CRUIMainWidget::layout(int left, int top, int right, int bottom) {
    if (_popupBackground)
        return;
    _history.currentWidget()->layout(left, top, right, bottom);
    _pos.left = left;
    _pos.top = top;
    _pos.right = right;
    _pos.bottom = bottom;
    if (_popup)
        _popup->layout(left, top, right, bottom);
}

/// draw now
void CRUIMainWidget::update() {
    bool needLayout, needDraw, animating;
    CRUICheckUpdateOptions(this, needLayout, needDraw, animating);
    setScreenUpdateMode(true, animating ? 30 : 0);
}

void CRUIMainWidget::startAnimation(int newpos, int duration, const CRUIMotionEvent * event) {
    if (_animation.active) {
        stopAnimation();
    }
    int oldpos = _history.pos();
    if (newpos == oldpos)
        return;
    CRUIWidget * newWidget = _history[newpos]->getWidget();
    CRUIWidget * oldWidget = _history[oldpos]->getWidget();
    if (!newWidget || !oldWidget)
        return;
    bool manual = event != NULL;
    int direction;
    if (oldpos < newpos)
        direction = -1;
    else
        direction = +1;
    CRLog::trace("starting animation %d -> %d", oldpos, newpos);
    CRReinitTimer();
    _animation.active = true;
    _animation.oldpos = oldpos;
    _animation.newpos = newpos;
    _animation.duration = duration * 10;
    _animation.direction = direction;
    _animation.progress = 0;
    _animation.manual = manual;
    if (event) {
        // manual
        (const_cast<CRUIMotionEvent *>(event))->setWidget(this);
        _animation.startPoint.x = event->getStartX();
        _animation.startPoint.y = event->getStartY();
    }

    oldWidget->measure(_pos.width(), _pos.height());
    oldWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    if (_history[oldpos]->getMode() != MODE_READ) {
        _animation.oldimage = new GLDrawBuf(_pos.width(), _pos.height(), 32, true);
        _animation.oldimage->beforeDrawing();
        oldWidget->draw(_animation.oldimage);
        _animation.oldimage->afterDrawing();
    }
    newWidget->measure(_pos.width(), _pos.height());
    newWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    if (_history[newpos]->getMode() != MODE_READ) {
        _animation.newimage = new GLDrawBuf(_pos.width(), _pos.height(), 32, true);
        _animation.newimage->beforeDrawing();
        newWidget->draw(_animation.newimage);
        _animation.newimage->afterDrawing();
    }


    if (duration == 0) {
        stopAnimation();
    } else {
        requestLayout();
        update();
    }
}

void CRUIMainWidget::stopAnimation() {
    if (!_animation.active)
        return;
    CRLog::trace("stopping animation %d, %d", _animation.oldpos, _animation.newpos);
    _animation.active = false;
    CRUIWidget * newWidget = _history[_animation.newpos]->getWidget();
    CRUIWidget * oldWidget = _history[_animation.oldpos]->getWidget();
    oldWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    newWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    if (_animation.oldimage) {
        delete _animation.oldimage;
        _animation.oldimage = NULL;
    }
    if (_animation.newimage) {
        delete _animation.newimage;
        _animation.newimage = NULL;
    }
    _history.setPos(_animation.newpos);
    requestLayout();
    update();
}

void CRUIMainWidget::animate(lUInt64 millisPassed) {
    CRUIWidget::animate(millisPassed);
    if (_animation.active) {
        if (!_animation.manual) {
            _animation.progress += (int)millisPassed * 10;
        }
        int p = _animation.progress;
        //CRLog::trace("animating ts = %lld,  passed = %d   %d of %d", (lUInt64)GetCurrentTimeMillis(), (int)millisPassed, (int)p, (int)_animation.duration);
        if (p > _animation.duration) {
            stopAnimation();
        } else {
            int x = _pos.width() * p / _animation.duration;
            //CRLog::trace("animation position %d", x);
            if (_animation.direction < 0) {
                _animation.oldimagex = _pos.left - x;
                _animation.newimagex = _pos.right - x;
            } else {
                _animation.oldimagex = _pos.left + x;
                _animation.newimagex = _pos.left - _pos.width() + x;
            }
        }
    }
}

class MainWidgetUpdateCallback : public CRRunnable {
    CRUIMainWidget * _main;
public:
    MainWidgetUpdateCallback(CRUIMainWidget * main) : _main(main) {}
    virtual void run() {
        _main->requestLayout();
        _main->update();
    }
};

CRRunnable * CRUIMainWidget::createUpdateCallback() {
    return new MainWidgetUpdateCallback(this);
}

bool CRUIMainWidget::isAnimating() {
    return _animation.active;
}


/// draws widget with its children to specified surface
void CRUIMainWidget::draw(LVDrawBuf * buf) {
    if (!_initialized)
        runStartupTasksIfNeeded();

    bool needLayout, needDraw, animating;
    CRUICheckUpdateOptions(this, needLayout, needDraw, animating);
    if (animating) {
        lUInt64 ts = GetCurrentTimeMillis();
        if (!_lastAnimationTs)
            _lastAnimationTs = ts;
        lUInt64 millisDiff = ts - _lastAnimationTs;
        /// call animate
        animate(millisDiff);
        _lastAnimationTs = ts;
    } else {
        _lastAnimationTs = 0;
    }

    if (_popupBackground) {
        //CRLog::trace("Drawing static background");
        buf->DrawRescaled(_popupBackground, 0, 0, _pos.width(), _pos.height(), 0);
        //_popupBackground->DrawTo(buf, 0, 0, 0, NULL);
    } else {
        if (_animation.active) {
            if (_animation.oldimage)
                _animation.oldimage->DrawTo(buf, _animation.oldimagex, _pos.top, 0, NULL);
            else {
                CRUIWidget * oldWidget = _history[_animation.oldpos]->getWidget();
                oldWidget->layout(_pos.left + _animation.oldimagex, _pos.top, _pos.right + _animation.oldimagex, _pos.bottom);
                oldWidget->draw(buf);
            }
            if (_animation.newimage)
                _animation.newimage->DrawTo(buf, _animation.newimagex, _pos.top, 0, NULL);
            else {
                CRUIWidget * newWidget = _history[_animation.newpos]->getWidget();
                newWidget->layout(_pos.left + _animation.newimagex, _pos.top, _pos.right + _animation.newimagex, _pos.bottom);
                newWidget->draw(buf);
            }
        } else {
            _history.currentWidget()->draw(buf);
        }
    }
    if (_popup) {
        //CRLog::trace("Drawing popup");
        _popup->draw(buf);
    }
    setScreenUpdateMode(false, animating ? 30 : 0);
}

/// motion event handler, returns true if it handled event
bool CRUIMainWidget::onTouchEvent(const CRUIMotionEvent * event) {
    return _history.currentWidget()->onTouchEvent(event);
}

/// handle menu or other action
bool CRUIMainWidget::onAction(const CRUIAction * action) {
    switch (action->id) {
    case CMD_EXIT:
        return true;
    }
    return false;
}

/// motion event handler - before children, returns true if it handled event
bool CRUIMainWidget::onTouchEventPreProcess(const CRUIMotionEvent * event) {
    // by returning of true, just ignore all events while animation is on
    if (_animation.active && _animation.manual) {
        switch(event->getAction()) {
        case ACTION_MOVE:
            {
                int dx = _animation.startPoint.x - event->getX();
                int p = dx * _animation.duration / _pos.width();
                if (_animation.direction > 0)
                    p = -p;
                _animation.progress = p > 0 ? p : 0;
                invalidate();
                CRLog::trace("manual animation progress %d", p);
            }
            break;
        case ACTION_CANCEL:
        case ACTION_DOWN:
        case ACTION_UP:
            _animation.manual = false;
            break;
        default:
            break;
        }

        return true;
    }
    return _animation.active;
}

/// return true if drag operation is intercepted
bool CRUIMainWidget::startDragging(const CRUIMotionEvent * event, bool vertical) {
    if (vertical)
        return false;
    int dx = event->getX() - event->getStartX();
    if (dx > 0 && !_history.hasBack())
        return false;
    if (dx < 0 && !_history.hasForward())
        return false;
    if (dx < 0) {
        // FORWARD dragging
        startAnimation(_history.pos() + 1, WINDOW_ANIMATION_DELAY, event);
        return true;
    } else {
        // BACK dragging
        startAnimation(_history.pos() - 1, WINDOW_ANIMATION_DELAY, event);
        return true;
    }
}

