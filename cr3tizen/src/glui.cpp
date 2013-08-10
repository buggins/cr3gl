/*
 * glui.cpp
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */


#include "glui.h"

using namespace CRUI;


//==============================================================================================================
// CRUIStyle

CRUITheme * currentTheme = NULL;

CRUITheme::CRUITheme(lString8 name) : CRUIStyle(NULL, name), _map(100)
{
	_theme = this;
}

LVFontRef CRUITheme::getFontForSize(lUInt8 size) {
	LVFontRef res;
	switch (size) {
	case FONT_SIZE_XSMALL:
		res = _fontXSmall;
		break;
	case FONT_SIZE_SMALL:
		res = _fontSmall;
		break;
	case FONT_SIZE_LARGE:
		res = _fontLarge;
		break;
	case FONT_SIZE_XLARGE:
		res = _fontXLarge;
		break;
	default:
		res = _font;
		break;
	}
	if (res.isNull())
		res = _font;
	return res;
}

CRUIStyle * CRUITheme::setFontForSize(lUInt8 size, LVFontRef font) {
	switch (size) {
	case FONT_SIZE_XSMALL:
		_fontXSmall = font;
		break;
	case FONT_SIZE_SMALL:
		_fontSmall = font;
		break;
	case FONT_SIZE_LARGE:
		_fontLarge = font;
		break;
	case FONT_SIZE_XLARGE:
		_fontXLarge = font;
		break;
	default:
		_font = font;
		break;
	}
	return this;
}

void CRUITheme::registerStyle(CRUIStyle * style)
{
	_map.set(style->styleId(), style);
}

CRUIStyle * CRUITheme::find(const lString8 &id) {
	CRUIStyle * res = _map.get(id);
	if (res)
		return res;
	return this;
}


CRUIStyle::CRUIStyle(CRUITheme * theme, lString8 id, lUInt8 stateMask, lUInt8 stateValue) : _theme(theme), _styleId(id), _fontSize(FONT_SIZE_UNSPECIFIED), _textColor(PARENT_COLOR), _parentStyle(NULL), _stateMask(stateMask), _stateValue(stateValue) {

}

CRUIStyle::~CRUIStyle() {

}

CRUIStyle * CRUIStyle::addSubstyle(lString8 id, lUInt8 stateMask, lUInt8 stateValue) {
	CRUIStyle * child = new CRUIStyle(_theme, id, stateMask, stateValue);
	child->_parentStyle = this;
	_substyles.add(child);
	if (_theme)
		_theme->registerStyle(child);
	return child;
}

bool CRUIStyle::matchState(lUInt8 stateValue)
{
	return (_stateMask & stateValue) == (_stateMask & _stateValue);
}

CRUIStyle * CRUIStyle::find(lUInt8 stateValue)
{
	for (int i = 0; i < _substyles.length(); i++)
		if (_substyles[i]->matchState(stateValue))
			return _substyles[i];
	return this;
}

CRUIStyle * CRUIStyle::find(const lString8 &id) {
	if (_styleId == id)
		return this;
	for (int i = 0; i < _substyles.length(); i++) {
		CRUIStyle * res = _substyles[i]->find(id);
		if (res)
			return res;
	}
	return NULL;
}

CRUIImageRef CRUIStyle::getBackground() {
	if (!_background.isNull())
		return _background;
	if (_parentStyle)
		return _parentStyle->getBackground();
	return CRUIImageRef();
}

LVFontRef CRUIStyle::getFont() {
	if (!_font.isNull())
		return _font;
	if (_fontSize != FONT_SIZE_UNSPECIFIED)
		return _theme->getFontForSize(_fontSize);
	if (_parentStyle)
		return _parentStyle->getFont();
	return _theme->getFontForSize(FONT_SIZE_MEDIUM);
}

lUInt32 CRUIStyle::getTextColor() {
	if (_textColor != PARENT_COLOR)
		return _textColor;
	if (_parentStyle)
		return _parentStyle->getTextColor();
	return 0x000000; // BLACK
}


CRUIWidget::CRUIWidget() : _layoutWidth(WRAP_CONTENT), _layoutHeight(WRAP_CONTENT),
	_minWidth(UNSPECIFIED), _maxWidth(UNSPECIFIED), _minHeight(UNSPECIFIED), _maxHeight(UNSPECIFIED),
	_measuredWidth(0), _measuredHeight(0),
	_parent(NULL), _fontSize(FONT_SIZE_UNSPECIFIED), _textColor(PARENT_COLOR)
{

}

CRUIWidget::~CRUIWidget() {

}

/// measure dimensions
void CRUIWidget::defMeasure(int baseWidth, int baseHeight, int width, int height) {
	if (_layoutWidth == FILL_PARENT && baseWidth != UNSPECIFIED)
		_measuredWidth = baseWidth;
	else if (_layoutWidth != WRAP_CONTENT)
		_measuredWidth = _layoutWidth;
	else
		_measuredWidth = _margin.left + _margin.right + _padding.left + _padding.right + width;
	if (_layoutHeight == FILL_PARENT && baseHeight != UNSPECIFIED)
		_measuredHeight = baseHeight;
	else if (_layoutHeight != WRAP_CONTENT)
		_measuredHeight = _layoutHeight;
	else
		_measuredHeight = _margin.top + _margin.bottom + _padding.top + _padding.bottom + height;
	if (_minWidth != UNSPECIFIED && _measuredWidth < _minWidth)
		_measuredWidth = _minWidth;
	if (_maxWidth != UNSPECIFIED && _measuredWidth > _maxWidth)
		_measuredWidth = _maxWidth;
	if (_minHeight != UNSPECIFIED && _measuredHeight < _minHeight)
		_measuredHeight = _minHeight;
	if (_maxHeight != UNSPECIFIED && _measuredHeight > _maxHeight)
		_measuredHeight = _maxHeight;
	if (baseWidth != UNSPECIFIED && _measuredWidth > baseWidth)
		_measuredWidth = baseWidth;
	if (baseHeight != UNSPECIFIED && _measuredHeight > baseHeight)
		_measuredHeight = baseHeight;
}

CRUIStyle * CRUIWidget::getStyle() {
	return currentTheme->find(_styleId);
}

CRUIImageRef CRUIWidget::getBackground() {
	if (!_background.isNull())
		return _background;
	return getStyle()->getBackground();
}

LVFontRef CRUIWidget::getFont() {
	if (!_font.isNull())
		return _font;
	if (_fontSize != FONT_SIZE_UNSPECIFIED)
		return currentTheme->getFontForSize(_fontSize);
	return getStyle()->getFont();
}

lUInt32 CRUIWidget::getTextColor() {
	if (_textColor != PARENT_COLOR)
		return _textColor;
	return getStyle()->getTextColor();
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

/// draws widget with its children to specified surface
void CRUIWidget::draw(LVDrawBuf * buf) {
	CRUIImageRef background = getBackground();
	if (!background.isNull()) {
		lvRect rc = _pos;
		rc.shrinkBy(_margin);
		background->draw(buf, rc);
	}
}

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
	LVDrawStateSaver saver(*buf);
	CRUIWidget::draw(buf);
	lvRect rc = _pos;
	rc.shrinkBy(_margin);
	buf->SetClipRect(&rc);
	rc.shrinkBy(_padding);
	buf->SetTextColor(getTextColor());
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
	LVDrawStateSaver saver(*buf);
	CRUIWidget::draw(buf);
	lvRect rc = _pos;
	rc.shrinkBy(_margin);
	buf->SetClipRect(&rc);
	rc.shrinkBy(_padding);
	if (!_image.isNull())
		_image->draw(buf, rc);
}





// Vertical Layout
/// measure dimensions
void CRUILinearLayout::measure(int baseWidth, int baseHeight) {
	int maxw = baseWidth - (_margin.left + _margin.right + _padding.left + _padding.right);
	int maxh = baseHeight - (_margin.top + _margin.bottom + _padding.top + _padding.bottom);
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
	clientRc.shrinkBy(_margin);
	clientRc.shrinkBy(_padding);
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
	LVDrawStateSaver saver(*buf);
	CRUIWidget::draw(buf);
	lvRect rc = _pos;
	rc.shrinkBy(_margin);
	rc.shrinkBy(_padding);
	buf->SetClipRect(&rc);
	for (int i=0; i<getChildCount(); i++) {
		getChild(i)->draw(buf);
	}
}



CRUIButton::CRUIButton(lString16 text, CRUIImageRef image, bool vertical)
: CRUILinearLayout(vertical), _icon(NULL), _label(NULL)
{
	_styleId = "BUTTON";
	if (!image.isNull()) {
		_icon = new CRUIImageWidget(image);
		addChild(_icon);
	}
	if (!text.empty()) {
		_label = new CRUITextWidget(text);
		addChild(_label);
	}
}


//============================================================================
// Resource resolver

lString16 CRResourceResolver::resourceToFileName(const char * res) {
	lString16 path = Utf8ToUnicode(res);
	if (path.empty())
		return lString16::empty_str;
	if (path[0] == '#') {
		// by resource id: TODO
	} else if (path[0] == '/') {
		// absolute path
		if (LVFileExists(path)) {
			return path;
		}
	} else {
		// relative path
		for (int i=0; i<_dirList.length(); i++) {
			lString16 dir = Utf8ToUnicode(_dirList[i]);
			LVAppendPathDelimiter(dir);
			lString16 fn = dir + path;
			if (LVFileExists(fn)) {
				return fn;
			}
		}
	}
	return lString16::empty_str;
}

LVImageSourceRef CRResourceResolver::getImageSource(const char * name) {
	LVImageSourceRef res;
	lString16 path = resourceToFileName(name);
	if (path.empty())
		return res;
	LVStreamRef stream = LVOpenFileStream(path.c_str(), LVOM_READ);
	if (!stream.isNull())
		res = LVCreateStreamImageSource(stream);
	if (!res.isNull() && res->GetWidth() > 0 && res->GetHeight() > 0)
		return res;
	return LVImageSourceRef();
}

CRUIImageRef CRResourceResolver::getIcon(const char * name) {
	LVImageSourceRef src = getImageSource(name);
	if (!src.isNull())
		return CRUIImageRef(new CRUIBitmapImage(src));
	return CRUIImageRef();
}


CRResourceResolver * resourceResolver = NULL;

void LVCreateResourceResolver(lString8Collection & dirList) {
	if (resourceResolver != NULL)
		delete resourceResolver;
	resourceResolver = new CRResourceResolver(dirList);
}
