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
    virtual bool onKeyEvent(const CRUIKeyEvent * event);

};

class CRCoverWidget : public CRUIWidget {
protected:
    CRUIMainWidget * _main;
    CRDirEntry * _book;
    int _dx;
    int _dy;
public:
    /// will own passed book w/o cloning
    CRCoverWidget(CRUIMainWidget * main, CRDirEntry * book, int dx, int dy);
    ~CRCoverWidget() { if (_book) delete _book; }
    /// calculates cover image size (to request in cache) by control size
    lvPoint calcCoverSize(int width, int height);
    virtual void setSize(int width, int height);
    /// sets book to clone of specified item
    virtual void setBook(const CRDirEntry * book);
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
};

#endif /* CRUIHOMEWIDGET_H_ */
