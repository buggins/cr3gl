/*
 * cruitheme.h
 *
 *  Created on: Aug 12, 2013
 *      Author: Vadim
 */

#ifndef CRUITHEME_H_
#define CRUITHEME_H_

#include <crengine.h>

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
public:
    virtual void draw(LVDrawBuf * buf, lvRect & rect, int xoffset = 0, int yoffset = 0) { buf->FillRect(rect, _color); CR_UNUSED2(xoffset, yoffset); }
	CRUISolidFillImage(lUInt32 color) : _color(color) { }
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
		PARENT_COLOR = 0xFFAAAAAA,
	};
	enum CRUIWidgetState {
		STATE_DISABLED = 1,
		STATE_FOCUSED = 2,
		STATE_PRESSED = 4,
	};
	// font sizes
	enum CRUIFontSizeOption {
		FONT_SIZE_UNSPECIFIED = 250,
		FONT_SIZE_XSMALL = 251,
		FONT_SIZE_SMALL = 252,
		FONT_SIZE_MEDIUM = 253,
		FONT_SIZE_LARGE = 254,
		FONT_SIZE_XLARGE = 255,
		FONT_USE_PARENT = 249,
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

class CRResourceResolver {
	lString8Collection _dirList;
	lString16 resourceToFileName(const char * res);
	LVHashTable<lString8, LVImageSourceRef> _imageSourceMap;
	LVHashTable<lString8, CRUIImageRef> _iconMap;
public:
    void setDirList(lString8Collection & dirList);
	CRResourceResolver(lString8Collection & dirList) : _dirList(dirList), _imageSourceMap(1000), _iconMap(1000) { }
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
	lUInt32 _align;
	CRUIImageRef _listDelimiterHorizontal;
	CRUIImageRef _listDelimiterVertical;
	LVPtrVector<CRUIStyle, true> _substyles;
	/// checks if state filter matches specified state
	virtual bool matchState(lUInt8 stateValue);
public:
	CRUIStyle(CRUITheme * theme, lString8 id = lString8::empty_str, lUInt8 stateMask = 0, lUInt8 stateValue = 0);
	virtual ~CRUIStyle();
	virtual CRUITheme * getTheme() { return _theme; }
	virtual CRUIStyle * addSubstyle(lString8 id = lString8::empty_str, lUInt8 stateMask = 0, lUInt8 stateValue = 0);
	virtual CRUIStyle * addSubstyle(const char * id = NULL, lUInt8 stateMask = 0, lUInt8 stateValue = 0) {
		return addSubstyle(lString8(id), stateMask, stateValue);
	}
	virtual CRUIStyle * addSubstyle(lUInt8 stateMask, lUInt8 stateValue) {
		return addSubstyle(lString8::empty_str, stateMask, stateValue);
	}
	/// try finding substyle for state, return this style if matching substyle is not found
	virtual CRUIStyle * find(lUInt8 stateValue);
	/// try to find style by id starting from this style, then substyles recursively, return NULL if not found
	virtual CRUIStyle * find(const lString8 &id);
	virtual const lString8 & styleId() const { return _styleId; }

	virtual void setStateFilter(lUInt8 mask, lUInt8 value) { _stateMask = mask; _stateValue = value; }

	CRUIStyle * setPadding(int w) { _padding.left = _padding.top = _padding.right = _padding.bottom = w; return this; }
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

	virtual CRUIStyle * setFontSize(lUInt8 fontSize) { _fontSize = fontSize; return this; }
	virtual CRUIStyle * setFont(LVFontRef font) { _font = font; return this; }
	virtual CRUIStyle * setTextColor(lUInt32 color) { _textColor = color; return this; }
	//virtual CRUIStyle * setBackground(CRUIImageRef background) { _background = background; return this; }
    virtual CRUIStyle * setBackground(const char * resourceName, bool tiled = false) { _background = resourceName; _backgroundTiled = tiled; _backgroundColor = COLOR_NONE; return this; }
	virtual CRUIStyle * setBackground(lUInt32 color) { _backgroundColor = color; _background.clear(); return this; }
    virtual CRUIStyle * setBackground2(const char * resourceName, bool tiled = false) { _background2 = resourceName; _background2Tiled = tiled; _background2Color = COLOR_NONE; return this; }
	virtual CRUIStyle * setBackground2(lUInt32 color) { _background2Color = color; _background2.clear(); return this; }
    virtual CRUIStyle * setListDelimiterHorizontal(CRUIImageRef img) { _listDelimiterHorizontal = img; return this; }
	virtual CRUIStyle *  setListDelimiterVertical(CRUIImageRef img) { _listDelimiterVertical = img; return this; }
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

class CRUITheme : public CRUIStyle {
protected:
	LVFontRef _fontXSmall;
	LVFontRef _fontSmall;
	LVFontRef _fontLarge;
	LVFontRef _fontXLarge;
	LVHashTable<lString8, CRUIStyle *> _map;
public:
	virtual CRUIStyle * find(const lString8 &id);
	void registerStyle(CRUIStyle * style);
	CRUIStyle * setFontForSize(lUInt8 size, LVFontRef font);
	LVFontRef getFontForSize(lUInt8 size);
	CRUITheme(lString8 name);
};

extern CRUITheme * currentTheme;


#endif /* CRUITHEME_H_ */
