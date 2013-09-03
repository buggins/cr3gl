/*
 * cruireadwidget.h
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

#ifndef CRUIREADWIDGET_H_
#define CRUIREADWIDGET_H_


#include "cruiwidget.h"

class CRUIMainWidget;
class CRUIReadWidget : public CRUIWidget {
    CRUIMainWidget * _main;
    LVDocView * _docview;
    bool _isDragging;
    lvPoint _dragStart;
    int _dragStartOffset;
    ScrollControl _scroll;

    class ScrollModePage {
    public:
        int pos;
        int dx;
        int dy;
        LVDrawBuf * drawbuf;
        ScrollModePage() : pos(0), dx(0), dy(0), drawbuf(NULL) { }
        ~ScrollModePage() {
            if (drawbuf)
                delete drawbuf;
        }
        bool intersects(int y0, int y1) {
            return !(pos + dy <= y0 || pos >= y1);
        }
    };
    class ScrollModePageCache {
        LVPtrVector<ScrollModePage> pages;
        LVDrawBuf * createBuf(int dx, int dy);
        int minpos;
        int maxpos;
        int dx;
        int dy;
    public:
        void setSize(int _dx, int _dy);
        ScrollModePageCache();
        /// ensure images are prepared
        void prepare(LVDocView * docview, int pos, int dx, int dy, int direction);
        /// draw
        void draw(LVDrawBuf * dst, int pos, int x, int y);
        /// remove images from cache
        void clear();
    };
    ScrollModePageCache _scrollCache;

    void animateScrollTo(int newpos, int speed);

public:
    CRUIReadWidget(CRUIMainWidget * main);
	virtual ~CRUIReadWidget();

    bool openBook(lString8 pathname);

	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);

    virtual void animate(lUInt64 millisPassed);
    virtual bool isAnimating();

    /// motion event handler, returns true if it handled event
    bool onTouchEvent(const CRUIMotionEvent * event);
    bool onKeyEvent(const CRUIKeyEvent * event);
    bool doCommand(int cmd, int param = 0);

};


#endif /* CRUIREADWIDGET_H_ */
