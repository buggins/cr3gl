/*
 * glui.h
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */

#ifndef GLUI_H_
#define GLUI_H_

#include <crengine.h>

class CRUIImage {
public:
	virtual int originalWidth() { return 1; }
	virtual int originalHeight() { return 1; }
	virtual void draw(LVDrawBuf * buf, lvRect & rect) = 0;
	virtual ~CRUIImage() { }
};
typedef LVRef<CRUIImage> CRUIImageRef;

class CRUISolidFillImage : public CRUIImage {
	lUInt32 _color;
public:
	virtual void draw(LVDrawBuf * buf, lvRect & rect) { buf->FillRect(rect, _color); }
	CRUISolidFillImage(lUInt32 color) : _color(color) { }
	virtual ~CRUISolidFillImage() { }
};

class CRUIBitmapImage : public CRUIImage {
	LVImageSourceRef _src;
public:
	virtual int originalWidth() { return _src->GetWidth(); }
	virtual int originalHeight() { return _src->GetHeight(); }
	virtual void draw(LVDrawBuf * buf, lvRect & rect) { buf->Draw(_src, rect.left, rect.top, originalWidth(), originalHeight(), false); }
	CRUIBitmapImage(LVImageSourceRef img) : _src(img) { }
	virtual ~CRUIBitmapImage() { }
};

namespace CRUI {
	enum {
		FILL_PARENT  = 0x40000000,
		WRAP_CONTENT = 0x20000000,
		UNSPECIFIED  = 0x10000000,
		PARENT_COLOR = 0xFFAAAAAA,
	};
	enum {
		STATE_ENABLED = 1,
		STATE_FOCUSED = 2,
		STATE_PRESSED = 4,
	};
	// font sizes
	enum {
		FONT_SIZE_UNSPECIFIED = 0,
		FONT_SIZE_XSMALL = 1,
		FONT_SIZE_SMALL = 2,
		FONT_SIZE_MEDIUM = 3,
		FONT_SIZE_LARGE = 4,
		FONT_SIZE_XLARGE = 5,
		FONT_USE_PARENT = 6,
	};
};

class CRUITheme;

class CRUIStyle {
protected:
	CRUITheme * _theme;
	lString8 _styleId;
	CRUIImageRef _background;
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
	LVPtrVector<CRUIStyle, true> _substyles;
	/// checks if state filter matches specified state
	virtual bool matchState(lUInt8 stateValue);
public:
	CRUIStyle(CRUITheme * theme, lString8 id = lString8::empty_str, lUInt8 stateMask = 0, lUInt8 stateValue = 0);
	virtual ~CRUIStyle();
	virtual CRUITheme * getTheme() { return _theme; }
	virtual CRUIStyle * addSubstyle(lString8 id = lString8::empty_str, lUInt8 stateMask = 0, lUInt8 stateValue = 0);
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
	virtual CRUIStyle * setBackground(CRUIImageRef background) { _background = background; return this; }
	virtual CRUIStyle * setBackground(lUInt32 color) { _background = CRUIImageRef(new CRUISolidFillImage(color)); return this; }
	virtual CRUIImageRef getBackground();
	virtual lUInt8 getFontSize() { return _fontSize; }
	virtual LVFontRef getFont();
	virtual lUInt32 getTextColor();
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

/// base class for all UI elements
class CRUIWidget {
protected:
	lString8 _styleId;
	lvRect _pos;
	lvRect _margin;
	lvRect _padding;
	int _layoutWidth;
	int _layoutHeight;
	int _minWidth;
	int _maxWidth;
	int _minHeight;
	int _maxHeight;
	int _measuredWidth;
	int _measuredHeight;
	CRUIWidget * _parent;
	CRUIImageRef _background;
	bool _layoutRequested;
	LVFontRef _font;
	lUInt8 _fontSize;
	lUInt32 _textColor;

	/// measure dimensions
	virtual void defMeasure(int baseWidth, int baseHeight, int contentWidth, int contentHeight);

public:

	CRUIWidget();
	virtual ~CRUIWidget();

	int getLayoutWidth() { return _layoutWidth; }
	int getLayoutHeight() { return _layoutHeight; }
	CRUIWidget * setLayoutParams(int width, int height) { _layoutWidth = width; _layoutHeight = height; return this; }

	CRUIWidget * setPadding(int w) { _padding.left = _padding.top = _padding.right = _padding.bottom = w; return this; }
	CRUIWidget * setMargin(int w) { _margin.left = _margin.top = _margin.right = _margin.bottom = w; return this; }
	CRUIWidget * setMinWidth(int v) { _minWidth = v; return this; }
	CRUIWidget * setMaxWidth(int v) { _maxWidth = v; return this; }
	CRUIWidget * setMinHeight(int v) { _minHeight = v; return this; }
	CRUIWidget * setMaxHeight(int v) { _maxHeight = v; return this; }

	virtual const lvRect & getPadding();
	virtual const lvRect & getMargin();
	virtual int getMinHeight();
	virtual int getMaxHeight();
	virtual int getMaxWidth();
	virtual int getMinWidth();

	CRUIWidget * setStyle(lString8 styleId) { _styleId = styleId; return this; }
	CRUIStyle * getStyle();

	virtual CRUIWidget * setText(lString16 text) { return this; }

	virtual CRUIWidget * setFont(LVFontRef font) { _font = font; requestLayout(); return this; }
	virtual CRUIWidget * setTextColor(lUInt32 color) { _textColor = color; requestLayout(); return this; }
	virtual CRUIWidget * setBackground(CRUIImageRef background) { _background = background; return this; }
	virtual CRUIWidget * setBackground(lUInt32 color) { _background = CRUIImageRef(new CRUISolidFillImage(color)); return this; }
	virtual CRUIImageRef getBackground();
	virtual LVFontRef getFont();
	virtual lUInt32 getTextColor();




	virtual int getChildCount() { return 0; }
	virtual CRUIWidget * getChild(int index) { return NULL; }
	virtual CRUIWidget * addChild(CRUIWidget * child) { return NULL; }
	virtual CRUIWidget * removeChild(int index) { return NULL; }
	virtual CRUIWidget * setParent(CRUIWidget * parent) { _parent = parent; return this; }
	/// returns parent widget pointer, NULL if it's top level widget
	virtual CRUIWidget * getParent() { return _parent; }



	virtual void requestLayout(bool updateParent = true) {
		_layoutRequested = true;
		if (updateParent && _parent)
			_parent->requestLayout(true);
	}

	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);

	int getMeasuredWidth() { return _measuredWidth; }
	int getMeasuredHeight() { return _measuredHeight; }

	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

class CRUITextWidget : public CRUIWidget {
protected:
	lString16 _text;
	int _maxLines;
public:
	virtual CRUIWidget * setMaxLines(int maxLines) { _maxLines = maxLines; requestLayout(); return this; }
	virtual CRUIWidget * setText(lString16 text) { _text = text; requestLayout(); return this; }

	CRUITextWidget(lString16 text, int maxLines = 1) : _text(text), _maxLines(maxLines) {}
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

class CRUIImageWidget : public CRUIWidget {
protected:
	CRUIImageRef _image;
public:
	CRUIImageWidget(CRUIImageRef image) : _image(image) { }
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

class CRUIContainerWidget : public CRUIWidget {
protected:
	LVPtrVector<CRUIWidget> _children;
public:
	virtual int getChildCount() { return _children.length(); }
	virtual CRUIWidget * getChild(int index) { return _children.get(index); }
	virtual CRUIWidget * addChild(CRUIWidget * child) { child->setParent(this); _children.add(child); return child; }
	virtual CRUIWidget * removeChild(int index) { return _children.remove(index); }
	virtual ~CRUIContainerWidget() { }
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

class CRUILinearLayout : public CRUIContainerWidget {
protected:
	bool _isVertical;
public:
	/// check orientation
	virtual bool isVertical() { return _isVertical; }
	/// sets orientation
	virtual CRUILinearLayout * setVertical(bool vertical) { _isVertical = vertical; requestLayout(); return this; }
	/// creates either vertical or horizontal linear layout
	CRUILinearLayout(bool vertical) : _isVertical(vertical) { }
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
};

class CRUIVerticalLayout : public CRUILinearLayout {
protected:
public:
	CRUIVerticalLayout() : CRUILinearLayout(true) {}
};

class CRUIHorizontalLayout : public CRUILinearLayout {
protected:
public:
	CRUIHorizontalLayout() : CRUILinearLayout(false) {}
};

class CRUIButton : public CRUILinearLayout {
protected:
	CRUIImageWidget * _icon;
	CRUITextWidget * _label;
public:
	CRUIButton(lString16 text, CRUIImageRef image = CRUIImageRef(), bool vertical = false);
};

class CRResourceResolver {
	lString8Collection _dirList;
	lString16 resourceToFileName(const char * res);
public:
	CRResourceResolver(lString8Collection & dirList) : _dirList(dirList) { }
	LVImageSourceRef getImageSource(const char * name);
	CRUIImageRef getIcon(const char * name);
	virtual ~CRResourceResolver() {}
};

extern CRResourceResolver * resourceResolver;
void LVCreateResourceResolver(lString8Collection & dirList);

#endif /* GLUI_H_ */
