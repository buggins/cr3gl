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
    CR_UNUSED(guard);
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
    CR_UNUSED(guard);
    CRLog::info("Opening database %s", pathname);
	int res = _db.open(pathname, false);
	return res;
}

/// read DB content to caches
bool CRBookDB::fillCaches() {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    CR_UNUSED(guard);
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
    CRLog::info("DB::fillCaches - %d authors, %d series, %d folders", authorCount, seriesCount, folderCount);
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

    err = stmt.prepare("SELECT " BOOKMARK_ALL_FIELDS
            " FROM bookmark WHERE type = 0;") != DB_OK || err;
    int count = 0;
    if (!err) {
        while (stmt.step() == DB_ROW) {
            BookDBBookmark * bmk = loadBookmark(stmt);
            _lastPositionCache.put(bmk->bookId, bmk);
            count++;
        }
        _lastPositionCache.sort();
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
        {
            SQLiteStatement astmt(&_db);
            astmt.prepare("SELECT author_fk FROM book_author WHERE book_fk = ?;");
            astmt.bindInt64(1, item->id);
            while (astmt.step() == DB_ROW) {
                lInt64 authorId = astmt.getInt64(0);
                BookDBAuthor * author = _authorCache.getClone(authorId);
                if (author)
                    item->authors.add(author);
            }
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
    err = stmt.prepare("SELECT " BOOK_TABLE_ALL_FIELDS
            " FROM book WHERE pathname = ?;") != DB_OK || err;
    if (!err) {
        stmt.bindText(1, path);
        return loadBookToCache(stmt);
    } else {
        return NULL;
    }
}

BookDBBook * CRBookDB::loadBookToCache(lInt64 id) {
	SQLiteStatement stmt(&_db);
	bool err = false;
    err = stmt.prepare("SELECT " BOOK_TABLE_ALL_FIELDS
            " FROM book WHERE id = ?;") != DB_OK || err;
    if (!err) {
        stmt.bindInt64(1, id);
        return loadBookToCache(stmt);
    } else {
        return NULL;
    }
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
    CR_UNUSED(guard);
    bool res = true;
	for (int i = 0; i<books.length(); i++) {
		res = saveBook(books[i]) && res;
	}
	return res;
}

/// saves last position for book
bool CRBookDB::saveLastPosition(BookDBBook * book, BookDBBookmark * pos) {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    CR_UNUSED(guard);
    BookDBBook * cached = _bookCache.get(book->pathname);
    if (book->id == 0 && cached)
        book->id = cached->id;
    if (!book->id)
        return false;
    pos->bookId = book->id;
    pos->type = 0;
    BookDBBookmark * existing = _lastPositionCache.find(book->id);
    if (!existing) {
        // insert new
        bool res = insertBookmark(pos);
        _lastPositionCache.put(pos->bookId, pos->clone());
        return res;
    } else {
        // update
        if (existing->operator ==(*pos))
            return true; // the same
        bool res = updateBookmark(pos, existing);
        _lastPositionCache.put(pos->bookId, pos->clone());
        return res;
    }
    return false;
}

bool CRBookDB::updateBookmark(BookDBBookmark * b, BookDBBookmark * fromCache)
{
    int typeIndex = -1;
    int percentIndex = -1;
    int shortcutIndex = -1;
    int timestampIndex = -1;
    int startPosIndex = -1;
    int endPosIndex = -1;
    int titleTextIndex = -1;
    int posTextIndex = -1;
    int commentTextIndex = -1;
    int timeElapsedIndex = -1;

    int index = 1;
    lString8 buf;
    if (b->type != fromCache->type) appendUpdateField(buf, "type", typeIndex, index);
    if (b->percent != fromCache->percent) appendUpdateField(buf, "percent", percentIndex, index);
    if (b->shortcut != fromCache->shortcut) appendUpdateField(buf, "shortcut", shortcutIndex, index);
    if (b->timestamp != fromCache->timestamp) appendUpdateField(buf, "time_stamp", timestampIndex, index);
    if (b->startPos != fromCache->startPos) appendUpdateField(buf, "start_pos", startPosIndex, index);
    if (b->endPos != fromCache->endPos) appendUpdateField(buf, "end_pos", endPosIndex, index);
    if (b->titleText != fromCache->titleText) appendUpdateField(buf, "title_text", titleTextIndex, index);
    if (b->posText != fromCache->posText) appendUpdateField(buf, "pos_text", posTextIndex, index);
    if (b->commentText != fromCache->commentText) appendUpdateField(buf, "comment_text", commentTextIndex, index);
    if (b->timeElapsed != fromCache->timeElapsed) appendUpdateField(buf, "time_elapsed", timeElapsedIndex, index);
    SQLiteStatement stmt(&_db);
    bool err = false;
    lString8 sql("UPDATE bookmark SET ");
    sql += buf;
    sql += " WHERE id = ?;";
    err = stmt.prepare(sql.c_str()) != DB_OK || err;

    if (!err) {
        stmt.bindInt64(index, b->id);
        if (typeIndex >= 0) stmt.bindInt(typeIndex, b->type);
        if (percentIndex >= 0) stmt.bindInt(percentIndex, b->percent);
        if (shortcutIndex >= 0) stmt.bindInt(shortcutIndex, b->shortcut);
        if (timestampIndex >= 0) stmt.bindInt64(timestampIndex, b->timestamp);
        if (startPosIndex >= 0) stmt.bindText(startPosIndex, b->startPos);
        if (endPosIndex >= 0) stmt.bindText(endPosIndex, b->endPos);
        if (titleTextIndex >= 0) stmt.bindText(titleTextIndex, b->titleText);
        if (posTextIndex >= 0) stmt.bindText(posTextIndex, b->posText);
        if (commentTextIndex >= 0) stmt.bindText(commentTextIndex, b->commentText);
        if (timeElapsedIndex >= 0) stmt.bindInt64(timeElapsedIndex, b->timeElapsed);

        if (stmt.step() == DB_DONE) {
            return true;
        }
    }
    return false;
}

bool CRBookDB::insertBookmark(BookDBBookmark * b)
{
    SQLiteStatement stmt(&_db);
    bool err = false;
    err = stmt.prepare("INSERT INTO bookmark "
                       "(book_fk, type, percent, shortcut, time_stamp, "
                       "start_pos, end_pos, title_text, pos_text, comment_text, time_elapsed) "
                       "VALUES (?,?,?,?,?,?,?,?,?,?,?);") != 0 || err;
    stmt.bindKey(1, b->bookId);
    stmt.bindInt(2, b->type);
    stmt.bindInt(3, b->percent);
    stmt.bindInt(4, b->shortcut);
    stmt.bindInt64(5, b->timestamp);
    stmt.bindText(6, b->startPos);
    stmt.bindText(7, b->endPos);
    stmt.bindText(8, b->titleText);
    stmt.bindText(9, b->posText);
    stmt.bindText(10, b->commentText);
    stmt.bindInt64(11, b->timeElapsed);
    if (stmt.step() == DB_DONE) {
        b->id = stmt.lastInsertId();
        return true;
    }
    return false;
}

/// saves last position for book
BookDBBookmark * CRBookDB::loadLastPosition(BookDBBook * book) {
    if (!book)
        return NULL;
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    CR_UNUSED(guard);
    if (!book->id) {
        BookDBBook * cached = _bookCache.get(book->pathname);
        if (cached)
            book->id = cached->id;
    }
    if (!book->id)
        return NULL;
    BookDBBookmark * res = _lastPositionCache.find(book->id);
    return res ? res->clone() : NULL;
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


//=================================================================
// PREFIX COLLECTION
class PrefixCollection {
    int _minLevel;
    int _maxSize;
    int _level;

    struct Item {
        int count;
        lInt64 bookId;
        lString16 name;
        Item() : count(0), bookId(0) {}
        Item(int cnt, lInt64 book, const lString16 & fullname) : count(cnt), bookId(book), name(fullname) {}
        Item(const Item & v) : count(v.count), bookId(v.bookId), name(v.name) { }
        Item & operator = (const Item & v) {
            count = v.count;
            bookId = v.bookId;
            if (!v.name.empty())
                name = v.name;
            return *this;
        }

        Item operator + (Item & v) {
            return Item(count + v.count,
                        count + v.count == 1 ? bookId + v.bookId : 0,
                        count + v.count == 1 ? name + v.name : lString16()
                        );
        }
    };

    LVHashTable<lString16, Item> _map;

    int itemsForLevel(int level);
    void compact();
    static lString16 truncate(const lString16 & s, int level);
public:

    void get(LVPtrVector<BookDBPrefixStats> & res);

    PrefixCollection(int minLevel, int maxSize) : _minLevel(minLevel), _maxSize(maxSize), _level(0), _map(1000) {
    }

    /// add pattern with number of books
    void add(const lString16 & value, int count, lInt64 bookId);
};

void uppercaseASCII(lString16 & str) {
    for (int i = 0; i < str.length(); i++) {
        lChar16 ch = str[i];
        if (ch >= 'a' && ch <='z')
            str[i] = ch - 'a' + 'A';
    }
}

lString16 PrefixCollection::truncate(const lString16 & s, int level) {
    if (!level)
        return s;
    if (s.length() > level && (s.length() != level + 1 || s[level] != '%')) {
        lString16 res = s.substr(0, level) + "%";
        uppercaseASCII(res);
        return res;
    }
    return s;
}

int PrefixCollection::itemsForLevel(int level) {
    LVHashTable<lString16, Item> map(1000);
    LVHashTable<lString16, Item>::iterator i = _map.forwardIterator();
    for (;;) {
        LVHashTable<lString16, Item>::pair * item = i.next();
        if (!item)
            break;
        lString16 s = truncate(item->key, level);
        map.set(s, Item());
    }
    return map.length();
}

static int compare_prefixes(const BookDBPrefixStats ** p1, const BookDBPrefixStats ** p2) {
    return (*p1)->prefix.compare((*p2)->prefix);
}

void PrefixCollection::get(LVPtrVector<BookDBPrefixStats> & res) {
    res.clear();
    LVHashTable<lString16, Item>::iterator i = _map.forwardIterator();
    for (;;) {
        LVHashTable<lString16, Item>::pair * item = i.next();
        if (!item)
            break;
        BookDBPrefixStats * p = new BookDBPrefixStats();
        p->prefix = item->value.count == 1 ? item->value.name : item->key;
        p->bookCount = item->value.count;
        p->bookId = item->value.bookId;
        res.add(p);
        CRLog::trace("returning prefix %s count=%d", LCSTR(p->prefix), p->bookCount);
    }
    res.sort(&compare_prefixes);
}

void PrefixCollection::compact() {
    int startLevel = _level ? _level - 1 : 7;
    int bestLevel = _minLevel;
    int bestLevelItems = 0;
    for (int i = startLevel; i >= _minLevel; i--) {
        int cnt = itemsForLevel(i);
        if (cnt < _maxSize) {
            bestLevel = i;
            bestLevelItems = cnt;
            if (cnt < _maxSize) {
                break;
            }
        }
    }
    if (bestLevel != _level && bestLevelItems < _map.length()) {
        CRLog::trace("Before compact %d->%d: %d items in map, expecting %d items after compact", _level, bestLevel, _map.length(), bestLevelItems);
        /// create tmp reduced map
        LVHashTable<lString16, Item> tmp(1000);
        {
            LVHashTable<lString16, Item>::iterator i = _map.forwardIterator();
            for (;;) {
                LVHashTable<lString16, Item>::pair * item = i.next();
                if (!item)
                    break;
                lString16 s = truncate(item->key, bestLevel);
                tmp.set(s, item->value + tmp.get(s));
            }
        }
        /// copy tmp to current
        _map.clear();
        _level = bestLevel;
        {
            LVHashTable<lString16, Item>::iterator i = tmp.forwardIterator();
            for (;;) {
                LVHashTable<lString16, Item>::pair * item = i.next();
                if (!item)
                    break;
                _map.set(item->key, item->value);
            }
        }
        CRLog::trace("After compact: %d items in map, expected %d items after compact", _map.length(), bestLevelItems);
    }
}

void PrefixCollection::add(const lString16 & value, int count, lInt64 bookId) {
    lString16 s = truncate(value, _level);
    _map.set(s, _map.get(s) + Item(count, bookId, value));
    if (_map.length() > _maxSize && (_level == 0 || _level > _minLevel))
        compact();
}

#define MAX_FIND_LEVEL_SIZE 20


bool CRBookDB::findPrefixes(SEARCH_FIELD field, lString16 searchString, lString8 folderFilter, LVPtrVector<BookDBPrefixStats> & prefixes)
{
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    CR_UNUSED(guard);
    bool err = false;
    int searchStringLen = searchString.endsWith("%") ? searchString.length() - 1 : searchString.length();
    if (searchStringLen > 0 && !searchString.endsWith("%"))
        searchString += "%";
    int minLevel = searchStringLen + 1;

    DBString pattern(UnicodeToUtf8(searchString).c_str());
    DBString folder(folderFilter.c_str());

    PrefixCollection pref(minLevel, MAX_FIND_LEVEL_SIZE);
    SQLiteStatement stmt(&_db);
    lString8 sql;
    if (field == SEARCH_FIELD_AUTHOR) {
        sql += "SELECT a.file_as, count(*) cnt, min(b.id) FROM author a"
                " JOIN book_author ba ON a.id=ba.author_fk"
                " JOIN book b ON b.id=ba.book_fk"
                " JOIN folder f ON f.id=b.folder_fk"
                " WHERE a.file_as IS NOT NULL AND a.file_as != ''";
        if (pattern.length())
            sql += " AND a.file_as LIKE ?";
        if (folder.length())
            sql += " AND f.name LIKE ?";
        sql += " GROUP BY a.file_as;";
    } else if (field == SEARCH_FIELD_TITLE) {
        sql +=
                "SELECT b.title, count(*) cnt, min(b.id) FROM book b"
                " JOIN folder f ON f.id=b.folder_fk"
                " WHERE b.title IS NOT NULL AND b.title != ''";
        if (pattern.length())
            sql += " AND b.title LIKE ?";
        if (folder.length())
            sql += " AND f.name LIKE ?";
        sql += " GROUP BY b.title;";
    } else if (field == SEARCH_FIELD_SERIES) {
        sql +=
                "SELECT s.name, count(*) cnt, min(b.id) FROM series s"
                " JOIN book b ON b.series_fk = s.id"
                " JOIN folder f ON f.id=b.folder_fk"
                " WHERE s.name IS NOT NULL AND s.name != ''";
        if (pattern.length())
            sql += " AND s.name LIKE ?";
        if (folder.length())
            sql += " AND f.name LIKE ?";
        sql += " GROUP BY s.name;";
    } else if (field == SEARCH_FIELD_FILENAME) {
        sql +=
                "SELECT b.filename, count(*) cnt, min(b.id) FROM book b"
                " JOIN folder f ON f.id=b.folder_fk"
                " WHERE b.title IS NOT NULL AND b.title != ''";
        if (pattern.length())
            sql += " AND b.filename LIKE ?";
        if (folder.length())
            sql += " AND f.filename LIKE ?";
        sql += " GROUP BY b.filename;";
    } else {
        return false;
    }
    err = stmt.prepare(sql.c_str()) != DB_OK || err;
    if (!err) {
        int param = 1;
        if (pattern.length())
            stmt.bindText(param++, pattern.c_str());
        if (folder.length())
            stmt.bindText(param++, folder.c_str());
        while(stmt.step() == DB_ROW) {
            lString16 value = Utf8ToUnicode(stmt.getText(0));
            int count = stmt.getInt(1);
            lInt64 bookId = stmt.getInt64(2);
            pref.add(value, count, bookId);
        }
        pref.get(prefixes);
    }
    CRLog::trace("found %d prefix items for %s", prefixes.length(), LCSTR(searchString));
    return !err;
}

/// searches BookDB by field - return files
bool CRBookDB::findBooks(SEARCH_FIELD field, lString16 searchString, lString8 folderFilter, LVPtrVector<BookDBBook> & loaded)
{

    bool err = false;
    LVArray<lInt64> ids;

    {
        CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
        CR_UNUSED(guard);

        DBString pattern(UnicodeToUtf8(searchString).c_str());
        DBString folder(folderFilter.c_str());

        SQLiteStatement stmt(&_db);
        lString8 sql;
        if (field == SEARCH_FIELD_AUTHOR) {
            sql += "SELECT b.id FROM author a"
                    " JOIN book_author ba ON a.id=ba.author_fk"
                    " JOIN book b ON b.id=ba.book_fk"
                    " JOIN folder f ON f.id=b.folder_fk"
                    " WHERE a.file_as IS NOT NULL AND a.file_as != ''";
            if (pattern.length())
                sql += " AND a.file_as LIKE ?";
            if (folder.length())
                sql += " AND f.name LIKE ?";
            sql += ";";
        } else if (field == SEARCH_FIELD_TITLE) {
            sql +=
                    "SELECT b.id FROM book b"
                    " JOIN folder f ON f.id=b.folder_fk"
                    " WHERE b.title IS NOT NULL AND b.title != ''";
            if (pattern.length())
                sql += " AND b.title LIKE ?";
            if (folder.length())
                sql += " AND f.name LIKE ?";
            sql += ";";
        } else if (field == SEARCH_FIELD_SERIES) {
            sql +=
                    "SELECT b.id FROM series s"
                    " JOIN book b ON b.series_fk = s.id"
                    " JOIN folder f ON f.id=b.folder_fk"
                    " WHERE s.name IS NOT NULL AND s.name != ''";
            if (pattern.length())
                sql += " AND s.name LIKE ?";
            if (folder.length())
                sql += " AND f.name LIKE ?";
            sql += ";";
        } else if (field == SEARCH_FIELD_FILENAME) {
            sql +=
                    "SELECT b.id FROM book b"
                    " JOIN folder f ON f.id=b.folder_fk"
                    " WHERE b.title IS NOT NULL AND b.title != ''";
            if (pattern.length())
                sql += " AND b.filename LIKE ?";
            if (folder.length())
                sql += " AND f.filename LIKE ?";
            sql += ";";
        } else {
            return false;
        }
        err = stmt.prepare(sql.c_str()) != DB_OK || err;
        if (!err) {
            int param = 1;
            if (pattern.length())
                stmt.bindText(param++, pattern.c_str());
            if (folder.length())
                stmt.bindText(param++, folder.c_str());
            while(stmt.step() == DB_ROW) {
                lInt64 bookId = stmt.getInt64(0);
                ids.add(bookId);
            }
        }
    }
    if (ids.length())
        err = !loadBooks(ids, loaded) || err;
    CRLog::trace("found %d books for %s", loaded.length(), LCSTR(searchString));
    return !err;
}

/// returns ptr to copy saved in cache
BookDBBookmark * CRBookLastPositionCache::find(lInt64 bookId) {
    int index = -1;
    if (_indexByBookId.get(bookId, index)) {
        return _bookmarks[index];
    }
    return NULL;
}
/// item will be stored as is, owned by _bookmarks
void CRBookLastPositionCache::put (lInt64 bookId, BookDBBookmark * item) {
    int index = -1;
    if (_indexByBookId.get(bookId, index)) {
        /// replace existing
        if (item != _bookmarks[index]) {
            delete _bookmarks[index];
            _bookmarks[index] = item;
        }
    } else {
        /// add new
        index = _bookmarks.length();
        _bookmarks.add(item);
        _indexByBookId.set(bookId, index);
    }
}

void CRBookLastPositionCache::remove(lInt64 bookId) {
    int index = -1;
    if (_indexByBookId.get(bookId, index)) {
        /// replace existing
        BookDBBookmark * item = _bookmarks.remove(index);
        _indexByBookId.remove(bookId);
        delete item;
    }
}

void CRBookLastPositionCache::clear() {
    _bookmarks.clear();
    _indexByBookId.clear();
}
static int last_position_comparator(const BookDBBookmark ** b1, const BookDBBookmark ** b2) {
    if ((*b1)->timestamp > (*b2)->timestamp)
        return -1;
    if ((*b1)->timestamp < (*b2)->timestamp)
        return 1;
    return 0;
}

void CRBookLastPositionCache::sort() {
    _bookmarks.sort(&last_position_comparator);
    /// fix map
    _indexByBookId.clear();
    for (int i = 0; i < _bookmarks.length(); i++) {
        _indexByBookId.set(_bookmarks[i]->bookId, i);
    }
}

/// protected by mutex
bool CRBookDB::loadRecentBooks(LVPtrVector<BookDBBook> & books, LVPtrVector<BookDBBookmark> & lastPositions) {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    CR_UNUSED(guard);
    books.clear();
    lastPositions.clear();
    if (!_lastPositionCache.length()) {
        int lastPosCount = loadLastPositionsToCache();
        CRLog::info("Loaded %d last position items", lastPosCount);
    }
    LVHashTable<lUInt64,int> indexByBook(_lastPositionCache.length() * 4);
    lString8 bookIdsToLoad;
    bookIdsToLoad.reserve(_lastPositionCache.length() * 10);
    books.reserve(_lastPositionCache.length());
    lastPositions.reserve(_lastPositionCache.length());
    for (int i = 0; i < _lastPositionCache.length(); i++) {
        books.add(NULL);
        lastPositions.add(_lastPositionCache[i]->clone());
        indexByBook.set(_lastPositionCache[i]->bookId, i);
    }
    for (int i = 0; i < _lastPositionCache.length(); i++) {
        lInt64 bookId = _lastPositionCache[i]->bookId;
        BookDBBook * cached = _bookCache.get(bookId);
        if (cached) {
            books[i] = cached->clone();
        } else {
            if (!bookIdsToLoad.empty())
                bookIdsToLoad += ",";
            char s[30];
            sprintf(s, "%lld", bookId);
            bookIdsToLoad += s;
        }
    }
    if (!bookIdsToLoad.empty()) {
        lString8 sql = lString8("SELECT " BOOK_TABLE_ALL_FIELDS " FROM book WHERE id IN (") + bookIdsToLoad + ");";
        SQLiteStatement stmt(&_db);
        bool err = false;
        err = stmt.prepare(sql.c_str()) != DB_OK || err;
        if (!err) {
            for (;;) {
                BookDBBook * book = loadBookToCache(stmt);
                if (!book)
                    break;
                books[indexByBook.get(book->id)] = book->clone();
            }
        }
    }
    return true;
}

/// protected by mutex
bool CRBookDB::loadBooks(LVArray<lInt64> & keys, LVPtrVector<BookDBBook> & loaded) {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    CR_UNUSED(guard);
    LVArray<lInt64> forLoad;
    for (int i = 0; i<keys.length(); i++) {
        BookDBBook * cached = _bookCache.get(keys[i]);
        if (cached)
            loaded.add(cached->clone());
        else
            forLoad.add(keys[i]);
    }
    for (int i = 0; i<forLoad.length(); i++) {
        // not found in cache
        BookDBBook * cached = loadBookToCache(forLoad[i]);
        if (cached)
            loaded.add(cached->clone());
    }
    return true;
}

bool CRBookDB::loadBooks(lString8Collection & pathnames, LVPtrVector<BookDBBook> & loaded, lString8Collection & notFound) {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    CR_UNUSED(guard);
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

static bool matchPrefix(const lString16 & s, const lString16 & prefix) {
    if (prefix.empty() || prefix == L"%")
        return true;
    if (prefix.endsWith("%")) {
        return s.startsWith(prefix.substr(prefix.length() - 1));
    }
    return s == prefix;
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

