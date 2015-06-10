#ifndef ONLINESTORE_H
#define ONLINESTORE_H

#include "cruilist.h"
#include "fileinfo.h"
#include "cruiwindow.h"
#include "crcoverpages.h"

class CRUITitleBarWidget;
class CRUIOpdsItemListWidget;

class CRUIMainWidget;

class CRUIOnlineStoreWidget : public CRUIWindowWidget, public CRUIOnListItemClickListener
        , public CRUIOnClickListener, public CRUIOnLongClickListener, public ExternalImageSourceCallback {
protected:
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
    /// returns true if all coverpages are available, false if background tasks are submitted
    virtual bool requestAllVisibleCoverpages();
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    virtual bool onKeyEvent(const CRUIKeyEvent * event);
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onLongClick(CRUIWidget * widget);
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
    virtual void setDirectory(LVClonePtr<BookDBCatalog> & _catalog, CRDirContentItem * _dir);
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

    virtual void cancelDownloads();

    virtual void showSearchPopup();
    virtual void openSearchResults(lString16 pattern);

    /// call to schedule download of image
    virtual bool onRequestImageDownload(CRDirEntry * book);
    virtual bool fetchCover(CRDirEntry * book);

    CRUIOnlineStoreWidget(CRUIMainWidget * main);
    virtual ~CRUIOnlineStoreWidget();
};

class CRUIOpdsDirItemWidget;
class CRUIOpdsBookItemWidget;
class CRUIOpdsProgressItemWidget;
class CRUIOpdsBrowserWidget;

class CRUIOpdsItemListWidget : public CRUIListWidget {
protected:
    CRDirContentItem * _dir;
    CRUIOpdsDirItemWidget * _folderWidget;
    CRUIOpdsBookItemWidget * _bookWidget;
    CRUIOpdsProgressItemWidget * _progressWidget;
    CRUIOnlineStoreWidget * _parent;
    int _coverDx;
    int _coverDy;
    bool _showProgressAsLastItem;
public:
    virtual void setScrollOffset(int offset);
    virtual void setProgressItemVisible(bool showProgress);
    virtual bool isAnimating();
    virtual void animate(lUInt64 millisPassed);
    virtual void calcCoverSize(int dx, int dy);
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
    CRUIOpdsItemListWidget(CRUIOnlineStoreWidget * parent);
    virtual int getItemCount();
    virtual CRUIWidget * getItemWidget(int index);
    virtual void setDirectory(CRDirContentItem * dir);
    /// return true if drag operation is intercepted
    virtual bool startDragging(const CRUIMotionEvent * event, bool vertical);
    /// returns true if all coverpages are available, false if background tasks are submitted
    virtual bool requestAllVisibleCoverpages();
};


#endif // ONLINESTORE_H
