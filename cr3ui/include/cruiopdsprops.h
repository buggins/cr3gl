#ifndef CRUIOPDSPROPS_H
#define CRUIOPDSPROPS_H


#include "cruilist.h"
#include "cruiscrollwidget.h"
#include "fileinfo.h"
#include "cruiwindow.h"

class CRUITitleBarWidget;
//class CRUIOpdsItemListWidget;

class CRUIMainWidget;

class CRUIOpdsPropsWidget : public CRUIWindowWidget, public CRUIOnClickListener, public CRUIOnLongClickListener {
    CRUITitleBarWidget * _title;
    CRUIScrollWidget * _scroll;
    //CRUIOpdsItemListWidget * _fileList;
    CRUIEditWidget * _edTitle;
    CRUIEditWidget * _edUrl;
    CRUIEditWidget * _edLogin;
    CRUIEditWidget * _edPassword;
    LVClonePtr<BookDBCatalog> _catalog;
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
    CRUIOpdsPropsWidget(CRUIMainWidget * main, LVClonePtr<BookDBCatalog> & catalog);
    virtual ~CRUIOpdsPropsWidget();
};

#endif // CRUIOPDSPROPS_H
