
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

LVStreamRef LVScanBookCover(lString8 _path) {
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
                }
            } else {
                res = GetFB2Coverpage(stream);
                if (res.isNull()) {
                    doc_format_t fmt;
                    if (DetectPDBFormat(stream, fmt)) {
                        res = GetPDBCoverpage(stream);
                    }
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
                        }
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

CRCoverFileCache::CRCoverFileCache(lString16 dir, int maxfiles, int maxsize) : _dir(dir), _maxfiles(maxfiles), _maxsize(maxsize), _nextId(0) {
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
    save();
    return p;
}

CRCoverFileCache::Entry * CRCoverFileCache::find(const lString8 & pathname) {
    for (LVQueue<Entry*>::Iterator iterator = _cache.iterator(); iterator.next(); ) {
        Entry * item = iterator.get();
        if (item) {
            if (item->pathname == pathname) {
                iterator.moveToHead();
                return item;
            }
        }
    }
    return NULL;
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
