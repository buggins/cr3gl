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

class CRUINowReadingWidget;
class CRUIRecentBooksListWidget;
class CRUIFileSystemDirsWidget;
class CRUILibraryWidget;
class CRUIOnlineCatalogsWidget;
class CRUIMainWidget;

class CRUIHomeWidget : public CRUIContainerWidget {
	CRUINowReadingWidget * _currentBook;
	CRUIRecentBooksListWidget * _recentBooksList;
	CRUIFileSystemDirsWidget * _fileSystem;
	CRUILibraryWidget * _library;
	CRUIOnlineCatalogsWidget * _onlineCatalogsList;
    CRUIMainWidget * _main;
public:
    CRUIMainWidget * getMain() { return _main; }
    CRUIHomeWidget(CRUIMainWidget * main);
    virtual ~CRUIHomeWidget() {}
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    void setLastBook(CRDirEntry * lastBook);
    const CRDirEntry * getLastBook();
};

#endif /* CRUIHOMEWIDGET_H_ */
