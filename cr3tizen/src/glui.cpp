/*
 * glui.cpp
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */


#include "glui.h"

CRUIWidget::CRUIWidget() : _width(FILL_PARENT), _height(WRAP_CONTENT), _measuredWidth(0), _measuredHeight(0), _parent(NULL)
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
	if (_width == FILL_PARENT)
		_measuredWidth = baseWidth;
	else if (_width != WRAP_CONTENT)
		_measuredWidth = _width;
	else
		_measuredWidth = _margin.left + _margin.right + _padding.left + _padding.right + width;
	int height = _font->getHeight();
	if (_height == FILL_PARENT)
		_measuredHeight = baseHeight;
	else if (_height != WRAP_CONTENT)
		_measuredHeight = _height;
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
