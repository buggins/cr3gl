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

};


#endif /* CRUIREADWIDGET_H_ */
