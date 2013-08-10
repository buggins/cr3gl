/*
 * glui.cpp
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */


#include "glui.h"

CRUIWidget::CRUIWidget() : _layoutWidth(WRAP_CONTENT), _layoutHeight(WRAP_CONTENT),
	_measuredWidth(0), _measuredHeight(0),
	_minWidth(UNSPECIFIED), _maxWidth(UNSPECIFIED), _minHeight(UNSPECIFIED), _maxHeight(UNSPECIFIED),
	_parent(NULL)
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
	if (!_background.isNull()) {
		lvRect rc = _pos;
		rc.shrinkBy(_margin);
		_background->draw(buf, rc);
	}
}

/// measure dimensions
void CRUITextWidget::measure(int baseWidth, int baseHeight) {
	int width = _font->getTextWidth(_text.c_str(), _text.length());
	int height = _font->getHeight();
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
	_font->DrawTextString(buf, rc.left, rc.top,
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
	_image->draw(buf, rc);
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
	defMeasure(baseWidth, baseHeight, biggestw, totalh);
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
		if (childRc.top > clientRc.bottom)
			childRc.top = clientRc.bottom;
		if (childRc.bottom > clientRc.bottom)
			childRc.bottom = clientRc.bottom;
		child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
		y = childRc.bottom;
	}
}
/// draws widget with its children to specified surface
void CRUIVerticalLayout::draw(LVDrawBuf * buf) {
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


//======================================================================================
// Horizontal Layout
/// measure dimensions
void CRUIHorizontalLayout::measure(int baseWidth, int baseHeight) {
	int maxw = baseWidth - (_margin.left + _margin.right + _padding.left + _padding.right);
	int maxh = baseHeight - (_margin.top + _margin.bottom + _padding.top + _padding.bottom);
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

/// updates widget position based on specified rectangle
void CRUIHorizontalLayout::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
	lvRect clientRc = _pos;
	clientRc.shrinkBy(_margin);
	clientRc.shrinkBy(_padding);
	lvRect childRc = clientRc;
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
/// draws widget with its children to specified surface
void CRUIHorizontalLayout::draw(LVDrawBuf * buf) {
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
