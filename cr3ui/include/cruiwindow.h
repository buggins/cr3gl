#ifndef CRUIWINDOW_H
#define CRUIWINDOW_H

#include "cruilayout.h"
#include "cruiaction.h"
#include "cruicontrols.h"
#include "cruilist.h"

class PopupControl {
public:
    lInt64 startTs;
    lInt64 endTs;
    CRUIWidget * popup; // popup widget
    CRUIWidget * popupBackground; // popup background widget
    lUInt32 outerColor; // to apply on surface outside popup
    int width;
    int height;
    int align;       // where is destination rectangle located
    int progress;
    bool closing;
    lvRect parentRect;
    lvRect srcRect;
    lvRect dstRect;
    lvRect margins;
    void close() {
    	CRLog::trace("PopupControl::close()");
        if (popup)
            delete popup;
        popup = NULL;
        if (popupBackground)
            delete popupBackground;
        popupBackground = NULL;
    }
    /// returns rect for current progress
    void getRect(lvRect & rc);
    /// calculates src and dst rectangles for updated parent position/size
    void layout(const lvRect & pos);
    /// update current position based on src and dst rectangles and progress
    void updateLayout(const lvRect & pos);
    /// calculates outer background color for current progress
    lUInt32 getColor();

    /// start animation of popup closing
    void animateClose();

    PopupControl() : popup(NULL), popupBackground(NULL), closing(false) {

    }

    ~PopupControl() {
        close();
    }
};

/// base class for full screen widgets, supporting popups
class CRUIWindowWidget : public CRUIFrameLayout, public CRUIDragListener {
protected:
    CRUIMainWidget * _main;
    CRUILinearLayout * _body;

    PopupControl _popupControl;

    void preparePopup(CRUIWidget * widget, int location, const lvRect & margins);

public:
    /// return true if drag operation is intercepted
    virtual bool onStartDragging(const CRUIMotionEvent * event, bool vertical);

    CRUIWindowWidget(CRUIMainWidget * main) : _main(main) {
    	_body = new CRUIVerticalLayout();
        _body->setId("WINDOW_BODY");
    	addChild(_body);
    }
    virtual ~CRUIWindowWidget() {  }

    /// returns main widget
    CRUIMainWidget * getMain() { return _main; }

    /// overriden to treat popup as first child
    virtual int getChildCount();
    /// overriden to treat popup as first child
    virtual CRUIWidget * getChild(int index);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    /// updates widget position based on specified rectangle
    void layout(int left, int top, int right, int bottom);

    /// motion event handler - before children, returns true if it handled event
    virtual bool onTouchEventPreProcess(const CRUIMotionEvent * event);
    virtual bool onKeyEvent(const CRUIKeyEvent * event);

    /// opens menu popup with specified list of actions
    virtual void showMenu(const CRUIActionList & actionList, int location, lvRect & margins, bool asToolbar);

    /// close popup menu, and call onAction
    virtual bool onMenuItemAction(const CRUIAction * action);

    /// override to handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual bool onAction(int actionId) {
        return onAction(CRUIActionByCode(actionId));
    }

    // apply changed settings
    virtual void applySettings(CRPropRef changed, CRPropRef oldSettings, CRPropRef newSettings) { CR_UNUSED3(changed, oldSettings, newSettings); }

    virtual void beforeNavigationFrom() {}
    virtual void afterNavigationFrom() {}
    virtual void beforeNavigationTo() {}
    virtual void afterNavigationTo() {}
};

class CRUITitleBarWidget : public CRUILinearLayout {
    CRUIButton * _backButton;
    CRUIButton * _menuButton;
    CRUITextWidget * _caption;
public:
    CRUITitleBarWidget(lString16 title, CRUIOnClickListener * buttonListener, CRUIOnLongClickListener * buttonLongClickListener, bool hasMenuButton);
    void setTitle(lString16 title) {
        _caption->setText(title);
    }
    void setTitle(const char * title) {
        _caption->setText(title);
    }
};


#endif // CRUIWINDOW_H
