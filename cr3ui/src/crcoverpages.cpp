
#include "crcoverpages.h"
#include "lvdocview.h"
#include "epubfmt.h"
#include "pdbfmt.h"

void CRDrawBookCover(LVDrawBuf * drawbuf, lString8 fontFace, CRDirEntry * book, LVImageSourceRef image, int bpp)
{
    CRLog::debug("drawBookCover called");
    lString16 title = book->getTitle();
    lString16 authors = book->getAuthorNames(false);
    lString16 seriesName = book->getSeriesName(false);
    if (title.empty() && authors.empty())
        title = Utf8ToUnicode(book->getFileName());
   if (drawbuf != NULL) {
        int factor = 1;
        int dx = drawbuf->GetWidth();
        int dy = drawbuf->GetHeight();
        int MIN_WIDTH = 300;
        int MIN_HEIGHT = 400;
        if (dx < MIN_WIDTH || dy < MIN_HEIGHT) {
            if (dx * 2 < MIN_WIDTH || dy * 2 < MIN_HEIGHT) {
                dx *= 3;
                dy *= 3;
                factor = 3;
            } else {
                dx *= 2;
                dy *= 2;
                factor = 2;
            }
        }
        LVDrawBuf * drawbuf2 = drawbuf;
        if (factor > 1)
            drawbuf2 = new LVColorDrawBuf(dx, dy, drawbuf->GetBitsPerPixel());

        if (bpp >= 16) {
            // native color resolution
            LVDrawBookCover(*drawbuf2, image, fontFace, title, authors, seriesName, 0);
            image.Clear();
        } else {
            LVGrayDrawBuf grayBuf(drawbuf2->GetWidth(), drawbuf2->GetHeight(), bpp);
            LVDrawBookCover(grayBuf, image, fontFace, title, authors, seriesName, 0);
            image.Clear();
            grayBuf.DrawTo(drawbuf2, 0, 0, 0, NULL);
        }
        if (factor > 1) {
            drawbuf->DrawRescaled(drawbuf2, 0, 0, drawbuf->GetWidth(), drawbuf->GetHeight(), 0);
            delete drawbuf2;
        }
    }
    CRLog::debug("drawBookCover finished");
}

LVStreamRef LVScanBookCover(lString8 _path) {
    lString16 path = Utf8ToUnicode(_path);
    CRLog::debug("scanBookCoverInternal(%s) called", LCSTR(path));
    lString16 arcname, item;
    LVStreamRef res;
    LVContainerRef arc;
    if (!LVSplitArcName(path, arcname, item)) {
        // not in archive
        LVStreamRef stream = LVOpenFileStream(path.c_str(), LVOM_READ);
        if (!stream.isNull()) {
            arc = LVOpenArchieve(stream);
            if (!arc.isNull()) {
                // ZIP-based format
                if (DetectEpubFormat(stream)) {
                    // EPUB
                    // extract coverpage from epub
                    res = GetEpubCoverpage(arc);
                }
            } else {
                res = GetFB2Coverpage(stream);
                if (res.isNull()) {
                    doc_format_t fmt;
                    if (DetectPDBFormat(stream, fmt)) {
                        res = GetPDBCoverpage(stream);
                    }
                }
            }
        }
    } else {
        CRLog::debug("scanBookCoverInternal() : is archive, item=%s, arc=%d", LCSTR(item), LCSTR(arcname));
        LVStreamRef arcstream = LVOpenFileStream(arcname.c_str(), LVOM_READ);
        if (!arcstream.isNull()) {
            arc = LVOpenArchieve(arcstream);
            if (!arc.isNull()) {
                LVStreamRef stream = arc->OpenStream(item.c_str(), LVOM_READ);
                if (!stream.isNull()) {
                    CRLog::debug("scanBookCoverInternal() : archive stream opened ok, parsing");
                    res = GetFB2Coverpage(stream);
                    if (res.isNull()) {
                        doc_format_t fmt;
                        if (DetectPDBFormat(stream, fmt)) {
                            res = GetPDBCoverpage(stream);
                        }
                    }
                }
            }
        }
    }
    if (!res.isNull())
        CRLog::debug("scanBookCoverInternal() : returned cover page array");
    else
        CRLog::debug("scanBookCoverInternal() : cover page data not found");
    return res;
}

