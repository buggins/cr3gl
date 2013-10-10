#ifndef CRUIWINDOW_H
#define CRUIWINDOW_H

#include "cruilayout.h"
#include "cruiaction.h"
#include "cruicontrols.h"

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
    CRUIMainWidget * _main;

    PopupControl _popupControl;

    void preparePopup(CRUIWidget * widget, int location, const lvRect & margins);

    /// draws popup above content
    virtual void drawPopup(LVDrawBuf * buf);

public:
    CRUIWindowWidget(CRUIMainWidget * main) : CRUILinearLayout(true), _main(main) {}
    virtual ~CRUIWindowWidget() {  }

    /// returns main widget
    CRUIMainWidget * getMain() { return _main; }

    /// overriden to treat popup as first child
    virtual int getChildCount();
    /// overriden to treat popup as first child
    virtual CRUIWidget * getChild(int index);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);

    /// motion event handler - before children, returns true if it handled event
    virtual bool onTouchEventPreProcess(const CRUIMotionEvent * event);

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
    CRUITitleBarWidget(lString16 title, CRUIOnClickListener * buttonListener, bool hasMenuButton);
    void setTitle(lString16 title) {
        _caption->setText(title);
    }
};


#endif // CRUIWINDOW_H
