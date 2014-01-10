/*
 * cruilist.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruilist.h"
#include "crui.h"

using namespace CRUI;

#define NO_DRAG (-1234567)
#define MAX_EXTRA_DRAG PT_TO_PX(10)
//===================================================================================================
// List

CRUIListWidget::CRUIListWidget(bool vertical, CRUIListAdapter * adapter)
: _vertical(vertical), _adapter(adapter),
  _ownAdapter(false), _scrollOffset(0),
  _maxScrollOffset(0), _topItem(0), _selectedItem(-1), _dragStartOffset(NO_DRAG),
  _colCount(1),
  _onItemClickListener(NULL), _onItemLongClickListener(NULL), _onStartDragCallback(NULL)
{
    setStyle("MENU_LIST");
    setFocusable(true);
}

bool CRUIListWidget::onItemClickEvent(int itemIndex) {
	if (_onItemClickListener)
		return _onItemClickListener->onListItemClick(this, itemIndex);
	return false;
}

bool CRUIListWidget::onItemLongClickEvent(int itemIndex) {
	if (_onItemLongClickListener)
		return _onItemLongClickListener->onListItemLongClick(this, itemIndex);
	return false;
}

void CRUIListWidget::setScrollOffset(int offset) {
	int oldOffset = _scrollOffset;
	bool isDragging = _dragStartOffset != NO_DRAG;
	int delta = isDragging ? MAX_EXTRA_DRAG : 0;
	_scrollOffset = offset;
	if (_scrollOffset > _maxScrollOffset + delta)
		_scrollOffset = _maxScrollOffset + delta;
	if (_scrollOffset < - delta)
		_scrollOffset = - delta;
	if (_scrollOffset != oldOffset)
		invalidate();
}

/// measure dimensions
void CRUIListWidget::measure(int baseWidth, int baseHeight) {
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
    lvRect padding;
	getPadding(padding);
	lvRect margin = getMargin();
	int maxw = baseWidth - (margin.left + margin.right + padding.left + padding.right);
	int maxh = baseHeight - (margin.top + margin.bottom + padding.top + padding.bottom);
	_itemSizes.clear();
	CRUIImageRef delimiter;
	int delimiterSize = 0;
	if (isVertical()) {
		delimiter = getStyle()->getListDelimiterVertical();
		if (!delimiter.isNull())
			delimiterSize = delimiter->originalHeight();
	} else {
		delimiter = getStyle()->getListDelimiterHorizontal();
		if (!delimiter.isNull())
			delimiterSize = delimiter->originalWidth();
	}
	if (_vertical) {
        // VERTICAL
		int biggestw = 0;
		int totalh = 0;

        for (int i=0; i<getItemCount() + _colCount - 1; i += _colCount) {
            int maxColH = 0;
            for (int col = 0; col < _colCount; col++) {
                CRUIWidget * item = i + col < getItemCount() ? getItemWidget(i + col) : NULL;
                if (item) {
                    item->measure(maxw, maxh);
                    lvPoint sz(item->getMeasuredWidth(), item->getMeasuredHeight());
                    //CRLog::trace("measured list item[%d] (%d,%d) minH=%d MIN_ITEM_PX=%d", i, sz.x, sz.y, item->getMinHeight(), MIN_ITEM_PX);
                    if (maxColH < sz.y)
                        maxColH = sz.y;
                    if (biggestw < sz.x)
                        biggestw = sz.x;
                    _itemSizes.add(sz);
                }
            }
            totalh += maxColH;
            if (i < getItemCount()  + _colCount - 1 - 1)
                totalh += delimiterSize;
        }
		if (biggestw > maxw)
			biggestw = maxw;
		if (totalh > maxh)
			totalh = maxh;
		defMeasure(baseWidth, baseHeight, biggestw, totalh);
	} else {
        // HORIZONTAL
		int biggesth = 0;
		int totalw = 0;

		for (int i=0; i<getItemCount(); i++) {
			CRUIWidget * item = getItemWidget(i);
			item->measure(maxw, maxh);
			lvPoint sz(item->getMeasuredWidth(), item->getMeasuredHeight());
			totalw += sz.x;
			if (biggesth < sz.y)
				biggesth = sz.y;
			_itemSizes.add(sz);
			if (i < getItemCount() - 1)
				totalw += delimiterSize;
		}
		if (biggesth > maxh)
			biggesth = maxh;
		if (totalw > maxw)
			totalw = maxw;
		defMeasure(baseWidth, baseHeight, totalw, biggesth);
	}
}

void CRUIListWidget::getItemRect(int index, lvRect & rc) {
    rc.clear();
    if (index >= 0 && index < _itemRects.length()) {
        rc = _itemRects[index];
        if (isVertical()) {
            rc.top -= _scrollOffset;
            rc.bottom -= _scrollOffset;
        } else {
            rc.left -= _scrollOffset;
            rc.right -= _scrollOffset;
        }
    }
}

bool CRUIListWidget::isItemVisible(int index) {
    lvRect rc;
    getItemRect(index, rc);
    lvRect clientRc = _pos;
    applyMargin(clientRc);
    return clientRc.intersects(rc);
}

/// updates widget position based on specified rectangle
void CRUIListWidget::layout(int left, int top, int right, int bottom) {
    CRUIWidget::layout(left, top, right, bottom);
    _pos.left = left;
    _pos.top = top;
    _pos.right = right;
    _pos.bottom = bottom;
    _layoutRequested = false;
    lvRect clientRc = _pos;
	applyMargin(clientRc);
	applyPadding(clientRc);
	int winsize = isVertical() ? clientRc.height() : clientRc.width();
	lvRect childRc = clientRc;
	_itemRects.clear();
	CRUIImageRef delimiter;
	int delimiterSize = 0;
	if (isVertical()) {
		delimiter = getStyle()->getListDelimiterVertical();
		if (!delimiter.isNull())
			delimiterSize = delimiter->originalHeight();
	} else {
		delimiter = getStyle()->getListDelimiterHorizontal();
		if (!delimiter.isNull())
			delimiterSize = delimiter->originalWidth();
	}
	if (_vertical) {
		int y = childRc.top;
		int y0 = y;
        for (int i=0; i<getItemCount() + _colCount - 1 /* && i < _itemSizes.length() */; i += _colCount) {
            int maxColH = 0;
            for (int col = 0; col < _colCount; col++) {
                int idx = i + col;
                if (idx < _itemSizes.length()) {
                    if (maxColH < _itemSizes[idx].y)
                        maxColH = _itemSizes[idx].y;
                }
            }
            childRc.top = y;
            childRc.bottom = y + maxColH;
            for (int col = 0; col < _colCount; col++) {
                int idx = i + col;
                if (idx < _itemSizes.length()) {
                    childRc.left = clientRc.left + col * clientRc.width() / _colCount;
                    childRc.right = col < _colCount - 1 ? clientRc.left + (col + 1) * clientRc.width() / _colCount : clientRc.right;
                    //CRLog::trace("layout list item [%d] (%d,%d, %d,%d)", i, childRc.left, childRc.top, childRc.right, childRc.bottom);
                    _itemRects.add(childRc);
                }
            }
            y = childRc.bottom;
            if (i < getItemCount() - 1)
                y += delimiterSize;
        }
		_maxScrollOffset = y - y0 - winsize > 0 ? y - y0 - winsize : 0;
	} else {
		int x = childRc.left;
		int x0 = x;
		for (int i=0; i < getItemCount() && i < _itemSizes.length(); i++) {
			lvPoint sz = _itemSizes[i];
			childRc.left = x;
			childRc.right = x + sz.x;
			_itemRects.add(childRc);
			x = childRc.right;
			if (i < getItemCount() - 1)
				x += delimiterSize;
		}
		_maxScrollOffset = x - x0 - winsize > 0 ? x - x0 - winsize : 0;
	}
}

/// draws widget with its children to specified surface
void CRUIListWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    CRUIWidget::draw(buf);
//	if (!_adapter)
//		return;
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	applyPadding(rc);
    setClipRect(buf, rc);

	lvRect clip;
	buf->GetClipRect(&clip);

	CRUIImageRef delimiter;
	int delimiterSize = 0;
	if (isVertical()) {
		delimiter = getStyle()->getListDelimiterVertical();
		if (!delimiter.isNull())
			delimiterSize = delimiter->originalHeight();
	} else {
		delimiter = getStyle()->getListDelimiterHorizontal();
		if (!delimiter.isNull())
			delimiterSize = delimiter->originalWidth();
	}
	for (int i=0; i<getItemCount() && i < _itemRects.length(); i++) {
		lvRect childRc = _itemRects[i];
		lvRect delimiterRc;
		if (_vertical) {
			childRc.top -= _scrollOffset;
			childRc.bottom -= _scrollOffset;
			delimiterRc = childRc;
			delimiterRc.top = childRc.bottom;
			delimiterRc.bottom = delimiterRc.top + delimiterSize;
		} else {
			childRc.left -= _scrollOffset;
			childRc.right -= _scrollOffset;
			delimiterRc = childRc;
			delimiterRc.left = childRc.right;
			delimiterRc.right = delimiterRc.left + delimiterSize;
		}
		if (clip.intersects(childRc)) {
            bool selected = (_selectedItem == i);
            bool enabled = isItemEnabled(i);
            CRUIWidget * item = getItemWidget(i);
            if (!item)
                continue;
            item->setState(selected ? STATE_FOCUSED : 0, STATE_FOCUSED);
            item->setState(!enabled ? STATE_DISABLED : 0, STATE_DISABLED);
            //CRLog::trace("drawing list item [%d] (%d,%d, %d,%d)", i, childRc.left, childRc.top, childRc.right, childRc.bottom);
            item->measure(childRc.width(), childRc.height());
            item->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			item->draw(buf);
		} else {
			//CRLog::trace("list item out of clip rect");
		}
        int maxItemWithDelimiter = (getItemCount() - _colCount - 1) / _colCount * _colCount + (_colCount);
        if (delimiterSize && i < maxItemWithDelimiter && clip.intersects(delimiterRc))
			delimiter->draw(buf, delimiterRc);
	}
}

lvPoint CRUIListWidget::getTileOffset() const {
	lvPoint res;
	if (_vertical)
		res.y = _scrollOffset;
	else
		res.x = _scrollOffset;
	return res;
}

int CRUIListWidget::itemFromPoint(int x, int y) {
	CRUIImageRef delimiter;
	int delimiterSize = 0;
	if (isVertical()) {
		delimiter = getStyle()->getListDelimiterVertical();
		if (!delimiter.isNull())
			delimiterSize = delimiter->originalHeight();
	} else {
		delimiter = getStyle()->getListDelimiterHorizontal();
		if (!delimiter.isNull())
			delimiterSize = delimiter->originalWidth();
	}
	lvPoint pt(x,y);
	for (int i=0; i<getItemCount() && i < _itemRects.length(); i++) {
		lvRect childRc = _itemRects[i];
		lvRect delimiterRc;
		if (_vertical) {
			childRc.top -= _scrollOffset;
			childRc.bottom -= _scrollOffset;
			delimiterRc = childRc;
			delimiterRc.top = childRc.bottom;
			delimiterRc.bottom = delimiterRc.top + delimiterSize;
		} else {
			childRc.left -= _scrollOffset;
			childRc.right -= _scrollOffset;
			delimiterRc = childRc;
			delimiterRc.left = childRc.right;
			delimiterRc.right = delimiterRc.left + delimiterSize;
		}
		if (childRc.isPointInside(pt) || delimiterRc.isPointInside(pt))
			return i;
	}
	return -1;
}

#define SCROLL_SPEED_CALC_INTERVAL 700
#define SCROLL_MIN_SPEED PT_TO_PX(1)
#define SCROLL_FRICTION 20

void CRUIListWidget::animate(lUInt64 millisPassed) {
    if (_scroll.animate(millisPassed)) {
        int oldpos = getScrollOffset();
        setScrollOffset(_scroll.pos());
        if (oldpos == getScrollOffset()) {
            _scroll.stop();
            invalidate();
        }
    }
}

bool CRUIListWidget::isAnimating() {
    return _scroll.isActive();
}

/// motion event handler, returns true if it handled event
bool CRUIListWidget::onTouchEvent(const CRUIMotionEvent * event) {
	int action = event->getAction();
	//CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
	int index = itemFromPoint(event->getX(), event->getY());
	int dx = event->getX() - event->getStartX();
	int dy = event->getY() - event->getStartY();
	int delta = isVertical() ? dy : dx;
    int delta2 = isVertical() ? dx : dy;
    bool isDragging = _dragStartOffset != NO_DRAG;
	//CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
	switch (action) {
	case ACTION_DOWN:
		_selectedItem = index;
        _scroll.stop();
		invalidate();
		//CRLog::trace("list DOWN");
		break;
	case ACTION_UP:
		{
			int itemIndex = _selectedItem;
			_selectedItem = -1;
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
            if (itemIndex != -1) {
                //CRLog::trace("UP ts=%lld downTs=%lld downDuration=%lld", event->getEventTimestamp(), event->getDownEventTimestamp(), event->getDownDuration());
                bool isLong = event->getDownDuration() > LONG_TOUCH_THRESHOLD; // 0.5 seconds threshold
				if (isLong && onItemLongClickEvent(itemIndex))
					return true;
	            invalidate();
				onItemClickEvent(itemIndex);
            } else {
            }
		}
		// fire onclick
		//CRLog::trace("list UP");
		break;
	case ACTION_FOCUS_IN:
		if (isDragging)
			setScrollOffset(_dragStartOffset - delta);
		else
			_selectedItem = index;
		invalidate();
		//CRLog::trace("list FOCUS IN");
		break;
	case ACTION_FOCUS_OUT:
		if (isDragging)
			setScrollOffset(_dragStartOffset - delta);
		else
			_selectedItem = -1;
		invalidate();
		return false; // to continue tracking
		//CRLog::trace("list FOCUS OUT");
		break;
	case ACTION_CANCEL:
		_selectedItem = -1;
		_dragStartOffset = NO_DRAG;
		setScrollOffset(_scrollOffset);
		//CRLog::trace("list CANCEL");
		break;
	case ACTION_MOVE:
		if (!isDragging && ((delta > DRAG_THRESHOLD) || (-delta > DRAG_THRESHOLD))) {
			_selectedItem = -1;
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
bool CRUIListWidget::startDragging(const CRUIMotionEvent * event, bool vertical) {
    CR_UNUSED2(event, vertical);
    return false;
}


//===================================================================================================

CRUIStringListAdapter::CRUIStringListAdapter() {
	_widget = new CRUITextWidget(lString16::empty_str);
	_widget->setStyle(lString8("LIST_ITEM"));
}
CRUIStringListAdapter::~CRUIStringListAdapter() {
	if (_widget) delete _widget;
}

CRUIStringListAdapter * CRUIStringListAdapter::setItems(lString16Collection & list) {
	_strings.clear();
	_strings.addAll(list);
	return this;
}

int CRUIStringListAdapter::getItemCount(CRUIListWidget * list)
{
    CR_UNUSED(list);
    return _strings.length();
}
CRUIWidget * CRUIStringListAdapter::getItemWidget(CRUIListWidget * list, int index)
{
    CR_UNUSED(list);
    _widget->setText(_strings[index]);
	return _widget;
}



