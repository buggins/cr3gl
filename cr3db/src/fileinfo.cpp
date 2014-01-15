/*
 * fileinfo.cpp
 *
 *  Created on: Aug 19, 2013
 *      Author: vlopatin
 */

// uncomment for simulation of long directory scan operation
//#define SLOW_SCAN_SIMULATION_FOR_TEST

#include <fb2def.h>
#define XS_IMPLEMENT_SCHEME 1
#include <fb2def.h>

#include "fileinfo.h"
#include <lvstream.h>

#include <epubfmt.h>
#include <pdbfmt.h>

#include <stringresource.h>
#include <cri18n.h>

#include <sys/stat.h>


using namespace CRUI;

static int find(const LVPtrVector<CRDirEntry> & entries, const lString8 & pathname) {
	for (int i = 0; i<entries.length(); i++) {
		if (!entries[i]->isDirectory() && entries[i]->getPathName() == pathname)
			return i;
	}
	return -1;
}

static bool splitArcName(const lString8 & pathname, lString8 & arcname, lString8 & fname) {
	int p = pathname.pos("@/");
	if (p > 0) {
		arcname = pathname.substr(0, p);
		fname = pathname.substr(p + 2);
		return true;
	}
	return false;
}

#define CONVERT_STR(x) (x.trim().length() ? LCSTR(x.trim()) : NULL)

static bool GetEPUBBookProperties(LVStreamRef stream, BookDBBook * pBookProps)
{
    LVContainerRef m_arc = LVOpenArchieve( stream );
    if ( m_arc.isNull() )
        return false; // not a ZIP archive

    // check root media type
    lString16 rootfilePath = EpubGetRootFilePath(m_arc);
    if ( rootfilePath.empty() )
    	return false;

    lString16 codeBase;
    codeBase=LVExtractPath(rootfilePath, false);

    LVStreamRef content_stream = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);
    if ( content_stream.isNull() )
        return false;

    ldomDocument * doc = LVParseXMLStream( content_stream );
    if ( !doc )
        return false;

    lString16 fileAs;
    lString16 author;
    ldomXPointer metadata = doc->createXPointer(lString16("package/metadata"));
    if (!metadata.isNull()) {
    	ldomNode * elem = metadata.getNode();
    	lString16 creatorId;
    	for (int i = 0; i<elem->getChildCount(); i++) {
    		ldomNode * child = elem->getChildNode(i);
    		lString16 tag = child->getNodeName();
    		lString16 id = child->getAttributeValue("id");
			lString16 text = child->getText();
    		if (tag == "creator") {
    			if (!author.empty())
    				pBookProps->authors.add(new BookDBAuthor(CONVERT_STR(author), CONVERT_STR(fileAs)));
    			author = text;
    			fileAs.clear();
    			creatorId = id;
    		} else if (tag == "meta") {
        		lString16 property = child->getAttributeValue("property");
        		lString16 refines = child->getAttributeValue("refines");
    	        lString16 name = child->getAttributeValue("name");
    	        lString16 content = child->getAttributeValue("content");
    			if (property == "file-as" && refines == lString16(L"#") + creatorId)
    				fileAs = child->getText();
    			else if (name == "calibre:series")
    	        	pBookProps->series = new BookDBSeries(CONVERT_STR(content));
    	        else if (name == "calibre:series_index")
    	        	pBookProps->seriesNumber = content.trim().atoi();
    		} else if (tag == "title") {
    			pBookProps->title = CONVERT_STR(text);
    		} else if (tag == "language") {
    			pBookProps->language = CONVERT_STR(text);
    		}
    	}
    }
	if (!author.empty())
		pBookProps->authors.add(new BookDBAuthor(CONVERT_STR(author), CONVERT_STR(fileAs)));

    delete doc;

    pBookProps->format = doc_format_epub;

    return true;
}

void extractFB2Authors( ldomDocument * doc, bool shortMiddleName, BookDBBook * props )
{
    lString16 authors;
    for ( int i=0; i<16; i++) {
        lString16 path = cs16("/FictionBook/description/title-info/author[") + fmt::decimal(i+1) + "]";
        ldomXPointer pauthor = doc->createXPointer(path);
        if ( !pauthor ) {
            //CRLog::trace( "xpath not found: %s", UnicodeToUtf8(path).c_str() );
            break;
        }
        lString16 firstName = pauthor.relative( L"/first-name" ).getText().trim();
        lString16 lastName = pauthor.relative( L"/last-name" ).getText().trim();
        lString16 middleName = pauthor.relative( L"/middle-name" ).getText().trim();
        lString16 author = firstName;
        lString16 fileAs = lastName;
        if ( !author.empty() )
            author += " ";
        if ( !middleName.empty() )
            author += shortMiddleName ? lString16(middleName, 0, 1) + "." : middleName;
        if ( !lastName.empty() && !author.empty() )
            author += " ";
        author += lastName;
        if (!fileAs.empty()) {
        	if (!firstName.empty() || !middleName.empty())
        		fileAs.append(", ");
        	if (!firstName.empty()) {
        		fileAs.append(firstName);
        		if (!middleName.empty())
        			fileAs.append(" ");
        	}
            if ( !middleName.empty() )
                fileAs += shortMiddleName ? lString16(middleName, 0, 1) + "." : middleName;
        }
        if (!author.empty()) {
        	props->authors.add(new BookDBAuthor(CONVERT_STR(author), CONVERT_STR(fileAs)));
        }
    }
}

// parse FB2 and EPUB properties
bool LVParseBookProperties(LVStreamRef stream, BookDBBook * props) {


    if ( props->format == doc_format_epub && DetectEpubFormat( stream ) ) {
        CRLog::trace("GetBookProperties() : epub format detected");
    	return GetEPUBBookProperties(stream, props);
    }

    CRLog::trace("GetBookProperties() : trying to parse as FB2");

    // read document
#if COMPACT_DOM==1
    ldomDocument doc(stream, 0);
#else
    ldomDocument doc;
#endif

    ldomDocumentWriter writer(&doc, true);
    doc.setNodeTypes( fb2_elem_table );
    doc.setAttributeTypes( fb2_attr_table );
    doc.setNameSpaceTypes( fb2_ns_table );
    LVXMLParser parser( stream, &writer );
    CRLog::trace( "checking format..." );
    if ( !parser.CheckFormat() ) {
    	// not FB2
        return false;
    }
    CRLog::trace( "parsing..." );
    if ( !parser.Parse() ) {
        return false;
    }
    CRLog::trace( "parsed" );
    extractFB2Authors(&doc, false, props);
    lString16 title = extractDocTitle( &doc );
    lString16 language = extractDocLanguage( &doc );
    int seriesNumber = 0;
    lString16 series = extractDocSeries(&doc, &seriesNumber);
    props->seriesNumber = seriesNumber;
    props->title = CONVERT_STR(title);
    if (series.length())
    	props->series = new BookDBSeries(LCSTR(series));
    props->language = CONVERT_STR(language);
    props->format = doc_format_fb2;
	return true;
}


bool LVListDirectory(const lString8 & path, bool isArchive, LVPtrVector<CRDirEntry> & entries, lUInt64 & hash);
bool LVCalcDirectoryHash(const lString8 & path, bool isArchive, lUInt64 & hash);

bool CRDirCacheItem::scan() {
	CRLog::trace("Scanning directory %s", getPathName().c_str());
	lUInt64 hash;
	bool res = LVListDirectory(getPathName(), isArchive(), _entries, hash);
	_hash = hash;
	_scanned = true;
	return res;
}

/// returns true for items like book-by-author, etc.
bool CRDirEntry::isSpecialItem() {
    if (_pathName.startsWith("@"))
        return true;
    return false;
}

/// for pathname like  @authors:ABC returns ABC
lString16 CRDirEntry::getFilterString() const {
    if (_pathName.startsWith("@")) {
        int pos = _pathName.pos(":");
        if (pos >= 0)
            return Utf8ToUnicode(_pathName.substr(pos + 1));
    }
    return lString16::empty_str;
}

lString8 CRDirEntry::getFileName() const {
	return UnicodeToUtf8(LVExtractFilename(Utf8ToUnicode(_pathName)));
}

lString16 CRDirEntry::getTitle() const {
	if (getBook())
		return Utf8ToUnicode(getBook()->title.c_str());
	return lString16();
}

lString16 CRDirEntry::getSeriesNameOnly() const {
    if (!getBook())
        return lString16();
    if (!getBook()->series.isNull()) {
        return Utf8ToUnicode(getBook()->series->name.c_str());
    }
    return lString16();
}

int CRDirEntry::getSeriesNumber() const {
    if (!getBook())
        return 0;
    return (getBook()->seriesNumber);
}

lString16 CRDirEntry::getSeriesName(bool numberFirst) const {
    if (!getBook())
        return lString16();
    lString16 text3;
    if (!getBook()->series.isNull()) {
        if (numberFirst) {
            if (getBook()->seriesNumber) {
                text3 += "#";
                text3 += lString16::itoa(getBook()->seriesNumber);
                text3 += " ";
            }
            text3 += Utf8ToUnicode(getBook()->series->name.c_str());
        } else {
            text3 += Utf8ToUnicode(getBook()->series->name.c_str());
            if (getBook()->seriesNumber) {
                if (text3.length())
                    text3 += " ";
                text3 += "#";
                text3 += lString16::itoa(getBook()->seriesNumber);
            }
        }
    }
    return text3;
}

lString16 CRDirEntry::getAuthorNames(bool fileAs) const {
    if (getBook()) {
        lString16 text1;
        for (int i = 0; i<getBook()->authors.length(); i++) {
            if (text1.length())
                text1 += L", ";
            if (fileAs)
                text1 += Utf8ToUnicode(getBook()->authors[i]->fileAs.c_str());
            else
                text1 += Utf8ToUnicode(getBook()->authors[i]->name.c_str());
        }
        return text1;
    }
    return lString16();
}

int CRDirContentItem::itemCount() const {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    return _entries.length();
}

CRDirEntry * CRDirContentItem::getItem(int index) const {
    CRGuard guard(const_cast<CRMutex*>(_mutex.get()));
    if (index >= 0 && index < _entries.length())
        return _entries[index];
    return NULL;
}

CRDirContentItem::CRDirContentItem(CRDirEntry * item) :  CRDirItem(item->getPathName(), item->isArchive()), _scanned(false), _scanning(false)
{
    _mutex = concurrencyProvider->createMutex();
}

CRDirContentItem::CRDirContentItem(const lString8 & pathname, bool isArchive) : CRDirItem(pathname, isArchive), _scanned(false), _scanning(false)
{
    _mutex = concurrencyProvider->createMutex();
}

CRDirCacheItem::CRDirCacheItem(CRDirEntry * item) :  CRDirContentItem(item->getPathName(), item->isArchive()), _hash(0)
{
}

CRDirCacheItem::CRDirCacheItem(const lString8 & pathname, bool isArchive) : CRDirContentItem(pathname, isArchive), _hash(0)
{
}


void CRDirContentItem::sort(int sortOrder)
{
    CR_UNUSED(sortOrder);
    // not implemented
}
bool CRDirContentItem::refresh() {
    CRGuard guard(_mutex);
    _scanning = true;
    bool res = true;
    if (needScan()) {
        res = scan();
    } else {
        CRLog::trace("CRDirCacheItem::refresh() - no scan is required");
    }
    _scanning = false;
    return res;
}

bool CRDirCacheItem::needScan() {
	if (!_scanned)
		return true;
	lUInt64 hash;
	bool res = LVCalcDirectoryHash(getPathName(), isArchive(), hash);
	if (res)
		return hash != _hash;
	return false;
}

static int title_comparator(const CRDirEntry ** item1, const CRDirEntry ** item2) {
	const CRDirEntry * e1 = *item1;
	const CRDirEntry * e2 = *item2;
	if (e1->isDirectory() && !e2->isDirectory())
		return -1;
	if (!e1->isDirectory() && e2->isDirectory())
		return 1;
	lString8 fn1 = e1->getFileName();
	lString8 fn2 = e2->getFileName();
	if (e1->isDirectory() && e2->isDirectory()) {
		return fn1.compare(fn2);
	}
	lString16 t1 = e1->getTitle();
	lString16 t2 = e2->getTitle();
	// move items w/o title to end of list
	if (t1.empty() && !t2.empty())
		return 1;
	if (!t1.empty() && t2.empty())
		return -1;
	int res = t1.compare(t2);
	if (res)
		return res;
	return fn1.compare(fn2);
}

void CRDirCacheItem::sort(int sortOrder) {
    CR_UNUSED(sortOrder);
    CRGuard guard(_mutex);
    _entries.sort(title_comparator);
}

static int access_time_comparator(const CRTopDirItem ** item1, const CRTopDirItem ** item2) {
    const CRTopDirItem * e1 = *item1;
    const CRTopDirItem * e2 = *item2;
    if (e1->getLastAccessTime() == e2->getLastAccessTime()) {
        if (e1->getDirType() == e2->getDirType())
            return e1->getPathName().compare(e2->getPathName());
        if (e1->getDirType() < e2->getDirType())
            return -1;
        return 1;
    }
    if (e1->getLastAccessTime() > e2->getLastAccessTime())
        return -1;
    return 1;
}

void CRTopDirList::sort(int sortOrder) {
    CR_UNUSED(sortOrder);
    _entries.sort(access_time_comparator);
}

CRTopDirItem * CRTopDirList::addItem(DIR_TYPE t, lString8 path, lUInt64 ts) {
    CRTopDirItem * item = new CRTopDirItem(t, path, ts);
    _entries.add(item);
    return item;
}

CRTopDirItem * CRTopDirList::itemByType(DIR_TYPE t) {
    for (int i = 0; i<_entries.length(); i++)
        if (_entries[i]->getDirType() == t)
            return _entries[i];
    return NULL;
}

CRTopDirItem * CRTopDirList::find(lString8 path) {
    for (int i = 0; i<_entries.length(); i++)
        if (_entries[i]->getPathName() == path)
            return _entries[i];
    return NULL;
}

void CRDirCache::addItem(CRDirContentItem * dir) {
	Item * item = new Item(dir);
	item->next = _head;
	if (_head)
		_head->prev = item;
	_head = item;
	_byName.set(dir->getPathName(), item);
}

void CRDirCache::removeItem(const lString8 & pathname) {
    Item * item = _byName.get(pathname);
    if (!item)
        return;
    if (item == _head)
        return;
    // remove from middle of list
    _byName.remove(pathname);
    if (item->prev)
        item->prev->next = item->next;
    else
        _head = item->next;
    if (item->next)
        item->next->prev = item->prev;
    delete item->dir;
    delete item;
}

CRDirCache::Item * CRDirCache::findItem(const lString8 &  pathname) {
	Item * res = _byName.get(pathname);
	return res;
}

void CRDirCache::moveToHead(CRDirCache::Item * item) {
	if (item == _head)
		return;
	// remove from middle of list
	if (item->prev)
		item->prev->next = item->next;
	if (item->next)
		item->next->prev = item->prev;
	// insert into head
	item->next = _head;
	if (_head)
		_head->prev = item;
	_head = item;
}

void CRDirCache::stop() {
    {
        CRGuard guard(_monitor);
        _stopped = true;
        while (_queue.length() > 0) {
            DirectoryScanTask * p = _queue.popFront();
            delete p;
        }
        _monitor->notifyAll();
    }
    _thread->join();
}

void CRDirCache::scan(const lString8 & pathname, CRDirScanCallback * callback) {
    //CRLog::trace("CRDirCache::scan - entering");
    if (!callback)
        callback = _defCallback;
    CRDirContentItem * dir = getOrAdd(pathname);
    //CRLog::trace("CRDirCache::scan - got dir");
    CRGuard guard(_monitor);
    //CRLog::trace("CRDirCache::scan - acquired lock");
    //CRLog::trace("CRCoverPageManager::prepare %s %dx%d", _book->getPathName().c_str(), dx, dy);
    if (_queue.length() < 0)
    	CRLog::fatal("CRDirCache::scan :: queue length < 0");
    if (_stopped) {
        CRLog::error("Ignoring new task since dir cache thread is stopped");
        return;
    }
    for (LVQueue<DirectoryScanTask*>::Iterator iterator = _queue.iterator(); iterator.next(); ) {
        DirectoryScanTask * item = iterator.get();
        if (item->isSame(pathname)) {
            CRLog::trace("CRDirCache::scan - before moveToHead queueLength was %d", _queue.length());
            iterator.moveToHead();
            CRLog::trace("CRDirCache::scan - after moveToHead queueLength is %d", _queue.length());
            return;
        }
    }
    if (_queue.length() < 0)
    	CRLog::fatal("CRDirCache::scan 2 :: queue length < 0");
    DirectoryScanTask * task = new DirectoryScanTask(dir, callback);
    CRLog::trace("CRDirCache::scan - adding new task %s queueLength was %d", pathname.c_str(), _queue.length());
    _queue.pushBack(task);
    if (_queue.length() < 0)
    	CRLog::fatal("CRDirCache::scan 3 :: queue length < 0");
    CRLog::trace("CRDirCache::scan - added new task %s queueLength = %d. Calling notifyAll", pathname.c_str(), _queue.length());
    _monitor->notifyAll();
}

void CRDirCache::run() {
    CRLog::info("CRDirCache thread started");
    for (;;) {
        if (_stopped)
            break;
        DirectoryScanTask * task = NULL;
        {
            //CRLog::trace("CRDirCache::run :: wait for lock");
            CRGuard guard(_monitor);
            if (_stopped)
                break;
            //CRLog::trace("CRDirCache::run :: lock acquired");
            if (_queue.length() < 0)
            	CRLog::fatal("CRDirCache::run :: queue length < 0");
            if (_queue.length() <= 0) {
                CRLog::trace("CRDirCache::run :: no tasks - calling monitor wait");
                _monitor->wait();
                CRLog::trace("CRDirCache::run :: done monitor wait - queue length = %d", _queue.length());
            }
            if (_stopped)
                break;
            CRLog::trace("CRDirCache::run - before popFront queueLength was %d", _queue.length());
            task = _queue.popFront();
            if (_queue.length() < 0)
            	CRLog::fatal("CRDirCache::run 2 :: queue length < 0");
            CRLog::trace("CRDirCache::run - after popFront queueLength is %d", _queue.length());
        }
        if (task) {
            CRDirContentItem * item = task->dir;
            CRLog::trace("CRDirCache::run :: calling refresh()");
            item->refresh();
            CRLog::trace("CRDirCache::run :: posting callback to GUI thread");
            concurrencyProvider->executeGui(task); // callback will be deleted in GUI thread
        }
    }
    CRLog::info("CRCoverPageManager thread finished");
}


CRDirCache::~CRDirCache() {
    stop();
    clear();
}

void CRDirCache::start() {
    _thread->start();
}

CRDirCache::CRDirCache() : _head(NULL), _byName(1000), _stopped(false), _defCallback(NULL) {
    _monitor = concurrencyProvider->createMonitor();
    _thread = concurrencyProvider->createThread(this);
}

CRDirContentItem * CRDirCache::getOrAdd(CRDirItem * dir) {
    CRDirContentItem * existing = find(dir);
	if (existing)
		return existing;
    CRGuard guard(_monitor);
    CRDirContentItem * newItem = NULL;
    DIR_TYPE type = dir->getDirType();
    if (type == DIR_TYPE_BOOKS_BY_AUTHOR || type == DIR_TYPE_BOOKS_BY_TITLE || type == DIR_TYPE_BOOKS_BY_SERIES || type == DIR_TYPE_BOOKS_BY_FILENAME) {
        newItem = new CRBookDBLookupItem(dir->getPathName());
    } else if (type == DIR_TYPE_OPDS_CATALOG) {
        newItem = new CROpdsCatalogsItem(dir);
    } else {
        newItem = new CRDirCacheItem(dir);
    }
	addItem(newItem);
	return newItem;
}

/// saves last position for book; fills ids for inserted items
bool CRDirCache::saveLastPosition(BookDBBook * book, BookDBBookmark * pos)
{
    CRGuard guard(_monitor);
    bookDB->saveLastPosition(book, pos);
    if (_recentBooks.onLastPositionUpdated(book, pos)) {
        // notify
        if (_defCallback)
            _defCallback->onDirectoryScanFinished(&_recentBooks);
        return true;
    }
    return false;
}

/// loads last position for book (returns cloned value), returns NULL if not found
BookDBBookmark * CRDirCache::loadLastPosition(BookDBBook * book) {
    CRGuard guard(_monitor);
    BookDBBookmark * res = bookDB->loadLastPosition(book);
    return res;
}

static bool isArchive(const lString8 & path) {
	LVContainerRef dir;
	LVStreamRef arcStream;
	arcStream = LVOpenFileStream(path.c_str(), LVOM_READ);
	if (arcStream.isNull()) {
		// cannot read file...
		return false;
	}
	dir = LVOpenArchieve(arcStream);
	if (!dir) {
		return false;
	}
	// Archive!
	return true;
}

CRDirContentItem * CRDirCache::getOrAdd(const lString8 & pathname) {
	CRDirItem dir(pathname, isArchive(pathname));
	return getOrAdd(&dir);
}

/// remove item from cache
void CRDirCache::remove(const lString8 & pathname) {
    if (pathname == RECENT_DIR_TAG)
        return;
    CRGuard guard(_monitor);
    removeItem(pathname);
    _monitor->notifyAll();
}

CRDirContentItem * CRDirCache::find(lString8 pathname) {
    if (pathname == RECENT_DIR_TAG)
        return &_recentBooks;
    CRGuard guard(_monitor);
    CRDirCache::Item * item = findItem(pathname);
	if (!item)
		return NULL;
	return item->dir;
}

void CRDirCache::clear() {
    CRGuard guard(_monitor);
    CR_UNUSED(guard);
    while (_head) {
		Item * item = _head;
		_head = item->next;
		delete item->dir;
		delete item;
	}
	_byName.clear();
	_monitor->notifyAll();
}

CRFileItem * CRDirCache::findBook(const lString8 & pathname) {
    Item * p = _head;
    while (p) {
        for (int i = 0; i < p->dir->itemCount(); i++) {
            CRDirEntry * item = p->dir->getItem(i);
            if (item->getPathName() == pathname && !item->isDirectory())
                return (CRFileItem*)(item->clone());
        }
        p = p->next;
    }
    return NULL;
}

CRFileItem * CRDirCache::scanFile(const lString8 & pathname) {
    CRFileItem * res = NULL;
    {
        CRGuard guard(_monitor);
        res = findBook(pathname);
        if (res)
            return res;
        res = new CRFileItem(pathname, false);
        BookDBBook * book = bookDB->loadBook(pathname);
        if (!book) {
            lString8 arcname;
            lString8 fname;
            lString8 path;
            if (splitArcName(pathname, arcname, fname)) {
                path = UnicodeToUtf8(LVExtractPath(Utf8ToUnicode(arcname), false));
            } else {
                path = UnicodeToUtf8(LVExtractPath(Utf8ToUnicode(pathname), false));
            }
            LVContainerRef container;
            book = LVParseBook(path, pathname, container);
        }
        if (book) {
            res->setBook(book);
            res->setParsed(true);
            return res;
        }
        delete res;
        return NULL;
    }
    // need scan
    return res;
}


bool LVCalcDirectoryHash(const lString8 & path, bool isArchive, lUInt64 & hash) {
	hash = 0;
	LVContainerRef dir;
	LVStreamRef arcStream;
	if (isArchive) {
		arcStream = LVOpenFileStream(path.c_str(), LVOM_READ);
		if (arcStream.isNull()) {
			return false;
		}
		dir = LVOpenArchieve(arcStream);
	} else {
		dir = LVOpenDirectory(Utf8ToUnicode(path).c_str());
	}
	if (!dir) {
		return false;
	}
	for (int i = 0; i < dir->GetObjectCount(); i++) {
		const LVContainerItemInfo * item = dir->GetObjectInfo(i);
		lString16 pathName = (lString16(dir->GetName()) + item->GetName());
		if (item->IsContainer()) {
			hash = hash * 31 + getHash(pathName);
		} else {
			hash = hash * 31 + getHash(pathName) + 1826327 * item->GetSize();
		}
	}
	return true;
}

BookDBBook * LVParseBook(const lString8 & path, const lString8 & pathName, LVContainerRef & arcContainer) {
    lString16 lower = Utf8ToUnicode(pathName);
    lower.lowercase();
    int fmt = LVDocFormatFromExtension(lower);
    CRLog::trace("going to parse %s", pathName.c_str());
    lString8 arcname;
    lString8 fname;
    lInt64 createTime = 0;
    LVContainerRef arc;
    LVStreamRef stream;
    LVStreamRef arcstream;
    if (splitArcName(pathName, arcname, fname)) {
        //CRLog::trace("item is from archive");
        struct stat fs;
        if (!stat(arcname.c_str(), &fs )) {
            createTime = fs.st_mtime * (lInt64)1000;
        }
        if (!arcContainer.isNull() && arcname == UnicodeToUtf8(arcContainer->GetName())) {
            CRLog::trace("Reusing opened archive");
            arc = arcContainer;
        }
        if (arc.isNull()) {
            arcstream = LVOpenFileStream(arcname.c_str(), LVOM_READ);
            if (!arcstream.isNull()) {
                //CRLog::trace("trying to open archive %s", arcname.c_str());
                arc = LVOpenArchieve(arcstream);
            }
        }
        if (!arc.isNull()) {
            //CRLog::trace("trying to open stream %s from archive %s", fname.c_str(), arcname.c_str());
            stream = arc->OpenStream(Utf8ToUnicode(fname).c_str(), LVOM_READ);
            //CRLog::error("returned from open stream");
        } else {
            CRLog::error("Failed to open archive %s", arcname.c_str());
        }
//        if (path.empty())
//            path = arcname;
    } else {
        fname = UnicodeToUtf8(LVExtractFilename(Utf8ToUnicode(pathName)));
        stream = LVOpenFileStream(pathName.c_str(), LVOM_READ);
        struct stat fs;
        if (!stat(pathName.c_str(), &fs )) {
            createTime = fs.st_mtime * (lInt64)1000;
        }
    }
    if (!stream.isNull()) {
        //CRLog::trace("processing stream");
        // read properties
        CRLog::trace("parsing book properties for %s", pathName.c_str());
        BookDBBook * book = new BookDBBook();
        book->pathname = pathName.c_str();
        if (arcname.length()) {
            book->arcname = arcname.c_str();
        } else {

        }
        book->createTime = createTime;
        book->format = fmt;
        book->filename = fname.c_str();
        book->filesize = (int)stream->GetSize();
        book->folder = new BookDBFolder(path.c_str());
        if (LVParseBookProperties(stream, book) || fmt != 0) {
            return book;
        } else {
            // cannot parse properties
            //delete book;
            return book;
        }
    } else {
        CRLog::error("Failed to open item %s", pathName.c_str());
        return NULL;
    }
}

bool LVListDirectory(const lString8 & path, bool isArchive, LVPtrVector<CRDirEntry> & entries, lUInt64 & hash) {
	hash = 0;
	LVContainerRef dir;
	LVStreamRef arcStream;
    lString16 lowerpath = Utf8ToUnicode(path);
    lowerpath.lowercase();
    isArchive = isArchive || lowerpath.endsWith(".zip");

#ifdef SLOW_SCAN_SIMULATION_FOR_TEST
    // for testing
    CRLog::trace("sleeping for 3 seconds");
    concurrencyProvider->sleepMs(3000);
    CRLog::trace("awake");
#endif
	if (isArchive) {
		arcStream = LVOpenFileStream(path.c_str(), LVOM_READ);
		if (arcStream.isNull()) {
			CRLog::error("LVListDirectory cannot open base archive stream");
			return false;
		}
		dir = LVOpenArchieve(arcStream);
	} else {
		dir = LVOpenDirectory(Utf8ToUnicode(path).c_str());
	}
	if (!dir) {
		CRLog::error("LVListDirectory - Failed to list %s %s", isArchive ? "archive" : "directory", path.c_str());
		return false;
	}
	lString8Collection forLoad;
	lString8Collection forParse;
	for (int i = 0; i < dir->GetObjectCount(); i++) {
		const LVContainerItemInfo * item = dir->GetObjectInfo(i);
        lString16 pathName = (lString16(dir->GetName()) + (isArchive ? "@/" : "") + item->GetName());
        lString16 pathNameLower = pathName;
        pathNameLower.lowercase();
        lString8 pathName8 = UnicodeToUtf8(pathName);
		if (item->IsContainer()) {
			hash = hash * 31 + getHash(pathName);
			// usual directory
			CRDirItem * subdir = new CRDirItem(pathName8, false);
			entries.add(subdir);
			CRLog::trace("subdir: %s", pathName8.c_str());
		} else {
			hash = hash * 31 + getHash(pathName) + 1826327 * item->GetSize();
			LVStreamRef stream = dir->OpenStream(item->GetName(), LVOM_READ);
			if (!stream.isNull()) {
                // only open zips
                lString16 name(item->GetName());
                name.lowercase();
                LVContainerRef arc = name.endsWith(L".zip") ? LVOpenArchieve(stream) : LVContainerRef();
				if (!arc.isNull()) {
					//CRLog::trace("archive: %s", pathName8.c_str());
					int knownFiles = 0;
					lString8 foundItem;
					// is archive
					for (int i = 0; i < arc->GetObjectCount(); i++) {
						const LVContainerItemInfo * item = arc->GetObjectInfo(i);
						if (item->IsContainer())
							continue;
						lString16 arcItem = item->GetName();
						//CRLog::trace("arc item: %s", LCSTR(arcItem));
						lString16 lower = arcItem;
						lower.lowercase();
						int fmt = LVDocFormatFromExtension(lower);
						if (fmt) {
							knownFiles++;
							if (knownFiles == 1) {
								// TODO: make arc+item path
								lString16 fn = pathName + L"@/" + arcItem;
								foundItem = UnicodeToUtf8(fn);
                            } else if (knownFiles > 1)
                                break;
						}
					}
					if (knownFiles == 1) {
						// single book inside archive
						CRFileItem * book = new CRFileItem(foundItem, true);
						entries.add(book);
						forLoad.add(foundItem);
						//CRLog::trace("single archive item: %s", foundItem.c_str());
					} else if (knownFiles > 1) {
						// several known files in archive
						CRDirItem * subdir = new CRDirItem(pathName8, true);
						entries.add(subdir);
						//CRLog::trace("%d archive items, treat as directory", knownFiles);
					}
					continue;
				}
			}
            // regular file
            int fmt = LVDocFormatFromExtension(pathNameLower);
			if (!fmt)
				continue; // UNKNOWN_FORMAT
			// try as normal file
			CRFileItem * book = new CRFileItem(pathName8, false);
			entries.add(book);
			forLoad.add(pathName8);
			//CRLog::trace("normal file %s of type %d", pathName8.c_str(), fmt);
		}
	}
	LVPtrVector<BookDBBook> loaded;
	LVPtrVector<BookDBBook> forSave;
	CRLog::trace("%d entries for load", forLoad.length());
	bookDB->loadBooks(forLoad, loaded, forParse);
	CRLog::trace("%d entries loaded, %d unknown", loaded.length(), forParse.length());
	for (int i = 0; i<loaded.length(); i++) {
		lString8 fn = lString8(loaded[i]->pathname.c_str());
		int found = find(entries, fn);
		if (found >= 0) {
			entries[found]->setBook(loaded[i]->clone());
			entries[found]->setParsed(true);
		}
	}
	CRLog::trace("%d entries for parse", forParse.length());
	for (int i = 0; i<forParse.length(); i++) {
		lString8 pathName = forParse[i];
        BookDBBook * book = LVParseBook(path, pathName, dir);
        if (book) {
            forSave.add(book);
        }
//		lString16 lower = Utf8ToUnicode(pathName);
//		lower.lowercase();
//		int fmt = LVDocFormatFromExtension(lower);
//		CRLog::trace("going to parse %s", pathName.c_str());
//		lString8 arcname;
//		lString8 fname;
//		lInt64 createTime = 0;
//		LVContainerRef arc;
//		LVStreamRef stream;
//		LVStreamRef arcstream;
//		if (splitArcName(pathName, arcname, fname)) {
//			//CRLog::trace("item is from archive");
//		    struct stat fs;
//		    if (!stat(arcname.c_str(), &fs )) {
//		        createTime = fs.st_mtime * (lInt64)1000;
//		    }
//		    arcstream = LVOpenFileStream(arcname.c_str(), LVOM_READ);
//		    if (!arcstream.isNull()) {
//				//CRLog::trace("trying to open archive %s", arcname.c_str());
//				arc = LVOpenArchieve(arcstream);
//				if (!arc.isNull()) {
//					//CRLog::trace("trying to open stream %s from archive %s", fname.c_str(), arcname.c_str());
//					stream = arc->OpenStream(Utf8ToUnicode(fname).c_str(), LVOM_READ);
//					//CRLog::error("returned from open stream");
//				} else {
//					CRLog::error("Failed to open archive %s", arcname.c_str());
//				}
//		    }
//		} else {
//			fname = UnicodeToUtf8(LVExtractFilename(Utf8ToUnicode(pathName)));
//			stream = LVOpenFileStream(pathName.c_str(), LVOM_READ);
//		    struct stat fs;
//		    if (!stat(pathName.c_str(), &fs )) {
//		        createTime = fs.st_mtime * (lInt64)1000;
//		    }
//		}
//		if (!stream.isNull()) {
//			//CRLog::trace("processing stream");
//			// read properties
//			CRLog::trace("parsing book properties for %s", pathName.c_str());
//			BookDBBook * book = new BookDBBook();
//			book->pathname = pathName.c_str();
//			if (arcname.length()) {
//				book->arcname = arcname.c_str();
//			} else {

//			}
//			book->createTime = createTime;
//			book->format = fmt;
//			book->filename = fname.c_str();
//			book->filesize = (int)stream->GetSize();
//			book->folder = new BookDBFolder(path.c_str());
//			if (LVParseBookProperties(stream, book) || fmt != 0) {
//				forSave.add(book);
//			} else {
//				// cannot parse properties
//				delete book;
//			}
//		} else {
//			CRLog::error("Failed to open item %s", pathName.c_str());
//		}
	}
	CRLog::trace("%d entries for save", forSave.length());
	if (forSave.length()) {
		bookDB->saveBooks(forSave);
		for (int i = 0; i < forSave.length(); i++) {
			lString8 fn = lString8(forSave[i]->pathname.c_str());
			int found = find(entries, fn);
			if (found >= 0) {
				entries[found]->setBook(forSave[i]->clone());
				entries[found]->setParsed(true);
			}
		}
	}
	CRLog::trace("done scanning of directory");
	return true;
}


CRDirCache * dirCache = NULL;


CRRecentBookItem::CRRecentBookItem(BookDBBook * book, BookDBBookmark * lastPosition) : CRFileItem(lString8(book->pathname.c_str()), false), _lastPosition(lastPosition->clone())
{
    setBook(book->clone());
}

bool CRRecentBooksItem::onLastPositionUpdated(BookDBBook * book, BookDBBookmark * position)
{
    int pos = -1;
    CRRecentBookItem * found = NULL;
    for (int i = 0; i < _entries.length(); i++) {
        //CRRecentBookItem * item = dynamic_cast<CRRecentBookItem *>(_entries[i]);
        CRRecentBookItem * item = static_cast<CRRecentBookItem *>(_entries[i]);
        if (item && item->getBook() && item->getBook()->id == book->id) {
            pos = i;
            found = item;
            break;
        }
    }
    if (pos == 0) {
        /// already first
        found->setLastPosition(position);
        return false; // no update required
    } else if (pos >= 0) {
        CRDirEntry * item = _entries.remove(pos);
        _entries.insert(0, item);
        item->setLastPosition(position);
    } else {
        // new book
        CRRecentBookItem * item = new CRRecentBookItem(book, position);
        _entries.insert(0, item);
    }
    return true; // order changed
}

/// load from DB
bool CROpdsCatalogsItem::scan() {
    if (_scanned)
        return true;
    LVPtrVector<BookDBCatalog> catalogs;
    bookDB->loadOpdsCatalogs(catalogs);
    for (int i = 0; i < catalogs.length(); i++) {
        BookDBCatalog * item = catalogs[i];
        CROpdsCatalogsItem * entry = new CROpdsCatalogsItem(item, lString8());
        entry->_pathName = lString8(OPDS_CATALOGS_TAG) + lString8::itoa(item->id);
        _entries.add(entry);
    }
    _scanned = true;
    return true;
}

bool CRRecentBooksItem::scan() {
    if (_scanned)
        return true;
    LVPtrVector<BookDBBook> books;
    LVPtrVector<BookDBBookmark> lastPositions;
    bookDB->loadRecentBooks(books, lastPositions);
    for (int i = 0; i < lastPositions.length(); i++) {
        if (books[i] && lastPositions[i]) {
            // item loaded completely
            lString16 pathname = Utf8ToUnicode(books[i]->pathname.c_str());
            bool exists = false;
            lString16 arc, file;
            if (LVSplitArcName(pathname, arc, file)) {
                exists = LVFileExists(arc);
            } else {
                exists = LVFileExists(pathname);
            }
            if (exists) {
                CRRecentBookItem * item = new CRRecentBookItem(books[i], lastPositions[i]);
                _entries.add(item);
            } else {
                CRLog::warn("Recent book file does not exist %s", LCSTR(pathname));
            }
        }
    }
    _scanned = true;
    return true;
}


void CRBookDBLookupItem::unlock() {
    _lockCount--;
    if (!_lockCount)
        dirCache->remove(_pathName);
}

bool CRBookDBLookupItem::scan() {
    if (_scanned)
        return true;
    SEARCH_FIELD type = SEARCH_FIELD_INVALID;
    lString16 pattern = getFilterString();
    const char * tag = NULL;
    if (_pathName.startsWith(BOOKS_BY_AUTHOR_TAG)) {
        tag = BOOKS_BY_AUTHOR_TAG;
        type = SEARCH_FIELD_AUTHOR;
    } else if (_pathName.startsWith(BOOKS_BY_TITLE_TAG)) {
        tag = BOOKS_BY_TITLE_TAG;
        type = SEARCH_FIELD_TITLE;
    } else if (_pathName.startsWith(BOOKS_BY_SERIES_TAG)) {
        tag = BOOKS_BY_SERIES_TAG;
        type = SEARCH_FIELD_SERIES;
    } else if (_pathName.startsWith(BOOKS_BY_FILENAME_TAG)) {
        tag = BOOKS_BY_FILENAME_TAG;
        type = SEARCH_FIELD_FILENAME;
    }
    if (type != SEARCH_FIELD_INVALID) {
        _entries.clear();
        if (pattern.endsWith("%") || pattern.empty()) {
            LVPtrVector<BookDBPrefixStats> prefixes;
            bookDB->findPrefixes(type, pattern, lString8(), prefixes);
            bool nonOneBookFound = false;
            for (int i = 0; i < prefixes.length(); i++) {
                BookDBPrefixStats * item = prefixes[i];
                if (item->bookCount != 1 || !item->bookId) {
                    nonOneBookFound = true;
                    break;
                }
            }
            if (!nonOneBookFound && (pattern.length() > 1 || pattern != "%" || type == SEARCH_FIELD_TITLE || type == SEARCH_FIELD_FILENAME)) {
                // only books (request contains non-empty pattern)
                // load books instead of prefixes
                CRLog::trace("only single boook items for prefix %s - loading %d items", LCSTR(pattern), prefixes.length());
                if (prefixes.length()) {
                    LVArray<lInt64> ids;
                    LVPtrVector<BookDBBook> loaded;
                    for (int i = 0; i < prefixes.length(); i++) {
                        BookDBPrefixStats * item = prefixes[i];
                        ids.add(item->bookId);
                    }
                    bookDB->loadBooks(ids, loaded);
                    for (int i = 0; i < loaded.length(); i++) {
                        CRFileItem * item = new CRFileItem(lString8(loaded[i]->pathname.c_str()), false);
                        item->setBook(loaded[i]->clone());
                        _entries.add(item);
                    }
                }
            } else {
                for (int i = 0; i < prefixes.length(); i++) {
                    BookDBPrefixStats * item = prefixes[i];
                    lString8 pathname = lString8(tag) + UnicodeToUtf8(item->prefix);
                    CRDirItem * p = new CRDirItem(pathname, false);
                    p->setBookCount(item->bookCount);
                    _entries.add(p);
                }
            }
        } else {
            // individual books
            LVPtrVector<BookDBBook> loaded;
            if (bookDB->findBooks(type, pattern, lString8(), loaded)) {
                for (int i = 0; i < loaded.length(); i++) {
                    CRFileItem * item = new CRFileItem(lString8(loaded[i]->pathname.c_str()), false);
                    item->setBook(loaded[i]->clone());
                    _entries.add(item);
                }
            }
        }
    }
    _scanned = true;
    return true;
}

DIR_TYPE CRDirItem::getDirType() const {
    if (_pathName.startsWith(BOOKS_BY_AUTHOR_TAG))
        return DIR_TYPE_BOOKS_BY_AUTHOR;
    if (_pathName.startsWith(BOOKS_BY_TITLE_TAG))
        return DIR_TYPE_BOOKS_BY_TITLE;
    if (_pathName.startsWith(BOOKS_BY_FILENAME_TAG))
        return DIR_TYPE_BOOKS_BY_FILENAME;
    if (_pathName.startsWith(BOOKS_BY_SERIES_TAG))
        return DIR_TYPE_BOOKS_BY_SERIES;

    if (_pathName.startsWith(SEARCH_RESULTS_PREFIX))
        return DIR_TYPE_BOOKS_SEARCH_RESULT;
    if (_pathName == SEARCH_RESULTS_TAG)
        return DIR_TYPE_BOOKS_SEARCH_RESULT;
    if (_pathName.startsWith(OPDS_CATALOGS_TAG) || _pathName.startsWith(OPDS_CATALOG_TAG))
        return DIR_TYPE_OPDS_CATALOG;
    return DIR_TYPE_NORMAL;
}

void CRSetupDirectoryCacheManager() {
    CRStopDirectoryCacheManager();
    dirCache = new CRDirCache();
}

void CRStopDirectoryCacheManager() {
    if (dirCache) {
        dirCache->stop();
        delete dirCache;
        dirCache = NULL;
    }
}

void CRStartDirectoryCacheManager() {
    if (dirCache) {
        dirCache->start();
    }
}

