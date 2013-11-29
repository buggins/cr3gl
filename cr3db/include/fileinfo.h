/*
 * fileinfo.h
 *
 *  Created on: Aug 19, 2013
 *      Author: vlopatin
 */

#ifndef FILEINFO_H_
#define FILEINFO_H_

#include <cr3db.h>
#include <crconcurrent.h>
#include <bookformats.h>
#include <lvstream.h>

namespace CRUI {
	enum FolderSortOrder {
		BY_TITLE,
		BY_TITLE_DESC,
		BY_AUTHOR,
		BY_AUTHOR_DESC,
		BY_SERIES,
		BY_SERIES_DESC,
		BY_DATE,
		BY_DATE_DESC,
	};
};



int LVDocFormatFromExtension(lString16 &pathName);
lString16 LVDocFormatName(int fmt);

enum DIR_TYPE {
    DIR_TYPE_INTERNAL_STORAGE,
    DIR_TYPE_SD_CARD,
    DIR_TYPE_FS_ROOT,
    DIR_TYPE_DEFAULT_BOOKS_DIR,
    DIR_TYPE_CURRENT_BOOK_DIR,
    DIR_TYPE_DOWNLOADS,
    DIR_TYPE_FAVORITE,
    DIR_TYPE_RECENT,
    DIR_TYPE_OPDS_CATALOG,
    DIR_TYPE_BOOKS_BY_AUTHOR,
    DIR_TYPE_BOOKS_BY_TITLE,
    DIR_TYPE_BOOKS_BY_SERIES,
    DIR_TYPE_BOOKS_BY_FILENAME,
    DIR_TYPE_BOOKS_SEARCH_RESULT,
    DIR_TYPE_NORMAL
};

// same as bmk_type
enum BOOKMARK_TYPE {
    BOOKMARK_LASTPOS,
    BOOKMARK_POS,
    BOOKMARK_COMMENT,
    BOOKMARK_CORRECTION
};

class CRDirEntry {
protected:
	lString8 _pathName;
	bool _isArchive;
public:
	CRDirEntry(const lString8 & pathname, bool archive) : _pathName(pathname), _isArchive(archive) {}
	virtual ~CRDirEntry() {}
    virtual const lString8 & getPathName() const { return _pathName; }
    virtual lString8 getFileName() const;
    virtual lString16 getTitle() const;
    virtual void setTitle(lString16 title) { CR_UNUSED(title); }
    virtual lString16 getDescription() const { return lString16(); }
    virtual void setDescription(lString16 description) { CR_UNUSED(description); }
    virtual lString8 getURL() { return lString8(); }
    lString16 getSeriesName(bool numberFirst) const;
    lString16 getSeriesNameOnly() const;
    int getSeriesNumber() const;
    lString16 getAuthorNames(bool fileAs) const;
	virtual bool isDirectory() const = 0;
	virtual bool isArchive() const { return _isArchive; }
	virtual BookDBBook * getBook() const { return NULL; }
    virtual void setBook(BookDBBook * book) { CR_UNUSED(book); }
    virtual BookDBBookmark * getLastPosition() const { return NULL; }
    virtual void setLastPosition(BookDBBookmark * bookmark) { CR_UNUSED(bookmark); }
    virtual bool isParsed() const { return false; }
    virtual void setParsed(bool parsed) { CR_UNUSED(parsed); }
    virtual CRDirEntry * clone() const { return NULL; }
    virtual DIR_TYPE getDirType() const { return DIR_TYPE_NORMAL; }
    virtual bool isRootDir() { DIR_TYPE t = getDirType(); return t == DIR_TYPE_SD_CARD || t == DIR_TYPE_INTERNAL_STORAGE || t == DIR_TYPE_FS_ROOT; }
    virtual lUInt64 getLastAccessTime() const { return 0; }
    /// for pathname like  @authors:ABC returns ABC
    virtual lString16 getFilterString() const;
    /// returns true for items like book-by-author, etc.
    virtual bool isSpecialItem();
    virtual int getBookCount() const { return 0; }
    virtual void setBookCount(int count) { CR_UNUSED(count); }
};

class CRFileItem : public CRDirEntry {
private:
	BookDBBook * _book;
	bool _parsed;
public:
	virtual void setParsed(bool parsed) { _parsed = parsed; }
	virtual bool isParsed() const { return _parsed; }
	virtual BookDBBook * getBook() const { return _book; }
	virtual void setBook(BookDBBook * book) { if (_book) delete _book; _book = book; }
	CRFileItem(const lString8 & pathname, bool archive) : CRDirEntry(pathname, archive), _book(NULL), _parsed(false) { }
	~CRFileItem() { if (_book) delete _book; }
	virtual bool isDirectory() const { return false; }
    virtual CRDirEntry * clone() const { CRFileItem * res = new CRFileItem(this->getPathName(), this->isArchive()); res->setParsed(this->_parsed); res->setBook(this->_book ? this->_book->clone() : NULL); return res; }
};

class CRDirItem : public CRDirEntry {
    int _bookCount;
    int _folderCount;
public:
    virtual DIR_TYPE getDirType() const;
    CRDirItem(const lString8 & pathname, bool isArchive) : CRDirEntry(pathname, isArchive), _bookCount(0), _folderCount(0) {}
	virtual bool isDirectory() const { return true; }
    virtual CRDirEntry * clone() const { CRDirItem * res = new CRDirItem(this->getPathName(), this->isArchive()); return res; }
    virtual int getBookCount() const { return _bookCount; }
    virtual void setBookCount(int count) { _bookCount = count; }
    virtual int getFolderCount() const { return _folderCount; }
    virtual void setFolderCount(int count) { _folderCount = count; }
};

class CRTopDirItem : public CRDirItem {
    DIR_TYPE _dirtype;
    lUInt64 _lastAccess;
public:
    CRTopDirItem(DIR_TYPE dirType, const lString8 & pathname, lUInt64 lastAccess = 0) : CRDirItem(pathname, false), _dirtype(dirType), _lastAccess(lastAccess) {}
    virtual DIR_TYPE getDirType() const { return _dirtype; }
    virtual CRDirEntry * clone() const { CRTopDirItem * res = new CRTopDirItem(this->getDirType(), this->getPathName()); return res; }
    virtual lUInt64 getLastAccessTime() const { return _lastAccess; }
    virtual void setLastAccessTime(lUInt64 ts) { _lastAccess = ts; }
};

class CRTopDirList : public CRDirItem {
    LVPtrVector<CRTopDirItem> _entries;
public:
    int itemCount() const { return _entries.length(); }
    CRTopDirItem * getItem(int index) const { return _entries[index]; }
    CRTopDirItem * addItem(DIR_TYPE t, lString8 path, lUInt64 ts = 0);
    CRTopDirItem * itemByType(DIR_TYPE t);
    CRTopDirItem * find(lString8 path);
    void clear() { _entries.clear(); }
    void addAll(CRTopDirList & v) {
        for (int i = 0; i < v.itemCount(); i++) {
            CRTopDirItem * item = v.getItem(i);
            addItem(item->getDirType(), item->getPathName(), item->getLastAccessTime());
        }
    }

    CRTopDirList() :  CRDirItem(lString8(), false) {}
    void sort(int sortOrder);
};

/// directory cache item
class CRDirContentItem : public CRDirItem {
    friend class CRDirCache;
protected:
    LVPtrVector<CRDirEntry> _entries;
    CRMutexRef _mutex;
    bool _scanned;
    volatile bool _scanning;
    int _lockCount; // to protect from cache cleanup while folder is opened
    virtual bool needScan() { return !_scanned; }
    virtual bool scan() = 0;
    virtual bool refresh();
public:
    virtual void lock() { _lockCount++; }
    virtual void unlock() { _lockCount--; }
    virtual void setParsed(bool parsed) { _scanned = parsed; }
    virtual bool isParsed() const { return _scanned; }
    virtual int itemCount() const;
    virtual CRDirEntry * getItem(int index) const;
    CRDirContentItem(CRDirEntry * item);
    CRDirContentItem(const lString8 & pathname, bool isArchive);
    virtual void sort(int sortOrder);
};


/// directory cache item
class CRDirCacheItem : public CRDirContentItem {
    friend class CRDirCache;
	lUInt64 _hash;
    virtual bool scan();
    virtual bool needScan();
public:
    CRDirCacheItem(CRDirEntry * item);
    CRDirCacheItem(const lString8 & pathname, bool isArchive);
    virtual bool isScanning() const { return _scanning; }
    void sort(int sortOrder);
};

#define RECENT_DIR_TAG "@recent"

#define SEARCH_RESULTS_TAG "@search"

#define BOOKS_BY_AUTHOR_TAG "@authors:"
#define BOOKS_BY_TITLE_TAG "@titles:"
#define BOOKS_BY_FILENAME_TAG "@filenames:"
#define BOOKS_BY_SERIES_TAG "@series:"

#define SEARCH_RESULTS_PREFIX "@search:"

#define OPDS_CATALOGS_TAG "@catalogs:"
#define OPDS_CATALOG_TAG "@catalog:"


class CRRecentBookItem : public CRFileItem {
    BookDBBookmark * _lastPosition;
public:
    virtual DIR_TYPE getDirType() const { return DIR_TYPE_RECENT; }
    /// creates new item; clones book and position inside
    CRRecentBookItem(BookDBBook * book, BookDBBookmark * lastPosition);
};

class CRRecentBooksItem : public CRDirContentItem {
protected:
    /// load from DB
    virtual bool scan();

public:
    /// returns true if order of books in list has been changed
    bool onLastPositionUpdated(BookDBBook * book, BookDBBookmark * position);
    virtual DIR_TYPE getDirType() const { return DIR_TYPE_RECENT; }
    CRRecentBooksItem() : CRDirContentItem(lString8(RECENT_DIR_TAG), false) {}
};

class CROpdsCatalogsItem : public CRDirContentItem {
protected:

    LVAutoPtr<BookDBCatalog> _catalog;

    lString8 _url;
    lString16 _title;
    lString16 _description;

    /// load from DB
    virtual bool scan();

public:
    //virtual void setCatalog(BookDBCatalog * catalog) { _catalog = catalog->clone(); }
    virtual DIR_TYPE getDirType() const { return DIR_TYPE_OPDS_CATALOG; }
    CROpdsCatalogsItem(const BookDBCatalog * catalog, lString8 url) : CRDirContentItem(lString8(OPDS_CATALOG_TAG) + lString8::itoa(catalog->id) + (url.empty() ? lString8() : lString8(":") + url), false), _catalog(catalog->clone()), _url(url) {}
    CROpdsCatalogsItem(const CRDirItem * dir) : CRDirContentItem(dir->getPathName(), false)
    {
        //_catalog = (((CROpdsCatalogsItem*)dir)->_catalog->clone());
    }
    virtual CRDirEntry * clone() const { CROpdsCatalogsItem * res = new CROpdsCatalogsItem(this); return res; }
    virtual void addEntry(CRDirEntry * entry) { _entries.add(entry); }
    BookDBCatalog * getCatalog() { return _catalog.get(); }
    void setCatalog(BookDBCatalog * catalog) {
        _catalog = catalog->clone();
    }

    virtual lString16 getTitle() const { return _title.empty() ? Utf8ToUnicode(_catalog->name.c_str()) : _title; }
    virtual void setTitle(lString16 title) { _title = title; }
    virtual lString16 getDescription() const { return _description; }
    virtual void setDescription(lString16 description) { _description = description; }
    virtual lString8 getURL() { return _url.empty() ? lString8(_catalog->url.c_str()) : _url; }
};

class CRBookDBLookupItem : public CRDirContentItem {
protected:
    /// load from DB
    virtual bool scan();
public:
    virtual void unlock();
    CRBookDBLookupItem(lString8 path) : CRDirContentItem(path, false) {}
};

class CRDirScanCallback {
public:
    virtual void onDirectoryScanFinished(CRDirContentItem * item) = 0;
    virtual ~CRDirScanCallback() {}
};

/// directory contents and file properties scanning manager
class CRDirCache  : public CRRunnable {

    CRRecentBooksItem _recentBooks;

    class DirectoryScanTask : public CRRunnable {
    public:
        CRDirContentItem * dir;
        CRDirScanCallback * callback;
        DirectoryScanTask(CRDirContentItem * _dir, CRDirScanCallback * _callback) : dir(_dir), callback(_callback) {}
        virtual ~DirectoryScanTask() {
        }
        bool isSame(lString8 _pathname) {
            return dir->getPathName() == _pathname;
        }
        virtual void run() {
            callback->onDirectoryScanFinished(dir);
        }
    };

    struct Item {
        CRDirContentItem * dir;
		Item * next;
		Item * prev;
        Item(CRDirContentItem * _dir) : dir(_dir), next(NULL), prev(NULL) { }
	};
	Item * _head;
	LVHashTable<lString8, Item*> _byName;
    void addItem(CRDirContentItem * dir);
	Item * findItem(const lString8 & pathname);
    void removeItem(const lString8 & pathname);
    void moveToHead(Item * item);

    bool _stopped;
    CRMonitorRef _monitor;
    CRThreadRef _thread;
    LVQueue<DirectoryScanTask *> _queue;

    CRDirScanCallback * _defCallback;

    void clear();
    CRFileItem * findBook(const lString8 & pathname);

public:

    void setDefaultCallback(CRDirScanCallback * callback) { _defCallback = callback; }

    void stop();
    virtual void run();

    CRDirCache();
    ~CRDirCache();
    CRDirContentItem * find(lString8 pathname);
    CRDirContentItem * find(CRDirItem * dir) { return find(dir->getPathName()); }
    CRDirContentItem * getOrAdd(CRDirItem * dir);
    CRDirContentItem * getOrAdd(const lString8 & pathname);
    void scan(const lString8 & pathname, CRDirScanCallback * callback = NULL);
    CRFileItem * scanFile(const lString8 & pathname);

    /// saves last position for book; fills ids for inserted items
    bool saveLastPosition(BookDBBook * book, BookDBBookmark * pos);
    /// loads last position for book (returns cloned value), returns NULL if not found
    BookDBBookmark * loadLastPosition(BookDBBook * book);

    /// remove item from cache
    void remove(const lString8 & pathname);
};

/// directory contents cache
extern CRDirCache * dirCache;

/// create dirCache
void CRSetupDirectoryCacheManager();
/// stop dirCache thread, remove dirCache
void CRStopDirectoryCacheManager();

BookDBBook * LVParseBook(const lString8 & path, const lString8 & pathName, LVContainerRef & arcContainer);

#endif /* FILEINFO_H_ */
