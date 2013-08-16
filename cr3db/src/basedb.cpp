/*
 * basedb.cpp
 *
 *  Created on: Aug 16, 2013
 *      Author: vlopatin
 */

#include "basedb.h"
#include <sqlite3.h>
#include <lvstring.h>

SQLiteDB::~SQLiteDB() {
	close();
}

int SQLiteDB::open(const char * pathname, bool readOnly)
{
	close();
	int res = sqlite3_open_v2(pathname, &_db, readOnly ? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if (res)
		_db = NULL;
	if (res)
		CRLog::error("SQLite Error %d while trying to open database %s", res, pathname);
	return res;
}

/// closes DB
int SQLiteDB::close()
{
	if (!_db)
		return -1;
	int res = sqlite3_close_v2(_db);
	_db = NULL;
	if (res)
		CRLog::error("SQLite Error %d while trying to close database %s", res, _filename ? _filename : "UNKNOWN");
	if (_filename)
		free((void*)_filename);
	_filename = NULL;
	return res;
}
/// returns true if DB is opened
bool SQLiteDB::isOpened() {
	return _db != NULL;
}


//=================================================================================================
// statement


SQLiteStatement::~SQLiteStatement() {
	close();
}

/// prepares query; returns 0 if no error
int SQLiteStatement::prepare(const char * sql) {
	close();
	if (!_db || !sql)
		return -1;
	int res = sqlite3_prepare_v2(
	  _db->getHandle(),            /* Database handle */
	  sql,       /* SQL statement, UTF-8 encoded */
	  strlen(sql),              /* Maximum length of zSql in bytes. */
	  &_stmt,  /* OUT: Statement handle */
	  NULL     /* OUT: Pointer to unused portion of zSql */
	);
	if (res) {
		CRLog::error("SQLite Error %d while preparing query %s", res, sql);
		_stmt = NULL;
	} else {
		_sql = strdup(sql);
	}
	return res;
}

/// returns true if query is prepared
bool SQLiteStatement::isOpened()
{
	return _stmt != NULL;
}

/// closes prepared query and frees all resources
int SQLiteStatement::close()
{
	if (!isOpened())
		return 0;
	int res = sqlite3_finalize(_stmt);
	_stmt = NULL;
	if (res) {
		CRLog::error("SQLite Error %d while unpreparing statement %s", res, _sql);
	}
	if (_sql) {
		free((void*)_sql);
		_sql = NULL;
	}
	_firstStepExecuted = false;
	_columnCount = 0;
	return res;
}

/// prepare statement for reexecution
int SQLiteStatement::reset(bool clearBindings) {
	int res = sqlite3_reset(_stmt);
	if (res) {
		CRLog::error("SQLite Error %d while trying to reset statement %s", res, _sql);
	} else {
		_firstStepExecuted = false;
		_columnCount = 0;
		sqlite3_clear_bindings(_stmt);
	}
	return res;
}

/// executes query and returns one row of result or returns next row of result
int SQLiteStatement::step() {
	if (!isOpened())
		return -1;
	int res = sqlite3_step(_stmt);
	if (res) {
		CRLog::error("SQLite Error %d while executing step() on statement %s", res, _sql);
	} else {
		if (!_firstStepExecuted) {
			_columnCount = sqlite3_column_count(_stmt);
			_firstStepExecuted = true;
		}
	}
	return res;
}

/// return number of bytes in contents of column with specified index
int SQLiteStatement::getColumnBytes(int index)
{
	if (!isOpened() || !_firstStepExecuted || index < 0 || index >= _columnCount) return 0;
	return sqlite3_column_bytes(_stmt, index);
}

/// return column type
int SQLiteStatement::getColumnType(int index)
{
	if (!isOpened() || !_firstStepExecuted || index < 0 || index >= _columnCount) return 0;
	return sqlite3_column_type(_stmt, index);
}

/// return column blob data pointer
const unsigned char * SQLiteStatement::getBlob(int index)
{
	if (!isOpened() || !_firstStepExecuted || index < 0 || index >= _columnCount) return NULL;
	return (const unsigned char *)sqlite3_column_blob(_stmt, index);
}

/// return column as double
double SQLiteStatement::getDouble(int index)
{
	if (!isOpened() || !_firstStepExecuted || index < 0 || index >= _columnCount) return 0;
	return sqlite3_column_double(_stmt, index);
}

/// return column value as int
int SQLiteStatement::getInt(int index)
{
	if (!isOpened() || !_firstStepExecuted || index < 0 || index >= _columnCount) return 0;
	return sqlite3_column_int(_stmt, index);
}

/// return column value as 64 bit int
lInt64 SQLiteStatement::getInt64(int index)
{
	if (!isOpened() || !_firstStepExecuted || index < 0 || index >= _columnCount) return 0;
	return sqlite3_column_int64(_stmt, index);
}
/// return column value text pointer
const char * SQLiteStatement::getText(int index)
{
	if (!isOpened() || !_firstStepExecuted || index < 0 || index >= _columnCount) return NULL;
	return (const char *)sqlite3_column_blob(_stmt, index);
}
