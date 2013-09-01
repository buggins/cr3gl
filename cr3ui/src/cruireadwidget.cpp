/*
 * cruireadwidget.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */


#include "cruireadwidget.h"
#include "crui.h"
#include "cruimain.h"

using namespace CRUI;

CRUIReadWidget::CRUIReadWidget(CRUIMainWidget * main) : _main(main), _isDragging(false), _dragStartOffset(0) {
    _docview = new LVDocView();
    _docview->setViewMode(DVM_SCROLL, 1);
    _docview->setFontSize(26);
}

CRUIReadWidget::~CRUIReadWidget() {}

/// measure dimensions
void CRUIReadWidget::measure(int baseWidth, int baseHeight) {

}

/// updates widget position based on specified rectangle
void CRUIReadWidget::layout(int left, int top, int right, int bottom) {
    _docview->Resize(right-left, bottom-top);
}

/// draws widget with its children to specified surface
void CRUIReadWidget::draw(LVDrawBuf * buf) {
    _docview->Draw(*buf, false);
}

bool CRUIReadWidget::openBook(lString8 pathname) {
    return _docview->LoadDocument(pathname.c_str());
}

#define DRAG_THRESHOLD 5

/// motion event handler, returns true if it handled event
bool CRUIReadWidget::onTouchEvent(const CRUIMotionEvent * event) {
    int action = event->getAction();
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
    //int dx = event->getX() - event->getStartX();
    int dy = event->getY() - event->getStartY();
    int delta = dy; //isVertical() ? dy : dx;
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
    switch (action) {
    case ACTION_DOWN:
        _isDragging = false;
        _dragStart.x = event->getX();
        _dragStart.y = event->getY();
        _dragStartOffset = _docview->GetPos();
        invalidate();
        //CRLog::trace("list DOWN");
        break;
    case ACTION_UP:
        {
            invalidate();
            _dragStartOffset = 0; //NO_DRAG;
            _isDragging = false;
//            setScrollOffset(_scrollOffset);
//            if (itemIndex != -1) {
//                //CRLog::trace("UP ts=%lld downTs=%lld downDuration=%lld", event->getEventTimestamp(), event->getDownEventTimestamp(), event->getDownDuration());
//                bool isLong = event->getDownDuration() > LONG_TOUCH_THRESHOLD; // 0.5 seconds threshold
//                if (isLong && onItemLongClickEvent(itemIndex))
//                    return true;
//                onItemClickEvent(itemIndex);
//            }
        }
        // fire onclick
        //CRLog::trace("list UP");
        break;
    case ACTION_FOCUS_IN:
//        if (isDragging)
//            setScrollOffset(_dragStartOffset - delta);
//        else
//            _selectedItem = index;
        invalidate();
        //CRLog::trace("list FOCUS IN");
        break;
    case ACTION_FOCUS_OUT:
//        if (isDragging)
//            setScrollOffset(_dragStartOffset - delta);
//        else
//            _selectedItem = -1;
        invalidate();
        return false; // to continue tracking
        //CRLog::trace("list FOCUS OUT");
        break;
    case ACTION_CANCEL:
        _isDragging = false;
        //setScrollOffset(_scrollOffset);
        //CRLog::trace("list CANCEL");
        break;
    case ACTION_MOVE:
        if (!_isDragging && ((delta > DRAG_THRESHOLD) || (-delta > DRAG_THRESHOLD))) {
            _isDragging = true;
            _docview->SetPos(_dragStartOffset - delta, false);
            invalidate();
            _main->update();
        } else if (_isDragging) {
            _docview->SetPos(_dragStartOffset - delta, false);
            invalidate();
            _main->update();
        }
        // ignore
        //CRLog::trace("list MOVE");
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
    }
    return true;
}

