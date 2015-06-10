/*
 * opdsbrowser.h
 *
 *  Created on: Oct 25, 2013
 *      Author: lve
 */

#ifndef OPDSBROWSER_H_
#define OPDSBROWSER_H_

#include "onlinestore.h"

class CRUITitleBarWidget;
class CRUIOpdsItemListWidget;

class CRUIMainWidget;

class CRUIOpdsBrowserWidget : public CRUIOnlineStoreWidget {
	CRUITitleBarWidget * _title;
    CRUIOpdsItemListWidget * _fileList;
    LVClonePtr<BookDBCatalog> _catalog;
    CROpdsCatalogsItem * _dir;
    int _requestId;
    lString8 _nextPartURL;
    int _coverTaskId;
    CRDirEntry* _coverTaskBook;
    LVPtrVector<CRDirEntry> _coversToLoad;
    lString8 _searchUrl;
public:
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);

    /// download result
    virtual void onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream);
    /// download progress
    virtual void onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded);

    virtual void afterNavigationTo();
    virtual void fetchNextPart();

    virtual void showSearchPopup();
    virtual void openSearchResults(lString16 pattern);

    CRUIOpdsBrowserWidget(CRUIMainWidget * main);
	virtual ~CRUIOpdsBrowserWidget();
};



#endif /* OPDSBROWSER_H_ */
