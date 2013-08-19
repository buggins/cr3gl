/*
 * fileinfo.h
 *
 *  Created on: Aug 19, 2013
 *      Author: vlopatin
 */

#ifndef FILEINFO_H_
#define FILEINFO_H_

#include <cr3db.h>

class CRDirEntry {
	lString8 _pathName;
	bool _isArchive;
public:
	const lString8 & getPathName() { return _pathName; }
	CRDirEntry(lString8 & pathname, bool archive) : _pathName(pathname), _isArchive(archive) {}
	virtual bool isDirectory() = 0;
	virtual bool isArchive() { return _isArchive; }
	virtual BookDBBook * getBook() { return NULL; }
	virtual void setBook(BookDBBook * book) { }
};

class CRFileItem : public CRDirEntry {
private:
	BookDBBook * _book;
public:
	virtual BookDBBook * getBook() { return _book; }
	virtual void setBook(BookDBBook * book) { if (_book) delete _book; _book = book; }
	CRFileItem(lString8 & pathname, bool archive) : CRDirEntry(pathname, archive), _book(NULL) { }
	~CRFileItem() { if (_book) delete _book; }
	virtual bool isDirectory() { return false; }
};

class CRDirItem : public CRDirEntry {
public:
	CRDirItem(lString8 & pathname, bool isArchive) : CRDirEntry(pathname, isArchive) {}
	virtual bool isDirectory() { return true; }
};

bool LVListDirectory(lString8 & path, LVPtrVector<CRDirEntry> & entries);

#endif /* FILEINFO_H_ */
