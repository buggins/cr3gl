/*
 * cruihomewidget.h
 *
 *  Created on: Aug 20, 2013
 *      Author: vlopatin
 */

#ifndef CRUIHOMEWIDGET_H_
#define CRUIHOMEWIDGET_H_

#include "cruilayout.h"
#include "cruilist.h"
#include "fileinfo.h"
#include "cruiwindow.h"

class CRUINowReadingWidget;
class CRUIRecentBooksListWidget;
class CRUIFileSystemDirsWidget;
class CRUILibraryWidget;
class CRUIOnlineCatalogsWidget;
class CRUIMainWidget;

class CRUIHomeWidget : public CRUIWindowWidget, public CRUIOnClickListener {
	CRUINowReadingWidget * _currentBook;
	CRUIRecentBooksListWidget * _recentBooksList;
	CRUIFileSystemDirsWidget * _fileSystem;
	CRUILibraryWidget * _library;
	CRUIOnlineCatalogsWidget * _onlineCatalogsList;
public:

    void updateFolderBookmarks();

    CRUIHomeWidget(CRUIMainWidget * main);
    virtual ~CRUIHomeWidget() {}
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    virtual bool onKeyEvent(const CRUIKeyEvent * event);

    virtual bool onClick(CRUIWidget * widget);
    /// handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }
};


#endif /* CRUIHOMEWIDGET_H_ */
