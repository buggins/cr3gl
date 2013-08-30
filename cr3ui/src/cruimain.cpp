#include "cruimain.h"

using namespace CRUI;

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
    return _currentWidget->isChild(widget);
}

void CRUIMainWidget::showHome() {
    setMode(MODE_HOME);
}

void CRUIMainWidget::onDirectoryScanFinished(CRDirCacheItem * item) {
    if (item->getPathName() == _pendingFolder) {
        CRLog::info("Directory %s is ready", _pendingFolder.c_str());
        if (_popup) {
            delete _popup;
            _popup = NULL;
        }
        _currentFolder = _pendingFolder;
        _folder->setDirectory(item);
        setMode(MODE_FOLDER);
        _pendingFolder.clear();
    }
}

void CRUIMainWidget::showFolder(lString8 folder) {
    if (_currentFolder != folder && _pendingFolder != folder) {
        _pendingFolder = folder;
        CRUITextWidget * pleaseWait = new CRUITextWidget(lString16("Please wait"));
        pleaseWait->setBackground(0xFFFFFF);
        pleaseWait->setPadding(PT_TO_PX(7));
        pleaseWait->setAlign(ALIGN_CENTER);
        pleaseWait->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        _popup = new CRUIPopupWindow(pleaseWait, 1000, 0xA0000000);
        //_popup->setBackground(0xC0000000); // dimming
        requestLayout();
        CRLog::info("Starting background directory scan for %s", _pendingFolder.c_str());
        dirCache->scan(folder, this);
    }
}

void CRUIMainWidget::openBook(lString8 pathname) {
    CRLog::debug("Opening book %s", pathname.c_str());
}

CRUIMainWidget::CRUIMainWidget() : _home(NULL), _folder(NULL), _read(NULL), _popup(NULL), _currentWidget(NULL), _screenUpdater(NULL) {
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

/// draws widget with its children to specified surface
void CRUIMainWidget::draw(LVDrawBuf * buf) {
    _currentWidget->draw(buf);
    if (_popup)
        _popup->draw(buf);
    bool needLayout, needDraw, animating;
    //CRLog::trace("Checking if draw is required");
    CRUICheckUpdateOptions(this, needLayout, needDraw, animating);
    setScreenUpdateMode(false, animating ? 30 : 0);
}

/// motion event handler, returns true if it handled event
bool CRUIMainWidget::onTouchEvent(const CRUIMotionEvent * event) {
    return _currentWidget->onTouchEvent(event);
}
