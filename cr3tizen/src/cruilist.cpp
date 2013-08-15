/*
 * cruilist.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruilist.h"

using namespace CRUI;

#define NO_DRAG (-1234567)
//===================================================================================================
// List

CRUIListWidget::CRUIListWidget(bool vertical, CRUIListAdapter * adapter)
: _vertical(vertical), _adapter(adapter),
  _ownAdapter(false), _scrollOffset(0),
  _maxScrollOffset(0), _topItem(0), _selectedItem(-1), _dragStartOffset(NO_DRAG)
{

}

/// measure dimensions
void CRUIListWidget::measure(int baseWidth, int baseHeight) {
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
		int biggestw = 0;
		int totalh = 0;

		for (int i=0; i<getItemCount(); i++) {
			CRUIWidget * item = getItemWidget(i);
			item->measure(maxw, maxh);
			lvPoint sz(item->getMeasuredWidth(), item->getMeasuredHeight());
			totalh += sz.y;
			if (biggestw < sz.x)
				biggestw = sz.x;
			_itemSizes.add(sz);
			if (i < getItemCount() - 1)
				totalh += delimiterSize;
		}
		if (biggestw > maxw)
			biggestw = maxw;
		if (totalh > maxh)
			totalh = maxh;
		defMeasure(baseWidth, baseHeight, biggestw, totalh);
	} else {
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

/// updates widget position based on specified rectangle
void CRUIListWidget::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
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
		for (int i=0; i<getItemCount() && i < _itemSizes.length(); i++) {
			lvPoint sz = _itemSizes[i];
			childRc.top = y;
			childRc.bottom = y + sz.y;
//			if (childRc.top > clientRc.bottom)
//				childRc.top = clientRc.bottom;
//			if (childRc.bottom > clientRc.bottom)
//				childRc.bottom = clientRc.bottom;
			_itemRects.add(childRc);
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
//			if (childRc.left > clientRc.right)
//				childRc.left = clientRc.right;
//			if (childRc.right > clientRc.right)
//				childRc.right = clientRc.right;
			_itemRects.add(childRc);
			x = childRc.right;
			if (i < getItemCount() - 1)
				x += delimiterSize;
		}
		_maxScrollOffset = x - x0 - winsize > 0 ? x - winsize : 0;
	}
}

/// draws widget with its children to specified surface
void CRUIListWidget::draw(LVDrawBuf * buf) {
	CRUIWidget::draw(buf);
	if (!_adapter)
		return;
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
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
		bool selected = _selectedItem == i;
		bool enabled = _adapter->isEnabled(i);
		CRUIWidget * item = getItemWidget(i);
		item->setState(selected ? STATE_FOCUSED : 0, STATE_FOCUSED);
		item->setState(!enabled ? STATE_DISABLED : 0, STATE_DISABLED);
		if (!item)
			continue;
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
		if (rc.intersects(childRc)) {
			item->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			item->draw(buf);
		}
		if (delimiterSize && i < getItemCount() - 1 && rc.intersects(delimiterRc))
			delimiter->draw(buf, delimiterRc);
	}
	//_scrollOffset++;
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

#define DRAG_THRESHOLD 5

/// motion event handler, returns true if it handled event
bool CRUIListWidget::onTouchEvent(const CRUIMotionEvent * event) {
	int action = event->getAction();
	//CRLog::trace("CRUIButton::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
	int index = itemFromPoint(event->getX(), event->getY());
	int dx = event->getX() - event->getStartX();
	int dy = event->getY() - event->getStartY();
	int delta = isVertical() ? dy : dx;
	bool isDragging = _dragStartOffset != NO_DRAG;
	CRLog::trace("CRUIButton::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
	switch (action) {
	case ACTION_DOWN:
		_selectedItem = index;
		invalidate();
		//CRLog::trace("button DOWN");
		break;
	case ACTION_UP:
		{
			_selectedItem = -1;
			invalidate();
			_dragStartOffset = NO_DRAG;
			bool isLong = event->getDownDuration() > 500; // 0.5 seconds threshold
//			if (isLong && onLongClickEvent())
//				return true;
//			onClickEvent();
			if (_scrollOffset < 0)
				_scrollOffset = 0;
			if (_scrollOffset > _maxScrollOffset)
				_scrollOffset = _maxScrollOffset;
		}
		// fire onclick
		//CRLog::trace("button UP");
		break;
	case ACTION_FOCUS_IN:
		if (isDragging)
			_scrollOffset = _dragStartOffset - delta;
		else
			_selectedItem = index;
		invalidate();
		//CRLog::trace("button FOCUS IN");
		break;
	case ACTION_FOCUS_OUT:
		if (isDragging)
			_scrollOffset = _dragStartOffset - delta;
		else
			_selectedItem = -1;
		invalidate();
		return false; // to continue tracking
		//CRLog::trace("button FOCUS OUT");
		break;
	case ACTION_CANCEL:
		_selectedItem = -1;
		_dragStartOffset = NO_DRAG;
		if (_scrollOffset < 0)
			_scrollOffset = 0;
		if (_scrollOffset > _maxScrollOffset)
			_scrollOffset = _maxScrollOffset;
		invalidate();
		//CRLog::trace("button CANCEL");
		break;
	case ACTION_MOVE:
		if (!isDragging && ((delta > DRAG_THRESHOLD) || (-delta > DRAG_THRESHOLD))) {
			_selectedItem = -1;
			_dragStartOffset = _scrollOffset;
			_scrollOffset = _dragStartOffset - delta;
		} else if (isDragging) {
			_scrollOffset = _dragStartOffset - delta;
		}
		invalidate();
		// ignore
		//CRLog::trace("button MOVE");
		break;
	default:
		return CRUIWidget::onTouchEvent(event);
	}
	return true;
}



//===================================================================================================

CRUIStringListAdapter::CRUIStringListAdapter() {
	_widget = new CRUITextWidget(lString16::empty_str);
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
	return _strings.length();
}
CRUIWidget * CRUIStringListAdapter::getItemWidget(CRUIListWidget * list, int index)
{
	_widget->setText(_strings[index]);
	return _widget;
}



