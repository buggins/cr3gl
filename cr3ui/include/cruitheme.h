/*
 * cruitheme.h
 *
 *  Created on: Aug 12, 2013
 *      Author: Vadim
 */

#ifndef CRUITHEME_H_
#define CRUITHEME_H_

#include <crengine.h>
#include <cri18n.h>

class CRUIImage {
public:
	virtual int originalWidth() { return 1; }
	virtual int originalHeight() { return 1; }
	virtual const CR9PatchInfo * getNinePatchInfo() { return NULL; }
	virtual bool isTiled() { return false; }
    virtual void draw(LVDrawBuf * buf, lvRect & rect, int xoffset = 0, int yoffset = 0) = 0;
    virtual void drawRotated(LVDrawBuf * buf, lvRect & rect, int angle) { draw(buf, rect); CR_UNUSED(angle); }
    virtual ~CRUIImage() { }
};
typedef LVRef<CRUIImage> CRUIImageRef;

class CRUISolidFillImage : public CRUIImage {
	lUInt32 _color;
    int _size;
public:
    virtual int originalWidth() { return _size; }
    virtual int originalHeight() { return _size; }
    virtual void draw(LVDrawBuf * buf, lvRect & rect, int xoffset = 0, int yoffset = 0) {
        buf->FillRect(rect, _color); CR_UNUSED2(xoffset, yoffset);
    }
    CRUISolidFillImage(lUInt32 color, int size = 1) : _color(color), _size(size) { }
	virtual ~CRUISolidFillImage() { }
};

class CRUIBitmapImage : public CRUIImage {
protected:
	LVImageSourceRef _src;
	bool _tiled;
public:
	virtual const CR9PatchInfo * getNinePatchInfo() { return _src->GetNinePatchInfo(); }
	virtual int originalWidth() { return _src->GetWidth() - (_src->GetNinePatchInfo() ? 2 : 0); }
	virtual int originalHeight() { return _src->GetHeight() - (_src->GetNinePatchInfo() ? 2 : 0); }
	virtual bool isTiled() { return _tiled; }
    virtual void draw(LVDrawBuf * buf, lvRect & rect, int xoffset = 0, int yoffset = 0);
    virtual void drawRotated(LVDrawBuf * buf, lvRect & rect, int angle);
    CRUIBitmapImage(LVImageSourceRef img, bool ninePatch = false, bool tiled = false);
	virtual ~CRUIBitmapImage() { }
};

class CRUIDrawBufImage : public CRUIImage {
protected:
    LVDrawBuf * _src;
public:
    virtual int originalWidth() { return _src->GetWidth(); }
    virtual int originalHeight() { return _src->GetHeight(); }
    virtual void draw(LVDrawBuf * buf, lvRect & rect, int xoffset = 0, int yoffset = 0);
    CRUIDrawBufImage(LVDrawBuf * img);
    virtual ~CRUIDrawBufImage() { }
};

namespace CRUI {
	enum CRUILayoutOption {
		FILL_PARENT  = 0x40000000,
		WRAP_CONTENT = 0x20000000,
		UNSPECIFIED  = 0x10000000,
        PARENT_COLOR = 0xFFAAAAAA
	};
	enum CRUIWidgetState {
		STATE_DISABLED = 1,
		STATE_FOCUSED = 2,
        STATE_PRESSED = 4
	};
	// font sizes
	enum CRUIFontSizeOption {
		FONT_SIZE_UNSPECIFIED = 250,
		FONT_SIZE_XSMALL = 251,
		FONT_SIZE_SMALL = 252,
		FONT_SIZE_MEDIUM = 253,
		FONT_SIZE_LARGE = 254,
		FONT_SIZE_XLARGE = 255,
        FONT_USE_PARENT = 249
	};

	enum CRUIAlignmentOption {
		ALIGN_UNSPECIFIED = 0,
		ALIGN_LEFT = 1,
		ALIGN_HCENTER = 2,
		ALIGN_RIGHT = 3,
		ALIGN_TOP = 0x10,
		ALIGN_VCENTER = 0x20,
		ALIGN_BOTTOM = 0x30,
		ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER,
		ALIGN_TOP_LEFT = ALIGN_TOP | ALIGN_LEFT,
		ALIGN_RIGHT_CENTER = ALIGN_RIGHT | ALIGN_CENTER,
		ALIGN_MASK_VERTICAL = 0xF0, // mask
		ALIGN_MASK_HORIZONTAL = 0x0F, // mask
	};

    enum CRUIEllipsisOption {
        ELLIPSIS_RIGHT,
        ELLIPSIS_MIDDLE,
        ELLIPSIS_LEFT
    };
}



class CRUIBackgroundImageResource {
    lString8 id;
    lString8 nameRes;
    lString16 name;
    lString8 fileName;
    bool tiled;
    bool ninePatch;
public:
    bool isTiled() const { return tiled; }
    bool isNinePatch() const { return ninePatch; }
    const lString8 & getId() const { return id; }
    lString16 getName() const { return name.empty() ? _16(nameRes.c_str()) : name; }
    const lString8 & getFileName() const { return fileName; }
    CRUIBackgroundImageResource(lString8 _id, lString8 _nameRes, lString8 _fileName, bool _tiled = true) : id(_id), nameRes(_nameRes), fileName(_fileName), tiled(_tiled), ninePatch(_fileName.pos(".9")>=0) {}
    CRUIBackgroundImageResource(lString8 _id, lString16 _name, lString8 _fileName, bool _tiled = true) : id(_id), name(_name), fileName(_fileName), tiled(_tiled), ninePatch(_fileName.pos(".9")>=0) {}
    CRUIBackgroundImageResource(const CRUIBackgroundImageResource & v) : id(v.id), nameRes(v.nameRes), name(v.name), fileName(v.fileName), tiled(v.tiled), ninePatch(v.ninePatch) {}
};

class CRResourceResolver {
	lString8Collection _dirList;
    lString8Collection _themeDirList;
    lString16 resourceToFileName(const char * res);
	LVHashTable<lString8, LVImageSourceRef> _imageSourceMap;
	LVHashTable<lString8, CRUIImageRef> _iconMap;
    lUInt32 _iconColorTransformAdd;
    lUInt32 _iconColorTransformMultiply;

    LVPtrVector<CRUIBackgroundImageResource> _backgroundResources;

    LVImageSourceRef applyColorTransform(LVImageSourceRef src);
public:
    void addBackground(CRUIBackgroundImageResource * res) { _backgroundResources.add(res); }
    int backgroundCount() const { return _backgroundResources.length(); }
    const CRUIBackgroundImageResource * getBackground(int index) { return _backgroundResources[index]; }
    const CRUIBackgroundImageResource * findBackground(lString8 id);
    LVImageSourceRef getBackgroundImageSource(lString8 id);
    CRUIImageRef getBackgroundImage(CRPropRef props);
    CRUIImageRef getBackgroundImage(lString8 id);
    void setDirList(lString8Collection & dirList);
    void setThemeDirList(lString8Collection & dirList);
    CRResourceResolver(lString8Collection & dirList)
        : _dirList(dirList), _imageSourceMap(1000),
        _iconMap(1000),
        _iconColorTransformAdd(COLOR_TRANSFORM_BRIGHTNESS_NONE), _iconColorTransformMultiply(COLOR_TRANSFORM_CONTRAST_NONE)
    {
    }
    void setIconColorTransform(lUInt32 add, lUInt32 multiply = 0x202020) {
        _iconColorTransformAdd = add;
        _iconColorTransformMultiply = multiply;
        clearImageCache();
    }
	LVImageSourceRef getImageSource(const char * name);
	void clearImageCache();
	CRUIImageRef getIcon(const char * name, bool tiled= false);
	virtual ~CRResourceResolver() {}
};

extern CRResourceResolver * resourceResolver;
void LVCreateResourceResolver(lString8Collection & dirList);


class CRUITheme;

#define COLOR_NONE 0xFFFFFFFF

class CRUIStyle {
protected:
	CRUITheme * _theme;
	lString8 _styleId;
	lString8 _background;
	lString8 _background2;
	bool _backgroundTiled;
	bool _background2Tiled;
	lUInt32 _backgroundColor;
	lUInt32 _background2Color;
    LVFontRef _font;
	lUInt8 _fontSize;
	lUInt32 _textColor;
	CRUIStyle * _parentStyle;
	lUInt8 _stateMask;
	lUInt8 _stateValue;
	lvRect _margin;
	lvRect _padding;
	int _minWidth;
	int _maxWidth;
	int _minHeight;
	int _maxHeight;
    int _layoutWidth;
    int _layoutHeight;
	lUInt32 _align;
	lString8 _listDelimiterHorizontal;
	lString8 _listDelimiterVertical;
    CRUIImageRef _listDelimiterHorizontalImg;
    CRUIImageRef _listDelimiterVerticalImg;
    LVPtrVector<CRUIStyle, true> _substyles;
	/// checks if state filter matches specified state
	virtual bool matchState(lUInt8 stateValue);
public:
	CRUIStyle(CRUITheme * theme, lString8 id = lString8::empty_str, lUInt8 stateMask = 0, lUInt8 stateValue = 0);
	virtual ~CRUIStyle();
	virtual CRUITheme * getTheme() { return _theme; }
    CRUIStyle * getParentStyle() { return _parentStyle; }
    const lString8 & getStyleId() { return _styleId; }
    void setStyleId(lString8 id) { _styleId = id; }
    virtual CRUIStyle * addSubstyle(lString8 id = lString8::empty_str, lUInt8 stateMask = 0, lUInt8 stateValue = 0);
	virtual CRUIStyle * addSubstyle(const char * id = NULL, lUInt8 stateMask = 0, lUInt8 stateValue = 0) {
		return addSubstyle(lString8(id), stateMask, stateValue);
	}
	virtual CRUIStyle * addSubstyle(lUInt8 stateMask, lUInt8 stateValue) {
		return addSubstyle(lString8::empty_str, stateMask, stateValue);
	}
    void setStateMask(lUInt8 mask) { _stateMask = mask; }
    void setStateValue(lUInt8 value) { _stateValue = value; }
    lUInt8 getStateMask() { return _stateMask; }
    lUInt8 getStateValue() { return _stateValue; }
    /// try finding substyle for state, return this style if matching substyle is not found
	virtual CRUIStyle * find(lUInt8 stateValue);
	/// try to find style by id starting from this style, then substyles recursively, return NULL if not found
	virtual CRUIStyle * find(const lString8 &id);
	virtual const lString8 & styleId() const { return _styleId; }

	virtual void setStateFilter(lUInt8 mask, lUInt8 value) { _stateMask = mask; _stateValue = value; }

    CRUIStyle * setLayoutParams(int width, int height) {
        _layoutWidth = (width == CRUI::FILL_PARENT ? CRUI::FILL_PARENT : (width == CRUI::WRAP_CONTENT ? CRUI::WRAP_CONTENT : CRUI::UNSPECIFIED));
        _layoutHeight = (height == CRUI::FILL_PARENT ? CRUI::FILL_PARENT : (height == CRUI::WRAP_CONTENT ? CRUI::WRAP_CONTENT : CRUI::UNSPECIFIED));
        return this;
    }
    CRUIStyle * setPadding(const lvRect & padding) { _padding = padding; return this; }
    CRUIStyle * setPadding(int w) { _padding.left = _padding.top = _padding.right = _padding.bottom = w; return this; }
    CRUIStyle * setMargin(const lvRect & margin) { _margin = margin; return this; }
    CRUIStyle * setMargin(int w) { _margin.left = _margin.top = _margin.right = _margin.bottom = w; return this; }
	CRUIStyle * setMinWidth(int v) { _minWidth = v; return this; }
	CRUIStyle * setMaxWidth(int v) { _maxWidth = v; return this; }
	CRUIStyle * setMinHeight(int v) { _minHeight = v; return this; }
	CRUIStyle * setMaxHeight(int v) { _maxHeight = v; return this; }
	const lvRect & getPadding() { return _padding; }
	const lvRect & getMargin() { return _margin; }
	virtual int getMinHeight();
	virtual int getMaxHeight();
	virtual int getMaxWidth();
	virtual int getMinWidth();
    virtual int getLayoutWidth();
    virtual int getLayoutHeight();

	virtual CRUIStyle * setFontSize(lUInt8 fontSize) { _fontSize = fontSize; return this; }
	virtual CRUIStyle * setFont(LVFontRef font) { _font = font; return this; }
	virtual CRUIStyle * setTextColor(lUInt32 color) { _textColor = color; return this; }
	//virtual CRUIStyle * setBackground(CRUIImageRef background) { _background = background; return this; }
    virtual CRUIStyle * setBackground(const char * resourceName, bool tiled = false) { _background = resourceName; _backgroundTiled = tiled; _backgroundColor = COLOR_NONE; return this; }
	virtual CRUIStyle * setBackground(lUInt32 color) { _backgroundColor = color; _background.clear(); return this; }
    virtual CRUIStyle * setBackground2(const char * resourceName, bool tiled = false) { _background2 = resourceName; _background2Tiled = tiled; _background2Color = COLOR_NONE; return this; }
	virtual CRUIStyle * setBackground2(lUInt32 color) { _background2Color = color; _background2.clear(); return this; }
    virtual CRUIStyle * setListDelimiterHorizontal(const char * imgRes) { _listDelimiterHorizontalImg.Clear(); _listDelimiterHorizontal = imgRes; return this; }
    virtual CRUIStyle *  setListDelimiterVertical(const char * imgRes) { _listDelimiterVerticalImg.Clear(); _listDelimiterVertical = imgRes; return this; }
    virtual CRUIStyle * setListDelimiterHorizontal(CRUIImageRef img) { _listDelimiterHorizontal.clear(); _listDelimiterHorizontalImg = img; return this; }
    virtual CRUIStyle *  setListDelimiterVertical(CRUIImageRef img) { _listDelimiterVertical.clear(); _listDelimiterVerticalImg = img; return this; }
    virtual CRUIImageRef getListDelimiterHorizontal();
	virtual CRUIImageRef getListDelimiterVertical();
    /// main (lower) layer of background
	virtual CRUIImageRef getBackground();
    /// additional (upper) layer of background
    virtual CRUIImageRef getBackground2();
    virtual lUInt8 getFontSize() { return _fontSize; }
	virtual LVFontRef getFont();
	virtual lUInt32 getTextColor();
	virtual lUInt32 getAlign() { return _align; }
	virtual CRUIStyle * setAlign(lUInt32 align) { _align = align; return this; }
};

#define COLOR_ID_ICON_COLOR_TRANSFORM_BRIGHTNESS "ICON_COLOR_TRANSFORM_BRIGHTNESS"
#define COLOR_ID_ICON_COLOR_TRANSFORM_CONTRAST "ICON_COLOR_TRANSFORM_BRIGHTNESS"
#define COLOR_ID_SLIDER_LINE_COLOR_OUTER "SLIDER_LINE_COLOR_OUTER"
#define COLOR_ID_SLIDER_LINE_COLOR_INNER "SLIDER_LINE_COLOR_INNER"
#define COLOR_ID_SLIDER_POINTER_COLOR_OUTER "SLIDER_POINTER_COLOR_OUTER"
#define COLOR_ID_SLIDER_POINTER_COLOR_INNER "SLIDER_POINTER_COLOR_INNER"

#define COLOR_MENU_POPUP_FADE 0xF0000000

class CRUITheme : public CRUIStyle {
protected:
	LVHashTable<lString8, CRUIStyle *> _map;
    LVHashTable<lString8, lUInt32> _colors;
    LVHashTable<lUInt32, LVFontRef> _fonts;
public:
    void remove(CRUIStyle * style);
	virtual CRUIStyle * findStyle(const lString8 &id, bool defaultToTheme = true);
	void registerStyle(CRUIStyle * style);
	LVFontRef getFontForSize(lUInt8 size);
    virtual lUInt8 getFontSize() { return CRUIStyle::getFontSize(); }
    int getFontSize(lUInt8 size);
	CRUITheme(lString8 name);
    void setColor(lString8 id, lUInt32 value) {
        _colors.set(id, value);
    }
    lUInt32 getColor(lString8 id) {
        lUInt32 cl = 0xFF0000;
        if (!_colors.get(id, cl)) {
            //CRLog::error("Color not found in theme: %s", id.c_str());
            cl = 0xFF0000;
        }
        return cl;
    }
    lUInt32 getColor(const char * id) {
        return _colors.get(lString8(id));
    }

    /// reads theme from XML file
    bool loadFromFile(lString8 fileName);
};

extern CRUITheme * currentTheme;


#endif /* CRUITHEME_H_ */
