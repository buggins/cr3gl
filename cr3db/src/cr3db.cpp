/*
 * cr3db.cpp
 *
 *  Created on: Aug 16, 2013
 *      Author: vlopatin
 */

#include "cr3db.h"
#include <lvstring.h>

DBString::DBString(const char * s)
{
	//CRLog::trace("DBString(%s)", s);
	str = s ? strdup(s) : NULL;
	//CRLog::trace("DBString(%s) - duplicated", s);
}

DBString::DBString(const DBString & s) {
	str = s.str ? strdup(s.str) : NULL;
}

DBString::~DBString()
{
	if (str)
		free(str);
}

void DBString::clear() {
	if (str)
		free(str);
	str = NULL;
}

int DBString::length() const {
	return str ? strlen(str) : 0;
}

char DBString::operator[] (int index) const {
	int len = length();
	if (index < 0 || index <= len)
		return 0;
	return str[index];
}

DBString & DBString::operator = (const DBString & s) {
	clear();
	str = s.str ? strdup(s.str) : NULL;
	return *this;
}

DBString & DBString::operator = (const char * s) {
	clear();
	str = s ? strdup(s) : NULL;
	return *this;
}

DBString & DBString::operator += (const char * s) {
	int thislength = length();
	int slength = s ? strlen(s) : 0;
	if (slength == 0)
		return *this;
	if (thislength == 0) {
		*this = s;
	} else {
		char * buf = (char *)malloc(thislength + slength + 1);
		memcpy(buf, str, thislength);
		memcpy(buf + thislength, s, slength + 1);
		*this = buf;
	}
	return *this;
}

bool DBString::operator == (const DBString & s) const {
	if (!str && !s)
		return true;
	if (!str || !s)
		return false;
	return strcmp(str, s.str) == 0;
}

bool DBString::operator == (const char * s) const {
	if (!str && !s)
		return true;
	if (!str || !s)
		return false;
	return strcmp(str, s) == 0;
}

lUInt32 getHash(const DBString & s) {
	if (!s)
		return 0;
	lUInt32 value = 1253;
	int len = s.length();
	const char * str = s.get();
	for (int i = 0; i < len; i++)
		value = value * 31 + str[i];
	return value;
}

#define DB_VERSION 21

/// creates/upgrades DB schema
bool CRBookDB::updateSchema()
{
	int currentVersion = _db.getVersion();
	bool err = false;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS author ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"name VARCHAR NOT NULL COLLATE NOCASE,"
			"file_as VARCHAR NULL COLLATE NOCASE,"
			"aliased_author_fk INTEGER NULL REFERENCES author(id)"
			");") < 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
            "author_name_index ON author (name);") < 0 || err;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS series ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"name VARCHAR NOT NULL COLLATE NOCASE"
			");") < 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
	        "series_name_index ON series (name) ;") < 0 || err;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS folder ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"name VARCHAR NOT NULL"
			");") < 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"folder_name_index ON folder (name);") < 0 || err;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS book ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"pathname VARCHAR NOT NULL,"
			"folder_fk INTEGER REFERENCES folder (id),"
			"filename VARCHAR NOT NULL,"
			"arcname VARCHAR,"
			"title VARCHAR COLLATE NOCASE,"
			"series_fk INTEGER REFERENCES series (id),"
			"series_number INTEGER,"
			"format INTEGER,"
			"filesize INTEGER,"
			"arcsize INTEGER,"
			"create_time INTEGER,"
			"last_access_time INTEGER, "
			"flags INTEGER DEFAULT 0, "
			"language VARCHAR DEFAULT NULL"
			");");
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_folder_index ON book (folder_fk);") < 0 || err;
	err = _db.executeUpdate("CREATE UNIQUE INDEX IF NOT EXISTS "
			"book_pathname_index ON book (pathname);") < 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_filename_index ON book (filename);") < 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_title_index ON book (title);") < 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_last_access_time_index ON book (last_access_time) ;") < 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_title_index ON book (title) ;") < 0 || err;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS book_author ("
			"book_fk INTEGER NOT NULL REFERENCES book (id),"
			"author_fk INTEGER NOT NULL REFERENCES author (id),"
			"PRIMARY KEY (book_fk, author_fk)"
			");") < 0 || err;
	err = _db.executeUpdate("CREATE UNIQUE INDEX IF NOT EXISTS "
			"author_book_index ON book_author (author_fk, book_fk) ;");
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS bookmark ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"book_fk INTEGER NOT NULL REFERENCES book (id),"
			"type INTEGER NOT NULL DEFAULT 0,"
			"percent INTEGER DEFAULT 0,"
			"shortcut INTEGER DEFAULT 0,"
			"time_stamp INTEGER DEFAULT 0,"
			"start_pos VARCHAR NOT NULL,"
			"end_pos VARCHAR,"
			"title_text VARCHAR,"
			"pos_text VARCHAR,"
			"comment_text VARCHAR, "
			"time_elapsed INTEGER DEFAULT 0"
			");") < 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"bookmark_book_index ON bookmark (book_fk) ;") < 0 || err;

	// ====================================================================
	if ( currentVersion<1 )
		err = !_db.addColumnIfNotExists("bookmark", "shortcut", "ALTER TABLE bookmark ADD COLUMN shortcut INTEGER DEFAULT 0;") || err;
	if ( currentVersion<4 )
		err = !_db.addColumnIfNotExists("book", "flags", "ALTER TABLE book ADD COLUMN flags INTEGER DEFAULT 0;") || err;
	if ( currentVersion<6 )
		err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS opds_catalog ("
				"id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"name VARCHAR NOT NULL COLLATE NOCASE, "
				"url VARCHAR NOT NULL COLLATE NOCASE, "
				"login VARCHAR NOT NULL COLLATE NOCASE, "
				"password VARCHAR NOT NULL COLLATE NOCASE, "
				"last_usage INTEGER DEFAULT 0"
				");") < 0 || err;
//	if (currentVersion < 7) {
//		addOPDSCatalogs(DEF_OPDS_URLS1);
//	}
//	if (currentVersion < 8)
//		addOPDSCatalogs(DEF_OPDS_URLS2);
	if (currentVersion < 13)
		err = !_db.addColumnIfNotExists("book", "language", "ALTER TABLE book ADD COLUMN language VARCHAR DEFAULT NULL;") || err;
	if (currentVersion < 15)
		err = !_db.addColumnIfNotExists("opds_catalog", "last_usage", "ALTER TABLE opds_catalog ADD COLUMN last_usage INTEGER DEFAULT 0;") || err;
	if (currentVersion < 16)
		err = !_db.addColumnIfNotExists("bookmark", "time_elapsed", "ALTER TABLE bookmark ADD COLUMN time_elapsed INTEGER DEFAULT 0;") || err;

	// CR new UI updates (since version 21)
	if (currentVersion < 21) {
		err = !_db.addColumnIfNotExists("author", "file_as", "ALTER TABLE author ADD COLUMN file_as VARCHAR NULL COLLATE NOCASE;") || err;
		err = !_db.addColumnIfNotExists("author", "aliased_author_fk", "ALTER TABLE author ADD COLUMN aliased_author_fk INTEGER NULL REFERENCES author(id);") || err;
		err = !_db.addColumnIfNotExists("opds_catalog", "login", "ALTER TABLE opds_catalog ADD COLUMN login VARCHAR NULL COLLATE NOCASE;") || err;
		err = !_db.addColumnIfNotExists("opds_catalog", "password", "ALTER TABLE opds_catalog ADD COLUMN password VARCHAR NULL COLLATE NOCASE;") || err;
	}

	//==============================================================
	// add more updates above this line

	if (err) {
		CRLog::error("DB schema upgrade error detected!");
	} else {
		// set current version
		if (currentVersion < DB_VERSION)
			_db.setVersion(DB_VERSION);
	}

	return !err;
}

/// open database file; returns 0 on success, error code otherwise
int CRBookDB::open(const char * pathname) {
	CRLog::info("Opening database %s", pathname);
	int res = _db.open(pathname, false);
	return res;
}

/// read DB content to caches
bool CRBookDB::fillCaches() {
	if (!isOpened())
		return false;
	CRLog::trace("Filling caches");
	bool err = false;
	_seriesCache.clear();
	_folderCache.clear();
	_authorCache.clear();
	int seriesCount = 0;
	int folderCount = 0;
	int authorCount = 0;
	CRLog::trace("Filling series cache");
	SQLiteStatement stmt(&_db);
	err = stmt.prepare("select id, name from series;") != 0 || err;
	while (stmt.step() == DB_ROW) {
		CRLog::trace("Reading row from series table");
		BookDBSeries * item = new BookDBSeries();
		item->id = stmt.getInt(0);
		item->name = stmt.getText(1);
		//CRLog::trace("Putting item %lld %s to cache", item->id, item->name.get());
		_seriesCache.put(item);
		seriesCount++;
	}
	CRLog::trace("Filling folder cache");
	err = stmt.prepare("select id, name from folder;") != 0 || err;
	while (stmt.step() == DB_ROW) {
		BookDBFolder * item = new BookDBFolder();
		item->id = stmt.getInt(0);
		item->name = stmt.getText(1);
		_folderCache.put(item);
		folderCount++;
	}
	CRLog::trace("Filling author cache");
	err = stmt.prepare("select id, name, file_as, aliased_author_fk from author;") != 0 || err;
	while (stmt.step() == DB_ROW) {
		BookDBAuthor * item = new BookDBAuthor();
		item->id = stmt.getInt(0);
		item->name = stmt.getText(1);
		item->fileAs = stmt.getText(2);
		item->aliasedAuthorId = stmt.getInt64(3);
		_authorCache.put(item);
		authorCount++;
	}
	CRLog::info("DB::fillCaches - %d authors, %d series, %d folders read", authorCount, seriesCount, folderCount);
	return !err;
}

bool CRBookDB::saveSeries(BookDBSeries * item) {
	CRLog::trace("saveSeries(%s)", item->name.get());
	BookDBSeries * byId = NULL;
	BookDBSeries * byName = NULL;
	if (item->id)
		byId = _seriesCache.get(item->id);
	byName = _seriesCache.get(item->name);
	if (byId && *byId == *item)
		return true;
	if (byName) {
		item->id = byName->id;
		return true;
	}
	SQLiteStatement stmt(&_db);
	bool err = false;
	if (byId) {
		// name changed? ignore...
	} else {
		err = stmt.prepare("INSERT INTO series (name) VALUES (?);") != 0 || err;
		if (!err) {
			CRLog::trace("calling bindText(1, %s, %d)", item->name.get(), item->name.length());
			stmt.bindText(1, item->name.get(), item->name.length());
			err = stmt.step() == DB_DONE || err;
			if (!err) {
				item->id = stmt.lastInsertId();
				BookDBSeries * cacheItem = item->clone();
				_seriesCache.put(cacheItem);
			}
		}
	}
	return !err;
}


bool CRBookDB::saveFolder(BookDBFolder * item) {
	//CRLog::trace("saveFolder(%s)", item->name.get());
	BookDBFolder * byId = NULL;
	BookDBFolder * byName = NULL;
	if (item->id)
		byId = _folderCache.get(item->id);
	byName = _folderCache.get(item->name);
	//CRLog::trace("existing item %s by name", byName ? "found" : "not found");
	if (byId && *byId == *item)
		return true;
	if (byName) {
		item->id = byName->id;
		return true;
	}
	SQLiteStatement stmt(&_db);
	bool err = false;
	if (byId) {
		// name changed? ignore...
	} else {
		//CRLog::trace("before prepare INSERT (%s, %d)", item->name.get(), item->name.length());
		err = stmt.prepare("INSERT INTO folder (name) VALUES (?);") != 0 || err;
		if (!err) {
			//CRLog::trace("calling bindText(1, %s, %d)", item->name.get(), item->name.length());
			stmt.bindText(1, item->name.get(), item->name.length());
			err = stmt.step() == DB_DONE || err;
			if (!err) {
				item->id = stmt.lastInsertId();
				BookDBFolder * cacheItem = item->clone();
				_folderCache.put(cacheItem);
			}
		}
	}
	return !err;
}

bool CRBookDB::saveAuthor(BookDBAuthor * item) {
	//CRLog::trace("saveAuthor()");
	BookDBAuthor * byId = NULL;
	BookDBAuthor * byName = NULL;
	if (item->id)
		byId = _authorCache.get(item->id);
	byName = _authorCache.get(item->name);
	if (byId && *byId == *item)
		return true;
	if (byName) {
		//CRLog::trace("found author with the same name");
		*item = *byName;
		return true;
	}
	SQLiteStatement stmt(&_db);
	bool err = false;
	if (byId) {
		//CRLog::trace("updating existing author");
		err = stmt.prepare("UPDATE author SET name = ?, file_as = ?, aliased_author_fk = ? WHERE id = ?") != 0 || err;
		if (!err) {
			stmt.bindText(1, item->name.get(), item->name.length());
			if (!item->fileAs)
				stmt.bindText(2, item->fileAs.get(), item->fileAs.length());
			else
				stmt.bindNull(2);
			if (item->aliasedAuthorId)
				stmt.bindInt64(3, item->aliasedAuthorId);
			else
				stmt.bindNull(3);
			stmt.bindInt64(4, item->id);
			err = stmt.step() == DB_DONE || err;
			if (!err) {
				BookDBAuthor * cacheItem = item->clone();
				_authorCache.put(cacheItem);
			}
		}
	} else {
		//CRLog::trace("inserting new author");
		err = stmt.prepare("INSERT INTO author (name, file_as, aliased_author_fk) VALUES (?, ?, ?);") != 0 || err;
		if (!err) {
			stmt.bindText(1, item->name.get(), item->name.length());
			if (item->fileAs.length() > 0)
				stmt.bindText(2, item->fileAs.get(), item->fileAs.length());
			else
				stmt.bindNull(2);
			if (item->aliasedAuthorId)
				stmt.bindInt64(3, item->aliasedAuthorId);
			else
				stmt.bindNull(3);
			err = stmt.step() == DB_DONE || err;
			if (!err) {
				item->id = stmt.lastInsertId();
				BookDBAuthor * cacheItem = item->clone();
				_authorCache.put(cacheItem);
			}
		}
	}
	return !err;
}


BookDBAuthor * BookDBAuthorCache::get(lInt64 key) {
	return _byId.get(key);
}

BookDBAuthor * BookDBAuthorCache::get(const DBString & name) {
	return _byName.get(name);
}

void BookDBAuthorCache::put(BookDBAuthor * item) {
	BookDBAuthor * oldById = _byId.get(item->id);
	if (oldById == item) { // already the same item
		return;
	}
	if (oldById) {
		_byId.remove(item->id);
		_byName.remove(oldById->name);
		delete oldById;
	}
	_byId.set(item->id, item);
	_byName.set(item->name, item);
}

void BookDBAuthorCache::clear() {
	LVPtrVector<BookDBAuthor> items;
	LVHashTable<lUInt64, BookDBAuthor *>::iterator iter = _byId.forwardIterator();
	for (;;) {
		LVHashTable<lUInt64, BookDBAuthor *>::pair * item = iter.next();
		if (!item)
			break;
		items.add(item->value);
	}
}



BookDBSeries * BookDBSeriesCache::get(lInt64 key) {
	return _byId.get(key);
}

BookDBSeries * BookDBSeriesCache::get(const DBString & name) {
	return _byName.get(name);
}

void BookDBSeriesCache::put(BookDBSeries * item) {
	BookDBSeries * oldById = _byId.get(item->id);
	if (oldById == item) { // already the same item
		return;
	}
	if (oldById) {
		_byId.remove(item->id);
		_byName.remove(oldById->name);
		delete oldById;
	}
	_byId.set(item->id, item);
	_byName.set(item->name, item);
}

void BookDBSeriesCache::clear() {
	LVPtrVector<BookDBSeries> items;
	LVHashTable<lUInt64, BookDBSeries *>::iterator iter = _byId.forwardIterator();
	for (;;) {
		LVHashTable<lUInt64, BookDBSeries *>::pair * item = iter.next();
		if (!item)
			break;
		items.add(item->value);
	}
}





BookDBFolder * BookDBFolderCache::get(lInt64 key) {
	return _byId.get(key);
}

BookDBFolder * BookDBFolderCache::get(const DBString & name) {
	return _byName.get(name);
}

void BookDBFolderCache::put(BookDBFolder * item) {
	BookDBFolder * oldById = _byId.get(item->id);
	if (oldById == item) { // already the same item
		return;
	}
	if (oldById) {
		_byId.remove(item->id);
		_byName.remove(oldById->name);
		delete oldById;
	}
	_byId.set(item->id, item);
	_byName.set(item->name, item);
}

void BookDBFolderCache::clear() {
	LVPtrVector<BookDBFolder> items;
	LVHashTable<lUInt64, BookDBFolder *>::iterator iter = _byId.forwardIterator();
	for (;;) {
		LVHashTable<lUInt64, BookDBFolder *>::pair * item = iter.next();
		if (!item)
			break;
		items.add(item->value);
	}
}

