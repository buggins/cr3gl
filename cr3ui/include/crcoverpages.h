#ifndef CRCOVERPAGES_H
#define CRCOVERPAGES_H

#include "lvstring.h"
#include "fileinfo.h"
#include "lvdrawbuf.h"
#include "lvstream.h"
#include "lvqueue.h"

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
    bool open();
    bool save();
    lString16 getFilename(Entry * item);
    LVStreamRef getStream(Entry * item);
    Entry * scan(const lString8 & pathname);
    Entry * find(const lString8 & pathname);
    CRCoverFileCache(lString16 dir, int maxitems, int maxfiles, int maxsize);
    ~CRCoverFileCache() { save(); }
};

extern CRCoverFileCache * coverCache;

#endif // CRCOVERPAGES_H
