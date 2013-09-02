/*
 * cruilist.h
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#ifndef CRUILIST_H_
#define CRUILIST_H_

#include "cruilayout.h"
#include "cruicontrols.h"

class CRUIListWidget;

/// list adapter - provider of widget for list contents
class CRUIListAdapter {
public:
	virtual int getItemCount(CRUIListWidget * list) = 0;
	virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index) = 0;
	virtual bool isEnabled(CRUIListWidget * list, int index) { return true; }
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
	int _maxScrollOffset;
	int _topItem;
	int _selectedItem;
	int _dragStartOffset;
	CRUIOnListItemClickListener * _onItemClickListener;
	CRUIOnListItemLongClickListener * _onItemLongClickListener;
	LVArray<lvPoint> _itemSizes;
	LVArray<lvRect> _itemRects;
    ScrollControl _scroll;
public:
    virtual void setSelectedItem(int index) { _selectedItem = index; invalidate(); }
    virtual int getSelectedItem() { return _selectedItem; }
    CRUIListWidget(bool vertical = true, CRUIListAdapter * adapter = NULL);
	int itemFromPoint(int x, int y);
	virtual bool isVertical() { return _vertical; }
	virtual CRUIListWidget * setAdapter(CRUIListAdapter * adapter, bool deleteOnWidgetDestroy = false) {
		_adapter = adapter;
		_ownAdapter = deleteOnWidgetDestroy;
		requestLayout();
		return this;
	}

	virtual CRUIOnListItemClickListener * getOnItemClickListener() { return _onItemClickListener; }
	virtual CRUIOnListItemLongClickListener * getOnItemLongClickListener() { return _onItemLongClickListener; }
	virtual CRUIOnListItemClickListener * setOnItemClickListener(CRUIOnListItemClickListener * listener) { CRUIOnListItemClickListener * old = _onItemClickListener; _onItemClickListener = listener; return old; }
	virtual CRUIOnListItemLongClickListener * setOnItemLongClickListener(CRUIOnListItemLongClickListener * listener) { CRUIOnListItemLongClickListener * old = _onItemLongClickListener; _onItemLongClickListener = listener; return old; }
	virtual bool onItemClickEvent(int itemIndex);
	virtual bool onItemLongClickEvent(int itemIndex);

	virtual int getScrollOffset() { return _scrollOffset; }
	virtual void setScrollOffset(int offset);

	virtual CRUIWidget * getItemWidget(int index) { return _adapter != NULL ? _adapter->getItemWidget(this, index) : 0; }
	virtual int getItemCount() { return _adapter != NULL ? _adapter->getItemCount(this) : 0; }
	virtual bool isItemEnabled(int index) { return _adapter != NULL ? _adapter->isEnabled(this, index) : true; }

    /// return true if drag operation is intercepted
    virtual bool startDragging(const CRUIMotionEvent * event, bool vertical);

	/// motion event handler, returns true if it handled event
	virtual bool onTouchEvent(const CRUIMotionEvent * event);
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
	virtual lvPoint getTileOffset() const;

    virtual void animate(lUInt64 millisPassed);
    virtual bool isAnimating();
};


#endif /* CRUILIST_H_ */
