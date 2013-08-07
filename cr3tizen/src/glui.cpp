/*
 * glui.cpp
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */


#include "glui.h"

CRUIWidget::CRUIWidget() : _layoutWidth(FILL_PARENT), _layoutHeight(WRAP_CONTENT), _measuredWidth(0), _measuredHeight(0), _parent(NULL)
{

}

CRUIWidget::~CRUIWidget() {

}

/// measure dimensions
void CRUIWidget::measure(int baseWidth, int baseHeight) {
	_measuredWidth = _margin.left + _margin.right + _padding.left + _padding.right;
	_measuredHeight = _margin.top + _margin.bottom + _padding.top + _padding.bottom;
}

/// updates widget position based on specified rectangle
void CRUIWidget::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
}

/// draws widget with its children to specified surface
void CRUIWidget::draw(LVDrawBuf * buf) {
	if (!_background.isNull()) {
		lvRect rc = _pos;
		rc.shrinkBy(_margin);
		_background->draw(buf, rc);
	}
}

/// measure dimensions
void CRUITextWidget::measure(int baseWidth, int baseHeight) {
	int width = _font->getTextWidth(_text.c_str(), _text.length());
	if (_layoutWidth == FILL_PARENT)
		_measuredWidth = baseWidth;
	else if (_layoutWidth != WRAP_CONTENT)
		_measuredWidth = _layoutWidth;
	else
		_measuredWidth = _margin.left + _margin.right + _padding.left + _padding.right + width;
	int height = _font->getHeight();
	if (_layoutHeight == FILL_PARENT)
		_measuredHeight = baseHeight;
	else if (_layoutHeight != WRAP_CONTENT)
		_measuredHeight = _layoutHeight;
	else
		_measuredHeight = _margin.top + _margin.bottom + _padding.top + _padding.bottom + height;
}

/// updates widget position based on specified rectangle
void CRUITextWidget::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
}
/// draws widget with its children to specified surface
void CRUITextWidget::draw(LVDrawBuf * buf) {
	LVDrawStateSaver saver(*buf);
	CRUIWidget::draw(buf);
	lvRect rc = _pos;
	rc.shrinkBy(_margin);
	rc.shrinkBy(_padding);
	buf->SetClipRect(&rc);
	_font->DrawTextString(buf, rc.left, rc.top,
            _text.c_str(), _text.length(),
            '?');
}



// Vertical Layout
/// measure dimensions
void CRUIVerticalLayout::measure(int baseWidth, int baseHeight) {
	int maxw = baseWidth - (_margin.left + _margin.right + _padding.left + _padding.right);
	int maxh = baseHeight - (_margin.top + _margin.bottom + _padding.top + _padding.bottom);
	int biggestw = 0;
	int totalh = 0;

	for (int i=0; i<getChildCount(); i++) {
		CRUIWidget * child = getChild(i);
		child->measure(maxw, maxh);
		totalh += child->getMeasuredHeight();
		if (biggestw < child->getMeasuredWidth())
			biggestw = child->getMeasuredWidth();
	}
	if (biggestw > maxw)
		biggestw = maxw;
	if (totalh > maxh)
		totalh = maxh;
	if (_layoutWidth == FILL_PARENT)
		_measuredWidth = baseWidth;
	else if (_layoutWidth != WRAP_CONTENT)
		_measuredWidth = _layoutWidth;
	else
		_measuredWidth = biggestw + (_margin.left + _margin.right + _padding.left + _padding.right);
	if (_measuredWidth > baseWidth)
		_measuredWidth = baseWidth;
	if (_layoutHeight == FILL_PARENT)
		_measuredHeight = baseHeight;
	else if (_layoutHeight != WRAP_CONTENT)
		_measuredHeight = _layoutHeight;
	else
		_measuredHeight = _margin.top + _margin.bottom + _padding.top + _padding.bottom + totalh;
}

/// updates widget position based on specified rectangle
void CRUIVerticalLayout::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
	lvRect clientRc = _pos;
	clientRc.shrinkBy(_margin);
	clientRc.shrinkBy(_padding);
	lvRect childRc = clientRc;
	int y = childRc.top;
	for (int i=0; i<getChildCount(); i++) {
		CRUIWidget * child = getChild(i);
		childRc.top = y;
		childRc.bottom = y + child->getMeasuredHeight();
		child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
		y = childRc.bottom;
	}
}
/// draws widget with its children to specified surface
void CRUIVerticalLayout::draw(LVDrawBuf * buf) {
	CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	for (int i=0; i<getChildCount(); i++) {
		getChild(i)->draw(buf);
	}
}
