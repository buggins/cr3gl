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
	CRDirEntry(const lString8 & pathname, bool archive) : _pathName(pathname), _isArchive(archive) {}
	virtual ~CRDirEntry() {}
	const lString8 & getPathName() { return _pathName; }
	virtual bool isDirectory() = 0;
	virtual bool isArchive() { return _isArchive; }
	virtual BookDBBook * getBook() { return NULL; }
	virtual void setBook(BookDBBook * book) { }
	virtual bool isParsed() { return false; }
	virtual void setParsed(bool parsed) {}
};

class CRFileItem : public CRDirEntry {
private:
	BookDBBook * _book;
	bool _parsed;
public:
	virtual void setParsed(bool parsed) { _parsed = parsed; }
	virtual bool isParsed() { return _parsed; }
	virtual BookDBBook * getBook() { return _book; }
	virtual void setBook(BookDBBook * book) { if (_book) delete _book; _book = book; }
	CRFileItem(const lString8 & pathname, bool archive) : CRDirEntry(pathname, archive), _book(NULL), _parsed(false) { }
	~CRFileItem() { if (_book) delete _book; }
	virtual bool isDirectory() { return false; }
};

class CRDirItem : public CRDirEntry {
public:
	CRDirItem(const lString8 & pathname, bool isArchive) : CRDirEntry(pathname, isArchive) {}
	virtual bool isDirectory() { return true; }
};

class CRDirCacheItem : public CRDirItem {
	LVPtrVector<CRDirEntry> _entries;
	bool _scanned;
public:
	CRDirCacheItem(CRDirEntry * item) :  CRDirItem(item->getPathName(), item->isArchive()) {}
	CRDirCacheItem(const lString8 & pathname, bool isArchive) : CRDirItem(pathname, isArchive) {}
	virtual void setParsed(bool parsed) { _scanned = parsed; }
	virtual bool isParsed() { return _scanned; }
	bool scan();
};


#endif /* FILEINFO_H_ */
