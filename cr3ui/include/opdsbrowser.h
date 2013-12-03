/*
 * opdsbrowser.h
 *
 *  Created on: Oct 25, 2013
 *      Author: lve
 */

#ifndef OPDSBROWSER_H_
#define OPDSBROWSER_H_

#include "cruilist.h"
#include "fileinfo.h"
#include "cruiwindow.h"
#include "crcoverpages.h"

class CRUITitleBarWidget;
class CRUIOpdsItemListWidget;

class CRUIMainWidget;

class CRUIOpdsBrowserWidget : public CRUIWindowWidget, public CRUIOnListItemClickListener
        , public CRUIOnClickListener, public CRUIOnLongClickListener, public ExternalImageSourceCallback {
	CRUITitleBarWidget * _title;
    CRUIOpdsItemListWidget * _fileList;
    BookDBCatalog * _catalog;
    CROpdsCatalogsItem * _dir;
    int _requestId;
    lString8 _nextPartURL;
    int _coverTaskId;
    CRDirEntry* _coverTaskBook;
    LVPtrVector<CRDirEntry> _coversToLoad;
    lString8 _searchUrl;
public:
    /// returns true if all coverpages are available, false if background tasks are submitted
    virtual bool requestAllVisibleCoverpages();
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    virtual bool onKeyEvent(const CRUIKeyEvent * event);
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onLongClick(CRUIWidget * widget);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
    virtual void setDirectory(BookDBCatalog * _catalog, CRDirContentItem * _dir);
    /// override to handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }

    /// download result
    virtual void onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream);
    /// download progress
    virtual void onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded);

    virtual void afterNavigationTo();
    virtual void afterNavigationFrom();
    virtual void fetchNextPart();

    void cancelDownloads();

    void showSearchPopup();
    void openSearchResults(lString16 pattern);

    /// call to schedule download of image
    virtual bool onRequestImageDownload(CRDirEntry * book);
    virtual bool fetchCover(CRDirEntry * book);

    CRUIOpdsBrowserWidget(CRUIMainWidget * main);
	virtual ~CRUIOpdsBrowserWidget();
};



#endif /* OPDSBROWSER_H_ */
