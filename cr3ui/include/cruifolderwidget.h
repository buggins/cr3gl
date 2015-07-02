/*
 * cruifolderwidget.h
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

#ifndef CRUIFOLDERWIDGET_H_
#define CRUIFOLDERWIDGET_H_

#include "cruilist.h"
#include "fileinfo.h"
#include "cruiwindow.h"

class CRUITitleBarWidget;
class CRUIFileListWidget;

class CRUIMainWidget;

class CRUIFolderWidget : public CRUIWindowWidget, public CRUIOnListItemClickListener, public CRUIOnListItemLongClickListener, public CRUIOnClickListener, public CRUIOnLongClickListener {
	CRUITitleBarWidget * _title;
	CRUIFileListWidget * _fileList;
    CRDirContentItem * _dir;
public:

    virtual bool createFolder(lString8 name);

    /// returns true if all coverpages are available, false if background tasks are submitted
    virtual bool requestAllVisibleCoverpages();
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    virtual bool onKeyEvent(const CRUIKeyEvent * event);
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onLongClick(CRUIWidget * widget);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
    virtual bool onListItemLongClick(CRUIListWidget * widget, int itemIndex);
    virtual void setDirectory(CRDirContentItem * _dir);
    /// override to handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }

    virtual void refreshContent();

    CRUIFolderWidget(CRUIMainWidget * main);
	virtual ~CRUIFolderWidget();
};


#endif /* CRUIFOLDERWIDGET_H_ */
