/*
 * cruilayout.h
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#ifndef CRUILAYOUT_H_
#define CRUILAYOUT_H_

#include "cruiwidget.h"

class CRUIContainerWidget : public CRUIWidget {
protected:
	LVPtrVector<CRUIWidget> _children;
public:
	virtual int getChildCount() { return _children.length(); }
	virtual CRUIWidget * getChild(int index) { return _children.get(index); }
	virtual CRUIWidget * addChild(CRUIWidget * child) { child->setParent(this); _children.add(child); return child; }
	virtual CRUIWidget * removeChild(int index) { return _children.remove(index); }
    virtual CRUIWidget * removeChild(CRUIWidget * child) { return _children.remove(child); }
    virtual ~CRUIContainerWidget() { _children.clear(); }
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

class CRUIFrameLayout : public CRUIContainerWidget {
public:
    /// creates frame layout (all children occupy the same space)
    CRUIFrameLayout() { }
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
};

class CRUITableLayout : public CRUIContainerWidget {
protected:
    int _colCount;
    struct Col {
        int width;
        int maxw; // max width of non-FILL columns
        bool fill; // has FILL_PARENT cell
        int weight; // max weight of FILL_PARENT cell
        Col() : width(0), maxw(-1), fill(false), weight(0) {}
    };
    struct Row {
        int height;
        int maxh; // max height of non-FILL columns
        bool fill; // has FILL_PARENT cell
        int weight; // max weight of FILL_PARENT cell
        Row() : height(0), maxh(-1), fill(false), weight(0) {}
    };


    lvPoint layoutTable(LVArray<Col> &_cols, LVArray<Row> & _rows, int maxw, int maxh, bool fillx, bool filly);
public:
    /// creates table (multicolumn vertical) layout
    CRUITableLayout(int colCount) : _colCount(colCount) { }
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
};



#endif /* CRUILAYOUT_H_ */
