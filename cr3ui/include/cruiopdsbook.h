#ifndef CRUIOPDSBOOK_H
#define CRUIOPDSBOOK_H


#include "cruilist.h"
#include "fileinfo.h"
#include "cruiwindow.h"
#include "cruicoverwidget.h"
#include "crcoverpages.h"

class CRUITitleBarWidget;
class CRUIMainWidget;

class CRUIBookDownloadWidget;
class CRUIRichTextWidget;
class CRUIOpdsBookWidget : public CRUIWindowWidget, public CRUIOnClickListener
        , public CRUIOnLongClickListener, public ExternalImageSourceCallback {
    CRUITitleBarWidget * _title;
    CROpdsCatalogsItem * _book;
    CRCoverWidget * _cover;
    CRUITextWidget * _caption;
    CRUITextWidget * _authors;
    CRUIRichTextWidget * _description;
    CRUITableLayout  * _buttonsTable;
    int _coverTaskId;
    CRDirEntry* _coverTaskBook;
    LVPtrVector<CRUIBookDownloadWidget, false> _downloads;
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
    virtual void afterNavigationFrom();
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }

    void measure(int baseWidth, int baseHeight);
    void updateCoverSize(int baseHeight);

    /// download result
    virtual void onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream);
    /// download progress
    virtual void onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded);

    /// call to schedule download of image
    virtual bool onRequestImageDownload(CRDirEntry * book);

    void cancelDownloads();

    CRUIOpdsBookWidget(CRUIMainWidget * main, CROpdsCatalogsItem * book);
    virtual ~CRUIOpdsBookWidget();
};

#endif // CRUIOPDSBOOK_H
