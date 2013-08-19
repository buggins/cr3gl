/*
 * fileinfo.cpp
 *
 *  Created on: Aug 19, 2013
 *      Author: vlopatin
 */

#include "fileinfo.h"
#include <lvstream.h>

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

bool LVListDirectory(lString8 & path, LVPtrVector<CRDirEntry> & entries) {
	LVContainerRef dir = LVOpenDirectory(Utf8ToUnicode(path).c_str(), NULL);
	if (!dir)
		return false;
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
		}
	}
	return true;
}
