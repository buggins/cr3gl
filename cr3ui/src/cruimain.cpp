#include "cruimain.h"

using namespace CRUI;

#define WINDOW_ANIMATION_DELAY 300
#define SLOW_OPERATION_POPUP_DELAY 800
#define SLOW_OPERATION_POPUP_DIMMING_DURATION 1200

void CRUIMainWidget::recreate() {
    if (_home)
        delete _home;
    if (_folder)
        delete _folder;
    if (_read)
        delete _read;
    _home = new CRUIHomeWidget(this);
    _folder = new CRUIFolderWidget(this);
    _read = new CRUIReadWidget(this);
    switch (_mode) {
    case MODE_HOME:
        _currentWidget = _home;
        break;
    case MODE_FOLDER:
        _currentWidget = _folder;
        break;
    case MODE_READ:
        _currentWidget = _read;
        break;
    }
    if (!_currentFolder.empty())
        _folder->setDirectory(dirCache->find(_currentFolder));
    requestLayout();
}

void CRUIMainWidget::setMode(VIEW_MODE mode) {
    _mode = mode;
    switch (_mode) {
    case MODE_HOME:
        _currentWidget = _home;
        break;
    case MODE_FOLDER:
        _currentWidget = _folder;
        break;
    case MODE_READ:
        _currentWidget = _read;
        break;
    }
    _currentWidget->requestLayout();
    requestLayout();
    update();
}

/// returns true if widget is child of this
bool CRUIMainWidget::isChild(CRUIWidget * widget) {
    return widget == this || _currentWidget->isChild(widget);
}

void CRUIMainWidget::showHome() {
    setMode(MODE_HOME);
}

void CRUIMainWidget::back() {
    startAnimation(_home, MODE_HOME, 1, WINDOW_ANIMATION_DELAY, false, false);
}

void CRUIMainWidget::onDirectoryScanFinished(CRDirCacheItem * item) {
    if (item->getPathName() == _pendingFolder) {
        CRLog::info("Directory %s is ready", _pendingFolder.c_str());
        hideSlowOperationPopup();
        // setup folder animation
        CRUIFolderWidget * newWidget = _folder;
        bool deleteOldWidget = false;
        if (_currentWidget == _folder) {
            newWidget = new CRUIFolderWidget(this);
            deleteOldWidget = true;
        }
        newWidget->setDirectory(item);
        _currentFolder = _pendingFolder;
        _pendingFolder.clear();
        startAnimation(newWidget, MODE_FOLDER, -1, WINDOW_ANIMATION_DELAY, deleteOldWidget, false);
        //_folder->setDirectory(item);
        _folder = newWidget;
        update();
    }
}

void CRUIMainWidget::showSlowOperationPopup()
{
    CRUITextWidget * pleaseWait = new CRUITextWidget(lString16("Please wait"));
    pleaseWait->setBackground(0xFFFFFF);
    pleaseWait->setPadding(PT_TO_PX(7));
    pleaseWait->setAlign(ALIGN_CENTER);
    pleaseWait->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
    _popup = new CRUIPopupWindow(pleaseWait, SLOW_OPERATION_POPUP_DELAY, SLOW_OPERATION_POPUP_DIMMING_DURATION, 0xA0000000);
}

void CRUIMainWidget::hideSlowOperationPopup()
{
    if (_popup) {
        delete _popup;
        _popup = NULL;
    }
}

void CRUIMainWidget::showFolder(lString8 folder) {
    if ((_currentFolder != folder && _pendingFolder != folder) || _mode != MODE_FOLDER) {
        showSlowOperationPopup();
        _pendingFolder = folder;
        //_popup->setBackground(0xC0000000); // dimming
        requestLayout();
        CRLog::info("Starting background directory scan for %s", _pendingFolder.c_str());
        dirCache->scan(folder, this);
    }
}

void CRUIMainWidget::openBook(lString8 pathname) {
    CRLog::debug("Opening book %s", pathname.c_str());
    _read->openBook(pathname);
    setMode(MODE_READ);
}

CRUIMainWidget::CRUIMainWidget() : _home(NULL), _folder(NULL), _read(NULL), _popup(NULL), _currentWidget(NULL), _screenUpdater(NULL), _lastAnimationTs(0) {
    _mode = MODE_HOME;
    recreate();
}

CRUIMainWidget::~CRUIMainWidget() {
    if (_home)
        delete _home;
    if (_folder)
        delete _folder;
    if (_read)
        delete _read;
}

int CRUIMainWidget::getChildCount() {
    int cnt = 0;
    if (_currentWidget)
        cnt++;
    if (_popup)
        cnt++;
    return cnt; //_currentWidget->getChildCount();
}

CRUIWidget * CRUIMainWidget::getChild(int index) {
    if (_popup && index == 0)
        return _popup;
    return _currentWidget;
    //return _currentWidget->getChild(index);
}

/// measure dimensions
void CRUIMainWidget::measure(int baseWidth, int baseHeight) {
    _currentWidget->measure(baseWidth, baseHeight);
    _measuredWidth = _currentWidget->getMeasuredWidth();
    _measuredHeight = _currentWidget->getMeasuredHeight();
    if (_popup)
        _popup->measure(baseWidth, baseHeight);
}

/// updates widget position based on specified rectangle
void CRUIMainWidget::layout(int left, int top, int right, int bottom) {
    _currentWidget->layout(left, top, right, bottom);
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

void CRUIMainWidget::startAnimation(CRUIWidget * newWidget, VIEW_MODE newMode, int direction, int duration, bool deleteOldWidget, bool manual) {
    if (_animation.active) {
        stopAnimation();
    }
    CRLog::trace("starting animation mode %d -> mode %d", _mode, newMode);
    CRReinitTimer();
    _animation.active = true;
    _animation.oldMode = _mode;
    _animation.newMode = newMode;
    _animation.oldWidget = _currentWidget;
    _animation.newWidget = newWidget;
    _animation.duration = duration * 10;
    _animation.direction = direction;
    _animation.progress = 0;
    _animation.deleteOldWidget = deleteOldWidget;
    _animation.manual = manual;
    newWidget->measure(_pos.width(), _pos.height());
}

void CRUIMainWidget::stopAnimation() {
    if (!_animation.active)
        return;
    CRLog::trace("stopping animation");
    _animation.active = false;
    _animation.manual = false;
    _animation.oldWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    if (_animation.deleteOldWidget)
        delete _animation.oldWidget;
    _currentWidget = _animation.newWidget;
    _currentWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    _mode = _animation.newMode;
}

void CRUIMainWidget::animate(lUInt64 millisPassed) {
    CRUIWidget::animate(millisPassed);
    if (_animation.active) {
        if (!_animation.manual) {
            _animation.progress += (int)millisPassed * 10;
        }
        int p = _animation.progress;
        CRLog::trace("animating ts = %lld,  passed = %d   %d of %d", (lUInt64)GetCurrentTimeMillis(), (int)millisPassed, (int)p, (int)_animation.duration);
        if (p > _animation.duration) {
            stopAnimation();
        } else {
            int x = _pos.width() * p / _animation.duration;
            CRLog::trace("animation position %d", x);
            lvRect rc1 = _pos;
            lvRect rc2 = _pos;
            if (_animation.direction < 0) {
                rc1.left -= x;
                rc1.right -= x;
                rc2.left += _pos.width() - x;
                rc2.right += _pos.width() - x;
            } else {
                rc1.left += x;
                rc1.right += x;
                rc2.left -= _pos.width() - x;
                rc2.right -= _pos.width() - x;
            }
            _animation.oldWidget->layout(rc1.left, rc1.top, rc1.right, rc1.bottom);
            _animation.newWidget->layout(rc2.left, rc2.top, rc2.right, rc2.bottom);
        }
    }
}

bool CRUIMainWidget::isAnimating() {
    return _animation.active;
}


/// draws widget with its children to specified surface
void CRUIMainWidget::draw(LVDrawBuf * buf) {
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

    if (_animation.active) {
        _animation.oldWidget->draw(buf);
        _animation.newWidget->draw(buf);
    } else {
        _currentWidget->draw(buf);
    }
    if (_popup)
        _popup->draw(buf);
    setScreenUpdateMode(false, animating ? 30 : 0);
}

/// motion event handler, returns true if it handled event
bool CRUIMainWidget::onTouchEvent(const CRUIMotionEvent * event) {
    return _currentWidget->onTouchEvent(event);
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
    if (dx > 0 && _currentWidget == _home)
        return false;
    if (dx < 0) {
        // FORWARD dragging
        return false; // no forward implemented so far
    } else {
        // BACK dragging
        (const_cast<CRUIMotionEvent *>(event))->setWidget(this);
        startAnimation(_home, MODE_HOME, 1, WINDOW_ANIMATION_DELAY, false, true);
        _animation.startPoint.x = event->getStartX();
        _animation.startPoint.y = event->getStartY();
        return true;
    }
}

