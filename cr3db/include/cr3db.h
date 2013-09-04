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
#include "lvref.h"
#include "lvstring.h"
#include "crconcurrent.h"


class BookDBEntity {
public:
	lInt64 id; //"id INTEGER PRIMARY KEY AUTOINCREMENT,"
	BookDBEntity() : id(0) {}
	BookDBEntity(lInt64 _id) : id(_id) {}
};

class BookDBAuthor : public BookDBEntity {
public:
	DBString name;
	DBString fileAs;
	lInt64 aliasedAuthorId;
	BookDBAuthor() : aliasedAuthorId(0) {}
	BookDBAuthor(const char * _name) : name(_name), aliasedAuthorId(0) {}
	BookDBAuthor(const char * _name, const char * _fileAs) : name(_name), fileAs(_fileAs), aliasedAuthorId(0) {}
	BookDBAuthor(const BookDBAuthor & v) {
		id = v.id;
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
	BookDBSeries(const BookDBSeries & v) {
		id = v.id;
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
	BookDBFolder(const BookDBFolder & v) {
		id = v.id;
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
	BookDBBook(const BookDBBook & v) {
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
	BookDBBookmark(const BookDBBookmark & v) {
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
	BookDBFolder * get(lInt64 key);
	BookDBFolder * getClone(lInt64 key) { BookDBFolder * res = key ? get(key) : NULL; return res ? res->clone() : NULL; }
	BookDBFolder * get(const DBString & name);
	void put(BookDBFolder * item);
	void clear();
	~BookDBFolderCache() { clear(); }
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


class CRBookDB {
    CRMutexRef _mutex;
    SQLiteDB _db;
	BookDBSeriesCache _seriesCache;
	BookDBAuthorCache _authorCache;
	BookDBFolderCache _folderCache;
	BookDBBookCache _bookCache;
	BookDBBook * loadBookToCache(lInt64 id);
	BookDBBook * loadBookToCache(const DBString & path);
	BookDBBook * loadBookToCache(SQLiteStatement & stmt);
	bool updateBook(BookDBBook * book, BookDBBook * fromCache);
	bool insertBook(BookDBBook * book);

    bool saveSeries(BookDBSeries * item);
    bool saveFolder(BookDBFolder * folder);
    bool saveAuthor(BookDBAuthor * author);
    bool saveBook(BookDBBook * book);
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


    /// protected by mutex
	bool saveBooks(LVPtrVector<BookDBBook> & books);
    /// protected by mutex
	bool loadBooks(lString8Collection & pathnames, LVPtrVector<BookDBBook> & loaded, lString8Collection & notFound);
    /// protected by mutex
    BookDBBook * loadBook(lString8 pathname);
};

extern CRBookDB * bookDB;

#endif /* CR3DB_H_ */
