/*
 * cr3db.h
 *
 *  Created on: Aug 16, 2013
 *      Author: vlopatin
 */

#ifndef CR3DB_H_
#define CR3DB_H_

#include "basedb.h"
#include "lvhashtable.h"
#include "lvptrvec.h"
#include "lvarray.h"
#include "lvref.h"
#include "lvstring.h"
#include "crconcurrent.h"
#include "lvqueue.h"
#include "lvhashtable.h"
#include "lvstring.h"

class BookDBEntity {
public:
	lInt64 id; //"id INTEGER PRIMARY KEY AUTOINCREMENT,"
	BookDBEntity() : id(0) {}
	BookDBEntity(lInt64 _id) : id(_id) {}
    BookDBEntity(const BookDBEntity & v) : id(v.id) {}
};

class BookDBAuthor : public BookDBEntity {
public:
	DBString name;
	DBString fileAs;
	lInt64 aliasedAuthorId;
	BookDBAuthor() : aliasedAuthorId(0) {}
	BookDBAuthor(const char * _name) : name(_name), aliasedAuthorId(0) {}
	BookDBAuthor(const char * _name, const char * _fileAs) : name(_name), fileAs(_fileAs), aliasedAuthorId(0) {}
    BookDBAuthor(const BookDBAuthor & v) : BookDBEntity(v.id) {
		name = v.name;
		fileAs = v.fileAs;
		aliasedAuthorId = v.aliasedAuthorId;
	}
	BookDBAuthor & operator = (const BookDBAuthor & v) {
		id = v.id;
		name = v.name;
		fileAs = v.fileAs;
		aliasedAuthorId = v.aliasedAuthorId;
		return *this;
	}
	bool operator == (const BookDBAuthor & v) const {
		if (this == NULL && &v == NULL)
			return true;
		if (this == NULL || &v == NULL)
			return false;
		return id == v.id &&
				name == v.name &&
				fileAs == v.fileAs &&
				aliasedAuthorId == v.aliasedAuthorId;
	}
	BookDBAuthor * clone() const { return new BookDBAuthor(*this); }
};

class BookDBSeries : public BookDBEntity {
public:
	DBString name;
	BookDBSeries() {}
	BookDBSeries(const char * _name) : name(_name) {}
    BookDBSeries(const BookDBSeries & v)  : BookDBEntity(v.id) {
		name = v.name;
	}
	BookDBSeries & operator = (const BookDBSeries & v) {
		id = v.id;
		name = v.name;
		return *this;
	}
	bool operator == (const BookDBSeries & v) const {
		if (this == NULL && &v == NULL)
			return true;
		if (this == NULL || &v == NULL)
			return false;
		return id == v.id &&
				name == v.name;
	}
	BookDBSeries * clone() const { return new BookDBSeries(*this); }
};

class BookDBFolder : public BookDBEntity {
public:
	DBString name;
	BookDBFolder() {}
	BookDBFolder(const char * _name) : name(_name) { }
    BookDBFolder(const BookDBFolder & v)  : BookDBEntity(v.id) {
		name = v.name;
	}
	BookDBFolder & operator = (const BookDBFolder & v) {
		id = v.id;
		name = v.name;
		return *this;
	}
	bool operator == (const BookDBFolder & v) const {
		if (this == NULL && &v == NULL)
			return true;
		if (this == NULL || &v == NULL)
			return false;
		return id == v.id &&
				name == v.name;
	}
	BookDBFolder * clone() const { return new BookDBFolder(*this); }
};

class BookDBFolderBookmark : public BookDBEntity {
public:
    DBString name;
    int type;
    lInt64 lastUsage;
    BookDBFolderBookmark() : type(0), lastUsage(0) {}
    BookDBFolderBookmark(const char * _name, int _type, lInt64 _lastUsage) : name(_name), type(_type), lastUsage(_lastUsage) { }
    BookDBFolderBookmark(const BookDBFolderBookmark & v)  : BookDBEntity(v.id) {
        name = v.name;
        type = v.type;
        lastUsage = v.lastUsage;
    }
    BookDBFolderBookmark & operator = (const BookDBFolderBookmark & v) {
        id = v.id;
        name = v.name;
        type = v.type;
        lastUsage = v.lastUsage;
        return *this;
    }
    bool operator == (const BookDBFolderBookmark & v) const {
        if (this == NULL && &v == NULL)
            return true;
        if (this == NULL || &v == NULL)
            return false;
        return id == v.id &&
                name == v.name &&
                type == v.type &&
                lastUsage == v.lastUsage;
    }
    BookDBFolderBookmark * clone() const { return new BookDBFolderBookmark(*this); }
};

class BookDBCatalog : public BookDBEntity {
public:
    DBString name;
    DBString url;
    DBString login;
    DBString password;
    lInt64 lastUsage;
    BookDBCatalog() {}
    BookDBCatalog(const char * _name, const char * _url) : name(_name), url(_url), lastUsage(GetCurrentTimeMillis()) { }
    BookDBCatalog(const BookDBCatalog & v) : BookDBEntity(v) {
        name = v.name;
        url = v.url;
        login = v.login;
        password = v.password;
        lastUsage = v.lastUsage;
    }
    BookDBCatalog & operator = (const BookDBCatalog & v) {
        id = v.id;
        name = v.name;
        url = v.url;
        login = v.login;
        password = v.password;
        lastUsage = v.lastUsage;
        return *this;
    }
    bool operator == (const BookDBCatalog & v) const {
        if (this == NULL && &v == NULL)
            return true;
        if (this == NULL || &v == NULL)
            return false;
        return id == v.id &&
                name == v.name &&
                url == v.url &&
                login == v.login &&
                password == v.password &&
                lastUsage == v.lastUsage;
    }
    BookDBCatalog * clone() const { return new BookDBCatalog(*this); }
};

class BookDBBook : public BookDBEntity {
public:
	DBString pathname; // "pathname VARCHAR NOT NULL,"
	LVAutoPtr<BookDBFolder> folder; //"folder_fk INTEGER REFERENCES folder (id),"
	DBString filename; //"filename VARCHAR NOT NULL,"
	DBString arcname; //"arcname VARCHAR,"
	DBString title; //"title VARCHAR COLLATE NOCASE,"
	LVAutoPtr<BookDBSeries> series; //"series_fk INTEGER REFERENCES series (id),"
	int seriesNumber; //"series_number INTEGER,"
	int format; //"format INTEGER,"
	int filesize; //"filesize INTEGER,"
	int arcsize; //"arcsize INTEGER,"
	lInt64 createTime; //"create_time INTEGER,"
	lInt64 lastAccessTime; //"last_access_time INTEGER, "
	int flags; // "flags INTEGER DEFAULT 0, "
	DBString language; // "language VARCHAR DEFAULT NULL"
	LVPtrVector<BookDBAuthor> authors;
	BookDBBook() : seriesNumber(0), format(0), filesize(0), arcsize(0), createTime(0), lastAccessTime(0), flags(0) {}
    BookDBBook(const BookDBBook & v)  : BookDBEntity(v.id) {
		assignFields(v);
		assignFolder(v);
		assignSeries(v);
		assignAuthors(v);
	}

	~BookDBBook() {
	}
	bool hasAuthor(BookDBAuthor * author);
	void assignSeries(const BookDBBook & v) {
		series = v.series.get() ? v.series->clone() : NULL;
	}
	void assignFolder(const BookDBBook & v) {
		folder = v.folder.get() ? v.folder->clone() : NULL;
	}
	void assignAuthors(const BookDBBook & v) {
		authors.clear();
		for (int i=0; i<v.authors.length(); i++)
			authors.add(v.authors[i]->clone());
	}
	void assignFields(const BookDBBook & v) {
		id = v.id;
		if (pathname != v.pathname) pathname = v.pathname;
		if (filename != v.filename) filename = v.filename;
		if (arcname != v.arcname) arcname = v.arcname;
		if (title != v.title) title = v.title;
		seriesNumber = v.seriesNumber;
		format = v.format;
		filesize = v.filesize;
		arcsize = v.arcsize;
		createTime = v.createTime;
		lastAccessTime = v.lastAccessTime;
		flags = v.flags;
	}
	BookDBBook & operator = (const BookDBBook & v) {
		assignFields(v);
		assignFolder(v);
		assignSeries(v);
		assignAuthors(v);
		return *this;
	}
	bool equalAuthors(const BookDBBook & v2) const {
		if (authors.length() != v2.authors.length())
			return false;
		for (int i = 0; i < authors.length(); i++) {
			bool found = false;
			for (int j = 0; j < v2.authors.length(); j++) {
				if (*authors[i] == *v2.authors[i]) {
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}
		return true;
	}
	bool equalFolders(const BookDBBook & v) const {
		return *folder == *v.folder;
	}
	bool equalSeries(const BookDBBook & v) const {
		return *series == *v.series;
	}
	bool equalFields(const BookDBBook & v) const {
		return id == v.id &&
				pathname == v.pathname &&
				filename == v.filename &&
				arcname == v.arcname &&
				title == v.title &&
				seriesNumber == v.seriesNumber &&
				format == v.format &&
				filesize == v.filesize &&
				arcsize == v.arcsize &&
				createTime == v.createTime &&
				lastAccessTime == v.lastAccessTime &&
				flags == v.flags;
	}
	bool operator == (const BookDBBook & v) const {
		return equalFields(v) &&
				equalFolders(v) &&
				equalSeries(v) &&
				equalAuthors(v);
	}
	BookDBBook * clone() const { return new BookDBBook(*this); }
};

class BookDBBookmark : public BookDBEntity {
public:
	lInt64 bookId; //"book_fk INTEGER NOT NULL REFERENCES book (id),"
	int type; //"type INTEGER NOT NULL DEFAULT 0,"
	int percent; //"percent INTEGER DEFAULT 0,"
	int shortcut; //"shortcut INTEGER DEFAULT 0,"
	lInt64 timestamp; //"time_stamp INTEGER DEFAULT 0,"
	DBString startPos; //"start_pos VARCHAR NOT NULL,"
	DBString endPos; //"end_pos VARCHAR,"
	DBString titleText; //"title_text VARCHAR,"
	DBString posText; //"pos_text VARCHAR,"
	DBString commentText; //"comment_text VARCHAR, "
	lInt64 timeElapsed; //"time_elapsed INTEGER DEFAULT 0"
	BookDBBookmark() : bookId(0), type(0), percent(0), shortcut(0), timestamp(0), timeElapsed(0) {}
    BookDBBookmark(const BookDBBookmark & v)  : BookDBEntity(v.id) {
		type = v.type;
		percent = v.percent;
		shortcut = v.shortcut;
		timestamp = v.timestamp;
		startPos = v.startPos;
		endPos = v.endPos;
		titleText = v.titleText;
		posText = v.posText;
		commentText = v.commentText;
		timeElapsed = v.timeElapsed;
	}
	BookDBBookmark & operator = (const BookDBBookmark & v) {
		id = v.id;
		type = v.type;
		percent = v.percent;
		shortcut = v.shortcut;
		timestamp = v.timestamp;
		startPos = v.startPos;
		endPos = v.endPos;
		titleText = v.titleText;
		posText = v.posText;
		commentText = v.commentText;
		timeElapsed = v.timeElapsed;
		return *this;
	}
	bool operator == (const BookDBBookmark & v) const {
		return id == v.id &&
				type == v.type &&
				percent == v.percent &&
				shortcut == v.shortcut &&
				timestamp == v.timestamp &&
				startPos == v.startPos &&
				endPos == v.endPos &&
				titleText == v.titleText &&
				posText == v.posText &&
				commentText == v.commentText &&
				timeElapsed == v.timeElapsed;
	}
    BookDBBookmark * clone() const { return new BookDBBookmark(*this); }
};

class BookDBPrefixStats {
public:
    lString16 prefix;
    int bookCount;
    lInt64 bookId; // valid only for book count = 1
};

class BookDBAuthorCache {
	LVHashTable<lUInt64, BookDBAuthor *> _byId;
	LVHashTable<DBString, BookDBAuthor *> _byName;
public:
	BookDBAuthorCache() : _byId(1000), _byName(1000) {}
	BookDBAuthor * get(lInt64 key);
	BookDBAuthor * getClone(lInt64 key) { BookDBAuthor * res = key ? get(key) : NULL; return res ? res->clone() : NULL; }
	BookDBAuthor * get(const DBString & name);
    void put(BookDBAuthor * item);
	void clear();
	~BookDBAuthorCache() { clear(); }
};

class BookDBSeriesCache {
	LVHashTable<lUInt64, BookDBSeries *> _byId;
	LVHashTable<DBString, BookDBSeries *> _byName;
public:
	BookDBSeriesCache() : _byId(1000), _byName(1000) {}
	BookDBSeries * get(lInt64 key);
	BookDBSeries * getClone(lInt64 key) { BookDBSeries * res = key ? get(key) : NULL; return res ? res->clone() : NULL; }
	BookDBSeries * get(const DBString & name);
    void put(BookDBSeries * item);
	void clear();
	~BookDBSeriesCache() { clear(); }
};

class BookDBFolderCache {
	LVHashTable<lUInt64, BookDBFolder *> _byId;
	LVHashTable<DBString, BookDBFolder *> _byName;
public:
	BookDBFolderCache() : _byId(1000), _byName(1000) {}
    BookDBFolder * getClone(lInt64 key) { BookDBFolder * res = key ? get(key) : NULL; return res ? res->clone() : NULL; }
    BookDBFolder * get(lInt64 key);
	BookDBFolder * get(const DBString & name);
	void put(BookDBFolder * item);
	void clear();
	~BookDBFolderCache() { clear(); }
};

class BookDBFolderBookmarkCache {
    LVHashTable<lUInt64, BookDBFolderBookmark *> _byId;
    LVHashTable<DBString, BookDBFolderBookmark *> _byName;
public:
    BookDBFolderBookmarkCache() : _byId(1000), _byName(1000) {}
    BookDBFolderBookmark * getClone(lInt64 key) { BookDBFolderBookmark * res = key ? get(key) : NULL; return res ? res->clone() : NULL; }
    BookDBFolderBookmark * get(lInt64 key);
    BookDBFolderBookmark * get(const DBString & name);
    void getAll(LVPtrVector<BookDBFolderBookmark> & folderBookmarks);
    void put(BookDBFolderBookmark * item);
    void remove(lInt64 id);
    void clear();
    ~BookDBFolderBookmarkCache() { clear(); }
};

class BookDBCatalogCache {
    LVHashTable<lUInt64, BookDBCatalog *> _byId;
    LVHashTable<DBString, BookDBCatalog *> _byName;
public:
    BookDBCatalogCache() : _byId(1000), _byName(1000) {}
    BookDBCatalog * getClone(lInt64 key) { BookDBCatalog * res = key ? get(key) : NULL; return res ? res->clone() : NULL; }
    BookDBCatalog * get(lInt64 key);
    BookDBCatalog * get(const DBString & name);
    void getAll(LVPtrVector<BookDBCatalog> & catalogs);
    void put(BookDBCatalog * item);
    void clear();
    ~BookDBCatalogCache() { clear(); }
};

class BookDBBookCache {
	struct Item {
		BookDBBook * book;
		Item * next;
		Item * prev;
		Item(BookDBBook * _book) : book(_book), next(NULL), prev(NULL) {}
	};
	LVHashTable<lUInt64, Item *> _byId;
	LVHashTable<DBString, Item *> _byName;
	Item * head;
	int count;
	void moveToHead(Item * item);
	void remove(Item * item);
public:
	BookDBBookCache() : _byId(1000), _byName(1000), head(NULL), count(0) {}
	BookDBBook * get(lInt64 key);
	BookDBBook * get(const DBString & path);
	void put(BookDBBook * item);
	void clear();
	~BookDBBookCache() { clear(); }
};

class CRBookLastPositionCache {
    LVPtrVector<BookDBBookmark> _bookmarks;
    LVHashTable<lUInt64, int> _indexByBookId;
public:
    CRBookLastPositionCache() : _indexByBookId(4096) {}
    void sort();
    int length() { return _bookmarks.length(); }
    BookDBBookmark * operator[] (int index) { return _bookmarks[index]; }
    /// returns ptr to copy saved in cache
    BookDBBookmark * find(lInt64 bookId);
    /// item will be stored as is, owned by _bookmarks
    void put (lInt64 bookId, BookDBBookmark * item);
    void remove(lInt64 bookId);
    void clear();
};

enum SEARCH_FIELD {
    SEARCH_FIELD_INVALID,
    SEARCH_FIELD_AUTHOR,
    SEARCH_FIELD_TITLE,
    SEARCH_FIELD_SERIES,
    SEARCH_FIELD_FILENAME
};

class CRBookDB {
    CRMutexRef _mutex;
    SQLiteDB _db;
	BookDBSeriesCache _seriesCache;
	BookDBAuthorCache _authorCache;
	BookDBFolderCache _folderCache;
    BookDBFolderBookmarkCache _folderBookmarkCache;
    BookDBCatalogCache _catalogCache;
	BookDBBookCache _bookCache;
    CRBookLastPositionCache _lastPositionCache;
	BookDBBook * loadBookToCache(lInt64 id);
	BookDBBook * loadBookToCache(const DBString & path);
	BookDBBook * loadBookToCache(SQLiteStatement & stmt);

    /// returns number of last positions loaded into cache
    int loadLastPositionsToCache();
    /// reads single bookmark row fileds
    BookDBBookmark * loadBookmark(SQLiteStatement & stmt);

    bool updateBookmark(BookDBBookmark * bookmark, BookDBBookmark * fromCache);
    bool insertBookmark(BookDBBookmark * bookmark);


    bool updateBook(BookDBBook * book, BookDBBook * fromCache);
	bool insertBook(BookDBBook * book);

    bool saveSeries(BookDBSeries * item);
    bool saveFolder(BookDBFolder * folder);
    bool saveAuthor(BookDBAuthor * author);
    bool saveBook(BookDBBook * book);

    void addOpdsCatalogs(const char * catalogs[]);
public:
    CRBookDB() : _mutex(concurrencyProvider->createMutex()) {
    }

	/// open database file; returns 0 on success, error code otherwise
    int open(const char * pathname);
	/// closes DB
    int close() {
        CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
        return _db.close();
    }
	/// returns true if DB is opened
	bool isOpened() { return _db.isOpened(); }
	/// creates/upgrades DB schema
	bool updateSchema();
	/// read DB content to caches
	bool fillCaches();

    bool saveFolderBookmark(BookDBFolderBookmark * folderBookmark);

    void updateFolderBookmarkUsage(lString8 path);
    void addFolderBookmark(lString8 path);
    bool removeFolderBookmark(BookDBFolderBookmark * folderBookmark);
    bool removeFolderBookmark(lString8 path);
    bool isFolderBookmarked(lString8 path);

    /// protected by mutex
	bool saveBooks(LVPtrVector<BookDBBook> & books);
    /// load books by pathnames; protected by mutex
	bool loadBooks(lString8Collection & pathnames, LVPtrVector<BookDBBook> & loaded, lString8Collection & notFound);
    /// load books by keys;  protected by mutex
    bool loadBooks(LVArray<lInt64> & keys, LVPtrVector<BookDBBook> & loaded);
    /// protected by mutex
    BookDBBook * loadBook(lString8 pathname);

    /// protected by mutex
    bool loadRecentBooks(LVPtrVector<BookDBBook> & books, LVPtrVector<BookDBBookmark> & lastPositions);

    bool loadOpdsCatalogs(LVPtrVector<BookDBCatalog> & catalogs);

    bool loadFolderBookmarks(LVPtrVector<BookDBFolderBookmark> & folderBookmarks);

    /// saves last position for book; fills ids for inserted items
    bool saveLastPosition(BookDBBook * book, BookDBBookmark * pos);
    /// saves bookmark for book; fills ids for inserted items
    bool saveBookmark(BookDBBook * book, BookDBBookmark * bookmark);
    /// removes bookmark for book
    bool removeBookmark(BookDBBook * book, BookDBBookmark * bookmark);
    /// loads all non-last-position bookmarks
    bool loadBookmarks(BookDBBook * book, LVPtrVector<BookDBBookmark> & bookmarks);
    /// loads last position for book (returns cloned value), returns NULL if not found
    BookDBBookmark * loadLastPosition(BookDBBook * book);

    /// searches BookDB by field - return prefixes
    bool findPrefixes(SEARCH_FIELD field, lString16 searchString, lString8 folderFilter, LVPtrVector<BookDBPrefixStats> & prefixes);
    /// searches BookDB by field - return files
    bool findBooks(SEARCH_FIELD field, lString16 searchString, lString8 folderFilter, LVPtrVector<BookDBBook> & loaded);
};

extern CRBookDB * bookDB;

#endif /* CR3DB_H_ */
