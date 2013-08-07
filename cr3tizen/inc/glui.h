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
	CRUIWidget * _parent;
	CRUIImageRef _background;
	LVFontRef _font;
	lUInt32 _textColor;
public:
	virtual int getChildCount() { return 0; }
	virtual CRUIWidget * getChild(int index) { return NULL; }
	virtual void addChild(CRUIWidget * child) { }
	virtual CRUIWidget * removeChild(int index) { return NULL; }
	CRUIWidget();
	virtual ~CRUIWidget();
	/// returns parent widget pointer, NULL if it's top level widget
	CRUIWidget * getParent() { return _parent; }
	/// measure dimensions
	virtual void measure(lvRect & baseRect, lvPoint & size);
	/// updates widget position based on specified rectangle
	virtual void layout(lvRect & pos);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

class CRUITextWidget : CRUIWidget {
protected:
	lString16 _text;
	bool _multiline;
public:
	CRUITextWidget(lString16 text) : _text(text) {}
};

class CRUIImageWidget : CRUIWidget {
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
	virtual void addChild(CRUIWidget * child) { _children.add(child); }
	virtual CRUIWidget * removeChild(int index) { return _children.remove(index); }
	virtual ~CRUIContainerWidget() { }
};

#endif /* GLUI_H_ */
