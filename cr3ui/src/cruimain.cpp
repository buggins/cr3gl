#include "cruimain.h"

void CRUIMainWidget::recreate() {
    if (_home)
        delete _home;
    if (_folder)
        delete _folder;
    if (_read)
        delete _read;
    _home = new CRUIHomeWidget();
    _folder = new CRUIFolderWidget();
    _read = new CRUIReadWidget();
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
}

/// returns true if widget is child of this
bool CRUIMainWidget::isChild(CRUIWidget * widget) {
    return _currentWidget->isChild(widget);
}

void CRUIMainWidget::showHome() {
    setMode(MODE_HOME);
}

void CRUIMainWidget::showFolder(lString8 folder) {
    if (_currentFolder != folder) {
        _currentFolder = folder;
        CRDirCacheItem * dir = dirCache->find(folder);
        if (!dir) {
            dir = dirCache->getOrAdd(folder);
            dir->scan();
        }
        _folder->setDirectory(dir);
        setMode(MODE_FOLDER);
        requestLayout();
    }
}

CRUIMainWidget::CRUIMainWidget() : _home(NULL), _folder(NULL), _read(NULL) {
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
    return _currentWidget->getChildCount();
}

CRUIWidget * CRUIMainWidget::getChild(int index) {
    return _currentWidget->getChild(index);
}

/// measure dimensions
void CRUIMainWidget::measure(int baseWidth, int baseHeight) {
    _currentWidget->measure(baseWidth, baseHeight);
    _measuredWidth = _currentWidget->getMeasuredWidth();
    _measuredHeight = _currentWidget->getMeasuredHeight();
}

/// updates widget position based on specified rectangle
void CRUIMainWidget::layout(int left, int top, int right, int bottom) {
    _currentWidget->layout(left, top, right, bottom);
    _pos.left = left;
    _pos.top = top;
    _pos.right = right;
    _pos.bottom = bottom;
}

/// draws widget with its children to specified surface
void CRUIMainWidget::draw(LVDrawBuf * buf) {
    _currentWidget->draw(buf);
}

/// motion event handler, returns true if it handled event
bool CRUIMainWidget::onTouchEvent(const CRUIMotionEvent * event) {
    return _currentWidget->onTouchEvent(event);
}
