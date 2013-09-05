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

namespace CRUI {
	enum DocFormat {
		UNKNOWN_FORMAT,
		FB2,
		TXT,
		RTF,
		EPUB,
		HTML,
		TXT_BOOKMARK,
		CHM,
		DOC,
		PDB
	};

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
    DIR_TYPE_NORMAL
};

class CRDirEntry {
	lString8 _pathName;
	bool _isArchive;
public:
	CRDirEntry(const lString8 & pathname, bool archive) : _pathName(pathname), _isArchive(archive) {}
	virtual ~CRDirEntry() {}
	const lString8 & getPathName() const { return _pathName; }
	lString8 getFileName() const;
	lString16 getTitle() const;
    lString16 getSeriesName(bool numberFirst) const;
    lString16 getSeriesNameOnly() const;
    int getSeriesNumber() const;
    lString16 getAuthorNames(bool fileAs) const;
	virtual bool isDirectory() const = 0;
	virtual bool isArchive() const { return _isArchive; }
	virtual BookDBBook * getBook() const { return NULL; }
	virtual void setBook(BookDBBook * book) { }
	virtual bool isParsed() const { return false; }
	virtual void setParsed(bool parsed) {}
    virtual CRDirEntry * clone() const { return NULL; }
    virtual DIR_TYPE getDirType() const { return DIR_TYPE_NORMAL; }
    virtual bool isRootDir() { DIR_TYPE t = getDirType(); return t == DIR_TYPE_SD_CARD || t == DIR_TYPE_INTERNAL_STORAGE || t == DIR_TYPE_FS_ROOT; }
    virtual lUInt64 getLastAccessTime() const { return 0; }
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
public:
	CRDirItem(const lString8 & pathname, bool isArchive) : CRDirEntry(pathname, isArchive) {}
	virtual bool isDirectory() const { return true; }
    virtual CRDirEntry * clone() const { CRDirItem * res = new CRDirItem(this->getPathName(), this->isArchive()); return res; }
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
    CRTopDirList() :  CRDirItem(lString8(), false) {}
    void sort(int sortOrder);
};

/// directory cache item
class CRDirContentItem : public CRDirItem {
    friend class CRDirCache;
protected:
    LVPtrVector<CRDirEntry> _entries;
    CRMutexRef _mutex;
public:
    virtual int itemCount() const;
    virtual CRDirEntry * getItem(int index) const;
    CRDirContentItem(CRDirEntry * item);
    CRDirContentItem(const lString8 & pathname, bool isArchive);
    virtual void sort(int sortOrder) {}
};


/// directory cache item
class CRDirCacheItem : public CRDirContentItem {
    friend class CRDirCache;
	bool _scanned;
	lUInt64 _hash;
    volatile bool _scanning;
    bool scan();
    bool needScan();
    bool refresh();
public:
    CRDirCacheItem(CRDirEntry * item);
    CRDirCacheItem(const lString8 & pathname, bool isArchive);
    virtual void setParsed(bool parsed) { _scanned = parsed; }
	virtual bool isParsed() const { return _scanned; }
    virtual bool isScanning() const { return _scanning; }
    void sort(int sortOrder);
};

#define RECENT_DIR_TAG "@recent"

class CRRecentBooksItem : public CRDirContentItem {
protected:
    bool load();
public:
    virtual DIR_TYPE getDirType() const { return DIR_TYPE_RECENT; }
    CRRecentBooksItem() : CRDirContentItem(lString8(RECENT_DIR_TAG), false) {}
};


class CRDirScanCallback {
public:
    virtual void onDirectoryScanFinished(CRDirCacheItem * item) = 0;
    virtual ~CRDirScanCallback() {}
};

/// directory contents and file properties scanning manager
class CRDirCache  : public CRRunnable {

    CRRecentBooksItem _recentBooks;

    class DirectoryScanTask : public CRRunnable {
    public:
        CRDirCacheItem * dir;
        CRDirScanCallback * callback;
        DirectoryScanTask(CRDirCacheItem * _dir, CRDirScanCallback * _callback) : dir(_dir), callback(_callback) {}
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
		CRDirCacheItem * dir;
		Item * next;
		Item * prev;
		Item(CRDirCacheItem * _dir) : dir(_dir), next(NULL), prev(NULL) { }
	};
	Item * _head;
	LVHashTable<lString8, Item*> _byName;
	void addItem(CRDirCacheItem * dir);
	Item * findItem(const lString8 & pathname);
	void moveToHead(Item * item);

    bool _stopped;
    CRMonitorRef _monitor;
    CRThreadRef _thread;
    LVQueue<DirectoryScanTask *> _queue;
    void clear();
    CRFileItem * findBook(const lString8 & pathname);
public:
    void stop();
    virtual void run();

    CRDirCache();
    ~CRDirCache();
    CRDirCacheItem * find(lString8 pathname);
	CRDirCacheItem * find(CRDirItem * dir) { return find(dir->getPathName()); }
	CRDirCacheItem * getOrAdd(CRDirItem * dir);
    CRDirCacheItem * getOrAdd(const lString8 & pathname);
    void scan(const lString8 & pathname, CRDirScanCallback * callback);
    CRFileItem * scanFile(const lString8 & pathname);
};

/// directory contents cache
extern CRDirCache * dirCache;

/// create dirCache
void CRSetupDirectoryCacheManager();
/// stop dirCache thread, remove dirCache
void CRStopDirectoryCacheManager();

#endif /* FILEINFO_H_ */
