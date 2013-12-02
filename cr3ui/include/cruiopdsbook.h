#ifndef CRUIOPDSBOOK_H
#define CRUIOPDSBOOK_H


#include "cruilist.h"
#include "fileinfo.h"
#include "cruiwindow.h"

class CRUITitleBarWidget;
class CRUIMainWidget;

class CRUIOpdsBookWidget : public CRUIWindowWidget, public CRUIOnClickListener, public CRUIOnLongClickListener {
    CRUITitleBarWidget * _title;
    CRUIEditWidget * _edTitle;
    CRUIEditWidget * _edUrl;
    CRUIEditWidget * _edLogin;
    CRUIEditWidget * _edPassword;
    BookDBCatalog * _catalog;
    void save();
public:
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    virtual bool onKeyEvent(const CRUIKeyEvent * event);
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onLongClick(CRUIWidget * widget);
    /// override to handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual void beforeNavigationFrom();
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }
    CRUIOpdsBookWidget(CRUIMainWidget * main, BookDBCatalog * catalog);
    virtual ~CRUIOpdsBookWidget();
};

#endif // CRUIOPDSBOOK_H
