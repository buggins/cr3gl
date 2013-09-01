/*
 * cruiwidget.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruiwidget.h"

using namespace CRUI;


CRUIWidget::CRUIWidget() : _state(0), _margin(UNSPECIFIED, UNSPECIFIED, UNSPECIFIED, UNSPECIFIED),
		_padding(UNSPECIFIED, UNSPECIFIED, UNSPECIFIED, UNSPECIFIED),
		_layoutWidth(WRAP_CONTENT), _layoutHeight(WRAP_CONTENT),
	_minWidth(UNSPECIFIED), _maxWidth(UNSPECIFIED), _minHeight(UNSPECIFIED), _maxHeight(UNSPECIFIED),
	_measuredWidth(0), _measuredHeight(0),
	_parent(NULL),
	_layoutRequested(true),
	_drawRequested(true),
	_fontSize(FONT_SIZE_UNSPECIFIED), _textColor(PARENT_COLOR),
	_align(0), _layoutWeight(1),
	_onTouchListener(NULL), _onClickListener(NULL), _onLongClickListener(NULL)
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
	if (forState && getState())
		res = res->find(getState());
	return res;
}

/// motion event handler - before children, returns true if it handled event
bool CRUIWidget::onTouchEventPreProcess(const CRUIMotionEvent * event) {
    return false;
}

/// motion event handler, returns true if it handled event
bool CRUIWidget::onTouchEvent(const CRUIMotionEvent * event) {
	if (_onTouchListener != NULL)
		return _onTouchListener->onTouch(this, event);
	return false;
}

/// click handler, returns true if it handled event
bool CRUIWidget::onClickEvent()
{
	if (_onClickListener != NULL)
		return _onClickListener->onClick(this);
	return false;
}

/// long click handler, returns true if it handled event
bool CRUIWidget::onLongClickEvent()
{
	if (_onLongClickListener != NULL)
		return _onLongClickListener->onLongClick(this);
	return false;
}


CRUIOnTouchEventListener * CRUIWidget::setOnTouchListener(CRUIOnTouchEventListener * listener)
{
	CRUIOnTouchEventListener * old = _onTouchListener;
	_onTouchListener = listener;
	return old;
}

CRUIOnClickListener * CRUIWidget::setOnClickListener(CRUIOnClickListener * listener)
{
	CRUIOnClickListener * old = _onClickListener;
	_onClickListener = listener;
	return old;
}

CRUIOnLongClickListener * CRUIWidget::setOnLongClickListener(CRUIOnLongClickListener * listener)
{
	CRUIOnLongClickListener * old = _onLongClickListener;
	_onLongClickListener = listener;
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
	_drawRequested = false;
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

void CRUIWidget::animate(lUInt64 millisPassed)
{
    for (int i = 0; i < getChildCount(); i++)
        getChild(i)->animate(millisPassed);
}

static void checkUpdateOptions(CRUIWidget * widget, bool & needLayout, bool & needRedraw, bool & animating) {
	if (widget->isLayoutRequested())
		needLayout = true;
	if (widget->isDrawRequested())
		needRedraw = true;
    if (widget->isAnimating())
        animating = true;
    for (int i = 0; i < widget->getChildCount(); i++)
        checkUpdateOptions(widget->getChild(i), needLayout, needRedraw, animating);
}
void CRUICheckUpdateOptions(CRUIWidget * widget, bool & needLayout, bool & needRedraw, bool & animating) {
	needLayout = false;
	needRedraw = false;
    animating = false;
    checkUpdateOptions(widget, needLayout, needRedraw, animating);
    if (needLayout || animating)
		needRedraw = true;
}

