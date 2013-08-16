/*
 * cr3db.h
 *
 *  Created on: Aug 16, 2013
 *      Author: vlopatin
 */

#ifndef CR3DB_H_
#define CR3DB_H_

#include "basedb.h"

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
	~DBString();
};

class CRBookDB {
	SQLiteDB _db;
public:
	/// open database file; returns 0 on success, error code otherwise
	int open(const char * pathname, bool readOnly) { return _db.open(pathname, readOnly); }
	/// closes DB
	int close() { return _db.close(); }
	/// returns true if DB is opened
	bool isOpened() { return _db.isOpened(); }
	/// creates/upgrades DB schema
	bool updateSchema();
};


#endif /* CR3DB_H_ */
