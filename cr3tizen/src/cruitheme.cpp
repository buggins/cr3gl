/*
 * cruitheme.cpp
 *
 *  Created on: Aug 12, 2013
 *      Author: Vadim
 */

#include "cruitheme.h"
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
	if (!style->styleId().empty() && _map.get(style->styleId()) == NULL)
		_map.set(style->styleId(), style);
}

CRUIStyle * CRUITheme::find(const lString8 &id) {
	CRUIStyle * res = _map.get(id);
	if (res)
		return res;
	return this;
}


CRUIStyle::CRUIStyle(CRUITheme * theme, lString8 id, lUInt8 stateMask, lUInt8 stateValue) :
		_theme(theme), _styleId(id),
		_fontSize(FONT_SIZE_UNSPECIFIED), _textColor(PARENT_COLOR), _parentStyle(NULL),
		_stateMask(stateMask), _stateValue(stateValue),
		_minWidth(UNSPECIFIED), _maxWidth(UNSPECIFIED), _minHeight(UNSPECIFIED), _maxHeight(UNSPECIFIED)
{

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

int CRUIStyle::getMinHeight()
{
	return _minHeight;
}
int CRUIStyle::getMaxHeight()
{
	return _maxHeight;
}
int CRUIStyle::getMaxWidth()
{
	return _maxWidth;
}
int CRUIStyle::getMinWidth()
{
	return _minWidth;
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


void CRUIBitmapImage::draw(LVDrawBuf * buf, lvRect & rect) {
	buf->Draw(_src, rect.left, rect.top, rect.width(), rect.height(), false);
}


CRUIBitmapImage::CRUIBitmapImage(LVImageSourceRef img, bool ninePatch) : _src(img) {
	if (ninePatch) {
		img->DetectNinePatch();
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
				//CRLog::debug("found resource file %s", LCSTR(fn));
				return fn;
			}
			//CRLog::debug("file %s not found", LCSTR(fn));
			if (fn.endsWith(".9") || (!fn.endsWith(".png") && !fn.endsWith(".jpeg") && !fn.endsWith(".jpg"))) {
				fn += L".png";
				if (LVFileExists(fn)) {
					//CRLog::debug("found resource file %s", LCSTR(fn));
					return fn;
				}
			}
		}
	}
	return lString16::empty_str;
}

LVImageSourceRef CRResourceResolver::getImageSource(const char * name) {
	LVImageSourceRef res;
	lString16 path = resourceToFileName(name);
	if (path.empty()) {
		CRLog::error("Resource not found: %s", name);
		return res;
	}
	CRLog::debug("loading image from file %s", LCSTR(path));
	LVStreamRef stream = LVOpenFileStream(path.c_str(), LVOM_READ);
	if (!stream.isNull())
		res = LVCreateStreamImageSource(stream);
	if (!res.isNull() && res->GetWidth() > 0 && res->GetHeight() > 0) {
		if (path.pos(".9.") >= 0) {
			if (!res->DetectNinePatch()) {
				CRLog::error("NinePatch detection failed for %s", name);
			}
		}
		return res;
	}
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
