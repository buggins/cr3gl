/*
 * basedb.h
 *
 *  Created on: Aug 16, 2013
 *      Author: vlopatin
 */

#ifndef BASEDB_H_
#define BASEDB_H_

#include <stdlib.h>
#include <lvtypes.h>

struct sqlite3;
struct sqlite3_stmt;

enum {
	DB_COL_TYPE_INTEGER = 1,
	DB_COL_TYPE_FLOAT = 2,
	DB_COL_TYPE_TEXT = 3,
	DB_COL_TYPE_BLOB = 4,
	DB_COL_TYPE_NULL = 5,
};

class SQLiteDB {
protected:
	sqlite3 * _db;
	const char * _filename;
public:
	SQLiteDB() : _db(NULL), _filename(NULL) {}
	virtual ~SQLiteDB();
	sqlite3 * getHandle() { return _db; }
	const char * getFileName() { return _filename; }
	/// open database file; returns 0 on success, error code otherwise
	int open(const char * pathname, bool readOnly);
	/// closes DB
	int close();
	/// returns true if DB is opened
	bool isOpened();
};

class SQLiteStatement {
protected:
	SQLiteDB * _db;
	sqlite3_stmt * _stmt;
	const char * _sql;
	bool _firstStepExecuted;
	int _columnCount;
public:
	SQLiteStatement(SQLiteDB * db) : _db(db), _stmt(NULL), _sql(NULL), _firstStepExecuted(false), _columnCount(0) { }
	const char * getQuery() { return _sql; }
	virtual ~SQLiteStatement();
	/// prepares query; returns 0 if no error
	int prepare(const char * sql);
	/// executes query and returns one row of result or returns next row of result
	int step();
	/// prepare statement for reexecution
	int reset(bool clearBindings);
	/// returns true if query is prepared
	bool isOpened();
	/// closes prepared query and frees all resources
	int close();

	/// column reading
	/// returns column count
	int getColumnCount() { return _columnCount; }
	/// return number of bytes in contents of column with specified index
	int getColumnBytes(int index);
	/// return column type
	int getColumnType(int index);
	/// return column blob data pointer
	const unsigned char * getBlob(int index);
	/// return column as double
	double getDouble(int index);
	/// return column value as int
	int getInt(int index);
	/// return column value as 64 bit int
	lInt64 getInt64(int index);
	/// return column value text pointer
	const char * getText(int index);

};

#endif /* BASEDB_H_ */
