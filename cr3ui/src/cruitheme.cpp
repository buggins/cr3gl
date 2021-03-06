/*
 * cruitheme.cpp
 *
 *  Created on: Aug 12, 2013
 *      Author: Vadim
 */

#include "cruitheme.h"
#include "cruisettings.h"
#include "crui.h"
#include "cruiconfig.h"

using namespace CRUI;

//==============================================================================================================
// CRUIStyle

CRUITheme * currentTheme = NULL;

CRUITheme::CRUITheme(lString8 name) : CRUIStyle(NULL, name), _map(100), _colors(100), _fonts(100)
{
	_theme = this;
	_align = ALIGN_LEFT | ALIGN_TOP;
    setColor(lString8(COLOR_ID_ICON_COLOR_TRANSFORM_BRIGHTNESS), 0x948880);
    setColor(lString8(COLOR_ID_ICON_COLOR_TRANSFORM_CONTRAST), 0x404040);
    setColor(lString8(COLOR_ID_SLIDER_LINE_COLOR_OUTER), 0x000000);
    setColor(lString8(COLOR_ID_SLIDER_LINE_COLOR_INNER), 0xFFFFFF);
    setColor(lString8(COLOR_ID_SLIDER_POINTER_COLOR_OUTER), 0x000000);
    setColor(lString8(COLOR_ID_SLIDER_POINTER_COLOR_INNER), 0xFFFFFF);
    _fontWeight = FONT_WEIGHT_NORMAL;
    _fontStyle = FONT_STYLE_NORMAL;
    _fontSize = FONT_SIZE_MEDIUM;
}

int CRUITheme::getFontSize(lUInt8 size) {
    int sz = size;
    switch (size) {
    case FONT_SIZE_XXSMALL:
        sz = deviceInfo.shortSide / 45;
        break;
    case FONT_SIZE_XSMALL:
        sz = deviceInfo.shortSide / 38;
        break;
    case FONT_SIZE_SMALL:
        sz = deviceInfo.shortSide / 32;
        break;
    case FONT_SIZE_MEDIUM:
        sz = deviceInfo.shortSide / 28;
        break;
    case FONT_SIZE_LARGE:
        sz = deviceInfo.shortSide / 24;
        break;
    case FONT_SIZE_XLARGE:
        sz = deviceInfo.shortSide / 19;
        break;
    default:
        break; // do nothing
    }
    if (sz > crconfig.maxFontSize)
        sz = crconfig.maxFontSize;
    if (sz < crconfig.minFontSize)
        sz = crconfig.minFontSize;
    return sz;
}

LVFontRef CRUITheme::getFontForSize(lUInt8 size, lUInt8 weight, lUInt8 style) {
    LVFontRef res = _fonts.get(size);
    if (!res.isNull())
        return res;
    int sz = getFontSize(size);
    res = fontMan->GetFont(sz, weight == FONT_WEIGHT_NORMAL ? 400 : 800, style == FONT_STYLE_ITALIC, css_ff_sans_serif, crconfig.uiFontFace, 0);
    _fonts.set(sz, res);
	return res;
}

void CRUITheme::registerStyle(CRUIStyle * style)
{
	if (!style->styleId().empty() || _map.get(style->styleId()) == NULL)
		_map.set(style->styleId(), style);
}

void CRUITheme::remove(CRUIStyle * style) {
	for (int i = 0; i < _substyles.length(); i++) {
		if (_substyles[i] == style) {
			if (!style->getStyleId().empty())
				_map.remove(style->getStyleId());
			_substyles.remove(i);
			break;
		}
	}
	delete style;
}

CRUIStyle * CRUITheme::findStyle(const lString8 &id, bool defaultToTheme) {
	if (id.empty())
		return this;
	CRUIStyle * res = _map.get(id);
	if (res)
		return res;
	return defaultToTheme ? this : NULL;
}

class CRUIThemeParser : public LVXMLParserCallback
{
protected:
	lString8 basePath;
    CRUITheme * theme;
    CRUIStyle * style;
    lString8 colorId;
    lString8 colorValue;
    bool insideColor;
public:
    CRUIThemeParser(CRUITheme * _theme, lString8 _basePath) : basePath(_basePath), theme(_theme), style(NULL), insideColor(false)
    {
    }
    /// called on parsing end
    virtual void OnStop() { }
    /// called on opening tag end
    virtual void OnTagBody() {}
    /// called on opening tag
    virtual ldomNode * OnTagOpen( const lChar16 * nsname, const lChar16 * tagname)
    {
        CR_UNUSED(nsname);
        if (!lStr_cmp(tagname, "theme")) {
            style = theme;
        } else if (!lStr_cmp(tagname, "style")) {
            if (style)
                style = style->addSubstyle(lString8());
        } else if (!lStr_cmp(tagname, "color")) {
            colorId.clear();
            colorValue.clear();
            insideColor = true;
        }
        return NULL;
    }
    /// called on closing
    virtual void OnTagClose( const lChar16 * nsname, const lChar16 * tagname )
    {
        CR_UNUSED(nsname);
        if (!lStr_cmp(tagname, "theme")) {
            style = NULL;
        } else if (!lStr_cmp(tagname, "style")) {
            if (style && style != theme) {
                if (!style->getStateMask() && style->getStateValue())
                    style->setStateMask(style->getStateValue());
                if (!style->styleId().empty() && style != theme) {
                    theme->registerStyle(style);
                }
                style = style->getParentStyle();
                if (!style)
                    style = theme;
            }
        } else if (!lStr_cmp(tagname, "color")) {
            if (!colorId.empty() && !colorValue.empty()) {
                lUInt32 cl;
                if (CRPropAccessor::parseColor(Utf8ToUnicode(colorValue), cl))
                    theme->setColor(colorId, cl);
            }
            insideColor = false;
        }
    }

    static bool split(lString8 str, lString8Collection & result, lString8 delimiter = lString8(",")) {
        result.split(str, delimiter);
        for (int i = 0; i < result.length(); i++) {
            result[i] = result[i].trim();
        }
        return result.length() > 0;
    }

    static lUInt8 parseState(lString8 str) {
        lUInt8 res = 0;
        if (str.pos("disabled") >= 0)
            res |= STATE_DISABLED;
        if (str.pos("focused") >= 0)
            res |= STATE_FOCUSED;
        if (str.pos("pressed") >= 0)
            res |= STATE_PRESSED;
        return res;
    }

    static int parseLayoutParam(lString8 str) {
        if (str == "fillParent" || str == "FILL_PARENT")
            return FILL_PARENT;
        return WRAP_CONTENT;
    }

    static int parseSize(lString8 str) {
        int result = 0;
        if (str == "unspecified" || str.empty())
            return UNSPECIFIED;
        if (str.endsWith("pt")) {
            result = str.substr(0, str.length() - 2).atoi();
            return PT_TO_PX(result);
        }
        if (str.endsWith("mm")) {
            result = str.substr(0, str.length() - 2).atoi();
            return MM_TO_PX(result);
        }
        if (str.endsWith("px")) {
            return str.substr(0, str.length() - 2).atoi();
        }
        if (str.endsWith("%")) {
            return str.substr(0, str.length() - 1).atoi() * deviceInfo.shortSide / 100;
        }
        if (str.endsWith("%mi")) { // % of min item size
            return str.substr(0, str.length() - 1).atoi() * MIN_ITEM_PX / 100;
        }
        return str.atoi();
    }

    static int parseAlign(lString8 str) {
        int res = 0;
        lString8Collection list;
        split(str, list, lString8("|"));
        for (int i = 0; i < list.length(); i++) {
            lString8 s = list[i];
            if (s == "left")
                res |= ALIGN_LEFT;
            else if (s == "right")
                res |= ALIGN_RIGHT;
            else if (s == "hcenter")
                res |= ALIGN_HCENTER;
            else if (s == "top")
                res |= ALIGN_TOP;
            else if (s == "bottom")
                res |= ALIGN_BOTTOM;
            else if (s == "vcenter")
                res |= ALIGN_VCENTER;
            else if (s == "center")
                res |= ALIGN_CENTER;
        }
        return res;
    }

    /// called on element attribute
    virtual void OnAttribute( const lChar16 * nsname, const lChar16 * attrname, const lChar16 * attrvalue )
    {
        CR_UNUSED(nsname);
        if (!style)
            return;
        lString8 value = UnicodeToUtf8(attrvalue);
        lString16 value16(attrvalue);
        lString8Collection list;
        if (insideColor) {
            if (!lStr_cmp(attrname, "id")) {
                colorId = value;
            } else if (!lStr_cmp(attrname, "value")) {
                colorValue = value;
            }
            return;
        }
        if (!lStr_cmp(attrname, "id")) {
        	if (style != theme) {
        		CRUIStyle * existing = theme->findStyle(value, false);
        		if (existing) {
        			theme->remove(style);
        			style = existing;
        		}
        	}
        	if (style->getStyleId().empty())
        		style->setStyleId(value);
        } else if (!lStr_cmp(attrname, "base")) {
        	if (!value.empty() && (!style || style == theme) && value != theme->getStyleId()) {
        		// load base theme
				lString8 baseThemePath = basePath + value;
				LVAppendPathDelimiter(baseThemePath);
				baseThemePath += "cr3theme.xml";
				theme->loadFromFile(baseThemePath);
        	}
        } else if (!lStr_cmp(attrname, "state")) {
            style->setStateValue(parseState(value));
        } else if (!lStr_cmp(attrname, "stateMask")) {
            style->setStateMask(parseState(value));
        } else if (!lStr_cmp(attrname, "minWidth")) {
            style->setMinWidth(parseSize(value));
        } else if (!lStr_cmp(attrname, "maxWidth")) {
            style->setMaxWidth(parseSize(value));
        } else if (!lStr_cmp(attrname, "minHeight")) {
            style->setMinHeight(parseSize(value));
        } else if (!lStr_cmp(attrname, "maxHeight")) {
            style->setMaxHeight(parseSize(value));
        } else if (!lStr_cmp(attrname, "layoutParams")) {
            split(value, list);
            if (list.length() == 2) {
                style->setLayoutParams(parseLayoutParam(list[0]), parseLayoutParam(list[1]));
            }
        } else if (!lStr_cmp(attrname, "margin")) {
            split(value, list);
            if (list.length() == 4) {
                lvRect rc(parseSize(list[0]), parseSize(list[1]), parseSize(list[2]), parseSize(list[3]));
                style->setMargin(rc);
            } else if (list.length() == 1) {
                style->setMargin(parseSize(list[0]));
            }
        } else if (!lStr_cmp(attrname, "padding")) {
            split(value, list);
            if (list.length() == 4) {
                lvRect rc(parseSize(list[0]), parseSize(list[1]), parseSize(list[2]), parseSize(list[3]));
                style->setPadding(rc);
            } else if (list.length() == 1) {
                style->setPadding(parseSize(list[0]));
            }
        } else if (!lStr_cmp(attrname, "align")) {
            style->setAlign(parseAlign(value));
        } else if (!lStr_cmp(attrname, "fontSize")) {
            if (value == "xsmall")
                style->setFontSize(FONT_SIZE_XSMALL);
            else if (value == "xxsmall")
                style->setFontSize(FONT_SIZE_XXSMALL);
            else if (value == "small")
                style->setFontSize(FONT_SIZE_SMALL);
            else if (value == "medium")
                style->setFontSize(FONT_SIZE_MEDIUM);
            else if (value == "large")
                style->setFontSize(FONT_SIZE_LARGE);
            else if (value == "xlarge")
                style->setFontSize(FONT_SIZE_XLARGE);
            else {
                style->setFontSize(parseSize(value));
            }
        } else if (!lStr_cmp(attrname, "fontWeight")) {
            if (value == "normal")
                style->setFontWeight(FONT_WEIGHT_NORMAL);
            else if (value == "bold")
                style->setFontWeight(FONT_WEIGHT_BOLD);
        } else if (!lStr_cmp(attrname, "fontStyle")) {
            if (value == "normal")
                style->setFontStyle(FONT_STYLE_NORMAL);
            else if (value == "italic")
                style->setFontStyle(FONT_STYLE_ITALIC);
        } else if (!lStr_cmp(attrname, "textColor")) {
            lUInt32 cl;
            if (CRPropAccessor::parseColor(value16, cl))
                style->setTextColor(cl);
        } else if (!lStr_cmp(attrname, "background")) {
            lUInt32 cl;
            if (CRPropAccessor::parseColor(value16, cl)) {
                style->setBackground(cl);
                return;
            }
            bool tiled = false;
            if (value.endsWith("|tiled")) {
                value = value.substr(0, value.length() - 6); // remove |tiled
                tiled = true;
            }
            style->setBackground(value.c_str(), tiled);
        } else if (!lStr_cmp(attrname, "background2")) {
            lUInt32 cl;
            if (CRPropAccessor::parseColor(value16, cl)) {
                style->setBackground2(cl);
                return;
            }
            bool tiled = false;
            if (value.endsWith("|tiled")) {
                value = value.substr(0, value.length() - 6); // remove |tiled
                tiled = true;
            }
            style->setBackground2(value.c_str(), tiled);
        } else if (!lStr_cmp(attrname, "listDelimiterHorizontal")) {
            if (value.startsWith("#") || value.startsWith("0x")) { // color[,size]
                split(value,list);
                int sz = 1;
                if (list.length() > 1)
                    sz = parseSize(list[1]);
                if (sz < 1)
                    sz = 1;
                lUInt32 cl;
                if (CRPropAccessor::parseColor(Utf8ToUnicode(list[0]), cl)) {
                    style->setListDelimiterHorizontal(CRUIImageRef(new CRUISolidFillImage(cl, sz)));
                    return;
                }
            }
            style->setListDelimiterHorizontal(value.c_str());
        } else if (!lStr_cmp(attrname, "listDelimiterVertical")) {
            if (value.startsWith("#") || value.startsWith("0x")) { // color[,size]
                split(value,list);
                int sz = 1;
                if (list.length() > 1)
                    sz = parseSize(list[1]);
                if (sz < 1)
                    sz = 1;
                lUInt32 cl;
                if (CRPropAccessor::parseColor(Utf8ToUnicode(list[0]), cl)) {
                    style->setListDelimiterVertical(CRUIImageRef(new CRUISolidFillImage(cl, sz)));
                    return;
                }
            }
            style->setListDelimiterVertical(value.c_str());
        }
    }
    /// called on text
    virtual void OnText( const lChar16 * text, int len, lUInt32 flags )
    {
        CR_UNUSED3(text, len, flags);
    }
    /// add named BLOB data to document
    virtual bool OnBlob(lString16 name, const lUInt8 * data, int size) {
        CR_UNUSED3(name, data, size);
        return false;
    }

};



/// reads theme from XML file
bool CRUITheme::loadFromFile(lString8 fileName) {
    CRLog::info("Loading theme %s", fileName.c_str());

    LVStreamRef stream = LVOpenFileStream(fileName.c_str(), LVOM_READ);
    if (stream.isNull())
        return false;

    lString16 fn16 = Utf8ToUnicode(fileName);
    fn16 = LVExtractPath(fn16, true); // this theme dir
    fn16 = fn16.substr(0, fn16.length() - 1); // remove trailing /
    fn16 = LVExtractPath(fn16, true); // base theme dir
    CRUIThemeParser reader(this, UnicodeToUtf8(fn16));
    LVXMLParser parser( stream, &reader );
    if ( !parser.CheckFormat() )
        return false;
    if ( !parser.Parse() )
        return false;
    if (crconfig.einkMode) {

    }
    return _substyles.length() > 0;
}



CRUIStyle::CRUIStyle(CRUITheme * theme, lString8 id, lUInt8 stateMask, lUInt8 stateValue) :
		_theme(theme), _styleId(id),
		_backgroundColor(COLOR_NONE),
		_background2Color(COLOR_NONE),
        _fontSize(FONT_SIZE_UNSPECIFIED), _fontWeight(FONT_WEIGHT_UNSPECIFIED), _fontStyle(FONT_STYLE_UNSPECIFIED), _textColor(PARENT_COLOR), _parentStyle(NULL),
		_stateMask(stateMask), _stateValue(stateValue),
		_minWidth(UNSPECIFIED), _maxWidth(UNSPECIFIED), _minHeight(UNSPECIFIED), _maxHeight(UNSPECIFIED),
        _layoutWidth(WRAP_CONTENT), _layoutHeight(WRAP_CONTENT),
		_align(ALIGN_TOP_LEFT)
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
	if (!_stateValue)
		return false;
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

int CRUIStyle::getLayoutWidth() {
    return _layoutWidth;
}

int CRUIStyle::getLayoutHeight() {
    return _layoutHeight;
}

CRUIImageRef CRUIStyle::getListDelimiterHorizontal() {
	if (!_listDelimiterHorizontal.empty())
		return resourceResolver->getIcon(_listDelimiterHorizontal.c_str(), false);
    if (!_listDelimiterHorizontalImg.isNull())
        return _listDelimiterHorizontalImg;
    if (_parentStyle)
		return _parentStyle->getListDelimiterHorizontal();
	return CRUIImageRef();
}

CRUIImageRef CRUIStyle::getListDelimiterVertical() {
	if (!_listDelimiterVertical.empty())
		return resourceResolver->getIcon(_listDelimiterVertical.c_str(), false);
    if (!_listDelimiterVerticalImg.isNull())
        return _listDelimiterVerticalImg;
    if (_parentStyle)
		return _parentStyle->getListDelimiterVertical();
	return CRUIImageRef();
}

CRUIImageRef CRUIStyle::getBackground() {
	if (!_background.empty()) {
		return resourceResolver->getIcon(_background.c_str(), _backgroundTiled);
	}
	if (_backgroundColor != COLOR_NONE) {
		return CRUIImageRef(new CRUISolidFillImage(_backgroundColor));
	}
	if (_parentStyle && _stateValue)
		return _parentStyle->getBackground();
	return CRUIImageRef();
}

CRUIImageRef CRUIStyle::getBackground2() {
	if (!_background2.empty()) {
		return resourceResolver->getIcon(_background2.c_str(), _background2Tiled);
	}
	if (_background2Color != COLOR_NONE) {
		return CRUIImageRef(new CRUISolidFillImage(_background2Color));
	}
    if (_parentStyle && _stateValue)
        return _parentStyle->getBackground2();
    return CRUIImageRef();
}

lUInt8 CRUIStyle::getFontSize() {
    if (_fontSize == FONT_SIZE_UNSPECIFIED) {
        if (_parentStyle && _parentStyle != this)
            return _parentStyle->getFontSize();
        return _theme->getFontSize();
    }
    return _fontSize;
}

lUInt8 CRUIStyle::getFontStyle() {
    if (_fontStyle == FONT_STYLE_UNSPECIFIED) {
        if (_parentStyle && _parentStyle != this)
            return _parentStyle->getFontStyle();
        return _theme->getFontStyle();
    }
    return _fontStyle;
}

lUInt8 CRUIStyle::getFontWeight() {
    if (_fontWeight == FONT_WEIGHT_UNSPECIFIED) {
        if (_parentStyle && _parentStyle != this)
            return _parentStyle->getFontWeight();
        return _theme->getFontWeight();
    }
    return _fontWeight;
}

LVFontRef CRUIStyle::getFont() {
	if (!_font.isNull())
		return _font;
    return _theme->getFontForSize(getFontSize(), getFontWeight(), getFontStyle());
}

lUInt32 CRUIStyle::getTextColor() {
	if (_textColor != PARENT_COLOR)
		return _textColor;
	if (_parentStyle)
		return _parentStyle->getTextColor();
	return 0x000000; // BLACK
}


void CRUIBitmapImage::drawRotated(LVDrawBuf * buf, lvRect & rect, int angle) {
    buf->DrawRotated(_src, rect.left, rect.top, rect.width(), rect.height(), angle);
}

void CRUIBitmapImage::draw(LVDrawBuf * buf, lvRect & rect, int xoffset, int yoffset) {
    if (_tiled) {
        LVDrawStateSaver s(*buf);
        CR_UNUSED(s);
        lvRect clip;
        buf->GetClipRect(&clip);
        clip.intersect(rect);
        buf->SetClipRect(&clip);

		int w = originalWidth();
		int h = originalHeight();
		if (w <= 0 || h <= 0)
			return;
		xoffset %= w;
		yoffset %= h;
		xoffset = (w - xoffset) % w;
		yoffset = (h - yoffset) % h;
		if (xoffset)
			xoffset = w - xoffset;
		if (yoffset)
			yoffset = h - yoffset;
		lvRect rc2;
		for (int y = rect.top - yoffset; y < rect.bottom; y += h) {
			rc2.top = y;
			rc2.bottom = y + h;
            if (rc2.bottom > rect.bottom)
                rc2.bottom = rect.bottom;
			for (int x = rect.left - xoffset; x < rect.right; x += w) {
				rc2.left = x;
				rc2.right = x + w;
                if (rc2.right > rect.right)
                    rc2.right = rect.right;
                buf->Draw(_src, rc2.left, rc2.top, w, h, false);
			}
		}
	} else {
		buf->Draw(_src, rect.left, rect.top, rect.width(), rect.height(), false);
	}
}

CRUIBitmapImage::CRUIBitmapImage(LVImageSourceRef img, bool ninePatch, bool tiled) : _src(img), _tiled(tiled) {
	if (ninePatch) {
		img->DetectNinePatch();
	}
}

void CRUIDrawBufImage::draw(LVDrawBuf * buf, lvRect & rect, int xoffset, int yoffset) {
    CR_UNUSED2(xoffset, yoffset);
    lvRect rc = rect;
    buf->DrawRescaled(_src, rc.left, rc.top, rc.width(), rc.height(), 0);
}

CRUIDrawBufImage::CRUIDrawBufImage(LVDrawBuf * img) : _src(img) {

}



//============================================================================
// Resource resolver

lString16 CRResourceResolver::resourceToFileName(const char * res) {
	lString16 path = Utf8ToUnicode(res);
	if (path.empty())
		return lString16::empty_str;
	if (path[0] == '#') {
		// by resource id: TODO
    } else if (path[0] == '/' || path[0] == ASSET_PATH_PREFIX || (path[1] == ':' && path[2] == '\\')) { // / @ :\\ ...
		// absolute path
		if (LVFileExists(path)) {
			return path;
		}
	} else {
        int dirs = _dirList.length();
        int themedirs = _themeDirList.length();
        // relative path
        for (int i=0; i<dirs + themedirs; i++) {
            lString16 dir = Utf8ToUnicode(i < dirs ? _dirList[i] : _themeDirList[i - dirs]);
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

void CRResourceResolver::setThemeDirList(lString8Collection & dirList) {
    _imageSourceMap.clear();
    _iconMap.clear();
    _themeDirList.clear();
    _themeDirList.addAll(dirList);
}

void CRResourceResolver::setDirList(lString8Collection & dirList) {
    _imageSourceMap.clear();
    _iconMap.clear();
    _dirList.clear();
    _dirList.addAll(dirList);
}

void CRResourceResolver::clearImageCache() {
	CRLog::info("CRResourceResolver::clearImageCache()");
	_imageSourceMap.clear();
	_iconMap.clear();
}

LVImageSourceRef CRResourceResolver::applyColorTransform(LVImageSourceRef src) {
    if (src.isNull() || (_iconColorTransformAdd == 0x808080 && _iconColorTransformMultiply == 0x202020))
        return src;
    return LVCreateColorTransformImageSource(src, _iconColorTransformAdd, _iconColorTransformMultiply);
}

const CRUIBackgroundImageResource * CRResourceResolver::findBackground(lString8 id) {
    for (int i = 0; i < _backgroundResources.length(); i ++) {
        if (_backgroundResources[i]->getId() == id)
            return _backgroundResources[i];
    }
    return NULL;
}

CRUIImageRef CRResourceResolver::getBackgroundImage(lString8 id) {
    LVImageSourceRef img = getBackgroundImageSource(id);
    if (img.isNull())
        return CRUIImageRef();
    return CRUIImageRef(new CRUIBitmapImage(img, img->GetNinePatchInfo() != NULL, true));
}

CRUIImageRef CRResourceResolver::getBackgroundImage(CRPropRef props) {
    bool textureEnabled = props->getBoolDef(PROP_BACKGROUND_IMAGE_ENABLED, true);
    lString8 backgroundId = UnicodeToUtf8(props->getStringDef(PROP_BACKGROUND_IMAGE));
    const CRUIBackgroundImageResource * res = findBackground(backgroundId);
    if (textureEnabled && res) {
        LVImageSourceRef img = getImageSource(res->getFileName().c_str());
        if (!img.isNull()) {
//            return CRUIImageRef(new CRUIBitmapImage(img, res->isNinePatch(), res->isTiled()));
            lUInt32 brightness = props->getColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS, COLOR_TRANSFORM_BRIGHTNESS_NONE);
            lUInt32 contrast = props->getColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST, COLOR_TRANSFORM_CONTRAST_NONE);
            return CRUIImageRef(new CRUIBitmapImage(LVCreateColorTransformImageSource(img, brightness, contrast), res->isNinePatch(), res->isTiled()));
        }
    }
    lUInt32 bgColor = props->getColorDef(PROP_BACKGROUND_COLOR, 0);
    return CRUIImageRef(new CRUISolidFillImage(bgColor, 1));
}

LVImageSourceRef CRResourceResolver::getBackgroundImageSource(lString8 id) {
    const CRUIBackgroundImageResource * res = findBackground(id);
    if (res)
        return getImageSource(res->getFileName().c_str());
    return LVImageSourceRef();
}

LVImageSourceRef CRResourceResolver::getImageSource(const char * name) {
	lString8 name8(name);
	LVImageSourceRef res = _imageSourceMap.get(name8);
	if (!res.isNull())
		return res;
	lString16 path = resourceToFileName(name);
	if (path.empty()) {
		CRLog::error("Resource not found: %s", name);
		return res;
	}
	CRLog::debug("loading image from file %s", LCSTR(path));
	LVStreamRef stream = LVOpenFileStream(path.c_str(), LVOM_READ);
	if (!stream.isNull()) {
		res = LVCreateStreamImageSource(stream);
		if (res.isNull())
			CRLog::error("Error while decoding image from stream of size %d", (int)stream->GetSize());
	}
	if (!res.isNull() && res->GetWidth() > 0 && res->GetHeight() > 0) {
        bool isIconResource = path.pos("icons/") >= 0 || path.pos("folder_icons/") >= 0;
        if (isIconResource)
            res = applyColorTransform(res);
        if (path.pos(".9.") >= 0) {
			if (!res->DetectNinePatch()) {
				CRLog::error("NinePatch detection failed for %s", name);
			}
		}
		_imageSourceMap.set(name8, res);
		return res;
	}
	return LVImageSourceRef();
}

CRUIImageRef CRResourceResolver::getIcon(const char * name, bool tiled) {
	if (!name || !name[0])
		return CRUIImageRef();
	lString8 name8(name);
	if (tiled)
		name8 += "-tiled";
	CRUIImageRef res = _iconMap.get(name8);
	if (!res.isNull())
		return res;
	LVImageSourceRef src = getImageSource(name);
	if (!src.isNull()) {
		res = CRUIImageRef(new CRUIBitmapImage(src, src->GetNinePatchInfo() != NULL, tiled));
		_iconMap.set(name8, res);
		return res;
	}
	CRLog::error("failed to load resource %s", name);
	return CRUIImageRef();
}


CRResourceResolver * resourceResolver = NULL;

void LVCreateResourceResolver(lString8Collection & dirList) {
	if (resourceResolver != NULL)
		delete resourceResolver;
	resourceResolver = new CRResourceResolver(dirList);
}
