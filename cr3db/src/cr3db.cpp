/*
 * cr3db.cpp
 *
 *  Created on: Aug 16, 2013
 *      Author: vlopatin
 */

#include "cr3db.h"
#include <lvstring.h>

CRBookDB * bookDB = NULL;


#define DB_VERSION 21

/// creates/upgrades DB schema
bool CRBookDB::updateSchema()
{
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
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
			");") < 0 || err;
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
			"author_book_index ON book_author (author_fk, book_fk) ;") < 0 || err;
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
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    CRLog::info("Opening database %s", pathname);
	int res = _db.open(pathname, false);
	return res;
}

/// read DB content to caches
bool CRBookDB::fillCaches() {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
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
    int lastPosCount = loadLastPositionsToCache();
    CRLog::info("DB::fillCaches - %d authors, %d series, %d folders, %d last positions read", authorCount, seriesCount, folderCount, lastPosCount);
	return !err;
}

bool CRBookDB::saveSeries(BookDBSeries * item) {
	if (!item)
		return true;
	//CRLog::trace("saveSeries(%s)", item->name.c_str());
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
			//CRLog::trace("calling bindText(1, %s, %d)", item->name.c_str(), item->name.length());
			stmt.bindText(1, item->name);
			err = (stmt.step() != DB_DONE) || err;
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
	if (!item)
		return true;
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
			stmt.bindText(1, item->name);
			err = (stmt.step() != DB_DONE) || err;
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
	if (!item)
		return true;
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
		err = stmt.prepare("UPDATE author SET name = ?, file_as = ?, aliased_author_fk = ? WHERE id = ?;") != 0 || err;
		if (!err) {
			stmt.bindText(1, item->name);
			if (!item->fileAs)
				stmt.bindText(2, item->fileAs);
			else
				stmt.bindNull(2);
			if (item->aliasedAuthorId)
				stmt.bindInt64(3, item->aliasedAuthorId);
			else
				stmt.bindNull(3);
			stmt.bindInt64(4, item->id);
			err = (stmt.step() != DB_DONE) || err;
			if (!err) {
				BookDBAuthor * cacheItem = item->clone();
				_authorCache.put(cacheItem);
			}
		}
	} else {
		//CRLog::trace("inserting new author");
		err = stmt.prepare("INSERT INTO author (name, file_as, aliased_author_fk) VALUES (?, ?, ?);") != 0 || err;
		if (!err) {
			stmt.bindText(1, item->name);
			if (item->fileAs.length() > 0)
				stmt.bindText(2, item->fileAs);
			else
				stmt.bindNull(2);
			if (item->aliasedAuthorId)
				stmt.bindInt64(3, item->aliasedAuthorId);
			else
				stmt.bindNull(3);
			err = (stmt.step() != DB_DONE) || err;
			if (!err) {
				item->id = stmt.lastInsertId();
				BookDBAuthor * cacheItem = item->clone();
				_authorCache.put(cacheItem);
			}
		}
	}
	return !err;
}

/*
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
 */

BookDBBookmark * CRBookDB::loadBookmark(SQLiteStatement & stmt) {
    BookDBBookmark * bmk = new BookDBBookmark();
    bmk->id = stmt.getInt64(0);
    bmk->bookId = stmt.getInt64(1); //"book_fk INTEGER NOT NULL REFERENCES book (id),"
    bmk->type = stmt.getInt(2); //"type INTEGER NOT NULL DEFAULT 0,"
    bmk->percent = stmt.getInt(3); //"percent INTEGER DEFAULT 0,"
    bmk->shortcut = stmt.getInt(4); //"shortcut INTEGER DEFAULT 0,"
    bmk->timestamp = stmt.getInt64(5); //"time_stamp INTEGER DEFAULT 0,"
    bmk->startPos = stmt.getText(6); //"start_pos VARCHAR NOT NULL,"
    bmk->endPos = stmt.getText(7); //"end_pos VARCHAR,"
    bmk->titleText = stmt.getText(8); //"title_text VARCHAR,"
    bmk->posText = stmt.getText(9); //"pos_text VARCHAR,"
    bmk->commentText = stmt.getText(10); //"comment_text VARCHAR, "
    bmk->timeElapsed = stmt.getInt64(11); //"time_elapsed INTEGER DEFAULT 0"
    return bmk;
}

#define BOOKMARK_ALL_FIELDS "id, book_fk, type, percent, shortcut, time_stamp, " \
        "start_pos, end_pos, title_text, pos_text, comment_text, time_elapsed"

int CRBookDB::loadLastPositionsToCache() {
    SQLiteStatement stmt(&_db);
    bool err = false;

    _lastPositionCache.clear();

    stmt.prepare("SELECT " BOOKMARK_ALL_FIELDS
            " FROM bookmark WHERE type = 0;");
    int count = 0;
    while (stmt.step() == DB_ROW) {
        BookDBBookmark * bmk = loadBookmark(stmt);
        _lastPositionCache.put(bmk->bookId, bmk);
        count++;
    }
    return count;
}

BookDBBook * CRBookDB::loadBookToCache(SQLiteStatement & stmt) {
	if (stmt.step() == DB_ROW) {
		// TODO: use reference from cache, instead of clone
		BookDBBook * item = new BookDBBook();
		item->id = stmt.getInt64(0);
		item->pathname = stmt.getText(1);
		lInt64 folderId = stmt.getInt64(2);
		item->filename = stmt.getText(3);
		item->arcname = stmt.getText(4);
		item->title = stmt.getText(5);
		lInt64 seriesId = stmt.getInt64(6);
		item->seriesNumber = stmt.getInt(7);
		item->format = stmt.getInt(8);
		item->filesize = stmt.getInt(9);
		item->arcsize = stmt.getInt(10);
		item->createTime = stmt.getInt64(11);
		item->lastAccessTime = stmt.getInt64(12);
		item->flags = stmt.getInt(13);
		item->language = stmt.getText(14);
		item->folder = _folderCache.getClone(folderId);
		item->series = _seriesCache.getClone(seriesId);
		stmt.prepare("SELECT author_fk FROM book_author WHERE book_fk = ?;");
		stmt.bindInt64(1, item->id);
		while (stmt.step() == DB_ROW) {
			lInt64 authorId = stmt.getInt64(0);
			BookDBAuthor * author = _authorCache.getClone(authorId);
			if (author)
				item->authors.add(author);
		}
		_bookCache.put(item);
		return item;
	}
	return NULL;
}

#define BOOK_TABLE_ALL_FIELDS \
				"id, pathname, folder_fk, filename, " \
				"arcname, title, series_fk, series_number, format, " \
				"filesize, arcsize, create_time, last_access_time, flags, language"
BookDBBook * CRBookDB::loadBookToCache(const DBString & path) {
	SQLiteStatement stmt(&_db);
	bool err = false;
	stmt.prepare("SELECT " BOOK_TABLE_ALL_FIELDS
			" FROM book WHERE pathname = ?;");
	stmt.bindText(1, path);
	return loadBookToCache(stmt);
}

BookDBBook * CRBookDB::loadBookToCache(lInt64 id) {
	SQLiteStatement stmt(&_db);
	bool err = false;
	stmt.prepare("SELECT " BOOK_TABLE_ALL_FIELDS
			" FROM book WHERE id = ?;");
	stmt.bindInt64(1, id);
	return loadBookToCache(stmt);
}

static void appendUpdateField(lString8 & buf, const char * paramName, int & fieldIndex, int & index) {
	if (buf.length())
		buf.append(",");
	buf.append(paramName);
	buf.append("=?");
	fieldIndex = index++;
}

bool BookDBBook::hasAuthor(BookDBAuthor * author) {
	for (int i = 0; i<authors.length(); i++)
		if (authors[i]->id == author->id)
			return true;
	return false;
}

bool CRBookDB::updateBook(BookDBBook * book, BookDBBook * fromCache)
{
	if (*book == *fromCache)
		return true; // nothing to save - no changes
	// there are some changes
	bool authorsChanged = !book->equalAuthors(*fromCache);
	bool folderChanged = !book->equalFolders(*fromCache);
	bool seriesChanged = !book->equalSeries(*fromCache);
	bool fieldsChanged = !book->equalFields(*fromCache);
	int pathnameIndex = -1;
	int folderIndex = -1;
	int filenameIndex = -1;
	int arcnameIndex = -1;
	int titleIndex = -1;
	int seriesIndex = -1;
	int seriesNumberIndex = -1;
	int formatIndex = -1;
	int filesizeIndex = -1;
	int arcsizeIndex = -1;
	int createTimeIndex = -1;
	int lastAccessTimeIndex = -1;
	int flagsIndex = -1;
	int languageIndex = -1;
	int index = 1;
	lString8 buf;
	if (book->pathname != fromCache->pathname) appendUpdateField(buf, "pathname", pathnameIndex, index);
	if (book->filename != fromCache->filename) appendUpdateField(buf, "filename", filenameIndex, index);
	if (book->arcname != fromCache->arcname) appendUpdateField(buf, "arcname", arcnameIndex, index);
	if (book->title != fromCache->title) appendUpdateField(buf, "title", titleIndex, index);
	if (book->seriesNumber != fromCache->seriesNumber) appendUpdateField(buf, "series_number", seriesNumberIndex, index);
	if (book->format != fromCache->format) appendUpdateField(buf, "format", formatIndex, index);
	if (book->filesize != fromCache->filesize) appendUpdateField(buf, "filesize", filesizeIndex, index);
	if (book->arcsize != fromCache->arcsize) appendUpdateField(buf, "arcsize", arcsizeIndex, index);
	if (book->createTime != fromCache->createTime) appendUpdateField(buf, "create_time", createTimeIndex, index);
	if (book->lastAccessTime != fromCache->lastAccessTime) appendUpdateField(buf, "last_access_time", lastAccessTimeIndex, index);
	if (book->flags != fromCache->flags) appendUpdateField(buf, "flags", flagsIndex, index);
	if (book->language != fromCache->language) appendUpdateField(buf, "language", languageIndex, index);
	// TODO: compare series and folder
	if (folderChanged) {
		//if (book->folder != fromCache->folder)
		appendUpdateField(buf, "folder_fk", folderIndex, index);
	}
	if (seriesChanged) {
		//if (book->series != fromCache->series)
		appendUpdateField(buf, "series_fk", seriesIndex, index);
	}

	SQLiteStatement stmt(&_db);
	bool err = false;
	lString8 sql("UPDATE book SET ");
	sql += buf;
	sql += " WHERE id = ?;";
	stmt.prepare(sql.c_str());
	stmt.bindInt64(index, book->id);
	if (pathnameIndex >= 0) stmt.bindText(pathnameIndex, book->pathname);
	if (filenameIndex >= 0) stmt.bindText(filenameIndex, book->filename);
	if (arcnameIndex >= 0) stmt.bindText(arcnameIndex, book->arcname);
	if (titleIndex >= 0) stmt.bindText(titleIndex, book->title);
	if (seriesNumberIndex >= 0) stmt.bindInt(seriesNumberIndex, book->seriesNumber);
	if (formatIndex >= 0) stmt.bindInt(formatIndex, book->format);
	if (filesizeIndex >= 0) stmt.bindInt(filesizeIndex, book->filesize);
	if (arcsizeIndex >= 0) stmt.bindInt(arcsizeIndex, book->arcsize);
	if (createTimeIndex >= 0) stmt.bindInt64(createTimeIndex, book->createTime);
	if (lastAccessTimeIndex >= 0) stmt.bindInt64(lastAccessTimeIndex, book->lastAccessTime);
	if (flagsIndex >= 0) stmt.bindInt(flagsIndex, book->flags);
	if (languageIndex >= 0) stmt.bindText(languageIndex, book->language);

	if (folderIndex >= 0) stmt.bindKey(folderIndex, !book->folder ? 0 : book->folder->id);
	if (seriesIndex >= 0) stmt.bindKey(seriesIndex, !book->series ? 0 : book->series->id);

	if (stmt.step() == DB_DONE) {
		if (authorsChanged) {
			// TODO: update authors
			lString8 addedAuthors;
			lString8 removedAuthors;
			char s[100];
			for (int i=0; i<book->authors.length(); i++) {
				if (!fromCache->hasAuthor(book->authors[i])) {
					if (addedAuthors.length())
						addedAuthors.append(",");
					sprintf(s, "(%lld,%lld)", book->id, book->authors[i]->id);
					addedAuthors.append(s);
				}
			}
			for (int i=0; i<fromCache->authors.length(); i++) {
				if (!book->hasAuthor(fromCache->authors[i])) {
					if (removedAuthors.length())
						removedAuthors.append(",");
					sprintf(s, "%lld", fromCache->authors[i]->id);
					addedAuthors.append(s);
				}
			}
			if (addedAuthors.length()) {
				lString8 sql("INSERT INTO book_author(book_fk, author_fk) VALUES ");
				sql.append(addedAuthors);
				sql.append(";");
				SQLiteStatement stmt(&_db);
				stmt.prepare(sql.c_str());
				if (stmt.step() != DB_DONE) {
					CRLog::error("Error while inserting book authors %s", sql.c_str());
					err = true;
				}
			}
			if (removedAuthors.length()) {
				lString8 sql("DELETE FROM book_author WHERE book_fk = ");
				sprintf(s, "%lld", book->id);
				sql.append(s);
				sql.append(" AND author_fk IN (");
				sql.append(removedAuthors);
				sql.append(");");
				SQLiteStatement stmt(&_db);
				stmt.prepare(sql.c_str());
				if (stmt.step() != DB_DONE) {
					CRLog::error("Error while removing book authors %s", sql.c_str());
					err = true;
				}
			}
		}
	} else {
		CRLog::error("Error while updating book table %s", sql.c_str());
	}
	// copy changes to cached instance
	if (fieldsChanged)
		fromCache->assignFields(*book);
	if (folderChanged)
		fromCache->assignFolder(*book);
	if (seriesChanged)
		fromCache->assignSeries(*book);
	if (authorsChanged)
		fromCache->assignAuthors(*book);
	return true;
}

bool CRBookDB::insertBook(BookDBBook * book)
{
	bool err = false;

	saveFolder(book->folder.get());
	saveSeries(book->series.get());

	SQLiteStatement stmt(&_db);
	err = stmt.prepare("INSERT INTO book (pathname, folder_fk, filename, arcname, title, series_fk, series_number, format, "
		"filesize, arcsize, create_time, last_access_time, flags, language) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);") != 0 || err;
	stmt.bindText(1, book->pathname);
	stmt.bindKey(2, !book->folder.isNull() ? book->folder->id: 0);
	stmt.bindText(3, book->filename);
	stmt.bindText(4, book->arcname);
	stmt.bindText(5, book->title);
	stmt.bindInt64(6, !book->series.isNull() ? book->series->id: 0);
	stmt.bindInt(7, book->seriesNumber);
	stmt.bindInt(8, book->format);
	stmt.bindInt(9, book->filesize);
	stmt.bindInt(10, book->arcsize);
	stmt.bindInt64(11, book->createTime);
	stmt.bindInt64(12, book->lastAccessTime);
	stmt.bindInt(13, book->flags);
	stmt.bindText(14, book->language);
	if (stmt.step() == DB_DONE) {
		// inserted OK
		book->id = stmt.lastInsertId();
		// inserting authors
		if (book->authors.length()) {
			CRLog::trace("inserting authors");
			lString8 buf;
			char s[64];
			for (int i=0; i<book->authors.length(); i++) {
				if (saveAuthor(book->authors[i])) {
					if (buf.length())
						buf.append(",");
					sprintf(s, "(%lld,%lld)", book->id, book->authors[i]->id);
					buf.append(s);
				} else {
					CRLog::error("Cannot save author");
				}
			}
			if (buf.length()) {
				SQLiteStatement stmt(&_db);
				err = stmt.prepare((lString8("INSERT INTO book_author (book_fk, author_fk) VALUES ") + buf + ";").c_str()) != 0 || err;
				if (stmt.step() == DB_DONE) {
					// Ok
				} else {
					CRLog::error("Error while inserting book_author records %s", buf.c_str());
				}
			}
		}
	} else {
		CRLog::error("Error while inserting book record %s", book->pathname.c_str());
	}
	BookDBBook * forCache = book->clone();
	_bookCache.put(forCache);
	return !err;
}

bool CRBookDB::saveBook(BookDBBook * book) {
	if (book->id) {
		BookDBBook * fromCache = _bookCache.get(book->id);
		if (!fromCache) {
			fromCache = loadBookToCache(book->id);
		}
		if (fromCache) {
			return updateBook(book, fromCache);
		}
	}
	BookDBBook * fromCache = _bookCache.get(book->pathname);
	if (fromCache) {
		*book = *fromCache;
		return true;
	}
	return insertBook(book);
}

bool CRBookDB::saveBooks(LVPtrVector<BookDBBook> & books) {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    bool res = true;
	for (int i = 0; i<books.length(); i++) {
		res = saveBook(books[i]) && res;
	}
	return res;
}

/// saves last position for book
bool CRBookDB::saveLastPosition(BookDBBook * book, BookDBBookmark * pos) {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    BookDBBook * cached = _bookCache.get(book->pathname);
    if (book->id == 0 && cached)
        book->id = cached->id;

    return false;
}

/// saves last position for book
BookDBBookmark * CRBookDB::loadLastPosition(BookDBBook * book) {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    BookDBBook * cached = _bookCache.get(book->pathname);
    if (book->id == 0 && cached)
        book->id = cached->id;
    if (!book->id)
        return NULL;
    BookDBBookmark *  res = _lastPositionCache.find(book->id);
    return res;
}

BookDBBook * CRBookDB::loadBook(lString8 pathname) {
    lString8Collection pathnames;
    LVPtrVector<BookDBBook> loaded;
    lString8Collection notFound;
    pathnames.add(pathname);
    loadBooks(pathnames, loaded, notFound);
    if (loaded.length() == 1)
        return loaded[0]->clone();
    return NULL;
}

bool CRBookDB::loadBooks(lString8Collection & pathnames, LVPtrVector<BookDBBook> & loaded, lString8Collection & notFound) {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    for (int i = 0; i<pathnames.length(); i++) {
		BookDBBook * cached = _bookCache.get(pathnames[i].c_str());
		if (cached)
			loaded.add(cached->clone());
		else {
			// not found in cache
			cached = loadBookToCache(pathnames[i].c_str());
			if (cached)
				loaded.add(cached->clone());
			else {
				// not found
				notFound.add(pathnames[i]);
			}
		}
	}
	return true;
}

BookDBBook * BookDBBookCache::get(lInt64 key) {
	Item * item = _byId.get(key);
	if (!item)
		return NULL;
	moveToHead(item);
	return item->book;
}

void BookDBBookCache::remove(Item * item) {
	if (item->prev)
		item->prev->next = item->next;
	else
		head = item->next;
	if (item->next)
		item->next->prev = item->prev;
	_byId.remove(item->book->id);
	_byName.remove(item->book->pathname);
	delete item->book;
	delete item;
}

void BookDBBookCache::moveToHead(Item * item) {
	if (head == item)
		return;
	if (item->prev)
		item->prev->next = item->next;
	else
		head = item->next;
	if (item->next)
		item->next->prev = item->prev;
	if (head)
		head->prev = item;
	item->next = head;
	item->prev = NULL;
    head = item;
}

BookDBBook * BookDBBookCache::get(const DBString & path) {
	Item * item = _byName.get(path);
	if (!item)
		return NULL;
	moveToHead(item);
	return item->book;
}

void BookDBBookCache::put(BookDBBook * book) {
	Item * item = new Item(book);
	if (head)
		head->prev = item;
	item->next = head;
	head = item;
	_byId.set(book->id, item);
	_byName.set(book->pathname, item);
}

void BookDBBookCache::clear() {
	while (head) {
		remove(head);
	}
}


//==================================================================


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

