#ifndef CRCOVERPAGES_H
#define CRCOVERPAGES_H

#include "lvstring.h"
#include "fileinfo.h"
#include "lvdrawbuf.h"
#include "lvstream.h"
#include "lvqueue.h"
#include "crconcurrent.h"

enum CoverCachingType {
    COVER_CACHED,   // cached in directory
    COVER_FROMBOOK, // read from book directly (it's fast enough for format)
    COVER_EMPTY     // no cover image in book
};

/// draw book cover to drawbuf
void CRDrawBookCover(LVDrawBuf * drawbuf, lString8 fontFace, CRDirEntry * book, LVImageSourceRef image, int bpp = 32);
bool LVBookFileExists(lString8 fname);

class CRCoverFileCache {
public:
    class Entry {
    public:
        lString8 pathname;
        int type;
        lString8 cachedFile;
        int size;
        Entry(lString8 fn, int _type) : pathname(fn), type(_type), size(0) {
        }
    };
private:
    LVQueue<Entry*> _cache;
    lString16 _dir;
    lString16 _filename;
    int _maxitems;
    int _maxfiles;
    int _maxsize;
    int _nextId;
    lString8 generateNextFilename();
    lString8 saveToCache(LVStreamRef stream);
    void checkSize();
    Entry * add(const lString8 & pathname, int type, int sz, lString8 & fn);
    Entry * put(const lString8 & pathname, int type, LVStreamRef stream);
    bool knownCachedFile(const lString8 & fn);
public:
    void clear();
    bool open();
    bool save();
    lString16 getFilename(Entry * item);
    LVStreamRef getStream(Entry * item);
    LVStreamRef getStream(const lString8 & pathname);
    Entry * scan(const lString8 & pathname);
    Entry * find(const lString8 & pathname);
    CRCoverFileCache(lString16 dir, int maxitems = 1000, int maxfiles = 200, int maxsize = 16*1024*1024);
    ~CRCoverFileCache() { save(); clear(); }
};

extern CRCoverFileCache * coverCache;

class CRCoverImageCache {
public:
    class Entry {
    public:
        CRDirEntry * book;
        int dx;
        int dy;
        LVDrawBuf * image;
        Entry(CRDirEntry * _book, int _dx, int _dy, LVDrawBuf * _image) : book(_book->clone()), dx(_dx), dy(_dy), image(_image) {}
        ~Entry() { if (image) delete image; }
    };
private:
    LVQueue<Entry*> _cache;
    int _maxitems;
    int _maxsize;
    void checkSize();
    Entry * put(CRDirEntry * _book, int _dx, int _dy, LVDrawBuf * _image);
    /// override to use non-standard draw buffers (e.g. OpenGL)
    virtual LVDrawBuf * createDrawBuf(int dx, int dy) { LVColorDrawBuf * res = new LVColorDrawBuf(dx, dy, 32); res->Clear(0xFF000000); return res; }
public:
    void clear();
    Entry * draw(CRDirEntry * _book, int dx, int dy);
    Entry * find(CRDirEntry * _book, int dx, int dy);
    CRCoverImageCache(int maxitems = 1000, int maxsize = 16*1024*1024);
    ~CRCoverImageCache() { clear(); }
};

extern CRCoverImageCache * coverImageCache;


class CRCoverPageManager : public CRRunnable {
    class CoverTask {
    public:
        CRDirEntry * book;
        int dx;
        int dy;
        CRRunnable * callback;
        CoverTask(CRDirEntry * _book, int _dx, int _dy, CRRunnable * _callback) : book(_book->clone()), dx(_dx), dy(_dy), callback(_callback) {}
        virtual ~CoverTask() {
            delete book;
        }
        bool isSame(CRDirEntry * _book, int _dx, int _dy) {
            return dx == _dx && dy == _dy && _book->getPathName() == book->getPathName();
        }
    };

    bool _stopped;
    CRMonitorRef _monitor;
    CRThreadRef _thread;
    LVQueue<CoverTask *> _queue;
public:
    CRCoverPageManager();
    virtual ~CRCoverPageManager();

    void stop();
    virtual void run();

    /// returns cover page, if it's already prepared and cached; call from GUI thread only!
    LVDrawBuf * getIfReady(CRDirEntry * _book, int dx, int dy);
    /// cancels pending coverpage task (if not yet started)
    void cancel(CRDirEntry * _book, int dx, int dy);
    /// prepares coverpage in background thread; calls readyCallback in GUI thread when done
    void prepare(CRDirEntry * _book, int dx, int dy, CRRunnable * readyCallback);
};

extern CRCoverPageManager * coverPageManager;

#endif // CRCOVERPAGES_H
