/*
 * glui.h
 *
 *  Created on: Aug 7, 2013
 *      Author: vlopatin
 */

#ifndef GLUI_H_
#define GLUI_H_

#include "cruiwidget.h"

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
	void init(lString16 text, CRUIImageRef image, bool vertical);
public:
	/// motion event handler, returns true if it handled event
	virtual bool onTouchEvent(const CRUIMotionEvent * event);
	CRUIButton(lString16 text, CRUIImageRef image = CRUIImageRef(), bool vertical = false);
	CRUIButton(lString16 text, const char * imageRes, bool vertical = false);
};


class CRUIListWidget;

/// list adapter - provider of widget for list contents
class CRUIListAdapter {
public:
	virtual int getItemCount(CRUIListWidget * list) = 0;
	virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index) = 0;
	virtual ~CRUIListAdapter() {}
};

class CRUIStringListAdapter : public CRUIListAdapter {
protected:
	lString16Collection _strings;
	CRUITextWidget * _widget;
public:
	CRUIStringListAdapter();
	virtual ~CRUIStringListAdapter();
	virtual CRUIStringListAdapter * addItem(const lString16 & s) { _strings.add(s); return this; }
	virtual CRUIStringListAdapter * addItem(const lChar16 * s) { _strings.add(s); return this; }
	virtual const lString16Collection & getItems() const { return _strings; }
	virtual CRUIStringListAdapter * setItems(lString16Collection & list);
	virtual int getItemCount(CRUIListWidget * list);
	virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
};

class CRUIListWidget : public CRUIWidget {
protected:
	bool _vertical;
	CRUIListAdapter * _adapter;
	bool _ownAdapter;
	int _scrollOffset;
	int _topItem;
	LVArray<lvPoint> _itemSizes;
	LVArray<lvRect> _itemRects;
public:
	virtual bool isVertical() { return _vertical; }
	virtual CRUIListWidget * setAdapter(CRUIListAdapter * adapter, bool deleteOnWidgetDestroy = false) {
		_adapter = adapter;
		_ownAdapter = deleteOnWidgetDestroy;
		return this;
	}
	virtual int getScrollOffset() { return _scrollOffset; }
	virtual void setScrollOffset(int offset) { _scrollOffset = offset; }
	virtual CRUIWidget * getItemWidget(int index) { return _adapter != NULL ? _adapter->getItemWidget(this, index) : 0; }
	virtual int getItemCount() { return _adapter != NULL ? _adapter->getItemCount(this) : 0; }
	CRUIListWidget(bool vertical = true, CRUIListAdapter * adapter = NULL);
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
	virtual lvPoint getTileOffset() const;
};

#endif /* GLUI_H_ */
