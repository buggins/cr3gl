/*
 * cruireadwidget.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */


#include "cruireadwidget.h"
#include "crui.h"
#include "cruimain.h"
#include "gldrawbuf.h"

using namespace CRUI;

CRUIReadWidget::CRUIReadWidget(CRUIMainWidget * main) : _main(main),
    _isDragging(false), _dragStartOffset(0)
{
    _docview = new LVDocView();
    _docview->setViewMode(DVM_SCROLL, 1);
    _docview->setFontSize(26);
}

CRUIReadWidget::~CRUIReadWidget() {}

/// measure dimensions
void CRUIReadWidget::measure(int baseWidth, int baseHeight) {
    _measuredWidth = baseWidth;
    _measuredHeight = baseHeight;
}

/// updates widget position based on specified rectangle
void CRUIReadWidget::layout(int left, int top, int right, int bottom) {
    _pos.left = left;
    _pos.top = top;
    _pos.bottom = bottom;
    _pos.right = right;
    _docview->Resize(right-left, bottom-top);
}

/// draws widget with its children to specified surface
void CRUIReadWidget::draw(LVDrawBuf * buf) {
    _scrollCache.prepare(_docview, _docview->GetPos(), _measuredWidth, _measuredHeight, 1);
    _scrollCache.draw(buf, _docview->GetPos(), _pos.left, _pos.top);
    //_docview->Draw(*buf, false);
}

bool CRUIReadWidget::openBook(lString8 pathname) {
    bool res = _docview->LoadDocument(pathname.c_str());
    if (!res) {
        _docview->createDefaultDocument(lString16("Cannot open document"), lString16("Error occured while trying to open document"));
    }
    return res;
}

#define DRAG_THRESHOLD 5
#define DRAG_THRESHOLD_X 15
#define SCROLL_SPEED_CALC_INTERVAL 2000
#define SCROLL_MIN_SPEED 3
#define SCROLL_FRICTION 13

void CRUIReadWidget::animate(lUInt64 millisPassed) {
    bool changed = _scroll.animate(millisPassed);
    if (changed) {
        int oldpos = _docview->GetPos();
        //CRLog::trace("scroll animation: new position %d", _scroll.pos());
        _docview->SetPos(_scroll.pos(), false);
        if (oldpos == _docview->GetPos()) {
            //CRLog::trace("scroll animation - stopping at %d since set position not changed position", _scroll.pos());
            // stopping: bounds
            _scroll.stop();
        }
    }
}

bool CRUIReadWidget::isAnimating() {
    return _scroll.isActive();
}

void CRUIReadWidget::animateScrollTo(int newpos, int speed) {
    CRLog::trace("animateScrollTo( %d -> %d )", _docview->GetPos(), newpos);
    _scroll.start(_docview->GetPos(), newpos, speed, SCROLL_FRICTION);
    //invalidate();
    //_main->update();
}

bool CRUIReadWidget::doCommand(int cmd, int param) {
    int pos = _docview->GetPos();
    int newpos = pos;
    int speed = 0;
    switch (cmd) {
    case DCMD_PAGEUP:
        newpos = pos - _pos.height() * 9 / 10;
        speed = _pos.height() * 2;
        break;
    case DCMD_PAGEDOWN:
        newpos = pos + _pos.height() * 9 / 10;
        speed = _pos.height() * 2;
        break;
    case DCMD_LINEUP:
        newpos = pos - _docview->getFontSize();
        speed = _pos.height() / 2;
        break;
    case DCMD_LINEDOWN:
        newpos = pos + _docview->getFontSize();
        speed = _pos.height() / 2;
        break;
    default:
        return _docview->doCommand((LVDocCmd)cmd, param);
    }
    if (pos != newpos) {
        animateScrollTo(newpos, speed);
    }
}

bool CRUIReadWidget::onKeyEvent(const CRUIKeyEvent * event) {
    if (event->getType() == KEY_ACTION_PRESS) {
        if (_scroll.isActive())
            _scroll.stop();
        int key = event->key();
        //CRLog::trace("keyDown(0x%04x) oldpos=%d", key,  _docview->GetPos());
        switch(key) {
        case CR_KEY_PGDOWN:
        case CR_KEY_SPACE:
            doCommand(DCMD_PAGEDOWN);
            break;
        case CR_KEY_PGUP:
            doCommand(DCMD_PAGEUP);
            break;
        case CR_KEY_HOME:
            _docview->doCommand(DCMD_BEGIN);
            break;
        case CR_KEY_END:
            _docview->doCommand(DCMD_END);
            break;
        case CR_KEY_UP:
            doCommand(DCMD_LINEUP, 1);
            break;
        case CR_KEY_DOWN:
            doCommand(DCMD_LINEDOWN, 1);
            break;
        default:
            break;
        }
    }
    //CRLog::trace("new pos=%d", _docview->GetPos());
    invalidate();
    return true;
}

/// motion event handler, returns true if it handled event
bool CRUIReadWidget::onTouchEvent(const CRUIMotionEvent * event) {
    int action = event->getAction();
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
    int dx = event->getX() - event->getStartX();
    int dy = event->getY() - event->getStartY();
    int delta = dy; //isVertical() ? dy : dx;
    int delta2 = dx; //isVertical() ? dy : dx;
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
    switch (action) {
    case ACTION_DOWN:
        _isDragging = false;
        _dragStart.x = event->getX();
        _dragStart.y = event->getY();
        _dragStartOffset = _docview->GetPos();
        if (_scroll.isActive())
            _scroll.stop();
        invalidate();
        //CRLog::trace("list DOWN");
        break;
    case ACTION_UP:
        {
            invalidate();
            if (_isDragging) {
                lvPoint speed = event->getSpeed(SCROLL_SPEED_CALC_INTERVAL);
                if (speed.y < -SCROLL_MIN_SPEED || speed.y > SCROLL_MIN_SPEED) {
                    _scroll.start(_docview->GetPos(), -speed.y, SCROLL_FRICTION);
                    CRLog::trace("Starting scroll with speed %d", _scroll.speed());
                }
            }
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
        } else if (!_isDragging) {
            if ((delta2 > DRAG_THRESHOLD_X) || (-delta2 > DRAG_THRESHOLD_X)) {
                _main->startDragging(event, false);
            }
        }
        // ignore
        //CRLog::trace("list MOVE");
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
    }
    return true;
}


CRUIReadWidget::ScrollModePageCache::ScrollModePageCache() : minpos(0), maxpos(0) {

}

LVDrawBuf * CRUIReadWidget::ScrollModePageCache::createBuf(int dx, int dy) {
    return new GLDrawBuf(dx, dy, 32, true);
}

void CRUIReadWidget::ScrollModePageCache::setSize(int _dx, int _dy) {
    if (dx != _dx || dy != _dy) {
        clear();
        dx = _dx;
        dy = _dy;
    }
}

/// ensure images are prepared
void CRUIReadWidget::ScrollModePageCache::prepare(LVDocView * _docview, int _pos, int _dx, int _dy, int direction) {
    setSize(_dx, _dy);
    if (_pos >= minpos && _pos + dy <= maxpos)
        return; // already prepared
    int y0 = direction > 0 ? (_pos - dy / 4) : (_pos - dy * 3 / 4);
    int y1 = direction > 0 ? (_pos + dy + dy * 3 / 4) : (_pos + dy + dy / 4);
    int pos0 = _pos / dy * dy;
    for (int i = pages.length() - 1; i >= 0; i--) {
        ScrollModePage * p = pages[i];
        if (!p->intersects(y0, y1)) {
            pages.remove(i);
            delete p;
        }
    }
    for (int i = 0; i < 2; i++) {
        int pos = pos0 + i * dy;
        bool found = false;
        for (int k = pages.length() - 1; k >= 0; k--) {
            if (pages[k]->pos == pos) {
                found = true;
                break;
            }
        }
        if (!found) {
            ScrollModePage * page = new ScrollModePage();
            page->dx = dx;
            page->dy = dy;
            page->pos = pos;
            page->drawbuf = createBuf(dx, dy);
            GLDrawBuf * buf = dynamic_cast<GLDrawBuf*>(page->drawbuf);
            buf->beforeDrawing();
            int oldpos = _docview->GetPos();
            _docview->SetPos(pos, false);
            _docview->Draw(*buf, false);
            _docview->SetPos(oldpos, false);
            buf->afterDrawing();
            pages.add(page);
            CRLog::trace("new page cache item %d..%d", page->pos, page->pos + page->dy);
        }
    }
    minpos = maxpos = -1;
    for (int k = 0; k < pages.length(); k++) {
        CRLog::trace("page cache item [%d] %d..%d", k, pages[k]->pos, pages[k]->pos + pages[k]->dy);
        if (minpos == -1 || minpos > pages[k]->pos) {
            minpos = pages[k]->pos;
        }
        if (maxpos == -1 || maxpos < pages[k]->pos + pages[k]->dy) {
            maxpos = pages[k]->pos + pages[k]->dy;
        }
    }
}

void CRUIReadWidget::ScrollModePageCache::draw(LVDrawBuf * dst, int pos, int x, int y) {
    GLDrawBuf * glbuf = dynamic_cast<GLDrawBuf *>(dst);
    if (glbuf) {
        //glbuf->beforeDrawing();
        for (int k = pages.length() - 1; k >= 0; k--) {
            if (pages[k]->intersects(pos, pos + dy)) {
                // draw fragment
                int y0 = pages[k]->pos - pos;
                pages[k]->drawbuf->DrawTo(glbuf, x, y + y0, 0, NULL);
            }
        }
        //glbuf->afterDrawing();
    }
}

void CRUIReadWidget::ScrollModePageCache::clear() {
    pages.clear();
    minpos = 0;
    maxpos = 0;
}
