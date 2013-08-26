#ifndef CRCOVERPAGES_H
#define CRCOVERPAGES_H

#include "lvstring.h"
#include "fileinfo.h"
#include "lvdrawbuf.h"
#include "lvstream.h"
#include "lvqueue.h"

/// draw book cover to drawbuf
void CRDrawBookCover(LVDrawBuf * drawbuf, lString8 fontFace, CRDirEntry * book, LVImageSourceRef image, int bpp = 32);
LVStreamRef LVScanBookCover(lString8 _path);
bool LVBookFileExists(lString8 fname);

class CRCoverFileCache {
public:
    enum {
        COVER_CACHED,   // cached in directory
        COVER_FROMBOOK, // read from book directly (it's fast enough for format)
        COVER_EMPTY     // no cover image in book
    };
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
    int _maxfiles;
    int _maxsize;
    int _nextId;
    bool open();
    bool save();
    lString8 generateNextFilename();
    lString8 saveToCache(LVStreamRef stream);
    Entry * add(const lString8 & pathname, int type, int sz, lString8 & fn);
public:
    Entry * find(const lString8 & pathname);
    Entry * put(const lString8 & pathname, int type, LVStreamRef stream);
    CRCoverFileCache(lString16 dir, int maxfiles, int maxsize);
    ~CRCoverFileCache() { save(); }
};

extern CRCoverFileCache * coverCache;

#endif // CRCOVERPAGES_H
