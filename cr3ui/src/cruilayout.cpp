/*
 * cruilayout.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruilayout.h"
#include "crui.h"
#include "gldrawbuf.h"

#define POPUP_ANIMATION_DURATION 200

using namespace CRUI;

// Vertical Layout
/// measure dimensions
void CRUILinearLayout::measure(int baseWidth, int baseHeight) {
	lvRect padding;
	getPadding(padding);
	lvRect margin = getMargin();
    int maxw = baseWidth;
    int maxh = baseHeight;
    if (getMaxWidth() != UNSPECIFIED && maxw > getMaxWidth())
        maxw = getMaxWidth();
    if (getMaxHeight() != UNSPECIFIED && maxh > getMaxHeight())
        maxh = getMaxHeight();
    maxw = maxw - (margin.left + margin.right + padding.left + padding.right);
    maxh = maxh - (margin.top + margin.bottom + padding.top + padding.bottom);
	int totalWeight = 0;
	if (_isVertical) {
		int biggestw = 0;
		int totalh = 0;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			if (child->getLayoutHeight() == CRUI::FILL_PARENT) {
				totalWeight += child->getLayoutWeight();
			} else {
				child->measure(maxw, maxh);
				totalh += child->getMeasuredHeight();
				if (biggestw < child->getMeasuredWidth())
					biggestw = child->getMeasuredWidth();
			}
		}
		int hleft = maxh - totalh;
		if (totalWeight < 1)
			totalWeight = 1;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			if (child->getLayoutHeight() == CRUI::FILL_PARENT) {
				int h = hleft * child->getLayoutWeight() / totalWeight;
				child->measure(maxw, h);
				totalh += child->getMeasuredHeight();
				if (biggestw < child->getMeasuredWidth())
					biggestw = child->getMeasuredWidth();
			}
		}
		if (biggestw > maxw)
			biggestw = maxw;
		if (totalh > maxh)
			totalh = maxh;
		defMeasure(baseWidth, baseHeight, biggestw, totalh);
	} else {
		int biggesth = 0;
		int totalw = 0;

		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			if (child->getLayoutWidth() == CRUI::FILL_PARENT) {
				totalWeight += child->getLayoutWeight();
			} else {
				child->measure(maxw, maxh);
				totalw += child->getMeasuredWidth();
				if (biggesth < child->getMeasuredHeight())
					biggesth = child->getMeasuredHeight();
			}
		}
		int wleft = maxw - totalw;
		if (totalWeight < 1)
			totalWeight = 1;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			if (child->getLayoutWidth() == CRUI::FILL_PARENT) {
				int w = wleft * child->getLayoutWeight() / totalWeight;
				child->measure(w, maxh);
				totalw += child->getMeasuredWidth();
				if (biggesth < child->getMeasuredHeight())
					biggesth = child->getMeasuredHeight();
			}
		}
		if (biggesth > maxh)
			biggesth = maxh;
		if (totalw > maxw)
			totalw = maxw;
		defMeasure(baseWidth, baseHeight, totalw, biggesth);
	}
}

/// updates widget position based on specified rectangle
void CRUILinearLayout::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
	lvRect clientRc = _pos;
	applyMargin(clientRc);
	applyPadding(clientRc);
	lvRect childRc = clientRc;
	if (_isVertical) {
		int y = childRc.top;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			childRc.top = y;
			if (i < getChildCount() - 1)
				childRc.bottom = y + child->getMeasuredHeight();
			else
				childRc.bottom = clientRc.bottom;
			if (childRc.top > clientRc.bottom)
				childRc.top = clientRc.bottom;
			if (childRc.bottom > clientRc.bottom)
				childRc.bottom = clientRc.bottom;
			child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			y = childRc.bottom;
		}
	} else {
		int x = childRc.left;
		for (int i=0; i<getChildCount(); i++) {
			CRUIWidget * child = getChild(i);
			childRc.left = x;
			if (i < getChildCount() - 1)
				childRc.right = x + child->getMeasuredWidth();
			else
				childRc.right = clientRc.right;
			if (childRc.left > clientRc.right)
				childRc.left = clientRc.right;
			if (childRc.right > clientRc.right)
				childRc.right = clientRc.right;
			child->layout(childRc.left, childRc.top, childRc.right, childRc.bottom);
			x = childRc.right;
		}
	}
}



//=======================================================================
// Container Widget

/// draws widget with its children to specified surface
void CRUIContainerWidget::draw(LVDrawBuf * buf) {
	CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	for (int i=0; i<getChildCount(); i++) {
		getChild(i)->draw(buf);
	}
}




//==========================================================

int CRUIWindowWidget::getChildCount() {
    return _children.length() + (_popupControl.popup ? 1 : 0);
}

CRUIWidget * CRUIWindowWidget::getChild(int index) {
    if (_popupControl.popup) {
        if (index == 0)
            return _popupControl.popup;
        else
            return _children.get(index - 1);
    } else {
        return _children.get(index);
    }
}

class CRUIPopupFrame : public CRUILinearLayout {
    PopupControl * _control;
    CRUIWidget * _body;
    CRUIWidget * _drawer;
public:
    CRUIPopupFrame(PopupControl * control, CRUIWidget * body, int drawerLocation) : CRUILinearLayout(drawerLocation == ALIGN_TOP || drawerLocation == ALIGN_BOTTOM), _control(control), _drawer(NULL) {
        _body = body;
        if (drawerLocation) {
            _drawer = new CRUIWidget();
            if (drawerLocation == ALIGN_TOP) {
                _drawer->setMinHeight(MIN_ITEM_PX / 2);
                _drawer->setMaxHeight(MIN_ITEM_PX / 2);
                _drawer->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
                addChild(_drawer);
                addChild(_body);
                setLayoutParams(FILL_PARENT, WRAP_CONTENT);
            } else if (drawerLocation == ALIGN_BOTTOM) {
                _drawer->setMinHeight(MIN_ITEM_PX / 2);
                _drawer->setMaxHeight(MIN_ITEM_PX / 2);
                _drawer->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
                addChild(_body);
                addChild(_drawer);
                setLayoutParams(FILL_PARENT, WRAP_CONTENT);
            } else if (drawerLocation == ALIGN_LEFT) {
                _drawer->setMinWidth(MIN_ITEM_PX / 2);
                _drawer->setMaxWidth(MIN_ITEM_PX / 2);
                _drawer->setLayoutParams(WRAP_CONTENT, FILL_PARENT);
                addChild(_drawer);
                addChild(_body);
                setLayoutParams(WRAP_CONTENT, FILL_PARENT);
            } else if (drawerLocation == ALIGN_RIGHT) {
                _drawer->setMinWidth(MIN_ITEM_PX / 2);
                _drawer->setMaxWidth(MIN_ITEM_PX / 2);
                _drawer->setLayoutParams(WRAP_CONTENT, FILL_PARENT);
                addChild(_body);
                addChild(_drawer);
                setLayoutParams(WRAP_CONTENT, FILL_PARENT);
            }
            _drawer->setStyle("POPUP_FRAME_DRAWER");
        } else {
            addChild(_body);
            setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        }
        setStyle("POPUP_FRAME");
    }

    virtual void animate(lUInt64 millisPassed) {
        CR_UNUSED(millisPassed);
        lInt64 ts = GetCurrentTimeMillis();
        if (ts <= _control->startTs) {
            _control->progress = 0;
        } else if (ts < _control->endTs) {
            _control->progress = (int)((ts - _control->startTs) * 1000 / (_control->endTs - _control->startTs));
        } else {
            _control->progress = 1000;
        }
        if (_control->closing && _control->progress >= 1000) {
            _control->close();
        }
    }

    virtual bool isAnimating() {
        return _control->progress < 1000;
    }

    /// returns true if point is inside control (excluding margins)
    virtual bool isPointInside(int x, int y) {
        CR_UNUSED2(x, y);
        // to handle all touch events
        return true;
    }
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event) {
        if (!_body->isPointInside(event->getX(), event->getY())) {
            if (event->getAction() == ACTION_UP)
                _control->animateClose();
            return true;
        }
        return true;
    }

    /// key event handler, returns true if it handled event
    virtual bool onKeyEvent(const CRUIKeyEvent * event) {
        if (event->key() == CR_KEY_BACK || event->key() == CR_KEY_ESC) {
            if (event->getType() == KEY_ACTION_RELEASE) {
                _control->animateClose();
            }
            return true;
        }
        return true;
    }
};

/// draws popup above content
void CRUIWindowWidget::drawPopup(LVDrawBuf * buf) {
    if (_popupControl.popup) {
        // outer space background
        buf->FillRect(_pos, _popupControl.getColor());
        lvRect rc;
        _popupControl.getRect(rc);
        _popupControl.popup->layout(rc.left, rc.top, rc.right, rc.bottom);
        _popupControl.popup->draw(buf);
    }
}

/// draws widget with its children to specified surface
void CRUIWindowWidget::draw(LVDrawBuf * buf) {
    CRUILinearLayout::draw(buf);
    drawPopup(buf);
}

/// start animation of popup closing
void PopupControl::animateClose() {
    if (!popup || closing)
        return;
    startTs = GetCurrentTimeMillis();
    endTs = startTs + POPUP_ANIMATION_DURATION;
    progress = 0;
    closing = true;
}

void PopupControl::layout(const lvRect & pos) {
    popup->measure(pos.width() - margins.left - margins.right, pos.height() - margins.top - margins.bottom);
    width = popup->getMeasuredWidth();
    height = popup->getMeasuredHeight();
    popup->layout(0, 0, width, height);
    lvRect rc = pos;
    lvRect srcrc = pos;
    if (align == ALIGN_TOP) {
        rc.bottom = rc.top + height;
        rc.left += margins.left;
        rc.right -= margins.right;
        srcrc = rc;
        srcrc.top -= height;
        srcrc.bottom -= height;
    } else if (align == ALIGN_BOTTOM) {
        rc.top = rc.bottom - height;
        rc.left += margins.left;
        rc.right -= margins.right;
        srcrc = rc;
        srcrc.top += height;
        srcrc.bottom += height;
    } else if (align == ALIGN_LEFT) {
        rc.right = rc.left + width;
        rc.top += margins.top;
        rc.bottom -= margins.bottom;
        srcrc = rc;
        srcrc.left -= width;
        srcrc.right -= width;
    } else if (align == ALIGN_RIGHT) {
        rc.left = rc.right - width;
        rc.top += margins.top;
        rc.bottom -= margins.bottom;
        srcrc = rc;
        srcrc.left += width;
        srcrc.right += width;
    } else {
        // center
        int dw = pos.width() - margins.left - margins.right - width;
        int dh = pos.height() - margins.top - margins.bottom - height;
        rc.left += dw / 2 + margins.left;
        rc.top += dh / 2 + margins.top;
        rc.right = rc.left + width;
        rc.bottom = rc.top+ height;
        srcrc = rc;
    }
    srcRect = srcrc;
    dstRect = rc;
}

lUInt32 PopupControl::getColor() {
    int p = closing ? 1000 - progress : progress;
    if (p <= 0) {
        return 0xFF000000 | outerColor;
    } else if (p >= 1000) {
        return outerColor;
    } else {
        int dstalpha = (outerColor >> 24) & 0xFF;
        int alpha = 255 + (dstalpha - 255) * p / 1000;
        if (alpha < 0)
            alpha = 0;
        else if (alpha > 255)
            alpha = 255;
        return (outerColor & 0xFFFFFF) | (alpha << 24);
    }
}

void PopupControl::getRect(lvRect & rc) {
    int p = closing ? 1000 - progress : progress;
    if (p <= 0) {
        rc = srcRect;
    } else if (p >= 1000) {
        rc = dstRect;
    } else {
        rc.left = srcRect.left + (dstRect.left - srcRect.left) * p / 1000;
        rc.right = srcRect.right + (dstRect.right - srcRect.right) * p / 1000;
        rc.top = srcRect.top + (dstRect.top - srcRect.top) * p / 1000;
        rc.bottom = srcRect.bottom + (dstRect.bottom - srcRect.bottom) * p / 1000;
    }

}

void CRUIWindowWidget::preparePopup(CRUIWidget * widget, int location, const lvRect & margins) {
    int handleLocation = 0;
    if (location == ALIGN_TOP)
        handleLocation = ALIGN_BOTTOM;
    else if (location == ALIGN_BOTTOM)
        handleLocation = ALIGN_TOP;
    else if (location == ALIGN_LEFT)
        handleLocation = ALIGN_RIGHT;
    else if (location == ALIGN_RIGHT)
        handleLocation = ALIGN_LEFT;
    CRUIPopupFrame * frame = new CRUIPopupFrame(&_popupControl, widget, handleLocation);
    widget = frame;
    _popupControl.close();
    _popupControl.popup = widget;
    _popupControl.align = location;
    _popupControl.margins = margins;
    _popupControl.layout(_pos);
    _popupControl.startTs = GetCurrentTimeMillis();
    _popupControl.endTs = _popupControl.startTs + POPUP_ANIMATION_DURATION;
    _popupControl.progress = 0;
    _popupControl.closing = false;
    _popupControl.outerColor = 0x80404040;
}
