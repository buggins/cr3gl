#ifndef CRCOVERPAGES_H
#define CRCOVERPAGES_H

#include "lvstring.h"
#include "fileinfo.h"
#include "lvdrawbuf.h"
#include "lvstream.h"
#include "lvqueue.h"
#include "crconcurrent.h"


class CoverTask;

/// cover page extracting, cache and rendering manager
class CRCoverPageManager : public CRRunnable {

    struct BookImage {
        int dx;
        int dy;
        lvRect clientRc;
        LVColorDrawBuf * buf;
        BookImage(LVImageSourceRef img, const lvRect & rc, int _dx, int _dy);
        ~BookImage() { delete buf; }
    };

    class BookImageCache {
        LVPtrVector<BookImage> items;
        int find(int dx, int dy);
    public:
        BookImage * get(LVImageSourceRef img, const lvRect & rc, int dx, int dy);
        BookImageCache() { }
        void clear() { items.clear(); }
        ~BookImageCache() { clear(); }
    };

    volatile bool _stopped;
    CRMonitorRef _monitor;
    CRThreadRef _thread;
    LVQueue<CoverTask *> _queue;
    CRRunnable * _allTasksFinishedCallback;
    volatile bool _taskIsRunning;
    LVImageSourceRef _bookImage; // book image template
    lvRect _bookImageClientRect; // where cover image should be placed inside book image
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
    void prepare(CRDirEntry * _book, int dx, int dy, CRRunnable * readyCallback);

    /// cancels pending coverpage task (if not yet started)
    void cancel(CRDirEntry * _book, int dx, int dy);
    /// cancels all pending coverpage tasks
    void cancelAll();

    /// removes all cached images from memory
    void clearImageCache();

    /// set callback to run when all tasks are finished
    void setAllTasksFinishedCallback(CRRunnable * allTasksFinishedCallback);

    /// set book image to draw covers on - instead of plain cover images
    void setCoverPageTemplate(LVImageSourceRef image, const lvRect & clientRect);

    /// draws book template and tells its client rect - returns false if book template is not set
    bool drawBookTemplate(LVDrawBuf * buf, lvRect & clientRect);

};

void CRSetupCoverpageManager(lString16 coverCacheDir, int maxitems = 1000, int maxfiles = 200, int maxsize = 16*1024*1024, int maxRenderCacheItems = 1000, int maxRenderCacheBytes = 16 * 1024 * 1024);
void CRStopCoverpageManager();

extern CRCoverPageManager * coverPageManager;

#endif // CRCOVERPAGES_H
