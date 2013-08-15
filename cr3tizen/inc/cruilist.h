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
	virtual bool isEnabled(int index) { return true; }
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
	LVArray<lvPoint> _itemSizes;
	LVArray<lvRect> _itemRects;
public:
	CRUIListWidget(bool vertical = true, CRUIListAdapter * adapter = NULL);
	int itemFromPoint(int x, int y);
	virtual void setSelectedItem(int item) { _selectedItem = item; invalidate(); }
	virtual int getSelectedItem() { return _selectedItem; }
	virtual bool isVertical() { return _vertical; }
	virtual CRUIListWidget * setAdapter(CRUIListAdapter * adapter, bool deleteOnWidgetDestroy = false) {
		_adapter = adapter;
		_ownAdapter = deleteOnWidgetDestroy;
		requestLayout();
		return this;
	}
	virtual int getScrollOffset() { return _scrollOffset; }
	virtual void setScrollOffset(int offset);
	virtual CRUIWidget * getItemWidget(int index) { return _adapter != NULL ? _adapter->getItemWidget(this, index) : 0; }
	virtual int getItemCount() { return _adapter != NULL ? _adapter->getItemCount(this) : 0; }
	/// motion event handler, returns true if it handled event
	virtual bool onTouchEvent(const CRUIMotionEvent * event);
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
	virtual lvPoint getTileOffset() const;
};


#endif /* CRUILIST_H_ */
