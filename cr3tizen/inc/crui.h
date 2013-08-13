/*
 * glui.h
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */

#ifndef GLUI_H_
#define GLUI_H_

#include <crengine.h>
#include "cruitheme.h"

/// base class for all UI elements
class CRUIWidget {
protected:
	lString8 _styleId;
	lUInt32 _state;
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


	lUInt32 getState() { return _state; }
	CRUIWidget * setState(lUInt32 state) { _state = state; return this; }

	int getLayoutWidth() { return _layoutWidth; }
	int getLayoutHeight() { return _layoutHeight; }
	CRUIWidget * setLayoutParams(int width, int height) { _layoutWidth = width; _layoutHeight = height; return this; }

	CRUIWidget * setPadding(int w) { _padding.left = _padding.top = _padding.right = _padding.bottom = w; return this; }
	CRUIWidget * setMargin(int w) { _margin.left = _margin.top = _margin.right = _margin.bottom = w; return this; }
	CRUIWidget * setMinWidth(int v) { _minWidth = v; return this; }
	CRUIWidget * setMaxWidth(int v) { _maxWidth = v; return this; }
	CRUIWidget * setMinHeight(int v) { _minHeight = v; return this; }
	CRUIWidget * setMaxHeight(int v) { _maxHeight = v; return this; }

	virtual void getMargin(lvRect & rc);
	virtual void getPadding(lvRect & rc);
	virtual void applyPadding(lvRect & rc);
	virtual void applyMargin(lvRect & rc);
	virtual const lvRect & getPadding();
	virtual const lvRect & getMargin();
	virtual int getMinHeight();
	virtual int getMaxHeight();
	virtual int getMaxWidth();
	virtual int getMinWidth();

	CRUIWidget * setStyle(lString8 styleId) { _styleId = styleId; return this; }
	CRUIStyle * getStyle(bool forState = false);

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


#endif /* GLUI_H_ */
