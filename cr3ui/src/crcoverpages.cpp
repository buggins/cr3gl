
#include "crcoverpages.h"
#include "lvdocview.h"
#include "epubfmt.h"
#include "pdbfmt.h"

void CRDrawBookCover(LVDrawBuf * drawbuf, lString8 fontFace, CRDirEntry * book, LVImageSourceRef image, int bpp)
{
    CRLog::debug("drawBookCover called");
    lString16 title = book->getTitle();
    lString16 authors = book->getAuthorNames(false);
    lString16 seriesName = book->getSeriesName(false);
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
            LVDrawBookCover(*drawbuf2, image, fontFace, title, authors, seriesName, 0);
            image.Clear();
        } else {
            LVGrayDrawBuf grayBuf(drawbuf2->GetWidth(), drawbuf2->GetHeight(), bpp);
            LVDrawBookCover(grayBuf, image, fontFace, title, authors, seriesName, 0);
            image.Clear();
            grayBuf.DrawTo(drawbuf2, 0, 0, 0, NULL);
        }
        if (factor > 1) {
            drawbuf->DrawRescaled(drawbuf2, 0, 0, drawbuf->GetWidth(), drawbuf->GetHeight(), 0);
            delete drawbuf2;
        }
    }
    CRLog::debug("drawBookCover finished");
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
        CRLog::debug("scanBookCoverInternal() : returned cover page array");
    else
        CRLog::debug("scanBookCoverInternal() : cover page data not found");
    return res;
}



CRCoverFileCache * coverCache = NULL;

CRCoverFileCache::CRCoverFileCache(lString16 dir, int maxitems, int maxfiles, int maxsize) : _dir(dir), _maxitems(maxitems), _maxfiles(maxfiles), _maxsize(maxsize), _nextId(0) {
    LVAppendPathDelimiter(_dir);
    _filename = _dir;
    _filename += "covercache.ini";
    LVCreateDirectory(_dir);
}

lString8 CRCoverFileCache::generateNextFilename() {
    return lString8::itoa(_nextId++) + ".img";
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
        return existing;
    }
    Entry * p = new Entry(pathname, type);
    if (type == COVER_CACHED) {
        lString8 cached = saveToCache(stream);
        if (!cached.empty()) {
            p->size = stream->GetSize();
            p->cachedFile = cached;
        } else {
            p->type = COVER_EMPTY;
        }
    }
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
        if ((totalBooks > _maxitems || totalFiles > _maxfiles || totalSize > _maxsize) && (totalBooks > 2)) {
            iterator.remove();
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
    Entry * item = find(pathname);
    if (!item) {
        item = scan(pathname);
    }
    if (!item)
        return LVStreamRef();
    return getStream(item);
}

LVStreamRef CRCoverFileCache::getStream(Entry * item) {
    if (item->type == COVER_EMPTY)
        return LVStreamRef();
    if (item->type == COVER_CACHED)
        return LVOpenFileStream(getFilename(item).c_str(), LVOM_READ);
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
    if (bytesRead != buf.length())
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
                if (sscanf(params.c_str(), "%d,%d,%s", t, sz, s) != 3)
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
                add(pathname, t, sz, lString8(s));
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
    return bytesWritten == buf.length();
}












CRCoverImageCache::Entry * CRCoverImageCache::draw(CRDirEntry * _book, int dx, int dy) {
    LVDrawBuf * drawbuf = createDrawBuf(dx, dy);
    LVStreamRef stream = coverCache->getStream(_book->getPathName());
    LVImageSourceRef image;
    if (!stream.isNull()) {
        image = LVCreateStreamImageSource(stream);
    }
    // TODO: fix font face
    CRDrawBookCover(drawbuf, lString8("Arial"), _book, image, 32);
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
        if ((totalItems > _maxitems || totalSize > _maxsize) && totalItems > 2) {
            iterator.remove();
            delete item;
        }
    }
}

void CRCoverImageCache::clear() {
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.remove();
        delete item;
    }
}

CRCoverImageCache * coverImageCache = NULL;
