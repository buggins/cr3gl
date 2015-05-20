
#include "cruiscrollwidget.h"
#include "cruilist.h"
#include "crui.h"

using namespace CRUI;

#define NO_DRAG (-1234567)
#define MAX_EXTRA_DRAG PT_TO_PX(10)
//===================================================================================================
// List

CRUIScrollWidget::CRUIScrollWidget(bool vertical)
    : CRUILinearLayout(vertical)
    , _scrollOffset(0)
    , _maxScrollOffset(0)
    , _totalSize(0)
    , _dragStartOffset(NO_DRAG)
    , _onStartDragCallback(NULL)
{
    setStyle("MENU_LIST");
    _scrollBar = new CRUIScrollBar(vertical, 0, 100, 0, 100);
    _scrollBar->setScrollPosCallback(this);
    _visibleSize = 0;
    _sbHidden = false;
}

CRUIScrollWidget::~CRUIScrollWidget() {
    delete _scrollBar;
}

/// sets orientation
CRUILinearLayout * CRUIScrollWidget::setVertical(bool vertical) {
    CRUILinearLayout::setVertical(vertical);
    _scrollBar->setVertical(vertical);
    return this;
}

/// returns true if widget is child of this
bool CRUIScrollWidget::isChild(CRUIWidget * widget) {
    return (widget == _scrollBar) || CRUILinearLayout::isChild(widget);
}

int CRUIScrollWidget::getChildCount() {
    return CRUILinearLayout::getChildCount() + (_sbHidden ? 0 : 1);
}

CRUIWidget * CRUIScrollWidget::getChild(int index) {
    if (index < CRUILinearLayout::getChildCount())
        return CRUILinearLayout::getChild(index);
    return _scrollBar;
}

void CRUIScrollWidget::updateScrollBar() {
    _scrollBar->setMaxScrollPos(_visibleSize + _maxScrollOffset);
    _scrollBar->setPageSize(_visibleSize);
    _scrollBar->setScrollPos(_scrollOffset);
    invalidate();
}

bool CRUIScrollWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
    CR_UNUSED2(widget, manual);
    setScrollOffset(pos);
    //CRLog::trace("scrollPos: %d", pos);
    return true;
}

void CRUIScrollWidget::setScrollOffset(int offset) {
    int oldOffset = _scrollOffset;
    bool isDragging = _dragStartOffset != NO_DRAG;
    int delta = isDragging ? MAX_EXTRA_DRAG : 0;
    _scrollOffset = offset;
    if (_scrollOffset > _maxScrollOffset + delta)
        _scrollOffset = _maxScrollOffset + delta;
    if (_scrollOffset < - delta)
        _scrollOffset = - delta;
    if (_scrollOffset != oldOffset) {
        layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
        updateScrollBar();
        invalidate();
    }
}

#define SCROLL_MAX_DIMENSION 1000000
/// measure dimensions
void CRUIScrollWidget::measure(int baseWidth, int baseHeight) {
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
    _scrollBar->measure(baseWidth, baseHeight);
    int sbsize = 0;
    if (isVertical()) {
        sbsize = _scrollBar->getMeasuredWidth();
    } else {
        sbsize = _scrollBar->getMeasuredHeight();
    }
    lvRect padding;
    getPadding(padding);
    lvRect margin = getMargin();
    int maxw = baseWidth - (margin.left + margin.right + padding.left + padding.right);
    int maxh = baseHeight - (margin.top + margin.bottom + padding.top + padding.bottom);
    if (isVertical()) {
        _sbHidden = true;
        CRUILinearLayout::measure(baseWidth - sbsize, UNSPECIFIED);
        _sbHidden = false;
        _totalSize = _measuredHeight;
        if (_measuredHeight > maxh)
            _measuredHeight = maxh;
    } else {
        _sbHidden = true;
        CRUILinearLayout::measure(UNSPECIFIED, baseHeight - sbsize);
        _sbHidden = false;
        _totalSize = _measuredHeight;
        if (_measuredWidth > maxw)
            _measuredWidth = maxw;
    }
}

/// updates widget position based on specified rectangle
void CRUIScrollWidget::layout(int left, int top, int right, int bottom) {
    int sbsize = 0;
    if (isVertical()) {
        sbsize = _scrollBar->getMeasuredWidth();
        _sbHidden = true;
        CRUILinearLayout::measure(right - left - sbsize, UNSPECIFIED);
        _totalSize = _measuredHeight;
        CRUILinearLayout::layout(left, top - _scrollOffset, right - sbsize, top - _scrollOffset + _measuredHeight);
        _sbHidden = false;
    } else {
        _sbHidden = true;
        sbsize = _scrollBar->getMeasuredHeight();
        CRUILinearLayout::measure(UNSPECIFIED, bottom - top - sbsize);
        _totalSize = _measuredWidth;
        CRUILinearLayout::layout(left - _scrollOffset, top, left - _scrollOffset + _measuredWidth, bottom - sbsize);
        _sbHidden = false;
    }
    _pos.left = left;
    _pos.top = top;
    _pos.right = right;
    _pos.bottom = bottom;
    _layoutRequested = false;
    _clientRect = _pos;
    applyMargin(_clientRect);
    applyPadding(_clientRect);

    _sbRect = _clientRect;
    if (isVertical()) {
        _sbRect.left = _sbRect.right - sbsize;
        _clientRect.right -= sbsize;
    } else {
        _sbRect.top = _sbRect.bottom - sbsize;
        _clientRect.bottom -= sbsize;
    }
    _visibleSize = isVertical() ? _clientRect.height() : _clientRect.width();
    _maxScrollOffset = _totalSize - _visibleSize > 0 ? _totalSize - _visibleSize : 0;

    _scrollBar->layout(_sbRect.left, _sbRect.top, _sbRect.right, _sbRect.bottom);

    updateScrollBar();
}

/// draws widget with its children to specified surface
void CRUIScrollWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    lvRect oldpos = _pos;
    int sbsize = 0;
    if (isVertical()) {
        sbsize = _scrollBar->getMeasuredWidth();
        _pos.right -= sbsize;
    } else {
        sbsize = _scrollBar->getMeasuredHeight();
        _pos.bottom -= sbsize;
    }
    _sbHidden = true;
    CRUILinearLayout::draw(buf);
    _sbHidden = false;
    _pos = oldpos;
    _scrollBar->draw(buf);
}

lvPoint CRUIScrollWidget::getTileOffset() const {
    lvPoint res;
    if (isVertical())
        res.y = _scrollOffset;
    else
        res.x = _scrollOffset;
    return res;
}

#define SCROLL_SPEED_CALC_INTERVAL 700
#define SCROLL_MIN_SPEED PT_TO_PX(1)
#define SCROLL_FRICTION 20

void CRUIScrollWidget::animate(lUInt64 millisPassed) {
    if (_scroll.animate(millisPassed)) {
        int oldpos = getScrollOffset();
        setScrollOffset(_scroll.pos());
        if (oldpos == getScrollOffset()) {
            _scroll.stop();
            invalidate();
        }
    }
}

bool CRUIScrollWidget::isAnimating() {
    return _scroll.isActive();
}

/// motion event handler, returns true if it handled event
bool CRUIScrollWidget::onTouchEvent(const CRUIMotionEvent * event) {
    int action = event->getAction();
    //CRLog::trace("CRUIScrollWidget::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
    int dx = event->getX() - event->getStartX();
    int dy = event->getY() - event->getStartY();
    int delta = isVertical() ? dy : dx;
    int delta2 = isVertical() ? dx : dy;
    bool isDragging = _dragStartOffset != NO_DRAG;
    //CRLog::trace("CRUIScrollWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
    switch (action) {
    case ACTION_DOWN:
        _scroll.stop();
        invalidate();
        //CRLog::trace("list DOWN");
        break;
    case ACTION_UP:
        {
            invalidate();
            setScrollOffset(_scrollOffset);
            if (_dragStartOffset != NO_DRAG) {
                lvPoint speed = event->getSpeed(SCROLL_SPEED_CALC_INTERVAL);
                int spd = isVertical() ? speed.y : speed.x;
                _dragStartOffset = NO_DRAG;
                if (spd < -SCROLL_MIN_SPEED || spd > SCROLL_MIN_SPEED) {
                    setScrollOffset(_scrollOffset);
                    _scroll.start(_scrollOffset, -spd, SCROLL_FRICTION);
                    CRLog::trace("Starting scroll with speed %d", _scroll.speed());
                } else {
                    setScrollOffset(_scrollOffset);
                }
                invalidate();
            }
        }
        // fire onclick
        //CRLog::trace("list UP");
        break;
    case ACTION_FOCUS_IN:
        if (isDragging)
            setScrollOffset(_dragStartOffset - delta);
        invalidate();
        //CRLog::trace("list FOCUS IN");
        break;
    case ACTION_FOCUS_OUT:
        if (isDragging)
            setScrollOffset(_dragStartOffset - delta);
        invalidate();
        return false; // to continue tracking
        //CRLog::trace("list FOCUS OUT");
        break;
    case ACTION_CANCEL:
        _dragStartOffset = NO_DRAG;
        setScrollOffset(_scrollOffset);
        //CRLog::trace("list CANCEL");
        break;
    case ACTION_MOVE:
        if (!isDragging && ((delta > DRAG_THRESHOLD) || (-delta > DRAG_THRESHOLD))) {
            _dragStartOffset = _scrollOffset;
            setScrollOffset(_dragStartOffset - delta);
            invalidate();
        } else if (isDragging) {
            setScrollOffset(_dragStartOffset - delta);
            invalidate();
        } else if (!isDragging) {
            if ((delta2 > DRAG_THRESHOLD_X) || (-delta2 > DRAG_THRESHOLD_X)) {
                if (_onStartDragCallback)
                    _onStartDragCallback->onStartDragging(event, !isVertical());
                else {
                    startDragging(event, !isVertical());
                    invalidate();
                }
            }
        }
        // ignore
        //CRLog::trace("list MOVE");
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
    }
    return true;
}

/// return true if drag operation is intercepted
bool CRUIScrollWidget::startDragging(const CRUIMotionEvent * event, bool vertical) {
    CR_UNUSED2(event, vertical);
    return false;
}



