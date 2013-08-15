/*
 * glui.cpp
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */


#include "crui.h"

using namespace CRUI;


int CRUIMotionEvent::findPointerId(int pointerId) {
	for (int i=0; i<_data.length(); i++)
		if (_data[i]->getPointerId() == pointerId)
			return i;
	return -1;
}

CRUIMotionEventItem::CRUIMotionEventItem(const CRUIMotionEventItem * previous, lUInt64 pointerId, int action, int x, int y, lUInt64 ts)
: _pointerId(pointerId),
  _action(action),
  _x(x),
  _y(y),
  _startX(previous ? previous->getStartX() : x),
  _startY(previous ? previous->getStartY() : y),
  _ts(ts),
  _downTs(previous ? previous->getDownEventTimestamp() : ts),
  _isOutside(previous ? previous->isOutside() : false),
  _widget(previous ? previous->getWidget() : NULL)
{
	//
}


CRUIEventManager::CRUIEventManager() : _rootWidget(NULL), _lastTouchEvent(NULL) {

}

bool CRUIEventManager::dispatchTouchEvent(CRUIWidget * widget, CRUIMotionEvent * event) {
	if (!widget)
		return false; // invalid widget
	if (!event->count()) // invalid event
		return false;
	bool pointInside = widget->isPointInside(event->getX(), event->getY());
	if (!pointInside && !event->getWidget())
		return false;
	int action = event->getAction();
	if (!event->getWidget()) { // if not not assigned on widget
		for (int i=0; i<widget->getChildCount(); i++) {
			CRUIWidget * child = widget->getChild(i);
			if (dispatchTouchEvent(child, event)) {
				if (action == ACTION_DOWN)
					event->setWidget(widget);
				return true;
			}
		}
	}
	bool oldIsOutside = event->get()->isOutside();
	if (action == ACTION_UP && !pointInside) {
		// if UP is outside of control - change to CANCEL
		event->changeAction(ACTION_CANCEL);
		action = ACTION_CANCEL;
	}
	if (oldIsOutside != !pointInside) {
		// in/out
		if (!pointInside && action == ACTION_MOVE) {// moving outside - already sent FOCUS_OUT, now just ignore
			// converting to FOCUS_OUT
			event->changeAction(ACTION_FOCUS_OUT);
			action = ACTION_FOCUS_OUT;
		}
		if (pointInside && action == ACTION_MOVE) {
			/// point is back in - send FOCUS IN
			event->changeAction(ACTION_FOCUS_IN);
			action = ACTION_FOCUS_IN;
		}
	} else {
		if (!pointInside && action == ACTION_MOVE) // moving outside - already sent FOCUS_OUT with response true, no tracking, now just ignore
			return false;
	}
	bool res = widget->onTouchEvent(event);
	if (!res && action == ACTION_FOCUS_OUT) // if FOCUS_OUT returned true - stop tracking movements outside
		event->_data[0]->_isOutside = true;
	if (pointInside)
		event->_data[0]->_isOutside = false;
	return res;
}

bool CRUIEventManager::dispatchTouchEvent(CRUIMotionEvent * event) {
	if (_rootWidget == NULL) {
		CRLog::error("Cannot dispatch touch event: no root widget");
		return false;
	}
	CRUIWidget * widget = event->getWidget();
	if (widget) {
		// event is tracked by widget
		if (!_rootWidget->isChild(widget))
			return false;
		return dispatchTouchEvent(widget, event);
	}
	if (event->getAction() != ACTION_DOWN) // skip non tracked event - only DOWN allowed
		return false;
	return dispatchTouchEvent(_rootWidget, event);
}



CRUIWidget::CRUIWidget() : _state(0), _margin(UNSPECIFIED, UNSPECIFIED, UNSPECIFIED, UNSPECIFIED), _padding(UNSPECIFIED, UNSPECIFIED, UNSPECIFIED, UNSPECIFIED), _layoutWidth(WRAP_CONTENT), _layoutHeight(WRAP_CONTENT),
	_minWidth(UNSPECIFIED), _maxWidth(UNSPECIFIED), _minHeight(UNSPECIFIED), _maxHeight(UNSPECIFIED),
	_measuredWidth(0), _measuredHeight(0),
	_parent(NULL), _fontSize(FONT_SIZE_UNSPECIFIED), _textColor(PARENT_COLOR),
	_align(0)
{

}

CRUIWidget::~CRUIWidget() {

}

void CRUIWidget::applyAlign(lvRect & rc, int contentWidth, int contentHeight)
{
	int extraw = rc.width() - contentWidth;
	int extrah = rc.height() - contentHeight;
	// center vertical
	lUInt32 valign = getVAlign();
	lUInt32 halign = getHAlign();
	if (extrah > 0) {
		if (valign == ALIGN_VCENTER)
			rc.top += extrah / 2;
		if (valign == ALIGN_BOTTOM)
			rc.top += extrah;
	}
	if (extraw > 0) {
		if (halign == ALIGN_RIGHT)
			rc.left += extraw;
		if (halign == ALIGN_HCENTER)
			rc.left += extraw / 2;
	}
}

/// returns true if point is inside control (excluding margins)
bool CRUIWidget::isPointInside(int x, int y) {
	lvRect rc = _pos;
	applyMargin(rc);
	lvPoint pt(x,y);
	return rc.isPointInside(pt);
}

/// returns true if widget is child of this
bool CRUIWidget::isChild(CRUIWidget * widget) {
	if (widget == this)
		return true;
	for (int i=0; i<getChildCount(); i++)
		if (getChild(i)->isChild(widget))
			return true;
	return false;
}

/// measure dimensions
void CRUIWidget::defMeasure(int baseWidth, int baseHeight, int width, int height)
{
	lvRect padding;
	getPadding(padding);
	lvRect margin = getMargin();
	int minWidth = getMinWidth();
	int maxWidth = getMaxWidth();
	int minHeight = getMinHeight();
	int maxHeight = getMaxHeight();
	if (_layoutWidth == FILL_PARENT && baseWidth != UNSPECIFIED)
		_measuredWidth = baseWidth;
	else if (_layoutWidth != WRAP_CONTENT)
		_measuredWidth = _layoutWidth;
	else
		_measuredWidth = margin.left + margin.right + padding.left + padding.right + width;
	if (_layoutHeight == FILL_PARENT && baseHeight != UNSPECIFIED)
		_measuredHeight = baseHeight;
	else if (_layoutHeight != WRAP_CONTENT)
		_measuredHeight = _layoutHeight;
	else
		_measuredHeight = margin.top + margin.bottom + padding.top + padding.bottom + height;
	if (minWidth != UNSPECIFIED && _measuredWidth < minWidth)
		_measuredWidth = minWidth;
	if (maxWidth != UNSPECIFIED && _measuredWidth > maxWidth)
		_measuredWidth = maxWidth;
	if (minHeight != UNSPECIFIED && _measuredHeight < minHeight)
		_measuredHeight = minHeight;
	if (maxHeight != UNSPECIFIED && _measuredHeight > maxHeight)
		_measuredHeight = maxHeight;
	if (baseWidth != UNSPECIFIED && _measuredWidth > baseWidth)
		_measuredWidth = baseWidth;
	if (baseHeight != UNSPECIFIED && _measuredHeight > baseHeight)
		_measuredHeight = baseHeight;
}

CRUIStyle * CRUIWidget::getStyle(bool forState) {
	CRUIStyle * res = currentTheme->find(_styleId);
	if (!res)
		return NULL;
	if (forState && getState())
		res = res->find(getState());
	return res;
}

/// motion event handler, returns true if it handled event
bool CRUIWidget::onTouchEvent(const CRUIMotionEvent * event) {
	if (_onTouchListener != NULL)
		return _onTouchListener->onTouch(this, event);
	return false;
}

CRUIOnTouchEventListener * CRUIWidget::setOnTouchListener(CRUIOnTouchEventListener * listener)
{
	CRUIOnTouchEventListener * old = _onTouchListener;
	_onTouchListener = listener;
	return old;
}

lUInt32 CRUIWidget::getAlign()
{
	lUInt32 valign = _align & ALIGN_MASK_VERTICAL;
	lUInt32 halign = _align & ALIGN_MASK_HORIZONTAL;
	CRUIStyle * style = getStyle(false);
	lUInt32 salign = 0;
	if (style)
		salign = style->getAlign();
	if (halign == ALIGN_UNSPECIFIED) {
		halign = salign & ALIGN_MASK_HORIZONTAL;
	}
	if (valign == ALIGN_UNSPECIFIED) {
		valign = salign & ALIGN_MASK_VERTICAL;
	}
	return halign | valign;
}

void CRUIWidget::getMargin(lvRect & rc) {
	rc = getMargin();
}
void CRUIWidget::getPadding(lvRect & rc) {
	rc = getPadding();
	CRUIImageRef bg = getBackground();
	if (!bg.isNull()) {
		if (bg->getNinePatchInfo())
			bg->getNinePatchInfo()->applyPadding(rc);
	}
}

void CRUIWidget::applyPadding(lvRect & rc)
{
	lvRect padding = getPadding();
	CRUIImageRef bg = getBackground();
	if (!bg.isNull()) {
		if (bg->getNinePatchInfo())
			bg->getNinePatchInfo()->applyPadding(padding);
	}
	rc.shrinkBy(padding);
}
void CRUIWidget::applyMargin(lvRect & rc)
{
	rc.shrinkBy(getMargin());
}

const lvRect & CRUIWidget::getPadding() {
	if (_padding.left != UNSPECIFIED)
		return _padding;
	else
		return getStyle()->getPadding();
}

const lvRect & CRUIWidget::getMargin() {
	if (_margin.left != UNSPECIFIED)
		return _margin;
	else
		return getStyle()->getMargin();
}

int CRUIWidget::getMinHeight()
{
	if (_minHeight != UNSPECIFIED)
		return _minHeight;
	else
		return getStyle()->getMinHeight();
}

int CRUIWidget::getMaxHeight()
{
	if (_maxHeight != UNSPECIFIED)
		return _maxHeight;
	else
		return getStyle()->getMaxHeight();
}

int CRUIWidget::getMaxWidth()
{
	if (_maxWidth != UNSPECIFIED)
		return _maxWidth;
	else
		return getStyle()->getMaxWidth();
}

int CRUIWidget::getMinWidth()
{
	if (_minWidth != UNSPECIFIED)
		return _minWidth;
	else
		return getStyle()->getMinWidth();
}

CRUIImageRef CRUIWidget::getBackground() {
	if (!_background.isNull())
		return _background;
	return getStyle(true)->getBackground();
}

LVFontRef CRUIWidget::getFont() {
	if (!_font.isNull())
		return _font;
	if (_fontSize == FONT_USE_PARENT) {
		if (_parent)
			return _parent->getFont();
	} else if (_fontSize != FONT_SIZE_UNSPECIFIED)
		return currentTheme->getFontForSize(_fontSize);
	return getStyle()->getFont();
}

lUInt32 CRUIWidget::getTextColor() {
	if (_textColor != PARENT_COLOR)
		return _textColor;
	return getStyle(true)->getTextColor();
}

/// measure dimensions
void CRUIWidget::measure(int baseWidth, int baseHeight) {
	defMeasure(baseWidth, baseHeight, 0, 0);
}

/// updates widget position based on specified rectangle
void CRUIWidget::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
}

bool CRUIWidget::setClipRect(LVDrawBuf * buf, lvRect & rc) {
	lvRect clip;
	buf->GetClipRect(&clip);
	lvRect rc2 = rc;
	rc2.intersect(clip);
	buf->SetClipRect(&rc2);
	return !rc2.isEmpty();
}

/// draws widget with its children to specified surface
void CRUIWidget::draw(LVDrawBuf * buf) {
	CRUIImageRef background = getBackground();
	if (!background.isNull()) {
		LVDrawStateSaver saver(*buf);
		lvRect rc = _pos;
		applyMargin(rc);
		setClipRect(buf, rc);
		if (background->isTiled()) {
			lvPoint offset = getTileOffset();
			background->draw(buf, rc, offset.x, offset.y);
		} else {
			background->draw(buf, rc);
		}
	}
}

CRUIWidget * CRUIWidget::childById(const lString8 & id) {
	if (_id == id)
		return this;
	for (int i = 0; i < getChildCount(); i++) {
		CRUIWidget * item = getChild(i)->childById(id);
		if (item)
			return item;
	}
	return NULL;
}

CRUIWidget * CRUIWidget::childById(const char * id) {
	return childById(lString8(id));
}




//=================================================================================

/// measure dimensions
void CRUITextWidget::measure(int baseWidth, int baseHeight) {
	int width = getFont()->getTextWidth(_text.c_str(), _text.length());
	int height = getFont()->getHeight();
	defMeasure(baseWidth, baseHeight, width, height);
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
	CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	buf->SetTextColor(getTextColor());
	int width = getFont()->getTextWidth(_text.c_str(), _text.length());
	int height = getFont()->getHeight();
	applyAlign(rc, width, height);
	getFont()->DrawTextString(buf, rc.left, rc.top,
            _text.c_str(), _text.length(),
            '?');
}



//=============================================================================
// Image Widget
/// measure dimensions
void CRUIImageWidget::measure(int baseWidth, int baseHeight) {
	int width = !_image ? 0 : _image->originalWidth();
	int height = !_image ? 0 : _image->originalHeight();
	defMeasure(baseWidth, baseHeight, width, height);
}

/// updates widget position based on specified rectangle
void CRUIImageWidget::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
}
/// draws widget with its children to specified surface
void CRUIImageWidget::draw(LVDrawBuf * buf) {
	CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	if (!_image.isNull()) {
		applyAlign(rc, _image->originalWidth(), _image->originalHeight());
		// don't scale
		rc.right = rc.left + _image->originalWidth();
		rc.bottom = rc.top + _image->originalHeight();
		// draw
		_image->draw(buf, rc);
	}
}





// Vertical Layout
/// measure dimensions
void CRUILinearLayout::measure(int baseWidth, int baseHeight) {
	lvRect padding;
	getPadding(padding);
	lvRect margin = getMargin();
	int maxw = baseWidth - (margin.left + margin.right + padding.left + padding.right);
	int maxh = baseHeight - (margin.top + margin.bottom + padding.top + padding.bottom);
	if (_isVertical) {
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
		defMeasure(baseWidth, baseHeight, biggestw, totalh);
	} else {
		int biggesth = 0;
		int totalw = 0;

		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			child->measure(maxw, maxh);
			totalw += child->getMeasuredWidth();
			if (biggesth < child->getMeasuredHeight())
				biggesth = child->getMeasuredHeight();
		}
		if (biggesth > maxh)
			biggesth = maxh;
		if (totalw > maxw)
			totalw = maxw;
		defMeasure(baseWidth, baseHeight, totalw, biggesth);
	}
}

/// updates widget position based on specified rectangle
void CRUILinearLayout::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
	lvRect clientRc = _pos;
	applyMargin(clientRc);
	applyPadding(clientRc);
	lvRect childRc = clientRc;
	if (_isVertical) {
		int y = childRc.top;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			childRc.top = y;
			childRc.bottom = y + child->getMeasuredHeight();
			if (childRc.top > clientRc.bottom)
				childRc.top = clientRc.bottom;
			if (childRc.bottom > clientRc.bottom)
				childRc.bottom = clientRc.bottom;
			child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			y = childRc.bottom;
		}
	} else {
		int x = childRc.left;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			childRc.left = x;
			childRc.right = x + child->getMeasuredWidth();
			if (childRc.left > clientRc.right)
				childRc.left = clientRc.right;
			if (childRc.right > clientRc.right)
				childRc.right = clientRc.right;
			child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			x = childRc.right;
		}
	}
}



//=======================================================================
// Container Widget

/// draws widget with its children to specified surface
void CRUIContainerWidget::draw(LVDrawBuf * buf) {
	CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	for (int i=0; i<getChildCount(); i++) {
		getChild(i)->draw(buf);
	}
}


CRUIButton::CRUIButton(lString16 text, const char * imageRes, bool vertical) : CRUILinearLayout(vertical), _icon(NULL), _label(NULL) {
	CRUIImageRef image;
	if (imageRes && imageRes[0])
		image = resourceResolver->getIcon(imageRes);
	init(text, image, vertical);
}

void CRUIButton::init(lString16 text, CRUIImageRef image, bool vertical) {
	_styleId = "BUTTON";
	if (!image.isNull()) {
		_icon = new CRUIImageWidget(image);
		if (vertical)
			_icon->setAlign(ALIGN_HCENTER | ALIGN_TOP);
		else
			_icon->setAlign(ALIGN_LEFT | ALIGN_VCENTER);
		addChild(_icon);
	}
	if (!text.empty()) {
		_label = new CRUITextWidget(text);
		if (vertical)
			_label->setAlign(ALIGN_TOP | ALIGN_HCENTER);
		else
			_label->setAlign(ALIGN_LEFT | ALIGN_VCENTER);
		if (!image.isNull()) {
			lvRect padding;
			getPadding(padding);
			lvRect lblPadding;
			_label->getPadding(lblPadding);
			if (vertical) {
				if (!lblPadding.top)
					lblPadding.top = padding.top * 2 / 3;
			} else {
				if (!lblPadding.left)
					lblPadding.left = padding.left * 2 / 3;
			}
			_label->setPadding(lblPadding);
		}
		addChild(_label);
	}
}

CRUIButton::CRUIButton(lString16 text, CRUIImageRef image, bool vertical)
: CRUILinearLayout(vertical), _icon(NULL), _label(NULL)
{
	init(text, image, vertical);
}




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
