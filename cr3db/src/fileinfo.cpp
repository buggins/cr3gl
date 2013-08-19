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

    lString16 author = doc->textFromXPath( lString16("package/metadata/creator")).trim();
    lString16 title = doc->textFromXPath( lString16("package/metadata/title")).trim();
    lString16 language = doc->textFromXPath( lString16("package/metadata/language")).trim();

    //pBookProps->author = author;
    pBookProps->title = UnicodeToUtf8(title).c_str();
    pBookProps->language = UnicodeToUtf8(language).c_str();

    for ( int i=1; i<20; i++ ) {
        ldomNode * item = doc->nodeFromXPath( lString16("package/metadata/meta[") << fmt::decimal(i) << "]" );
        if ( !item )
            break;
        lString16 name = item->getAttributeValue("name");
        lString16 content = item->getAttributeValue("content");
        if (name == "calibre:series")
        	pBookProps->series = new BookDBSeries(UnicodeToUtf8(content.trim()).c_str());
        else if (name == "calibre:series_index")
        	pBookProps->seriesNumber = content.trim().atoi();
    }

    delete doc;

    return true;
}

bool LVParseBookProperties(LVStreamRef stream, BookDBBook * props) {
    if ( DetectEpubFormat( stream ) ) {
        CRLog::trace("GetBookProperties() : epub format detected");
    	return GetEPUBBookProperties(stream, props);
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
    lString16 authors = extractDocAuthors( &doc, lString16("|"), false );
    lString16 title = extractDocTitle( &doc );
    lString16 language = extractDocLanguage( &doc );
    int seriesNumber = 0;
    lString16 series = extractDocSeries(&doc, &seriesNumber);
    props->seriesNumber = seriesNumber;
    props->title = LCSTR(title);
    //props->author = LCSTR(authors);
    if (series.length())
    	props->series = new BookDBSeries(LCSTR(series));
    props->language = LCSTR(language);
	return true;
}

bool LVListDirectory(lString8 & path, LVPtrVector<CRDirEntry> & entries) {
	LVContainerRef dir = LVOpenDirectory(Utf8ToUnicode(path).c_str(), NULL);
	if (!dir)
		return false;
	lString8Collection forLoad;
	lString8Collection forParse;
	for (int i = 0; i < dir->GetObjectCount(); i++) {
		const LVContainerItemInfo * item = dir->GetObjectInfo(i);
		lString16 pathName = (lString16(dir->GetName()) + item->GetName());
		lString8 pathName8 = UnicodeToUtf8(pathName);
		if (item->IsContainer()) {
			// usual directory
			CRDirItem * subdir = new CRDirItem(pathName8, false);
			entries.add(subdir);
		} else {
			LVStreamRef stream = dir->OpenStream(item->GetName(), LVOM_READ);
			if (!stream.isNull()) {
				LVContainerRef arc = LVOpenArchieve(stream);
				if (!arc.isNull()) {
					int knownFiles = 0;
					lString8 foundItem;
					// is archive
					for (int i = 0; i < dir->GetObjectCount(); i++) {
						const LVContainerItemInfo * item = dir->GetObjectInfo(i);
						lString16 arcItem = item->GetName();
						int fmt = LVDocFormatFromExtension(arcItem);
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
					} else if (knownFiles > 1) {
						// several known files in archive
						CRDirItem * subdir = new CRDirItem(pathName8, true);
						entries.add(subdir);
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
		}
	}
	LVPtrVector<BookDBBook> loaded;
	LVPtrVector<BookDBBook> forSave;
	bookDB->loadBooks(forLoad, loaded, forParse);
	for (int i = 0; i<loaded.length(); i++) {
		lString8 fn = lString8(loaded[i]->pathname.c_str());
		int found = find(entries, fn);
		if (found >= 0) {
			entries[found]->setBook(loaded[i]->clone());
			entries[found]->setParsed(true);
		}
	}
	for (int i = 0; i<forParse.length(); i++) {
		lString8 pathName = forParse[i];
		lString8 arcname;
		lString8 fname;
		lInt64 createTime = 0;
		LVContainerRef arc;
		LVStreamRef stream;
		if (splitArcName(pathName, arcname, fname)) {
		    struct stat fs;
		    if (!stat(arcname.c_str(), &fs )) {
		        createTime = fs.st_mtime * (lInt64)1000;
		    }
			stream = LVOpenFileStream(arcname.c_str(), LVOM_READ);
			arc = LVOpenArchieve(stream);
			if (!arc.isNull()) {
				stream = arc->OpenStream(Utf8ToUnicode(fname).c_str(), LVOM_READ);
			} else {
				stream = NULL;
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
			// read properties
			BookDBBook * book = new BookDBBook();
			book->pathname = pathName.c_str();
			if (arcname.length()) {
				book->arcname = arcname.c_str();
			} else {

			}
			book->filename = fname.c_str();
			book->filesize = (int)stream->GetSize();
			if (LVParseBookProperties(stream, book)) {
				forSave.add(book);
			} else {
				// cannot parse properties
				delete book;
			}
		}
	}
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
	return true;
}
