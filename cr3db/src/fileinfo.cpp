/*
 * fileinfo.cpp
 *
 *  Created on: Aug 19, 2013
 *      Author: vlopatin
 */

#include "fileinfo.h"
#include <lvstream.h>

#include <epubfmt.h>
#include <pdbfmt.h>

#include <fb2def.h>

#define XS_IMPLEMENT_SCHEME 1
#include <fb2def.h>
#include <sys/stat.h>

enum DocFormat {
	UNKNOWN_FORMAT,
	FB2,
	TXT,
	RTF,
	EPUB,
	HTML,
	TXT_BOOKMARK,
	CHM,
	DOC,
	PDB
};

int LVDocFormatFromExtension(lString16 &pathName) {
	if (pathName.endsWith(".fb2"))
		return FB2;
	if (pathName.endsWith(".txt") || pathName.endsWith(".tcr") || pathName.endsWith(".pml"))
		return TXT;
	if (pathName.endsWith(".rtf"))
		return RTF;
	if (pathName.endsWith(".epub"))
		return EPUB;
	if (pathName.endsWith(".htm") || pathName.endsWith(".html") || pathName.endsWith(".shtml") || pathName.endsWith(".xhtml"))
		return HTML;
	if (pathName.endsWith(".txt.bmk"))
		return TXT_BOOKMARK;
	if (pathName.endsWith(".chm"))
		return CHM;
	if (pathName.endsWith(".doc"))
		return DOC;
	if (pathName.endsWith(".pdb") || pathName.endsWith(".prc") || pathName.endsWith(".mobi") || pathName.endsWith(".azw"))
		return PDB;
	return UNKNOWN_FORMAT;
}

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

#if 0
static bool GetBookProperties(const char *name,  BookProperties * pBookProps)
{
    CRLog::trace("GetBookProperties( %s )", name);

    // check archieve
    lString16 arcPathName;
    lString16 arcItemPathName;
    bool isArchiveFile = LVSplitArcName( lString16(name), arcPathName, arcItemPathName );

    // open stream
    LVStreamRef stream = LVOpenFileStream( (isArchiveFile ? arcPathName : Utf8ToUnicode(lString8(name))).c_str() , LVOM_READ);
    if (!stream) {
        CRLog::error("cannot open file %s", name);
        return false;
    }


    if ( DetectEpubFormat( stream ) ) {
        CRLog::trace("GetBookProperties() : epub format detected");
    	return GetEPUBBookProperties( name, stream, pBookProps );
    }

    time_t t = (time_t)time(0);

    if ( isArchiveFile ) {
        int arcsize = (int)stream->GetSize();
        LVContainerRef container = LVOpenArchieve(stream);
        if ( container.isNull() ) {
            CRLog::error( "Cannot read archive contents from %s", LCSTR(arcPathName) );
            return false;
        }
        stream = container->OpenStream(arcItemPathName.c_str(), LVOM_READ);
        if ( stream.isNull() ) {
            CRLog::error( "Cannot open archive file item stream %s", LCSTR(lString16(name)) );
            return false;
        }
    }
    struct stat fs;
    if ( !stat( name, &fs ) ) {
        t = fs.st_mtime;
    }

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
        return false;
    }
    CRLog::trace( "parsing..." );
    if ( !parser.Parse() ) {
        return false;
    }
    CRLog::trace( "parsed" );
    #if 0
        char ofname[512];
        sprintf(ofname, "%s.xml", name);
        CRLog::trace("    writing to file %s", ofname);
        LVStreamRef out = LVOpenFileStream(ofname, LVOM_WRITE);
        doc.saveToStream(out, "utf16");
    #endif
    lString16 authors = extractDocAuthors( &doc, lString16("|"), false );
    lString16 title = extractDocTitle( &doc );
    lString16 language = extractDocLanguage( &doc );
    lString16 series = extractDocSeries( &doc, &pBookProps->seriesNumber );
#if SERIES_IN_AUTHORS==1
    if ( !series.empty() )
        authors << "    " << series;
#endif
    pBookProps->title = title;
    pBookProps->author = authors;
    pBookProps->series = series;
    pBookProps->filesize = (long)stream->GetSize();
    pBookProps->filename = lString16(name);
    pBookProps->filedate = getDateTimeString( t );
    pBookProps->language = language;
    return true;
}
#endif

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


    if ( DetectEpubFormat( stream ) ) {
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
	return true;
}


bool LVListDirectory(const lString8 & path, bool isArchive, LVPtrVector<CRDirEntry> & entries, lUInt64 & hash);
bool LVCalcDirectoryHash(const lString8 & path, bool isArchive, lUInt64 & hash);

bool CRDirCacheItem::scan() {
	CRLog::trace("Scanning directory %s", getPathName().c_str());
	lUInt64 hash;
	bool res = LVListDirectory(getPathName(), isArchive(), _entries, hash);
	setParsed(true);
	_hash = hash;
	return res;
}

bool CRDirCacheItem::needScan() {
	lUInt64 hash;
	bool res = LVCalcDirectoryHash(getPathName(), isArchive(), hash);
	if (res)
		return hash != _hash;
	return false;
}

void CRDirCache::addItem(CRDirCacheItem * dir) {
	Item * item = new Item(dir);
	item->next = _head;
	if (_head)
		_head->prev = item;
	_head = item;
	_byName.set(dir->getPathName(), item);
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

CRDirCacheItem * CRDirCache::add(CRDirItem * dir) {
	CRDirCacheItem * existing = find(dir);
	if (existing)
		return existing;
	CRDirCacheItem * newItem = new CRDirCacheItem(dir);
	addItem(newItem);
	return newItem;
}

CRDirCacheItem * CRDirCache::find(lString8 pathname) {
	CRDirCache::Item * item = findItem(pathname);
	if (!item)
		return NULL;
	return item->dir;
}

void CRDirCache::clear() {
	while (_head) {
		Item * item = _head;
		_head = item->next;
		delete item->dir;
		delete item;
	}
	_byName.clear();
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

bool LVListDirectory(const lString8 & path, bool isArchive, LVPtrVector<CRDirEntry> & entries, lUInt64 & hash) {
	hash = 0;
	LVContainerRef dir;
	LVStreamRef arcStream;
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
		lString16 pathName = (lString16(dir->GetName()) + item->GetName());
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
				LVContainerRef arc = LVOpenArchieve(stream);
				if (!arc.isNull()) {
					CRLog::trace("archive: %s", pathName8.c_str());
					int knownFiles = 0;
					lString8 foundItem;
					// is archive
					for (int i = 0; i < arc->GetObjectCount(); i++) {
						const LVContainerItemInfo * item = arc->GetObjectInfo(i);
						if (item->IsContainer())
							continue;
						lString16 arcItem = item->GetName();
						CRLog::trace("arc item: %s", LCSTR(arcItem));
						lString16 lower = arcItem;
						lower.lowercase();
						int fmt = LVDocFormatFromExtension(lower);
						if (fmt) {
							knownFiles++;
							if (knownFiles == 1) {
								// TODO: make arc+item path
								lString16 fn = pathName + L"@/" + arcItem;
								foundItem = UnicodeToUtf8(fn);
							}
						}
					}
					if (knownFiles == 1) {
						// single book inside archive
						CRFileItem * book = new CRFileItem(foundItem, true);
						entries.add(book);
						forLoad.add(foundItem);
						CRLog::trace("single archive item: %s", foundItem.c_str());
					} else if (knownFiles > 1) {
						// several known files in archive
						CRDirItem * subdir = new CRDirItem(pathName8, true);
						entries.add(subdir);
						CRLog::trace("%d archive items, treat as directory", knownFiles);
					}
					continue;
				}
			}
			int fmt = LVDocFormatFromExtension(pathName);
			if (!fmt)
				continue; // UNKNOWN_FORMAT
			// try as normal file
			CRFileItem * book = new CRFileItem(pathName8, false);
			entries.add(book);
			forLoad.add(pathName8);
			CRLog::trace("normal file %s of type %d", pathName8.c_str(), fmt);
		}
	}
	LVPtrVector<BookDBBook> loaded;
	LVPtrVector<BookDBBook> forSave;
	CRLog::trace("%d entries for load", forLoad.length());
	bookDB->loadBooks(forLoad, loaded, forParse);
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
		    arcstream = LVOpenFileStream(arcname.c_str(), LVOM_READ);
		    if (!arcstream.isNull()) {
				//CRLog::trace("trying to open archive %s", arcname.c_str());
				arc = LVOpenArchieve(arcstream);
				if (!arc.isNull()) {
					//CRLog::trace("trying to open stream %s from archive %s", fname.c_str(), arcname.c_str());
					stream = arc->OpenStream(Utf8ToUnicode(fname).c_str(), LVOM_READ);
					//CRLog::error("returned from open stream");
				} else {
					CRLog::error("Failed to open archive %s", arcname.c_str());
				}
		    }
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
			book->filename = fname.c_str();
			book->filesize = (int)stream->GetSize();
			book->folder = new BookDBFolder(path.c_str());
			if (LVParseBookProperties(stream, book)) {
				forSave.add(book);
			} else {
				// cannot parse properties
				delete book;
			}
		} else {
			CRLog::error("Failed to open item %s", pathName.c_str());
		}
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
