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

class CRUITextToSpeechCallback {
public:
    virtual void onSentenceFinished() = 0;
    virtual ~CRUITextToSpeechCallback() {}
};


class CRUIDocView : public LVDocView {
    CRUIImageRef background;
    //CRUIImageRef backgroundScrollLeft;
    //CRUIImageRef backgroundScrollRight;
    lUInt32 _coverColor;
    bool _showCover;
    bool _pageAnimationSupportsCoverFrame;
public:
    CRUIDocView();
    /// clears page background
    virtual void drawPageBackground( LVDrawBuf & drawbuf, int offsetX, int offsetY, int alpha = 0);
    virtual void setBackground(CRUIImageRef img);
    /// applies properties, returns list of not recognized properties
    virtual CRPropRef propsApply( CRPropRef props );
    lString16 getLink(int x, int y, int r);
    lString16 getLink(int x, int y);
    lvRect calcCoverFrameWidths(lvRect rc);
    void drawCoverFrame(LVDrawBuf & drawbuf, lvRect outerRect, lvRect innerRect);
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

class CRUIBookmarksWidget : public CRUIWindowWidget, public CRUIListAdapter
        , public CRUIOnListItemClickListener
        , public CRUIOnListItemLongClickListener
        , public CRUIOnClickListener
        , public CRUIOnLongClickListener
{
    CRUIReadWidget * _readWidget;
    CRUITitleBarWidget * _title;
    BookDBBookmark * _selectedItem;
    CRUIListWidget * _list;
    LVPtrVector<BookDBBookmark> _bookmarks;
    CRUIHorizontalLayout * _itemWidget;
    CRUITextWidget * _chapter;
    CRUITextWidget * _page;
public:
    // list adapter methods
    virtual int getItemCount(CRUIListWidget * list);
    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index);
    // list item click
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex);
    virtual bool onListItemLongClick(CRUIListWidget * widget, int itemIndex);
    // on click
    virtual bool onClick(CRUIWidget * widget);
    virtual bool onLongClick(CRUIWidget * widget);
    /// override to handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }

    CRUIBookmarksWidget(CRUIMainWidget * main, CRUIReadWidget * read);
    virtual ~CRUIBookmarksWidget() { delete _itemWidget; }
};

enum ReaderToolbarPosition {
    READER_TOOLBAR_OFF,
    READER_TOOLBAR_TOP,
    READER_TOOLBAR_LEFT,
    READER_TOOLBAR_SHORT_SIDE,
    READER_TOOLBAR_LONG_SIDE
};

enum PageFlipAnimation {
    PAGE_ANIMATION_NONE,
    PAGE_ANIMATION_SLIDE,
    PAGE_ANIMATION_SLIDE2,
    PAGE_ANIMATION_FADE,
    PAGE_ANIMATION_3D
};

class CRUIReadMenu;
class CRUITextToSpeechCallback;
class CRUIReadWidget : public CRUIWindowWidget
        , public CRDocumentLoadCallback
        , public CRDocumentRenderCallback
        , public LVDocViewCallback
        , public CRUITextToSpeechCallback
        , public CRUIOnScrollPosCallback
{
    CRUIDocView * _docview;
    CRUIDocView * _pinchSettingPreview;
    bool _isDragging;
    lvPoint _dragStart;
    lvPoint _dragPos;
    int _dragStartOffset;
    ScrollControl _scroll;
    LVDocViewMode _viewMode;
    PageFlipAnimation _pageAnimation;
    lvRect _clientRect;
    lvRect _bookRect;

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
        int getLastDirection() { return 1; }
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
        bool back;
        LVDrawBuf * drawbuf;
        PagedModePage() : pageNumber(0), numPages(1), dx(0), dy(0), tdx(0), tdy(0), back(false), drawbuf(NULL) { }
        ~PagedModePage() {
        	//CRLog::trace("Deleting page %08x", (lUInt32) drawbuf);
            if (drawbuf)
                delete drawbuf;
        }
    };
    class PagedModePageCache {
        LVPtrVector<PagedModePage> pages;
        LVDrawBuf * createBuf();
        int numPages;
        int pageCount;
        int dx;
        int dy;
        int tdx;
        int tdy;
        int direction;
        int newPage;
        int pageAnimation;
    public:
        int dir() { return direction; }
        int getNewPage() { return newPage; }
        void setNewPage(int page) { newPage = page; }
        void preparePage(LVDocView * docview, int page, bool back = false);
        void clearExcept(int page1, int page2);
        PagedModePage * findPage(int page);
        PagedModePage * findPageBack(int page);
        void setSize(int _dx, int _dy, int _numPages, int _pageCount);
        PagedModePageCache();
        /// ensure images are prepared
        void prepare(LVDocView * docview, int page, int dx, int dy, int direction, bool force, int pageAnimation);
        int getLastDirection() { return direction; }
        void calcDragPositionProgress(int startx, int currx, int direction, int & progress, int & xx);
        /// draw
        void draw(LVDrawBuf * dst, int pageNumber, int direction, int progress, int x, int y, int startx = -1, int currx = -1);
        void drawFolded(LVDrawBuf * buf, PagedModePage * page1, PagedModePage * page1back, PagedModePage * page2, int xx, int diam, int x, int y);
        void drawFolded(LVDrawBuf * buf, PagedModePage * page, int srcx1, int srcx2, int dstx1, int dstx2, float angle1, float angle2, int y);
        /// remove images from cache
        void clear();
    };
    PagedModePageCache _pagedCache;

    struct SelectionControl  {
        bool timerStarted; // lont tap timer started
        bool selecting;
        bool popupActive;
        lString16 startPos;
        lString16 endPos;
        lvRect startCursorPos;
        lvRect endCursorPos;
        lString16 selectionText;
        SelectionControl() : timerStarted(false), selecting(false), popupActive(false) {

        }
    };
    SelectionControl _selection;


    void cancelSelection();
    void startSelectionTimer(int x, int y);
    void updateSelection(int x, int y);
    void selectionDone(int x, int y);
    void updateSelectionBookmark();
    void addSelectionBookmark();
    void updateBookmarks();
    virtual void onPopupClosing(CRUIWidget * popup);
    /// handle timer event; return true to allow recurring timer event occur more times, false to stop
    virtual bool onTimerEvent(lUInt32 timerId);


    void animateScrollTo(int newpos, int speed);
    void animatePageFlip(int newpage, int speed);

    bool _locked;

    CRFileItem * _fileItem; // owned
    BookDBBookmark * _lastPosition; // owned
    LVPtrVector<BookDBBookmark> _bookmarks;
    LVAutoPtr<BookDBBookmark> _selectionBookmark;

    bool _startPositionIsUpdated;

    bool _ttsInProgress;

    lString16 _lastSearchPattern;

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

    bool updateReadingPosition();

    CRUIReadMenu * _toolbar;
    int _toolbarPosition;
    CRUIScrollBar * _scrollbar;

    bool _volumeKeysEnabled;

    void setToolbarPosition(int position);
    bool isToolbarVertical(int baseWidth, int baseHeight);

    void setScrollBarVisible(bool v);
    void updateScrollbar();

public:
    CRUIReadWidget(CRUIMainWidget * main);
    virtual ~CRUIReadWidget();

    void removeBookmark(lInt64 id);
    LVPtrVector<BookDBBookmark> & getBookmarks() { return _bookmarks; }

    virtual void onSentenceFinished();
    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual);

    /// restore last position from DB
    bool restorePosition();
    /// save current book position to DB
    void updatePosition();

    const lString8 & getPathName() { return _fileItem ? _fileItem->getPathName() : lString8::empty_str; }

    virtual void onDocumentLoadFinished(lString8 pathname, bool success);
    virtual void onDocumentRenderFinished(lString8 pathname);

    CRUIDocView * getDocView() { return _docview; }

    bool findText(lString16 pattern, int origin, bool reverse, bool caseInsensitive);

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

    /// schedule saving of current position
    void postUpdatePosition(int delay = 1000);
    /// cancel update position
    void cancelPositionUpdateTimer();

    /// prepare next image for fast page flip
    void prepareNextPage();

    void onScrollAnimationStop();

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

    void showFindTextPopup();
    void showGoToPercentPopup();
    void showReaderMenu();
    CRUIReadMenu * createReaderMenu(bool forToolbar);

    void showTOC();
    void showBookmarks();

    void updateVolumeControls();
    void startReadAloud();
    void stopReadAloud();

    bool hasTOC();
    lString16 getCurrentPositionDesc();
    int getCurrentPositionPercent();
    void goToPercent(int percent);
    void goToPage(int page);

    void goToPosition(lString16 path);

    /// move by page w/o animation
    void moveByPage(int direction);

    virtual void beforeNavigationFrom();
    virtual void afterNavigationFrom();
    virtual void afterNavigationTo();


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
