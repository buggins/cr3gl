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

class CRUITitleBarWidget;
class CRUIFileListWidget;

class CRUIMainWidget;

class CRUIFolderWidget : public CRUILinearLayout, public CRUIOnListItemClickListener, public CRUIOnClickListener {
	CRUITitleBarWidget * _title;
	CRUIFileListWidget * _fileList;
	CRDirCacheItem * _dir;
    CRUIMainWidget * _main;
public:
    /// returns true if all coverpages are available, false if background tasks are submitted
    virtual bool requestAllVisibleCoverpages();
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
    CRUIMainWidget * getMain() { return _main; }
    virtual void setDirectory(CRDirCacheItem * _dir);
    CRUIFolderWidget(CRUIMainWidget * main);
	virtual ~CRUIFolderWidget();
};


#endif /* CRUIFOLDERWIDGET_H_ */
