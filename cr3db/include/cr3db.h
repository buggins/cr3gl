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

class DBString {
	char * str;
public:
	DBString() : str(NULL) { }
	DBString(const char * s);
	void clear();
	int length() const;
	char operator[] (int index) const;
	DBString & operator = (const char * s);
	DBString & operator = (const DBString & s);
	bool operator == (const DBString & s) const;
	bool operator == (const char * s) const;
	bool operator !() const { return !str; }
	const char * get() const { return str; }
	~DBString();
};
lUInt32 getHash(const DBString & s);


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
		return id == v.id &&
				name == v.name &&
				fileAs == v.fileAs &&
				aliasedAuthorId == v.aliasedAuthorId;
	}
};

class BookDBSeries : public BookDBEntity {
public:
	DBString name;
	BookDBSeries() {}
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
		return id == v.id &&
				name == v.name;
	}
};

class BookDBFolder : public BookDBEntity {
public:
	DBString name;
	BookDBFolder() {}
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
		return id == v.id &&
				name == v.name;
	}
};

class BookDBBook : public BookDBEntity {
public:
	DBString pathname; // "pathname VARCHAR NOT NULL,"
	lInt64 folderId; //"folder_fk INTEGER REFERENCES folder (id),"
	DBString filename; //"filename VARCHAR NOT NULL,"
	DBString arcname; //"arcname VARCHAR,"
	DBString title; //"title VARCHAR COLLATE NOCASE,"
	DBString seriesId; //"series_fk INTEGER REFERENCES series (id),"
	int seriesNumber; //"series_number INTEGER,"
	int format; //"format INTEGER,"
	int filesize; //"filesize INTEGER,"
	int arcsize; //"arcsize INTEGER,"
	lInt64 createTime; //"create_time INTEGER,"
	lInt64 lastAccessTime; //"last_access_time INTEGER, "
	int flags; // "flags INTEGER DEFAULT 0, "
	DBString language; // "language VARCHAR DEFAULT NULL"
	BookDBBook() : folderId(0), seriesNumber(0), format(0), filesize(0), arcsize(0), createTime(0), lastAccessTime(0), flags(0) {}
	BookDBBook(const BookDBBook & v) {
		id = v.id;
		pathname = v.pathname;
		filename = v.filename;
		arcname = v.arcname;
		title = v.title;
		seriesId = v.seriesId;
		seriesNumber = v.seriesNumber;
		format = v.format;
		filesize = v.filesize;
		arcsize = v.arcsize;
		createTime = v.createTime;
		lastAccessTime = v.lastAccessTime;
		flags = v.flags;
	}
	BookDBBook & operator = (const BookDBBook & v) {
		id = v.id;
		pathname = v.pathname;
		filename = v.filename;
		arcname = v.arcname;
		title = v.title;
		seriesId = v.seriesId;
		seriesNumber = v.seriesNumber;
		format = v.format;
		filesize = v.filesize;
		arcsize = v.arcsize;
		createTime = v.createTime;
		lastAccessTime = v.lastAccessTime;
		flags = v.flags;
		return *this;
	}
	bool operator == (const BookDBBook & v) const {
		return id == v.id &&
				pathname == v.pathname &&
				filename == v.filename &&
				arcname == v.arcname &&
				title == v.title &&
				seriesId == v.seriesId &&
				seriesNumber == v.seriesNumber &&
				format == v.format &&
				filesize == v.filesize &&
				arcsize == v.arcsize &&
				createTime == v.createTime &&
				lastAccessTime == v.lastAccessTime &&
				flags == v.flags;
	}
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
	BookDBFolder * get(const DBString & name);
	void put(BookDBFolder * item);
	void clear();
	~BookDBFolderCache() { clear(); }
};


class CRBookDB {
	SQLiteDB _db;
	BookDBSeriesCache _seriesCache;
	BookDBAuthorCache _authorCache;
	BookDBFolderCache _folderCache;
public:
	/// open database file; returns 0 on success, error code otherwise
	int open(const char * pathname);
	/// closes DB
	int close() { return _db.close(); }
	/// returns true if DB is opened
	bool isOpened() { return _db.isOpened(); }
	/// creates/upgrades DB schema
	bool updateSchema();
	/// read DB content to caches
	bool fillCaches();
};



#endif /* CR3DB_H_ */
