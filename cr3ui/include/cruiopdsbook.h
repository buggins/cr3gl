#ifndef CRUIOPDSBOOK_H
#define CRUIOPDSBOOK_H


#include "cruilist.h"
#include "fileinfo.h"
#include "cruiwindow.h"
#include "cruicoverwidget.h"

class CRUITitleBarWidget;
class CRUIMainWidget;

class CRUIRichTextWidget;
class CRUIOpdsBookWidget : public CRUIWindowWidget, public CRUIOnClickListener, public CRUIOnLongClickListener {
    CRUITitleBarWidget * _title;
    CROpdsCatalogsItem * _book;
    CRCoverWidget * _cover;
    CRUITextWidget * _caption;
    CRUITextWidget * _authors;
    CRUIRichTextWidget * _description;
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
    CRUIOpdsBookWidget(CRUIMainWidget * main, CROpdsCatalogsItem * book);
    virtual ~CRUIOpdsBookWidget();
};

#endif // CRUIOPDSBOOK_H
