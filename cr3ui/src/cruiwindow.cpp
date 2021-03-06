#include "cruiwindow.h"
#include "crui.h"
#include "cruimain.h"
#include "cruiconfig.h"

using namespace CRUI;

#define POPUP_ANIMATION_DURATION 200
//#define POPUP_ANIMATION_DURATION 2000

//==========================================================

int CRUIWindowWidget::getChildCount() {
    return _children.length() + (_popupControl.popup ? 1 : 0) + (_popupControl.popupBackground ? 1 : 0);
}

CRUIWidget * CRUIWindowWidget::getChild(int index) {
    if (index < _children.length())
        return _children[index];
    if (index == _children.length()) {
        if (_popupControl.popupBackground)
            return _popupControl.popupBackground;
    }
    return _popupControl.popup;
}

/// sets widget as default for focus
void CRUIWindowWidget::setDefaultWidget(CRUIWidget * widget) {
    if (!widget) {
        _defaultControlId.clear();
        return;
    }
    lString8 id = widget->getId();
    _defaultControlId = id;
}

/// returns default widget for window, NULL if not found or deleted
CRUIWidget * CRUIWindowWidget::getDefaultWidget() {
    if (_defaultControlId.empty())
        return NULL;
    return childById(_defaultControlId);
}

/// returns focused widget inside window, NULL if not found
CRUIWidget * CRUIWindowWidget::getFocusedWidget() {
    CRUIWidget * res = CRUIEventManager::getFocusedWidget();
    if (isChild(res))
        return res;
    return NULL;
}

static bool isBetterDistance(int bestd, int newd) {
    if (bestd >= 0 && newd < 0)
        return false;
    if (bestd < 0 && newd >= 0)
        return true;
    if (bestd < 0 && newd < 0)
        return bestd > newd; // turn around
    // both positive
    return bestd > newd;
}

static bool smallDist(int n1, int n2) {
    int d = n1 - n2;
    return (d <= MIN_ITEM_PX / 10 && d >= - MIN_ITEM_PX / 10);
}

static bool isBetterFocusPosition(CRUI_FOCUS_OP op, lvRect & currpos, lvRect & bestpos, lvRect & newpos) {
    int dx = newpos.left - currpos.left;
    int dy = newpos.top - currpos.top;
    int bdx = bestpos.left - currpos.left;
    int bdy = bestpos.top - currpos.top;
    switch(op) {
    case FOCUS_PREV:
    case FOCUS_LEFT:
        if (smallDist(dy, bdy)) {
            // on the same line
            bool back = dy < 0;
            return isBetterDistance(-bdx, -dx) ^ back;
        } else {
            // different lines
            return isBetterDistance(-(bdx + bdy * 1000), -(dx + dy * 1000));
        }
    case FOCUS_NEXT:
    case FOCUS_RIGHT:
        if (smallDist(dy, bdy)) {
            // on the same line
            bool back = dy < 0;
            return isBetterDistance(bdx, dx) ^ back;
        } else {
            // different lines
            return isBetterDistance((bdx + bdy * 1000), (dx + dy * 1000));
        }
    case FOCUS_UP:
        if (smallDist(dx, bdx)) {
            // on the same line
            bool back = dx < 0;
            return isBetterDistance(-bdy, -dy) ^ back;
        } else {
            // different lines
            return isBetterDistance(-(bdy + bdx * 1000), -(dy + dx * 1000));
        }
    case FOCUS_DOWN:
        if (smallDist(dx, bdx)) {
            // on the same line
            bool back = dx < 0;
            return isBetterDistance(bdy, dy) ^ back;
        } else {
            // different lines
            return isBetterDistance((bdy + bdx * 1000), (dy + dx * 1000));
        }
    default:
        return false;
    }
}

void CRUIWindowWidget::findBestFocus(CRUI_FOCUS_OP op, CRUIWidget * root, CRUIWidget * current, lvRect & currentPos, lvRect & bestFoundPos, CRUIWidget * & bestFoundWidget) {
    if (root->getVisibility() != VISIBLE)
        return;
    if (current && root == current)
        return;
    if (root->canFocus()) {
        lvRect pos = root->getPos();
        if (bestFoundWidget) {
            if (!isBetterFocusPosition(op, currentPos, bestFoundPos, pos))
                return; // already found better choice
        }
        bestFoundPos = pos;
        bestFoundWidget = root;
    } else {
        // recursive search
        for (int i = 0; i < root->getChildCount(); i++) {
            findBestFocus(op, root->getChild(i), current, currentPos, bestFoundPos, bestFoundWidget);
        }
    }
}

/// moves focus
bool CRUIWindowWidget::moveFocus(CRUI_FOCUS_OP op) {
    CRUIWidget * current = getFocusedWidget();
    lvRect rc;
    if (current) {
        rc = current->getPos();
    }
    if (op == FOCUS_OFF) {
        CRUIEventManager::dispatchFocusChange(NULL);
        invalidate();
        return true;
    }
    if (op == FOCUS_DEFAULT || current == NULL) {
        CRUIEventManager::dispatchFocusChange(getDefaultWidget());
        invalidate();
        return true;
    }
    lvRect bestPos;
    CRUIWidget * bestWidget = NULL;
    findBestFocus(op, this, current, rc, bestPos, bestWidget);
    if (bestWidget) {
        CRLog::trace("new focus: {%d,%d,%d,%d}", bestPos.left, bestPos.top, bestPos.right, bestPos.bottom);
        CRUIEventManager::dispatchFocusChange(bestWidget);
        CRUIImageRef bg = bestWidget->getBackground();
        invalidate();
        return true;
    }
    return false;
}

void CRUIWindowWidget::afterNavigationTo() {
    if (!crconfig.touchMode)
        moveFocus(FOCUS_DEFAULT);
}

/// return true if drag operation is intercepted
bool CRUIWindowWidget::onStartDragging(const CRUIMotionEvent * event, bool vertical) {
    return getMain()->startDragging(event, vertical);
}

CRUI_FOCUS_OP keyToFocusOp(int key, bool shift) {
    switch(key) {
    case CR_KEY_UP:
        return FOCUS_UP;
    case CR_KEY_DOWN:
        return FOCUS_DOWN;
    case CR_KEY_LEFT:
        return FOCUS_LEFT;
    case CR_KEY_RIGHT:
        return FOCUS_RIGHT;
    case CR_KEY_TAB:
        return shift ? FOCUS_PREV : FOCUS_NEXT;
    default:
        return FOCUS_OFF;
    }
}

bool CRUIWindowWidget::onKeyEvent(const CRUIKeyEvent * event) {
    int key = event->key();
	//CRLog::trace("CRUIWindowWidget::onKeyEvent(%d  0x%x)", key, key);
    if (_popupControl.popup && event->getType() == KEY_ACTION_PRESS && key == CR_KEY_MENU) {
    	return true;
    }
    if (_popupControl.popup && event->getType() == KEY_ACTION_RELEASE && key == CR_KEY_MENU) {
		_popupControl.close();
		invalidate();
    	return true;
    }
    bool altPressed = (event->modifiers() & CR_KEY_MODIFIER_ALT) != 0;
#if CR3_OSX
    altPressed = altPressed ||  ((event->modifiers() & CR_KEY_MODIFIER_CONTROL) != 0);
#endif
    if (event->getType() == KEY_ACTION_PRESS) {
        if (key == CR_KEY_RETURN && altPressed != 0) {
            onAction(ACTION_TOGGLE_FULLSCREEN);
            return true;
        }
        if (key == CR_KEY_ESC || key == CR_KEY_BACK) {
            return true;
        }
        CRUI_FOCUS_OP focusOp = keyToFocusOp(key, (event->modifiers() & CR_KEY_MODIFIER_SHIFT) != 0);
        if (focusOp != FOCUS_OFF) {
            if (moveFocus(focusOp)) {
                _main->update(false);
            }
            return true;
        }
    } else if (event->getType() == KEY_ACTION_RELEASE) {
        if (key == CR_KEY_RETURN && (event->modifiers() & CR_KEY_MODIFIER_ALT) != 0)
            return true;
        if (key == CR_KEY_ESC || key == CR_KEY_BACK) {
        	if (_popupControl.popup) {
        		_popupControl.close();
        		invalidate();
        	} else {
        		_main->back();
        	}
            return true;
        }
        CRUI_FOCUS_OP focusOp = keyToFocusOp(key, (event->modifiers() & CR_KEY_MODIFIER_SHIFT) != 0);
        if (focusOp != FOCUS_OFF) {
            return true;
        }
    }
    return false;
}



class CRUIPopupFrame : public CRUILinearLayout {
    PopupControl * _control;
    CRUIWidget * _body;
    CRUIWidget * _handle;
public:
    CRUIPopupFrame(PopupControl * control, CRUIWidget * body, int drawerLocation) : CRUILinearLayout(drawerLocation == ALIGN_TOP || drawerLocation == ALIGN_BOTTOM), _control(control), _handle(NULL) {
        _body = body;
        int handleSize = MIN_ITEM_PX / 5;
        if (drawerLocation) {
            _handle = new CRUIWidget();
            if (drawerLocation == ALIGN_TOP) {
                _handle->setMinHeight(handleSize);
                _handle->setMaxHeight(handleSize);
                _handle->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
                addChild(_handle);
                addChild(_body);
                setLayoutParams(FILL_PARENT, WRAP_CONTENT);
            } else if (drawerLocation == ALIGN_BOTTOM) {
                _handle->setMinHeight(handleSize);
                _handle->setMaxHeight(handleSize);
                _handle->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
                addChild(_body);
                addChild(_handle);
                setLayoutParams(FILL_PARENT, WRAP_CONTENT);
            } else if (drawerLocation == ALIGN_LEFT) {
                _handle->setMinWidth(handleSize);
                _handle->setMaxWidth(handleSize);
                _handle->setLayoutParams(WRAP_CONTENT, FILL_PARENT);
                addChild(_handle);
                addChild(_body);
                setLayoutParams(WRAP_CONTENT, FILL_PARENT);
            } else if (drawerLocation == ALIGN_RIGHT) {
                _handle->setMinWidth(handleSize);
                _handle->setMaxWidth(handleSize);
                _handle->setLayoutParams(WRAP_CONTENT, FILL_PARENT);
                addChild(_body);
                addChild(_handle);
                setLayoutParams(WRAP_CONTENT, FILL_PARENT);
            }
            _handle->setStyle("POPUP_FRAME_HANDLE");
        } else {
            addChild(_body);
            setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        }
        setStyle("POPUP_FRAME");
    }

    virtual ~CRUIPopupFrame() {
        CRLog::trace("~CRUIPopupFrame()");
    }

    /// call to set focus to appropriate child once widget appears on screen
    virtual bool initFocus() {
        return _body->initFocus();
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
            initFocus();
        }
        if (_control->closing && _control->progress >= 1000) {
            _control->close();
        }
    }

    virtual bool isAnimating() {
        return _control->progress < 1000 || isAnimatingRecursive();
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
            bool handled = false;
            if (_control->wantsTouchEventsOutside) {
                handled = _body->onTouchEvent(event);
            }
            if (event->getAction() == ACTION_UP && !handled) {
                invalidate();
                _control->animateClose();
                return true;
            }
        }
        return true;
    }

    /// key event handler, returns true if it handled event
    virtual bool onKeyEvent(const CRUIKeyEvent * event) {
        if (event->key() == CR_KEY_BACK || event->key() == CR_KEY_ESC || event->key() == CR_KEY_MENU) {
            if (event->getType() == KEY_ACTION_RELEASE) {
                _control->animateClose();
                invalidate();
            }
            return true;
        }
        return true;
    }
};

/// draws widget with its children to specified surface
void CRUIWindowWidget::draw(LVDrawBuf * buf) {
    _popupControl.updateLayout(_pos);
    CRUIFrameLayout::draw(buf);
}

/// start animation of popup closing
void PopupControl::animateClose() {
    if (!popup || closing)
        return;
    if (crconfig.einkMode) {
        close();
        return;
    }
    startTs = GetCurrentTimeMillis();
    endTs = startTs + POPUP_ANIMATION_DURATION;
    progress = 0;
    closing = true;
}

/// update current position based on src and dst rectangles and progress
void PopupControl::updateLayout(const lvRect & pos) {
    if (!popup)
        return;
    if (parentRect != pos)
        layout(parentRect);
    if (popupBackground) {
        popupBackground->layout(parentRect.left, parentRect.top, parentRect.right, parentRect.bottom);
        popupBackground->setBackground(getColor());
    }
    lvRect rc;
    getRect(rc);
    popup->layout(rc.left, rc.top, rc.right, rc.bottom);
}

void PopupControl::layout(const lvRect & pos) {
    parentRect = pos;
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

void PopupControl::close() {
    CRLog::trace("PopupControl::close()");
    if (popup) {
        if (owner)
            owner->onPopupClosing(body);
        delete popup;
    }
    popup = NULL;
    body = NULL;
    if (popupBackground)
        delete popupBackground;
    popupBackground = NULL;
}

void PopupControl::getRect(lvRect & rc) {
    int p = closing ? 1000 - progress : progress;
    if (p <= 0) {
        rc = closing ? dstRect : srcRect;
    } else if (p >= 1000) {
        rc = closing ? srcRect : dstRect;
    } else {
        rc.left = srcRect.left + (dstRect.left - srcRect.left) * p / 1000;
        rc.right = srcRect.right + (dstRect.right - srcRect.right) * p / 1000;
        rc.top = srcRect.top + (dstRect.top - srcRect.top) * p / 1000;
        rc.bottom = srcRect.bottom + (dstRect.bottom - srcRect.bottom) * p / 1000;
    }

}

void CRUIWindowWidget::preparePopup(CRUIWidget * body, int location, const lvRect & margins, int backgroundAlpha, bool showHandle, bool wantsTouchEventsOutside) {
    //CRLog::trace("preparing popup: it's %s", _popupControl.popup ? "already exist" : "not yet created");
    if (crconfig.einkMode)
        backgroundAlpha = 0;
    int handleLocation = 0;
    if (location == ALIGN_TOP)
        handleLocation = ALIGN_BOTTOM;
    else if (location == ALIGN_BOTTOM)
        handleLocation = ALIGN_TOP;
    else if (location == ALIGN_LEFT)
        handleLocation = ALIGN_RIGHT;
    else if (location == ALIGN_RIGHT)
        handleLocation = ALIGN_LEFT;
    CRUIPopupFrame * frame = new CRUIPopupFrame(&_popupControl, body, showHandle ? handleLocation : 0);
    frame->setBackgroundAlpha(backgroundAlpha);
    CRUIWidget * widget = frame;
    _popupControl.close();
    _popupControl.popup = widget;
    _popupControl.body = body;
    _popupControl.popupBackground = new CRUIWidget();
    _popupControl.align = location;
    _popupControl.margins = margins;
    _popupControl.wantsTouchEventsOutside = wantsTouchEventsOutside;
    _popupControl.layout(_pos);
    _popupControl.startTs = GetCurrentTimeMillis();
    _popupControl.endTs = _popupControl.startTs + POPUP_ANIMATION_DURATION;
    _popupControl.progress = crconfig.einkMode ? 10000 : 0;
    _popupControl.closing = false;
    _popupControl.outerColor = crconfig.einkMode ? 0xFFFFFFFF : COLOR_MENU_POPUP_FADE;
    //CRLog::trace("prepared popup");
    invalidate();
}

class CRUIListMenu : public CRUIListWidget /*, public CRUIListAdapter*/ {
    CRUIWindowWidget * _window;
    CRUIActionList _actionList;
    CRUIHorizontalLayout * _itemLayout;
    CRUIImageWidget * _itemIcon;
    CRUITextWidget * _itemText;
public:
    CRUIListMenu(CRUIWindowWidget * window, const CRUIActionList & actionList) : _window(window), _actionList(actionList) {
        _itemIcon = new CRUIImageWidget();
        _itemText = new CRUITextWidget();
        _itemLayout = new CRUIHorizontalLayout();
        _itemLayout->addChild(_itemIcon);
        _itemLayout->addChild(_itemText);
        _itemText->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _itemIcon->setStyle("MENU_ITEM_ICON");
        _itemText->setStyle("MENU_ITEM_TEXT");
        _itemLayout->setStyle("MENU_ITEM");
        _itemLayout->setMinHeight(MIN_ITEM_PX);
        //setAdapter(this);
    }
    virtual int getItemCount() { //CRUIListWidget * list
        //CR_UNUSED(list);
        return _actionList.length();
    }

    virtual CRUIWidget * getItemWidget(int index) { //CRUIListWidget * list
        //CR_UNUSED(list);
        _itemIcon->setImage(_actionList[index]->icon_res);
        _itemText->setText(_actionList[index]->getName());
        return _itemLayout;
    }
    virtual bool onItemClickEvent(int itemIndex) {
        _window->onMenuItemAction(_actionList[itemIndex]);
        return true;
    }
};

/// override to handle menu or other action
bool CRUIWindowWidget::onAction(const CRUIAction * action) {
    return getMain()->onAction(action);
}

/// close popup menu, and call onAction
bool CRUIWindowWidget::onMenuItemAction(const CRUIAction * _action) {
    CRUIAction action(*_action);
    _popupControl.close();
    if (onAction(&action)) {
        return true;
    }
    return _main->onAction(&action);
}

void CRUIWindowWidget::showMenu(const CRUIActionList & actionList, int location, lvRect & margins, bool asToolbar) {
    CR_UNUSED(asToolbar);
    CRUIListMenu * menu = new CRUIListMenu(this, actionList);
    preparePopup(menu, location, margins);
}

/// motion event handler - before children, returns true if it handled event
bool CRUIWindowWidget::onTouchEventPreProcess(const CRUIMotionEvent * event) {
	// hide virtual keyboard when touching outside focused control
	CRUIWidget * focused = CRUIEventManager::getFocusedWidget();
    if (event->getAction() == ACTION_DOWN && focused && isChild(focused)) {
    	lvPoint pt(event->getX(), event->getY());
    	lvRect rc(focused->getPos());
    	rc.extend(PT_TO_PX(3));
    	if (!rc.isPointInside(pt)) {
    		if (CRUIEventManager::isVirtualKeyboardShown())
    			CRUIEventManager::hideVirtualKeyboard();
    	} else {
    		if (!CRUIEventManager::isVirtualKeyboardShown() && focused->needKeyboard())
    			CRUIEventManager::showVirtualKeyboard(0, focused->getText(), false);
    	}
        return false;
    }
    if (!_popupControl.popup)
        return false;
    return false;
}

/// updates widget position based on specified rectangle
void CRUIWindowWidget::layout(int left, int top, int right, int bottom) {
    CRUIFrameLayout::layout(left, top, right, bottom);
    if (_popupControl.popup) {
        _popupControl.layout(_pos);
        _popupControl.updateLayout(_pos);
        //_popupControl.
    }
}


CRUITitleBarWidget::CRUITitleBarWidget(lString16 title, CRUIOnClickListener * buttonListener, CRUIOnLongClickListener * buttonLongClickListener, bool hasMenuButton) : CRUILinearLayout(false) {
    setStyle("TOOL_BAR");
    setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    if (hasMenuButton)
        _menuButton = new CRUIImageButton("menu_more");
    else
        _menuButton = NULL;
    //_backButton = new CRUIImageButton("ic_menu_back");
    //_backButton = new CRUIImageButton("00_button_left");
    //_backButton = new CRUIImageButton("backspace");
    _backButton = new CRUIImageButton("back");
    _caption = new CRUITextWidget(title);
    _caption->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _caption->setFontSize(FONT_SIZE_MEDIUM);
    _caption->setAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    _caption->setPadding(PT_TO_PX(2));
    addChild(_backButton);
    addChild(_caption);
    if (hasMenuButton)
        addChild(_menuButton);
    setMinHeight(MIN_ITEM_PX * 3 / 4);
    //_caption->setBackground(0xC0C0C040);
    if (hasMenuButton) {
        _menuButton->setId(lString8("MENU"));
        _menuButton->setOnClickListener(buttonListener);
        _menuButton->setOnLongClickListener(buttonLongClickListener);
    }
    _backButton->setId(lString8("BACK"));
    _backButton->setOnClickListener(buttonListener);
    _backButton->setOnLongClickListener(buttonLongClickListener);
}
