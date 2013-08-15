/*
 * cruilist.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruilist.h"

using namespace CRUI;

//===================================================================================================
// List

CRUIListWidget::CRUIListWidget(bool vertical, CRUIListAdapter * adapter) : _vertical(vertical), _adapter(adapter), _ownAdapter(false), _scrollOffset(0), _topItem(0) {

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
			y += delimiterSize;
		}
	} else {
		int x = childRc.left;
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
			x += delimiterSize;
		}
	}
}

/// draws widget with its children to specified surface
void CRUIListWidget::draw(LVDrawBuf * buf) {
	CRUIWidget::draw(buf);
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
		CRUIWidget * item = getItemWidget(i);
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
	_scrollOffset++;
}

lvPoint CRUIListWidget::getTileOffset() const {
	lvPoint res;
	if (_vertical)
		res.y = _scrollOffset;
	else
		res.x = _scrollOffset;
	return res;
}


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



