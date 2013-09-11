/*
 * basedb.cpp
 *
 *  Created on: Aug 16, 2013
 *      Author: vlopatin
 */

#include "basedb.h"
#include <sqlite3.h>
#include <lvstring.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#if USE_LSTRING8_FOR_DB != 1
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
	const char * str = s.c_str();
	for (int i = 0; i < len; i++)
		value = value * 31 + str[i];
	return value;
}
#endif

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

/// returns autoincrement key value generated by last insert
lInt64 SQLiteDB::lastInsertId() {
	if (!isOpened())
		return 0;
	return sqlite3_last_insert_rowid(_db);
}

/// returns true if table exists
bool SQLiteDB::tableExists(const char * tableName) {
	if (!isOpened())
		return false;
	char sql[512];
	sprintf(sql, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='%s';", tableName);
	SQLiteStatement stmt(this);
	if (stmt.prepare(sql))
		return false;
	if (stmt.step() != DB_ROW)
		return false;
	return stmt.getInt(0) > 0;
}

static bool columnNamePresentInCreateTable(const char * createTable, const char * columnName) {
	if (!createTable || !columnName)
		return false;
	int len = strlen(columnName);
	const char * pos = strstr(createTable, columnName);
	if (!pos)
		return false;
	char charBefore = pos[-1];
	char charAfter = pos[len];
	return !isalnum(charBefore) && !isalnum(charAfter);
}

/// returns true if column exists in table
bool SQLiteDB::columnExists(const char * tableName, const char * columnName) {
	if (!isOpened())
		return false;
	char sql[512];
	sprintf(sql, "SELECT sql FROM sqlite_master WHERE type='table' AND name='%s';", tableName);
	SQLiteStatement stmt(this);
	if (stmt.prepare(sql))
		return false;
	if (stmt.step() != DB_ROW)
		return false;
	const char * createTable = stmt.getText(0);
	//CRLog::trace("%s", createTable);
	return columnNamePresentInCreateTable(createTable, columnName);
}

/// checks if column present, adds if no such column; returns true if success, false if any error
bool SQLiteDB::addColumnIfNotExists(const char * table, const char * columnName, const char * alterTableSql) {
	if (columnExists(table, columnName))
		return true;
	int res = executeUpdate(alterTableSql);
	return res >= 0;
}

/// gets database schema version
int SQLiteDB::getVersion() {
	if (!isOpened())
		return -1;
	SQLiteStatement stmt(this);
	if (stmt.prepare("PRAGMA user_version;"))
		return -1;
	if (stmt.step() == DB_ROW)
		return stmt.getInt(0);
	return -1;
}

/// sets database schema version
void SQLiteDB::setVersion(int version) {
	if (!isOpened())
		return;
	SQLiteStatement stmt(this);
	char sql[64];
	sprintf(sql, "PRAGMA user_version = %d;", version);
	if (stmt.prepare(sql))
		return;
	stmt.step();
}


/// runs update, returns number of affected rows; -1 if error
int SQLiteDB::executeUpdate(const char * sql) {
	if (!isOpened())
		return -1;
	SQLiteStatement stmt(this);
	if (stmt.prepare(sql))
		return -1;
	int res = stmt.step();
	if (res != DB_DONE)
		return -1;
	return stmt.rowsAffected();
}

//=================================================================================================
// statement


SQLiteStatement::~SQLiteStatement() {
	close();
}

/// prepares query; returns 0 if no error
int SQLiteStatement::prepare(const char * sql) {
	close();
	if (!_db || !sql || !sql[0])
		return -1;
	if (sql[strlen(sql) - 1] != ';') {
		CRLog::error("No semicolon at end of statement %s", sql);
	}
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
		_parameterCount = sqlite3_bind_parameter_count(_stmt);
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
	_rowsAffected = 0;
	_parameterCount = 0;
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
		_rowsAffected = 0;
		sqlite3_clear_bindings(_stmt);
	}
	return res;
}

/// executes query and returns one row of result or returns next row of result
int SQLiteStatement::step() {
	if (!isOpened())
		return -1;
	int res = sqlite3_step(_stmt);
	if (res != DB_OK && res != DB_ROW && res != DB_DONE) {
		CRLog::error("SQLite Error %d while executing step() on statement %s", res, _sql);
	} else {
		if (!_firstStepExecuted) {
			_columnCount = sqlite3_column_count(_stmt);
			_firstStepExecuted = true;
			_rowsAffected = sqlite3_changes(_db->getHandle());
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
		CRLog::error("SQLite - column index %d is out of bounds 0..%d", _columnCount - 1);
		return true;
	}
	return false;
}

bool SQLiteStatement::checkParameterIndexError(int index) {
	if (!isOpened()) {
		CRLog::error("SQLite - trying to access closed statement");
		return true;
	}
	if (index < 1 || index > _parameterCount) {
		CRLog::error("SQLite - parameter index %d is out of bounds 1..%d", _parameterCount);
		return true;
	}
	return false;
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
	//CRLog::trace("bindText called for (%d, %s)", index, str);
	if (!str)
		return bindNull(index);
	char * copy = (char *) malloc(len + 1);
	memcpy(copy, str, len);
	copy[len] = 0;
	//CRLog::trace("bindText(%d, %s)", index, copy);
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



