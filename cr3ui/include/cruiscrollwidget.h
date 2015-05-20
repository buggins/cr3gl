#ifndef CRUISCROLLWIDGET_H
#define CRUISCROLLWIDGET_H

#include "cruilist.h"
#include "cruilayout.h"
#include "cruicontrols.h"

class CRUIScrollWidget : public CRUILinearLayout, CRUIOnScrollPosCallback {
protected:
    int _scrollOffset;
    int _maxScrollOffset;
    int _totalSize;
    int _visibleSize;
    int _dragStartOffset;
    bool _sbHidden;
    lvRect _clientRect;
    lvRect _sbRect;
    CRUIDragListener * _onStartDragCallback;
    ScrollControl _scroll;
    CRUIScrollBar * _scrollBar;
public:

    CRUIScrollWidget(bool vertical = true);
    virtual ~CRUIScrollWidget();

    // scrollbar as a child
    virtual int getChildCount();
    virtual CRUIWidget * getChild(int index);
    /// returns true if widget is child of this
    virtual bool isChild(CRUIWidget * widget);

    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual);

    virtual CRUIDragListener * getOnDragListener() { return _onStartDragCallback; }
    virtual void setOnDragListener(CRUIDragListener * listener) { _onStartDragCallback = listener; }

    /// sets orientation
    virtual CRUILinearLayout * setVertical(bool vertical);

    virtual void updateScrollBar();
    virtual int getScrollOffset() { return _scrollOffset; }
    virtual void setScrollOffset(int offset);

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


#endif // CRUISCROLLWIDGET_H
