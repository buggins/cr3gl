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

class CRUIDocView;

class CRUIReadWidget : public CRUIWindowWidget, public CRDocumentLoadCallback, public CRDocumentRenderCallback, public LVDocViewCallback
{
    CRUIDocView * _docview;
    bool _isDragging;
    lvPoint _dragStart;
    int _dragStartOffset;
    ScrollControl _scroll;

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

    void animateScrollTo(int newpos, int speed);

    bool _locked;

    CRFileItem * _fileItem; // owned
    BookDBBookmark * _lastPosition; // owned


public:

    /// restore last position from DB
    bool restorePosition();
    /// save current book position to DB
    void updatePosition();

    const lString8 & getPathName() { return _fileItem ? _fileItem->getPathName() : lString8::empty_str; }

    virtual void onDocumentLoadFinished(lString8 pathname, bool success);
    virtual void onDocumentRenderFinished(lString8 pathname);

    CRUIDocView * getDocView() { return _docview; }

    CRUIReadWidget(CRUIMainWidget * main);
	virtual ~CRUIReadWidget();

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

    /// motion event handler, returns true if it handled event
    bool onTouchEvent(const CRUIMotionEvent * event);
    bool onKeyEvent(const CRUIKeyEvent * event);
    bool doCommand(int cmd, int param = 0);
    int pointToTapZone(int x, int y);
    /// override to handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// override to handle menu or other action - by id
    virtual bool onAction(int actionId) { return CRUIWindowWidget::onAction(actionId); }

    // apply changed settings
    virtual void applySettings(CRPropRef changed);


    bool onTapZone(int zone, bool additionalAction);

    void prepareScroll(int direction);

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
