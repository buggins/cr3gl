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

/// base class for all UI elements
class CRUIWidget {
protected:
	lvRect _pos;
	lvRect _margin;
	lvRect _padding;
	int _width;
	int _height;
	int _measuredWidth;
	int _measuredHeight;
	CRUIWidget * _parent;
	CRUIImageRef _background;
public:

	enum {
		FILL_PARENT = -1,
		WRAP_CONTENT = -2
	};


	virtual int getChildCount() { return 0; }
	virtual CRUIWidget * getChild(int index) { return NULL; }
	virtual CRUIWidget * addChild(CRUIWidget * child) { return NULL; }
	virtual CRUIWidget * removeChild(int index) { return NULL; }
	CRUIWidget();
	virtual ~CRUIWidget();
	CRUIWidget * setParent(CRUIWidget * parent) { _parent = parent; return this; }
	/// returns parent widget pointer, NULL if it's top level widget
	CRUIWidget * getParent() { return _parent; }

	CRUIWidget * setBackground(CRUIImageRef background) { _background = background; return this; }

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
	bool _multiline;
	LVFontRef _font;
	lUInt32 _textColor;
public:
	virtual CRUITextWidget * setTextColor(lUInt32 color) { _textColor = color; return this; }
	virtual CRUITextWidget * setText(lString16 text) { _text = text; return this; }
	virtual CRUITextWidget * setFont(LVFontRef font) { _font = font; return this; }
	CRUITextWidget(lString16 text, bool multiline = false) : _text(text), _multiline(multiline), _textColor(0) {}
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
};

class CRUIVerticalLayout : public CRUIContainerWidget {
protected:
public:
};

#endif /* GLUI_H_ */
