/*
 * cruireadwidget.h
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

#ifndef CRUIREADWIDGET_H_
#define CRUIREADWIDGET_H_


#include "cruiwidget.h"
#include "cruilayout.h"
#include "fileinfo.h"
#include "cruiwindow.h"

class CRUIMainWidget;

class CRDocumentLoadCallback {
public:
    virtual void onDocumentLoadFinished(lString8 pathname, bool success) = 0;
    virtual ~CRDocumentLoadCallback() {}
};

class CRDocumentRenderCallback {
public:
    virtual void onDocumentRenderFinished(lString8 pathname) = 0;
    virtual ~CRDocumentRenderCallback() {}
};

class CRUIDocView : public LVDocView {
    CRUIImageRef background;
    //CRUIImageRef backgroundScrollLeft;
    //CRUIImageRef backgroundScrollRight;
public:
    CRUIDocView() : LVDocView() {
        //background = resourceResolver->getIcon("leather.jpg", true);
        background = CRUIImageRef(new CRUISolidFillImage(0xFFFFFF));
    }
    /// clears page background
    virtual void drawPageBackground( LVDrawBuf & drawbuf, int offsetX, int offsetY ) {
//    	CRUIImageRef background = resourceResolver->getIcon("paper1.jpg", true);
//        CRUIImageRef backgroundScrollLeft = resourceResolver->getIcon("scroll-edge-left", true);
//        CRUIImageRef backgroundScrollRight = resourceResolver->getIcon("scroll-edge-right", true);
        lvRect rc(0, 0, drawbuf.GetWidth(), drawbuf.GetHeight());
        background->draw(&drawbuf, rc, offsetX, offsetY);
//        drawbuf.FillRect(rc, 0xE0E0C040);
//        if (!backgroundScrollLeft.isNull() && !backgroundScrollRight.isNull()) {
//			lvRect leftrc = rc;
//			leftrc.right = leftrc.left + backgroundScrollLeft->originalWidth();
//			backgroundScrollLeft->draw(&drawbuf, leftrc, 0, offsetY);
//			lvRect rightrc = rc;
//			rightrc.left = rightrc.right - backgroundScrollRight->originalWidth();
//			backgroundScrollRight->draw(&drawbuf, rightrc, 0, offsetY);
//        }
    }
    virtual void setBackground(CRUIImageRef img) {
        background = img;
    }
    /// applies properties, returns list of not recognized properties
    virtual CRPropRef propsApply( CRPropRef props );
};

class CRUIReadWidget;
class LVTocItem;
class CRUITOCWidget : public CRUIWindowWidget, public CRUIListAdapter
        , public CRUIOnListItemClickListener
        , public CRUIOnClickListener
        , public CRUIOnLongClickListener
{
    CRUIReadWidget * _readWidget;
    CRUITitleBarWidget * _title;
    CRUIListWidget * _list;
    LVPtrVector<LVTocItem, false> _toc;
    CRUIHorizontalLayout * _itemWidget;
    CRUITextWidget * _chapter;
    CRUITextWidget * _page;
public:
    // list adapter methods
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    // list item click
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
    // on click
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onLongClick(CRUIWidget * widget);
    /// override to handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }

    CRUITOCWidget(CRUIMainWidget * main, CRUIReadWidget * read);
    virtual ~CRUITOCWidget() { delete _itemWidget; }
};

class CRUIReadWidget : public CRUIWindowWidget
        , public CRDocumentLoadCallback
        , public CRDocumentRenderCallback
        , public LVDocViewCallback
{
    CRUIDocView * _docview;
    CRUIDocView * _pinchSettingPreview;
    bool _isDragging;
    lvPoint _dragStart;
    int _dragStartOffset;
    ScrollControl _scroll;
    LVDocViewMode _viewMode;

    class ScrollModePage {
    public:
        int pos;
        int dx;
        int dy;
        LVDrawBuf * drawbuf;
        ScrollModePage() : pos(0), dx(0), dy(0), drawbuf(NULL) { }
        ~ScrollModePage() {
            if (drawbuf)
                delete drawbuf;
        }
        bool intersects(int y0, int y1) {
            return !(pos + dy <= y0 || pos >= y1);
        }
    };
    class ScrollModePageCache {
        LVPtrVector<ScrollModePage> pages;
        LVDrawBuf * createBuf();
        int minpos;
        int maxpos;
        int dx;
        int dy;
        int tdx;
        int tdy;
    public:
        void setSize(int _dx, int _dy);
        ScrollModePageCache();
        /// ensure images are prepared
        void prepare(LVDocView * docview, int pos, int dx, int dy, int direction, bool force);
        /// draw
        void draw(LVDrawBuf * dst, int pos, int x, int y);
        /// remove images from cache
        void clear();
    };
    ScrollModePageCache _scrollCache;

    class PagedModePage {
    public:
    	int pageNumber; // number of left page, 0 based
    	int numPages; // 1 or 2
    	int dx;
    	int dy;
        int tdx;
        int tdy;
        LVDrawBuf * drawbuf;
        PagedModePage() : pageNumber(0), numPages(1), dx(0), dy(0), tdx(0), tdy(0), drawbuf(NULL) { }
        ~PagedModePage() {
            if (drawbuf)
                delete drawbuf;
        }
    };
    class PagedModePageCache {
        LVPtrVector<PagedModePage> pages;
        LVDrawBuf * createBuf();
        int numPages;
        int dx;
        int dy;
        int tdx;
        int tdy;
    public:
        void preparePage(LVDocView * docview, int page);
        void clearExcept(int page1, int page2);
        PagedModePage * findPage(int page);
        void setSize(int _dx, int _dy, int _numPages);
        PagedModePageCache();
        /// ensure images are prepared
        void prepare(LVDocView * docview, int page, int dx, int dy, int direction, bool force);
        /// draw
        void draw(LVDrawBuf * dst, int pageNumber, int direction, int progress);
        /// remove images from cache
        void clear();
    };
    PagedModePageCache _pagedCache;

    void animateScrollTo(int newpos, int speed);

    bool _locked;

    CRFileItem * _fileItem; // owned
    BookDBBookmark * _lastPosition; // owned

    bool _startPositionIsUpdated;

    enum {
    	PINCH_OP_NONE,
    	PINCH_OP_HORIZONTAL,
    	PINCH_OP_VERTICAL,
    	PINCH_OP_DIAGONAL
    };


    int _pinchOp;
    int _pinchOpStartDx;
    int _pinchOpStartDy;
    int _pinchOpCurrentDx;
    int _pinchOpCurrentDy;
    int _pinchOpSettingValue;

    void startPinchOp(int op, int dx, int dy);
    void updatePinchOp(int dx, int dy);
    void endPinchOp(int dx, int dy, bool cancel);
public:
    CRUIReadWidget(CRUIMainWidget * main);
    virtual ~CRUIReadWidget();


    /// restore last position from DB
    bool restorePosition();
    /// save current book position to DB
    void updatePosition();

    const lString8 & getPathName() { return _fileItem ? _fileItem->getPathName() : lString8::empty_str; }

    virtual void onDocumentLoadFinished(lString8 pathname, bool success);
    virtual void onDocumentRenderFinished(lString8 pathname);

    CRUIDocView * getDocView() { return _docview; }

    bool openBook(const CRFileItem * file);
    void closeBook();

    /// returns true if document is ready, false if background rendering is in progress
    bool renderIfNecessary();

	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);

    virtual void animate(lUInt64 millisPassed);
    virtual bool isAnimating();

    /// overriden to treat popup as first child
    virtual int getChildCount();
    /// overriden to treat popup as first child
    virtual CRUIWidget * getChild(int index);

    virtual void clearImageCaches();

	/// returns true to allow parent intercept this widget which is currently handled by this widget
	virtual bool allowInterceptTouchEvent(const CRUIMotionEvent * event);
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    virtual bool onKeyEvent(const CRUIKeyEvent * event);
    bool doCommand(int cmd, int param = 0);
    int pointToTapZone(int x, int y);
    /// override to handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }

    // apply changed settings
    virtual void applySettings(CRPropRef changed, CRPropRef oldSettings, CRPropRef newSettings);


    bool onTapZone(int zone, bool additionalAction);

    void prepareScroll(int direction);

    void showReaderMenu();
    void showTOC();
    bool hasTOC();
    lString16 getCurrentPositionDesc();
    int getCurrentPositionPercent();
    void goToPercent(int percent);

    void goToPosition(lString16 path);

    virtual void beforeNavigationFrom();


    // DocView callback
    /// on starting file loading
    virtual void OnLoadFileStart( lString16 filename );
    /// format detection finished
    virtual void OnLoadFileFormatDetected(doc_format_t fileFormat);
    /// file loading is finished successfully - drawCoveTo() may be called there
    virtual void OnLoadFileEnd();
    /// first page is loaded from file an can be formatted for preview
    virtual void OnLoadFileFirstPagesReady();
    /// file progress indicator, called with values 0..100
    virtual void OnLoadFileProgress( int percent);
    /// document formatting started
    virtual void OnFormatStart();
    /// document formatting finished
    virtual void OnFormatEnd();
    /// format progress, called with values 0..100
    virtual void OnFormatProgress(int percent);
    /// format progress, called with values 0..100
    virtual void OnExportProgress(int percent);
    /// file load finiished with error
    virtual void OnLoadFileError(lString16 message);
    /// Override to handle external links
    virtual void OnExternalLink(lString16 url, ldomNode * node);
    /// Called when page images should be invalidated (clearImageCache() called in LVDocView)
    virtual void OnImageCacheClear();
    /// return true if reload will be processed by external code, false to let internal code process it
    virtual bool OnRequestReload();
};


#endif /* CRUIREADWIDGET_H_ */
