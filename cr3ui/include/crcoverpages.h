#ifndef CRCOVERPAGES_H
#define CRCOVERPAGES_H

#include "lvstring.h"
#include "fileinfo.h"
#include "lvdrawbuf.h"
#include "lvstream.h"
#include "lvqueue.h"
#include "crconcurrent.h"


class CoverTask;

class ExternalImageSourceCallback {
public:
    /// call to schedule download of image
    virtual bool onRequestImageDownload(CRDirEntry * book) = 0;
    ~ExternalImageSourceCallback() {}
};

/// cover page extracting, cache and rendering manager
class CRCoverPageManager : public CRRunnable {

    struct BookImage {
        int dx;
        int dy;
        lvRect clientRc;
        LVColorDrawBuf * buf;
        lUInt32 color;
        lUInt32 neutralColor;
        BookImage(LVImageSourceRef img, const lvRect & rc, const lvRect & neutralRc, int _dx, int _dy);
        ~BookImage() { delete buf; }
    };

    class BookImageCache {
        LVPtrVector<BookImage> items;
        int find(int dx, int dy);
    public:
        BookImage * get(LVImageSourceRef img, const lvRect & rc, const lvRect & neutralRc, int dx, int dy);
        BookImageCache() { }
        void clear() { items.clear(); }
        ~BookImageCache() { clear(); }
    };

    volatile bool _stopped;
    CRMonitorRef _monitor;
    CRThreadRef _thread;
    LVQueue<CoverTask *> _queue;
    LVQueue<CoverTask *> _externalSourceQueue; // queue of cover images to be downloaded outside
    CRRunnable * _allTasksFinishedCallback;
    volatile bool _taskIsRunning;
    LVImageSourceRef _bookImage; // book image template
    lvRect _bookImageClientRect; // where cover image should be placed inside book image
    lvRect _bookImageNeutralRect; // rect where color is neutral - and should not be corrected
    BookImageCache _bookImageCache;


    void allTasksFinished();

public:
    CRCoverPageManager();
    virtual ~CRCoverPageManager();

    /// stop thread
    void stop();
    /// thread function, don't call
    virtual void run();

    /// returns cover page, if it's already prepared and cached; call from GUI thread only!
    LVDrawBuf * getIfReady(CRDirEntry * _book, int dx, int dy);

    /// prepares coverpage in background thread; calls readyCallback in GUI thread when done
    /// if downloadCallback is passed, and image source is url, saves task in list of external source tasks prepares coverpage in background thread; calls readyCallback in GUI thread when done
    void prepare(CRDirEntry * _book, int dx, int dy, CRRunnable * readyCallback, ExternalImageSourceCallback * downloadCallback = NULL);
    /// once external image source downloaded image, call this method to set image file and continue coverpage preparation
    void setExternalImage(CRDirEntry * _book, LVStreamRef & stream);

    /// cancels pending coverpage task (if not yet started)
    void cancel(CRDirEntry * _book, int dx, int dy);
    /// cancels all pending coverpage tasks
    void cancelAll();

    /// removes all cached images from memory
    void clearImageCache();

    /// set callback to run when all tasks are finished
    void setAllTasksFinishedCallback(CRRunnable * allTasksFinishedCallback);

    /// set book image to draw covers on - instead of plain cover images
    void setCoverPageTemplate(LVImageSourceRef image, const lvRect & clientRect, const lvRect & bookImageNeutralRect);

    /// draws book template and tells its client rect - returns false if book template is not set
    bool drawBookTemplate(LVDrawBuf * buf, lvRect & clientRect, lUInt32 & avgColor, lUInt32 & neutralColor);

};

void CRSetupCoverpageManager(lString16 coverCacheDir, int maxitems = 1000, int maxfiles = 200, int maxsize = 16*1024*1024, int maxRenderCacheItems = 1000, int maxRenderCacheBytes = 16 * 1024 * 1024);
void CRStopCoverpageManager();

extern CRCoverPageManager * coverPageManager;

#endif // CRCOVERPAGES_H
