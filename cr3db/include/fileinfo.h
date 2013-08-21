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
	lString8 getFileName();
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
	lUInt64 _hash;
public:
	int itemCount() { return _entries.length(); }
	CRDirEntry * getItem(int index) { return _entries[index]; }
	CRDirCacheItem(CRDirEntry * item) :  CRDirItem(item->getPathName(), item->isArchive()), _scanned(false), _hash(0) {}
	CRDirCacheItem(const lString8 & pathname, bool isArchive) : CRDirItem(pathname, isArchive), _scanned(false), _hash(0) {}
	virtual void setParsed(bool parsed) { _scanned = parsed; }
	virtual bool isParsed() { return _scanned; }
	bool refresh();
	bool scan();
	bool needScan();
};

class CRDirCache {
	struct Item {
		CRDirCacheItem * dir;
		Item * next;
		Item * prev;
		Item(CRDirCacheItem * _dir) : dir(_dir), next(NULL), prev(NULL) { }
	};
	Item * _head;
	LVHashTable<lString8, Item*> _byName;
	void addItem(CRDirCacheItem * dir);
	Item * findItem(const lString8 & pathname);
	void moveToHead(Item * item);
public:
	CRDirCache() : _head(NULL), _byName(1000) {}
	~CRDirCache() { clear(); }
	CRDirCacheItem * find(lString8 pathname);
	CRDirCacheItem * find(CRDirItem * dir) { return find(dir->getPathName()); }
	CRDirCacheItem * getOrAdd(CRDirItem * dir);
	CRDirCacheItem * getOrAdd(const lString8 & pathname);
	void clear();
};

extern CRDirCache * dirCache;

#endif /* FILEINFO_H_ */
