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

class CRUIWidget;

enum {
	ACTION_DOWN,
	ACTION_MOVE,
	ACTION_UP,
	ACTION_FOCUS_OUT,
	ACTION_FOCUS_IN,
	ACTION_CANCEL,
};

class CRUIMotionEventItem {
	friend class CRUIEventManager;
	int _pointerId;
	int _action;
	int _x;
	int _y;
	CRUIWidget * _widget;
	void setWidget(CRUIWidget * widget) { _widget = widget; }
public:
	CRUIMotionEventItem(int pointerId, int action, int x, int y, CRUIWidget * widget = NULL)
	: _pointerId(pointerId), _action(action), _x(x), _y(y), _widget(widget) {
		//
	}
	int getX() const { return _x; }
	int getY() const { return _y; }
	int getAction() const { return _action; }
	int getPointerId() const { return _pointerId; }
	CRUIWidget * getWidget() const { return _widget; }
};

class CRUIMotionEvent {
	friend class CRUIEventManager;
	LVPtrVector<CRUIMotionEventItem> _data;
	void addEvent(CRUIMotionEventItem * event) { _data.add(event); }
public:
	int count() const { return _data.length(); }
	const CRUIMotionEventItem * operator[] (int index) const { return index >= 0 && index<_data.length() ? _data[index] : NULL; }
	int getX(int index) const { return index >= 0 && index<_data.length() ? _data[index]->getX() : 0; }
	int getY(int index) const { return index >= 0 && index<_data.length() ? _data[index]->getY() : 0; }
	int getAction(int index) const { return index >= 0 && index<_data.length() ? _data[index]->getAction() : 0; }
	int getPointerId(int index) const { return index >= 0 && index<_data.length() ? _data[index]->getPointerId() : 0; }
	/// find pointer index by pointer id, returns -1 if not found
	int findPointerId(int pointerId);
};

class CRUIOnTouchEventListener {
public:
	virtual bool onTouch(CRUIWidget * widget, const CRUIMotionEvent * event) = 0;
};

/// base class for all UI elements
class CRUIWidget {
protected:
	lString8 _id;
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
	lUInt32 _align;
	CRUIOnTouchEventListener * _onTouchListener;

	/// measure dimensions
	virtual void defMeasure(int baseWidth, int baseHeight, int contentWidth, int contentHeight);
	/// correct rectangle bounds according to alignment
	virtual void applyAlign(lvRect & rc, int contentWidth, int contentHeight);

	bool setClipRect(LVDrawBuf * buf, lvRect & rc);

public:

	CRUIWidget();
	virtual ~CRUIWidget();

	/// motion event handler, returns true if it handled event
	virtual bool onTouchEvent(const CRUIMotionEvent * event);
	virtual CRUIOnTouchEventListener * setOnTouchListener(CRUIOnTouchEventListener * listener);
	virtual CRUIOnTouchEventListener * getOnTouchListener() { return _onTouchListener; }
	virtual lvPoint getTileOffset() const { return lvPoint(); }
	const lString8 & getId() { return _id; }
	CRUIWidget * setId(const lString8 & id) { _id = id; return this; }
	lUInt32 getState() { return _state; }
	CRUIWidget * setState(lUInt32 state) { _state = state; return this; }

	virtual lUInt32 getAlign();
	virtual lUInt32 getHAlign() { return getAlign() & CRUI::ALIGN_MASK_HORIZONTAL; }
	virtual lUInt32 getVAlign() { return getAlign() & CRUI::ALIGN_MASK_VERTICAL; }
	virtual CRUIWidget * setAlign(lUInt32 align) { _align = align; return this; }

	int getLayoutWidth() { return _layoutWidth; }
	int getLayoutHeight() { return _layoutHeight; }
	CRUIWidget * setLayoutParams(int width, int height) { _layoutWidth = width; _layoutHeight = height; return this; }

	CRUIWidget * setPadding(int w) { _padding.left = _padding.top = _padding.right = _padding.bottom = w; return this; }
	CRUIWidget * setMargin(int w) { _margin.left = _margin.top = _margin.right = _margin.bottom = w; return this; }
	CRUIWidget * setPadding(const lvRect & rc) { _padding = rc; return this; }
	CRUIWidget * setMargin(const lvRect & rc) { _margin = rc; return this; }
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
	virtual CRUIWidget * childById(const lString8 & id);
	virtual CRUIWidget * childById(const char * id);
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
	void init(lString16 text, CRUIImageRef image, bool vertical);
public:
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
