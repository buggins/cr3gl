#ifndef CRCOVERPAGES_H
#define CRCOVERPAGES_H

#include "lvstring.h"
#include "fileinfo.h"
#include "lvdrawbuf.h"
#include "lvstream.h"

/// draw book cover to drawbuf
void CRDrawBookCover(LVDrawBuf * drawbuf, lString8 fontFace, CRDirEntry * book, LVImageSourceRef image, int bpp = 32);
LVStreamRef LVScanBookCover(lString8 _path);


#endif // CRCOVERPAGES_H
