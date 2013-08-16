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
	str = s ? strdup(s) : NULL;
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



#define DB_VERSION 20

/// creates/upgrades DB schema
bool CRBookDB::updateSchema()
{
	int currentVersion = _db.getVersion();
	bool err = false;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS author ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"name VARCHAR NOT NULL COLLATE NOCASE"
			")") >= 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
            "author_name_index ON author (name) ") >= 0 || err;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS series ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"name VARCHAR NOT NULL COLLATE NOCASE"
			")") >= 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
	        "series_name_index ON series (name) ") >= 0 || err;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS folder ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"name VARCHAR NOT NULL"
			")") >= 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"folder_name_index ON folder (name) ") >= 0 || err;
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
			")");
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_folder_index ON book (folder_fk) ") >= 0 || err;
	err = _db.executeUpdate("CREATE UNIQUE INDEX IF NOT EXISTS "
			"book_pathname_index ON book (pathname) ") >= 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_filename_index ON book (filename) ") >= 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_title_index ON book (title) ") >= 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_last_access_time_index ON book (last_access_time) ") >= 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"book_title_index ON book (title) ") >= 0 || err;
	err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS book_author ("
			"book_fk INTEGER NOT NULL REFERENCES book (id),"
			"author_fk INTEGER NOT NULL REFERENCES author (id),"
			"PRIMARY KEY (book_fk, author_fk)"
			")") >= 0 || err;
	err = _db.executeUpdate("CREATE UNIQUE INDEX IF NOT EXISTS "
			"author_book_index ON book_author (author_fk, book_fk) ");
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
			")") >= 0 || err;
	err = _db.executeUpdate("CREATE INDEX IF NOT EXISTS "
			"bookmark_book_index ON bookmark (book_fk) ") >= 0 || err;

	// ====================================================================
	if ( currentVersion<1 )
		err = !_db.addColumnIfNotExists("bookmark", "shortcut", "ALTER TABLE bookmark ADD COLUMN shortcut INTEGER DEFAULT 0") || err;
	if ( currentVersion<4 )
		err = !_db.addColumnIfNotExists("book", "flags", "ALTER TABLE book ADD COLUMN flags INTEGER DEFAULT 0") || err;
	if ( currentVersion<6 )
		err = _db.executeUpdate("CREATE TABLE IF NOT EXISTS opds_catalog ("
				"id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"name VARCHAR NOT NULL COLLATE NOCASE, "
				"url VARCHAR NOT NULL COLLATE NOCASE, "
				"last_usage INTEGER DEFAULT 0"
				")") >= 0 || err;
//	if (currentVersion < 7) {
//		addOPDSCatalogs(DEF_OPDS_URLS1);
//	}
//	if (currentVersion < 8)
//		addOPDSCatalogs(DEF_OPDS_URLS2);
	if (currentVersion < 13)
		err = !_db.addColumnIfNotExists("book", "language", "ALTER TABLE book ADD COLUMN language VARCHAR DEFAULT NULL") || err;
	if (currentVersion < 15)
		err = !_db.addColumnIfNotExists("opds_catalog", "last_usage", "ALTER TABLE opds_catalog ADD COLUMN last_usage INTEGER DEFAULT 0") || err;
	if (currentVersion < 16)
		err = !_db.addColumnIfNotExists("bookmark", "time_elapsed", "ALTER TABLE bookmark ADD COLUMN time_elapsed INTEGER DEFAULT 0") || err;

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
