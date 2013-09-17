
#include "crcoverpages.h"
#include "lvdocview.h"
#include "epubfmt.h"
#include "pdbfmt.h"
#include "lvqueue.h"

//#define USE_GL_COVERPAGE_CACHE 0 // no GL context under this thread anyway

//#if USE_GL_COVERPAGE_CACHE
//#include "gldrawbuf.h"
//#endif

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
    virtual LVDrawBuf * createDrawBuf(int dx, int dy);
public:
    void clear();
    Entry * draw(CRDirEntry * _book, int dx, int dy);
    Entry * find(CRDirEntry * _book, int dx, int dy);
    CRCoverImageCache(int maxitems = 1000, int maxsize = 16*1024*1024);
    virtual ~CRCoverImageCache() { clear(); }
};

extern CRCoverImageCache * coverImageCache;

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

void CRDrawBookCover(LVDrawBuf * drawbuf, lString8 fontFace, CRDirEntry * book, LVImageSourceRef image, int bpp)
{
    //CRLog::debug("drawBookCover called");
    lString16 title = book->getTitle();
    lString16 authors = book->getAuthorNames(false);
    lString16 seriesName = book->getSeriesNameOnly();
    int seriesNumber = book->getSeriesNumber();
    if (title.empty() && authors.empty())
        title = Utf8ToUnicode(book->getFileName());
    if (drawbuf != NULL) {
        int factor = 1;
        int dx = drawbuf->GetWidth();
        int dy = drawbuf->GetHeight();
        int MIN_WIDTH = 300;
        int MIN_HEIGHT = 400;
        if (dx < MIN_WIDTH || dy < MIN_HEIGHT) {
            if (dx * 2 < MIN_WIDTH || dy * 2 < MIN_HEIGHT) {
                dx *= 3;
                dy *= 3;
                factor = 3;
            } else {
                dx *= 2;
                dy *= 2;
                factor = 2;
            }
        }
        LVDrawBuf * drawbuf2 = drawbuf;
        if (factor > 1)
            drawbuf2 = new LVColorDrawBuf(dx, dy, drawbuf->GetBitsPerPixel());

        if (bpp >= 16) {
            // native color resolution
            LVDrawBookCover(*drawbuf2, image, fontFace, title, authors, seriesName, seriesNumber);
            image.Clear();
        } else {
            LVGrayDrawBuf grayBuf(drawbuf2->GetWidth(), drawbuf2->GetHeight(), bpp);
            LVDrawBookCover(grayBuf, image, fontFace, title, authors, seriesName, seriesNumber);
            image.Clear();
            grayBuf.DrawTo(drawbuf2, 0, 0, 0, NULL);
        }
        if (factor > 1) {
            drawbuf->DrawRescaled(drawbuf2, 0, 0, drawbuf->GetWidth(), drawbuf->GetHeight(), 0);
            delete drawbuf2;
        }
    }
    //CRLog::debug("drawBookCover finished");
}

bool LVBookFileExists(lString8 fname) {
    lString16 fn = Utf8ToUnicode(fname);
    lString16 arc;
    lString16 file;
    if (LVSplitArcName(fn, arc, file)) {
        return LVFileExists(arc);
    } else {
        return LVFileExists(fn);
    }
}

LVStreamRef LVGetBookCoverStream(lString8 _path) {
    lString16 path = Utf8ToUnicode(_path);
    lString16 arcname, item;
    LVStreamRef res;
    LVContainerRef arc;
    if (!LVSplitArcName(path, arcname, item)) {
        // not in archive
        LVStreamRef stream = LVOpenFileStream(path.c_str(), LVOM_READ);
        if (!stream.isNull()) {
            arc = LVOpenArchieve(stream);
            if (!arc.isNull()) {
                // ZIP-based format
                if (DetectEpubFormat(stream)) {
                    // EPUB
                    // extract coverpage from epub
                    res = GetEpubCoverpage(arc);
                    return res;
                }
            } else {
                doc_format_t fmt;
                if (DetectPDBFormat(stream, fmt)) {
                    res = GetPDBCoverpage(stream);
                    return res;
                }
            }
        }
    } else {
        LVStreamRef arcstream = LVOpenFileStream(arcname.c_str(), LVOM_READ);
        if (!arcstream.isNull()) {
            arc = LVOpenArchieve(arcstream);
            if (!arc.isNull()) {
                LVStreamRef stream = arc->OpenStream(item.c_str(), LVOM_READ);
                if (!stream.isNull()) {
                    doc_format_t fmt;
                    if (DetectPDBFormat(stream, fmt)) {
                        res = GetPDBCoverpage(stream);
                        return res;
                    }
                }
            }
        }
    }
    return res;
}

LVStreamRef LVScanBookCover(lString8 _path, int & type) {
    type = COVER_EMPTY;
    lString16 path = Utf8ToUnicode(_path);
    CRLog::debug("scanBookCoverInternal(%s) called", LCSTR(path));
    lString16 arcname, item;
    LVStreamRef res;
    LVContainerRef arc;
    if (!LVSplitArcName(path, arcname, item)) {
        // not in archive
        LVStreamRef stream = LVOpenFileStream(path.c_str(), LVOM_READ);
        if (!stream.isNull()) {
            arc = LVOpenArchieve(stream);
            if (!arc.isNull()) {
                // ZIP-based format
                if (DetectEpubFormat(stream)) {
                    // EPUB
                    // extract coverpage from epub
                    res = GetEpubCoverpage(arc);
                    if (!res.isNull())
                        type = COVER_FROMBOOK;
                }
            } else {
                res = GetFB2Coverpage(stream);
                if (res.isNull()) {
                    doc_format_t fmt;
                    if (DetectPDBFormat(stream, fmt)) {
                        res = GetPDBCoverpage(stream);
                        if (!res.isNull())
                            type = COVER_FROMBOOK;
                    }
                } else {
                    type = COVER_CACHED;
                }
            }
        }
    } else {
        CRLog::debug("scanBookCoverInternal() : is archive, item=%s, arc=%d", LCSTR(item), LCSTR(arcname));
        LVStreamRef arcstream = LVOpenFileStream(arcname.c_str(), LVOM_READ);
        if (!arcstream.isNull()) {
            arc = LVOpenArchieve(arcstream);
            if (!arc.isNull()) {
                LVStreamRef stream = arc->OpenStream(item.c_str(), LVOM_READ);
                if (!stream.isNull()) {
                    CRLog::debug("scanBookCoverInternal() : archive stream opened ok, parsing");
                    res = GetFB2Coverpage(stream);
                    if (res.isNull()) {
                        doc_format_t fmt;
                        if (DetectPDBFormat(stream, fmt)) {
                            res = GetPDBCoverpage(stream);
                            if (!res.isNull())
                                type = COVER_FROMBOOK;
                        }
                    } else {
                        type = COVER_CACHED;
                    }
                }
            }
        }
    }
    if (!res.isNull())
        CRLog::debug("scanBookCoverInternal() : returned cover page stream");
    else
        CRLog::debug("scanBookCoverInternal() : cover page data not found");
    return res;
}



CRCoverFileCache * coverCache = NULL;

CRCoverFileCache::CRCoverFileCache(lString16 dir, int maxitems, int maxfiles, int maxsize) : _dir(dir), _maxitems(maxitems), _maxfiles(maxfiles), _maxsize(maxsize), _nextId(0) {
	CRLog::info("Created CRCoverFileCache - maxItems=%d maxFiles=%d maxSize=%d at %s", _maxitems, _maxfiles, _maxsize, LCSTR(dir));
    LVAppendPathDelimiter(_dir);
    _filename = _dir;
    _filename += "covercache.ini";
    LVCreateDirectory(_dir);
}

lString8 CRCoverFileCache::generateNextFilename() {
    char s[32];
    _nextId++;
    sprintf(s, "%08d.img", _nextId);
    return lString8(s);
}

lString8 CRCoverFileCache::saveToCache(LVStreamRef stream) {
    if (stream.isNull())
        return lString8::empty_str;
    lString8 fn = generateNextFilename();
    lString16 fn16 = _dir + Utf8ToUnicode(fn);
    LVStreamRef out = LVOpenFileStream(fn16.c_str(), LVOM_WRITE);
    if (out.isNull())
        return lString8::empty_str;
    lvsize_t sz = stream->GetSize();
    lvsize_t saved = LVPumpStream(out.get(), stream.get());
    if (sz != saved)
        return lString8::empty_str;
    return fn;
}

CRCoverFileCache::Entry * CRCoverFileCache::add(const lString8 & pathname, int type, int sz, lString8 & fn) {
    Entry * p = new Entry(pathname, type);
    p->size = sz;
    p->cachedFile = fn;
    _cache.pushBack(p);
    return p;
}

CRCoverFileCache::Entry * CRCoverFileCache::scan(const lString8 & pathname) {
    int type = COVER_EMPTY;
    LVStreamRef stream = LVScanBookCover(pathname, type);
    return put(pathname, type, stream);
}

CRCoverFileCache::Entry * CRCoverFileCache::put(const lString8 & pathname, int type, LVStreamRef stream) {
    Entry * existing = find(pathname);
    if (existing) {
    	CRLog::error("CRCoverFileCache::put - existing item found for %s", pathname.c_str());
        return existing;
    }
    if (stream.isNull() || stream->GetSize() == 0)
        type = COVER_EMPTY;
    int sz = !stream.isNull() ? (int)stream->GetSize() : 0;
    Entry * p = new Entry(pathname, type);
    if (type == COVER_CACHED) {
        lString8 cached = saveToCache(stream);
        if (!cached.empty()) {
            p->size = sz;
            p->cachedFile = cached;
        } else {
            p->type = COVER_EMPTY;
        }
    }
    CRLog::trace("Adding item %s type=%d size=%d", p->pathname.c_str(), p->type, p->size);
    _cache.pushFront(p);
    checkSize();
    save();
    return p;
}

lString16 CRCoverFileCache::getFilename(Entry * item) {
    return _dir + Utf8ToUnicode(item->cachedFile);
}

void CRCoverFileCache::checkSize() {
    int totalBooks = 0;
    int totalFiles = 0;
    int totalSize = 0;
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.get();
        if (item->type == COVER_CACHED) {
            totalSize += item->size;
            totalFiles++;
        }
        totalBooks++;
        if ((totalBooks > _maxitems || totalFiles > _maxfiles || totalSize > _maxsize) && (totalBooks > 10)) {
            iterator.remove();
            //CRLog::trace("CRCoverFileCache::checkSize() - totalBooks:%d totalFiles:%d totalSize:%d Removing cover file item %s", totalBooks, totalFiles, totalSize, item->pathname.c_str());
            if (item->type == COVER_CACHED)
                LVDeleteFile(getFilename(item));
            delete item;
        }
    }
}

bool CRCoverFileCache::knownCachedFile(const lString8 & fn) {
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.get();
        if (item->cachedFile == fn)
            return true;
    }
    return false;
}

CRCoverFileCache::Entry * CRCoverFileCache::find(const lString8 & pathname) {
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.get();
        if (item->pathname == pathname) {
            iterator.moveToHead();
            return item;
        }
    }
    return NULL;
}

void CRCoverFileCache::clear() {
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.remove();
        delete item;
    }
}

LVStreamRef CRCoverFileCache::getStream(const lString8 & pathname) {
	//CRLog::trace("CRCoverFileCache::getStream %s", pathname.c_str());
    Entry * item = find(pathname);
    if (!item) {
    	//CRLog::trace("CRCoverFileCache::getStream Cache item %s not found, scanning", pathname.c_str());
        item = scan(pathname);
//        if (!item)
//        	CRLog::trace("item %s scanned, no coverpage found", pathname.c_str());
//        else
//        	CRLog::trace("item %s scanned, type=%d size=%d", item->pathname.c_str(), item->type, item->size);
    }
    if (!item)
        return LVStreamRef();
    return getStream(item);
}

LVStreamRef CRCoverFileCache::getStream(Entry * item) {
    //CRLog::trace("CRCoverFileCache::getStream(type = %d)", item->type);
    if (item->type == COVER_EMPTY)
        return LVStreamRef();
    if (item->type == COVER_CACHED)
        return LVOpenFileStream(getFilename(item).c_str(), LVOM_READ);
    //CRLog::trace("Calling LVGetBookCoverStream(%s)", item->pathname.c_str());
    return LVGetBookCoverStream(item->pathname);
}

bool CRCoverFileCache::open() {
    LVStreamRef in = LVOpenFileStream(_filename.c_str(), LVOM_READ);
    if (in.isNull())
        return false;
    int sz = (int)in->GetSize();
    lString8 buf;
    buf.append(sz, ' ');
    lvsize_t bytesRead;
    in->Read(buf.modify(), buf.length(), &bytesRead);
    if ((int)bytesRead != buf.length())
        return false;
    lString8Collection lines;
    lines.split(buf, lString8("\n"));
    if (lines.length() < 1)
        return false;
    _nextId = lines[0].atoi();
    for (int i = 1; i < lines.length(); i++) {
        lString8 line = lines[i];
        int p = line.rpos("=");
        if (p > 0) {
            lString8 pathname = line.substr(0, p);
            lString8 params = line.substr(p + 1);
            if (params.length() > 0 && params.length() < 100) {
                int t = 0;
                int sz = 0;
                char s[100];
                if (sscanf(params.c_str(), "%d,%d,%s", &t, &sz, s) != 3)
                    return false;
                lString16 coverfile = _dir + Utf8ToUnicode(s);
                if (t == COVER_CACHED && !LVFileExists(coverfile)) {
                    // cover file deleted
                    continue;
                }
                if (t != COVER_CACHED && t != COVER_EMPTY && t != COVER_FROMBOOK)
                    continue;
                if (!LVBookFileExists(pathname)) {
                    // book file deleted
                    if (t == COVER_CACHED)
                        LVDeleteFile(coverfile);
                    continue;
                }
                lString8 s8(s);
                add(pathname, t, sz, s8);
            }
        }
    }
    /// delete unknown files from cache directory
    LVContainerRef dir = LVOpenDirectory(_dir.c_str(), L"*.img");
    if (!dir.isNull()) {
        for (int i = 0; i<dir->GetObjectCount(); i++) {
            const LVContainerItemInfo * item = dir->GetObjectInfo(i);
            if (!item->IsContainer()) {
                lString8 fn = UnicodeToUtf8(item->GetName());
                if (!fn.endsWith(".img"))
                    continue;
                if (!knownCachedFile(fn))
                    LVDeleteFile(_dir + item->GetName());
            }
        }
    }
    return true;
}

bool CRCoverFileCache::save() {
    LVStreamRef out = LVOpenFileStream(_filename.c_str(), LVOM_WRITE);
    if (out.isNull())
        return false;
    lString8 buf;
    buf << lString8::itoa(_nextId) << "\n";
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.get();
        buf << item->pathname << "=" << lString8::itoa(item->type) << "," << lString8::itoa(item->size) << "," << item->cachedFile << "\n";
    }
    lvsize_t bytesWritten = 0;
    out->Write(buf.c_str(), buf.length(), &bytesWritten);
    return (int)bytesWritten == buf.length();
}












CRCoverImageCache::Entry * CRCoverImageCache::draw(CRDirEntry * _book, int dx, int dy) {
	CRLog::trace("CRCoverImageCache::draw called for %s", _book->getPathName().c_str());
    LVStreamRef stream = coverCache->getStream(_book->getPathName());
    LVImageSourceRef image;
    if (!stream.isNull()) {
        image = LVCreateStreamImageSource(stream);
    }
    // TODO: fix font face
    LVDrawBuf * drawbuf = createDrawBuf(dx, dy);
    drawbuf->beforeDrawing();
    CRDrawBookCover(drawbuf, lString8("Arial"), _book, image, 32);
    drawbuf->afterDrawing();
    return put(_book, dx, dy, drawbuf);
}

CRCoverImageCache::Entry * CRCoverImageCache::find(CRDirEntry * _book, int dx, int dy) {
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.get();
        if (item->book->getPathName() == _book->getPathName() && item->dx == dx && item->dy == dy) {
            iterator.moveToHead();
            return item;
        }
    }
    return NULL;
}

CRCoverImageCache::Entry * CRCoverImageCache::put(CRDirEntry * _book, int _dx, int _dy, LVDrawBuf * _image) {
    Entry * existing = find(_book, _dx, _dy);
    if (existing) {
        return existing;
    }
    Entry * p = new Entry(_book->clone(), _dx, _dy, _image);
    _cache.pushFront(p);
    checkSize();
    return p;
}

void CRCoverImageCache::checkSize() {
    int totalItems = 0;
    int totalSize = 0;
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.get();
        totalSize += item->dx * item->dy * 4;
        totalItems++;
        if ((totalItems > _maxitems || totalSize > _maxsize) && totalItems > 10) {
            CRLog::trace("CRCoverImageCache::checkSize() removing item %s %dx%d", item->book->getPathName().c_str(), item->dx, item->dy);
            iterator.remove();
            delete item;
        }
    }
}

LVDrawBuf * CRCoverImageCache::createDrawBuf(int dx, int dy) {
#if USE_GL_COVERPAGE_CACHE == 1
    GLDrawBuf * res = new GLDrawBuf(dx, dy, 32, true);
    return res;
#else
    LVColorDrawBuf * res = new LVColorDrawBuf(dx, dy, 32);
    res->Clear(0xFF000000);
    return res;
#endif
}

void CRCoverImageCache::clear() {
    CRLog::info("CRCoverImageCache::clear()");
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.remove();
        delete item;
    }
}

CRCoverImageCache::CRCoverImageCache(int maxitems, int maxsize) : _maxitems(maxitems), _maxsize(maxsize){
    CRLog::info("CRCoverImageCache:: maxitems=%d maxsize=%d", maxitems, maxsize);
}

CRCoverImageCache * coverImageCache = NULL;

void CRCoverPageManager::allTasksFinished() {
    // already under lock in background thread
    if (_allTasksFinishedCallback) {
        concurrencyProvider->executeGui(_allTasksFinishedCallback); // callback will be deleted in GUI thread
        _allTasksFinishedCallback = NULL;
    }
}

/// set book image to draw covers on - instead of plain cover images
void CRCoverPageManager::setCoverPageTemplate(LVImageSourceRef image, const lvRect & clientRect) {
    _bookImage = image;
    _bookImageClientRect = clientRect;
    clearImageCache();
}

int CRCoverPageManager::BookImageCache::find(int dx, int dy) {
    for (int i = 0; i < items.length(); i++) {
        if (items[i]->dx == dx && items[i]->dy == dy)
            return i;
    }
    return -1;
}

#define MAX_BOOK_IMAGE_CACHE_SIZE 3
CRCoverPageManager::BookImage * CRCoverPageManager::BookImageCache::get(LVImageSourceRef img, const lvRect & rc, int dx, int dy) {
    int index = find(dx, dy);
    if (index >= 0) {
        if (index > 0)
            items.move(0, index);
        return items[0];
    }
    if (items.length() >= MAX_BOOK_IMAGE_CACHE_SIZE)
        delete items.remove(items.length() - 1);
    items.insert(0, new CRCoverPageManager::BookImage(img, rc, dx, dy));
    return items[0];
}

CRCoverPageManager::BookImage::BookImage(LVImageSourceRef img, const lvRect & rc, int _dx, int _dy) : dx(_dx), dy(_dy) {
    clientRc.left = rc.left * dx / img->GetWidth();
    clientRc.top = rc.top * dy / img->GetHeight();
    clientRc.right = rc.right * dx / img->GetWidth();
    clientRc.bottom = rc.bottom * dy / img->GetHeight();
    buf = new LVColorDrawBuf(dx, dy, 32);
    buf->Clear(0xFF000000); // transparent
    buf->Draw(img, 0, 0, dx, dy, false);
}

void CRCoverPageManager::setAllTasksFinishedCallback(CRRunnable * allTasksFinishedCallback) {
    // called from GUI thread
    CRGuard guard(_monitor);
    if (_stopped || (!_taskIsRunning && _queue.length() == 0)) {
        // call immediately
        allTasksFinishedCallback->run();
        delete allTasksFinishedCallback;
    }
    /// set callback
    if (_allTasksFinishedCallback)
        delete _allTasksFinishedCallback;
    _allTasksFinishedCallback = allTasksFinishedCallback;
}


void CRCoverPageManager::stop() {
    if (_stopped)
        return;
    {
        CRGuard guard(_monitor);
        CRLog::trace("CRCoverPageManager::stop() - under lock");
        _stopped = true;
        CRLog::trace("CRCoverPageManager::stop() deleting all tasks from queue");
        while (_queue.length() > 0) {
            CoverTask * p = _queue.popFront();
            delete p;
        }
        CRLog::trace("CRCoverPageManager::stop() deleting all tasks from queue");
        _monitor->notifyAll();
    }
    _thread->join();
}

void CRCoverPageManager::run() {
    CRLog::info("CRCoverPageManager::run thread started");
    // testing tizen concurrency

    for (;;) {
        if (_stopped)
            break;
        CoverTask * task = NULL;
        {
            //CRLog::trace("CRCoverPageManager::run :: wait for lock");
            CRGuard guard(_monitor);
            if (_stopped)
                break;
            //CRLog::trace("CRCoverPageManager::run :: lock acquired");
            if (_queue.length() <= 0) {
                allTasksFinished();
                //CRLog::trace("CRCoverPageManager::run :: calling monitor wait");
                _monitor->wait();
                //CRLog::trace("CRCoverPageManager::run :: done monitor wait");
            }
            if (_stopped)
                break;
            task = _queue.popFront();
            if (task)
                _taskIsRunning = true;
            else
                allTasksFinished();
        }
        /// execute w/o lock
        if (task) {

            //CRLog::trace("CRCoverPageManager: searching in cache");
            CRCoverImageCache::Entry * entry = coverImageCache->find(task->book, task->dx, task->dy);
            if (!entry) {
                //CRLog::trace("CRCoverPageManager: rendering new coverpage image ");
                entry = coverImageCache->draw(task->book, task->dx, task->dy);
            }
            //CRLog::trace("CRCoverPageManager: calling ready callback");
            if (task->callback)
                concurrencyProvider->executeGui(task->callback); // callback will be deleted in GUI thread
            delete task;
            _taskIsRunning = false;
        }
        // process next event
    }
    CRLog::info("CRCoverPageManager thread finished");
}

LVDrawBuf * CRCoverPageManager::getIfReady(CRDirEntry * _book, int dx, int dy)
{
    //CRLog::trace("CRCoverPageManager::getIfReady :: wait for lock");
    CRGuard guard(_monitor);
    //CRLog::trace("CRCoverPageManager::getIfReady :: lock acquired");
    CRCoverImageCache::Entry * existing = coverImageCache->find(_book, dx, dy);
    if (existing) {
        //CRLog::trace("CRCoverPageManager::getIfReady - found existing image for %s %dx%d", _book->getPathName().c_str(), dx, dy);
        return existing->image;
    }
    //CRLog::trace("CRCoverPageManager::getIfReady - not found %s %dx%d", _book->getPathName().c_str(), dx, dy);
    return NULL;
}

void CRCoverPageManager::prepare(CRDirEntry * _book, int dx, int dy, CRRunnable * readyCallback)
{
    //CRLog::trace("CRCoverPageManager::prepare :: wait for lock");
    CRGuard guard(_monitor);
    //CRLog::trace("CRCoverPageManager::prepare %s %dx%d", _book->getPathName().c_str(), dx, dy);
    if (_stopped) {
        CRLog::error("Ignoring new task since cover page manager is stopped");
        return;
    }
    CRCoverImageCache::Entry * existing = coverImageCache->find(_book, dx, dy);
    if (existing) {
        // we are in GUI thread now
        if (readyCallback) {
            readyCallback->run();
            delete readyCallback;
        }
        return;
    }
    _queue.iterator();
    for (LVQueue<CoverTask*>::Iterator iterator = _queue.iterator(); iterator.next(); ) {
        CoverTask * item = iterator.get();
        if (item->isSame(_book, dx, dy)) {
            //CRLog::trace("CRCoverPageManager::prepare %s %dx%d -- there is already such task in queue", _book->getPathName().c_str(), dx, dy);
            // already has the same task in queue
            iterator.moveToHead();
            return;
        }
    }
    _book = _book->clone();
    CoverTask * task = new CoverTask(_book, dx, dy, readyCallback);
    _queue.pushBack(task);
    _monitor->notify();
    //CRLog::trace("CRCoverPageManager::prepare - added new task %s %dx%d", _book->getPathName().c_str(), dx, dy);
}

/// cancels all pending coverpage tasks
void CRCoverPageManager::cancelAll() {
    CRGuard guard(_monitor);
    CRLog::trace("CRCoverPageManager::cancelAll");
    for (LVQueue<CoverTask*>::Iterator iterator = _queue.iterator(); iterator.next(); ) {
        CoverTask * item = iterator.get();
        iterator.remove();
        delete item->callback;
        delete item;
    }
}

/// removes all cached images from memory
void CRCoverPageManager::clearImageCache() {
    CRGuard guard(_monitor);
    CRLog::trace("CRCoverPageManager::clearImageCache()");
    coverImageCache->clear();
}

void CRCoverPageManager::cancel(CRDirEntry * _book, int dx, int dy)
{
    CRGuard guard(_monitor);
    CRLog::trace("CRCoverPageManager::cancel %s %dx%d", _book->getPathName().c_str(), dx, dy);
    for (LVQueue<CoverTask*>::Iterator iterator = _queue.iterator(); iterator.next(); ) {
        CoverTask * item = iterator.get();
        if (item->isSame(_book, dx, dy)) {
            iterator.remove();
            delete item->callback;
            delete item;
            break;
        }
    }
}

CRCoverPageManager::CRCoverPageManager() : _stopped(false), _allTasksFinishedCallback(NULL), _taskIsRunning(false) {
    _monitor = concurrencyProvider->createMonitor();
    _thread = concurrencyProvider->createThread(this);
    _thread->start();
}

CRCoverPageManager::~CRCoverPageManager() {
    stop();
}

CRCoverPageManager * coverPageManager = NULL;

void CRSetupCoverpageManager(lString16 coverCacheDir, int maxitems, int maxfiles, int maxsize, int maxRenderCacheItems, int maxRenderCacheBytes) {
    CRStopCoverpageManager();
    coverCache = new CRCoverFileCache(coverCacheDir, maxitems, maxfiles, maxsize);
    coverCache->open();
    coverImageCache = new CRCoverImageCache(maxRenderCacheItems, maxRenderCacheBytes);
    coverPageManager = new CRCoverPageManager();
}

void CRStopCoverpageManager() {
    if (coverPageManager) {
    	CRLog::trace("Calling coverPageManager->stop()");
        coverPageManager->stop();
        delete coverPageManager;
        coverPageManager = NULL;
    }
    if (coverImageCache) {
    	CRLog::trace("Deleting coverImageCache");
        delete coverImageCache;
        coverImageCache = NULL;
    }
    if (coverCache) {
    	CRLog::trace("Deleting coverCache");
        delete coverCache;
        coverCache = NULL;
    }

}
