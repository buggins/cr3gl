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


bool SQLiteStatement::checkColumnIndexError(int index) {
	if (!isOpened()) {
		CRLog::error("SQLite - trying to read from closed statement");
		return true;
	}
	if (!_firstStepExecuted) {
		CRLog::error("SQLite - trying to read from non-executed statement");
		return true;
	}
	if (index < 0 || index >= _columnCount) {
		CRLog::error("SQLite - column index %d is out of bounds");
		return true;
	}
	return false;
}

bool SQLiteStatement::checkParameterIndexError(int index) {

}

/// return number of bytes in contents of column with specified index
int SQLiteStatement::getColumnBytes(int index)
{
	if (checkColumnIndexError(index)) return 0;
	return sqlite3_column_bytes(_stmt, index);
}

/// return column type
int SQLiteStatement::getColumnType(int index)
{
	if (checkColumnIndexError(index)) return 0;
	return sqlite3_column_type(_stmt, index);
}

/// return column blob data pointer
const unsigned char * SQLiteStatement::getBlob(int index)
{
	if (checkColumnIndexError(index)) return NULL;
	return (const unsigned char *)sqlite3_column_blob(_stmt, index);
}

/// return column as double
double SQLiteStatement::getDouble(int index)
{
	if (checkColumnIndexError(index)) return 0;
	return sqlite3_column_double(_stmt, index);
}

/// return column value as int
int SQLiteStatement::getInt(int index)
{
	if (checkColumnIndexError(index)) return 0;
	return sqlite3_column_int(_stmt, index);
}

/// return column value as 64 bit int
lInt64 SQLiteStatement::getInt64(int index)
{
	if (checkColumnIndexError(index)) return 0;
	return sqlite3_column_int64(_stmt, index);
}
/// return column value text pointer
const char * SQLiteStatement::getText(int index)
{
	if (checkColumnIndexError(index)) return NULL;
	return (const char *)sqlite3_column_blob(_stmt, index);
}



/// set NULL to parameter
int SQLiteStatement::bindNull(int index) {
	if (checkParameterIndexError(index)) return -1;
	return sqlite3_bind_null(_stmt, index);
}

/// set int to parameter
int SQLiteStatement::bindInt(int index, int value) {
	if (checkParameterIndexError(index)) return -1;
	return sqlite3_bind_int(_stmt, index, value);
}

/// set 64-bit int to parameter
int SQLiteStatement::bindInt64(int index, lInt64 value) {
	if (checkParameterIndexError(index)) return -1;
	return sqlite3_bind_int64(_stmt, index, value);
}

/// set utf-8 text string to parameter
int SQLiteStatement::bindText(int index, const char * str, int len) {
	if (checkParameterIndexError(index)) return -1;
	if (!str)
		return bindNull(index);
	char * copy = (char *) malloc(len + 1);
	memcpy(copy, str, len);
	copy[len] = 0;
	return sqlite3_bind_text(_stmt, index, copy, len, free);
}

/// set blob value to parameter
int SQLiteStatement::bindBlob(int index, const void * data, int len) {
	if (checkParameterIndexError(index)) return -1;
	if (!data)
		return bindNull(index);
	char * copy = (char *) malloc(len);
	memcpy(copy, data, len);
	return sqlite3_bind_blob(_stmt, index, copy, len, free);
}
