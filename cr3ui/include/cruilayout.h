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

class PopupControl {
public:
    lInt64 startTs;
    lInt64 endTs;
    CRUIWidget * popup; // popup widget
    lUInt32 outerColor; // to apply on surface outside popup
    int width;
    int height;
    int align;       // where is destination rectangle located
    int progress;
    bool closing;
    lvRect srcRect;
    lvRect dstRect;
    lvRect margins;
    void close() {
        if (popup)
            delete popup;
        popup = NULL;
    }
    /// returns rect for current progress
    void getRect(lvRect & rc);
    /// calculates src and dst rectangles for updated parent position/size
    void layout(const lvRect & pos);
    /// calculates outer background color for current progress
    lUInt32 getColor();

    /// start animation of popup closing
    void animateClose();

    PopupControl() : popup(NULL), closing(false) {

    }

    ~PopupControl() {
        close();
    }
};

/// base class for full screen widgets, supporting popups
class CRUIWindowWidget : public CRUILinearLayout {
protected:

    PopupControl _popupControl;

    void preparePopup(CRUIWidget * widget, int location, const lvRect & margins);

    /// draws popup above content
    virtual void drawPopup(LVDrawBuf * buf);

public:
    CRUIWindowWidget() : CRUILinearLayout(true) {}
    virtual ~CRUIWindowWidget() {  }
    /// overriden to treat popup as first child
    virtual int getChildCount();
    /// overriden to treat popup as first child
    virtual CRUIWidget * getChild(int index);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);

};



#endif /* CRUILAYOUT_H_ */
