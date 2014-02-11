/*
 * CRUIOpdsBrowserWidget.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */


#include "crui.h"
#include "opdsbrowser.h"
#include "cruilist.h"
#include "cruicontrols.h"
#include "crcoverpages.h"
#include "cruimain.h"
#include "cruicoverwidget.h"
#include "stringresource.h"

using namespace CRUI;


class CRUIOpdsDirItemWidget : public CRUILinearLayout {
public:
    int _iconDx;
    int _iconDy;
    CRUIImageWidget * _icon;
    CRUILinearLayout * _layout;
    CRUILinearLayout * _infolayout;
    CRUITextWidget * _line1;
    CRUITextWidget * _line2;
    CRUITextWidget * _line3;
    CRUITextWidget * _line4;
    CRDirEntry * _entry;
    CRUIOpdsDirItemWidget(int iconDx, int iconDy, const char * iconRes) : CRUILinearLayout(false), _iconDx(iconDx), _iconDy(iconDy), _entry(NULL) {
        _icon = new CRUIImageWidget(iconRes);
        //_icon->setMinWidth(iconDx);
        //_icon->setMinHeight(iconDy);
        //_icon->setMaxWidth(iconDx);
        //_icon->setMaxHeight(iconDy);
        _icon->setAlign(ALIGN_CENTER);
        _icon->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        addChild(_icon);
        _layout = new CRUILinearLayout(true);
        _infolayout = new CRUILinearLayout(false);
        _line1 = new CRUITextWidget();
        _line1->setFontSize(FONT_SIZE_SMALL);
        _line2 = new CRUITextWidget();
        _line2->setFontSize(FONT_SIZE_MEDIUM);
        _line3 = new CRUITextWidget();
        _line3->setFontSize(FONT_SIZE_SMALL);
        _line4 = new CRUITextWidget();
        _line4->setFontSize(FONT_SIZE_XSMALL);
        _line4->setAlign(ALIGN_RIGHT | ALIGN_VCENTER);
        CRUIWidget * spacer1 = new CRUIWidget();
        spacer1->setLayoutParams(FILL_PARENT, FILL_PARENT);
        CRUIWidget * spacer2 = new CRUIWidget();
        spacer2->setLayoutParams(FILL_PARENT, FILL_PARENT);

        _line3->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _infolayout->addChild(_line3);
        _infolayout->addChild(_line4);

        _layout->addChild(spacer1);
        _layout->addChild(_line1);
        _layout->addChild(_line2);
        _layout->addChild(_infolayout);
        _layout->addChild(spacer2);
        _layout->setPadding(lvRect(PT_TO_PX(4), 0, PT_TO_PX(1), 0));
        _layout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _layout->setMaxHeight(deviceInfo.minListItemSize * 3 / 2);
        //_layout->setMinHeight(_iconDy);
        //_layout->setBackground(0x80C0C0C0);
        addChild(_layout);
        setMinWidth(MIN_ITEM_PX);
        setMinHeight(MIN_ITEM_PX * 2 / 3);
        setMargin(PT_TO_PX(1));
        setStyle("LIST_ITEM");
    }

    void setDir(CRDirEntry * entry, int iconDx, int iconDy) {
        _iconDx = iconDx;
        _iconDy = iconDy;
        //_icon->setMinWidth(iconDx);
        //_icon->setMinHeight(iconDy);
        //_icon->setMaxWidth(iconDx);
        //_icon->setMaxHeight(iconDy);
        //_layout->setMaxHeight(_iconDy);
        //_layout->setMinHeight(_iconDy);
        int maxHeight = deviceInfo.minListItemSize * 2 / 3;
        CRUIImageRef icon = _icon->getImage();
        if (!icon.isNull())
            if (maxHeight < icon->originalHeight() + PT_TO_PX(2))
                maxHeight = icon->originalHeight() + PT_TO_PX(2);
        _layout->setMinHeight(deviceInfo.minListItemSize * 2 / 3);
        _layout->setMaxHeight(maxHeight);

        _entry = entry;
    }
};

class CRUIOpdsProgressItemWidget : public CRUILinearLayout {
public:
    int _iconDx;
    int _iconDy;
    CRUISpinnerWidget * _icon;
    CRUILinearLayout * _layout;
    CRUITextWidget * _line1;
    CRUITextWidget * _line2;
    CRUITextWidget * _line3;
    CRUIOpdsProgressItemWidget(int iconDx, int iconDy, const char * iconRes) : CRUILinearLayout(false), _iconDx(iconDx), _iconDy(iconDy) {
        _icon = new CRUISpinnerWidget(iconRes, 360);
        _icon->setAlign(ALIGN_CENTER);
        _icon->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        addChild(_icon);
        _layout = new CRUILinearLayout(true);
        _line1 = new CRUITextWidget();
        _line1->setFontSize(FONT_SIZE_SMALL);
        _line2 = new CRUITextWidget();
        _line2->setFontSize(FONT_SIZE_MEDIUM);
        _line2->setMaxLines(2);
        _line3 = new CRUITextWidget();
        _line3->setFontSize(FONT_SIZE_SMALL);
        _line3->setLayoutParams(FILL_PARENT, WRAP_CONTENT);

        _line2->setText(lString16("Loading OPDS catalog content..."));

        _layout->addChild(_line1);
        _layout->addChild(_line2);
        _layout->addChild(_line3);
        _layout->setPadding(lvRect(PT_TO_PX(4), 0, PT_TO_PX(1), 0));
        _layout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _layout->setMaxHeight(deviceInfo.minListItemSize * 3 / 2);
        //_layout->setMinHeight(_iconDy);
        //_layout->setBackground(0x80C0C0C0);
        addChild(_layout);
        setMinWidth(MIN_ITEM_PX);
        setMinHeight(MIN_ITEM_PX * 2 / 3);
        setMargin(PT_TO_PX(1));
        setStyle("LIST_ITEM");
    }

    void update(int iconDx, int iconDy) {
        _iconDx = iconDx;
        _iconDy = iconDy;
        //_icon->setMinWidth(iconDx);
        //_icon->setMinHeight(iconDy);
        //_icon->setMaxWidth(iconDx);
        //_icon->setMaxHeight(iconDy);
        //_layout->setMaxHeight(_iconDy);
        //_layout->setMinHeight(_iconDy);
        int maxHeight = deviceInfo.minListItemSize * 2 / 3;
        CRUIImageRef icon = _icon->getImage();
        if (!icon.isNull())
            if (maxHeight < icon->originalHeight() + PT_TO_PX(2))
                maxHeight = icon->originalHeight() + PT_TO_PX(2);
        _layout->setMinHeight(deviceInfo.minListItemSize * 2 / 3);
        _layout->setMaxHeight(maxHeight);
    }
};

class CRUIOpdsBookItemWidget : public CRUILinearLayout {
public:
    int _iconDx;
    int _iconDy;
    CRCoverWidget * _icon;
    CRUILinearLayout * _layout;
    CRUILinearLayout * _infolayout;
    CRUITextWidget * _line1;
    CRUITextWidget * _line2;
    CRUITextWidget * _line3;
    CRUITextWidget * _line4;
    CRDirEntry * _entry;
    CRUIOpdsBookItemWidget(int iconDx, int iconDy, CRUIMainWidget * callback, ExternalImageSourceCallback * downloadCallback) : CRUILinearLayout(false), _iconDx(iconDx), _iconDy(iconDy), _entry(NULL) {
        _icon = new CRCoverWidget(callback, NULL, iconDx, iconDy, downloadCallback);
        _icon->setSize(iconDx, iconDy);
        addChild(_icon);
        _layout = new CRUILinearLayout(true);
        _infolayout = new CRUILinearLayout(false);
        _line1 = new CRUITextWidget();
        _line1->setFontSize(FONT_SIZE_SMALL);
        _line2 = new CRUITextWidget();
        _line2->setFontSize(FONT_SIZE_MEDIUM);
        _line3 = new CRUITextWidget();
        _line3->setFontSize(FONT_SIZE_SMALL);
        _line4 = new CRUITextWidget();
        _line4->setFontSize(FONT_SIZE_XSMALL);
        _line4->setAlign(ALIGN_RIGHT | ALIGN_VCENTER);
        CRUIWidget * spacer1 = new CRUIWidget();
        spacer1->setLayoutParams(FILL_PARENT, FILL_PARENT);
        CRUIWidget * spacer2 = new CRUIWidget();
        spacer2->setLayoutParams(FILL_PARENT, FILL_PARENT);

        _line3->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _infolayout->addChild(_line3);
        _infolayout->addChild(_line4);

        _layout->addChild(spacer1);
        _layout->addChild(_line1);
        _layout->addChild(_line2);
        _layout->addChild(_infolayout);
        _layout->addChild(spacer2);
        _layout->setPadding(lvRect(PT_TO_PX(4), 0, PT_TO_PX(1), 0));
        _layout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _layout->setMaxHeight(_iconDy);
        _layout->setMinHeight(_iconDy);
        //_layout->setBackground(0x80C0C0C0);
        addChild(_layout);
        setMinWidth(MIN_ITEM_PX);
        setMinHeight(MIN_ITEM_PX);
        setMargin(PT_TO_PX(1));
        setStyle("LIST_ITEM");
    }
    void setBook(CRDirEntry * entry, int iconDx, int iconDy) {
        _iconDx = iconDx;
        _iconDy = iconDy;
        _icon->setSize(iconDx, iconDy);
        _icon->setBook(entry);
        _layout->setMaxHeight(_iconDy);
        _layout->setMinHeight(_iconDy);
        _entry = entry;
    }
    lvPoint calcCoverSize(int w, int h) {
        return _icon->calcCoverSize(w, h);
    }
};

//static lString16 sizeToString(int size) {
//    if (size < 4096)
//        return lString16::itoa(size);
//    else if (size < 4096*1024)
//        return lString16::itoa(size / 1024) + L"K";
//    else
//        return lString16::itoa(size / 1024 / 1024) + L"M";
//}

class CRUIOpdsItemListWidget : public CRUIListWidget {
protected:
    CRDirContentItem * _dir;
    CRUIOpdsDirItemWidget * _folderWidget;
    CRUIOpdsBookItemWidget * _bookWidget;
    CRUIOpdsProgressItemWidget * _progressWidget;
    CRUIOpdsBrowserWidget * _parent;
    int _coverDx;
    int _coverDy;
    bool _showProgressAsLastItem;
public:
    virtual void setScrollOffset(int offset) {
        bool oldVisible = _showProgressAsLastItem && isItemVisible(_dir->itemCount());
        int oldOffset = _scrollOffset;
        CRUIListWidget::setScrollOffset(offset);
        if (_scrollOffset != oldOffset && _showProgressAsLastItem) {
            if (isItemVisible(_dir->itemCount()) && !oldVisible) {
                CRLog::trace("calling _parent->fetchNextPart()");
                _parent->fetchNextPart();
            }
        }
    }

    void setProgressItemVisible(bool showProgress) {
        if (_showProgressAsLastItem != showProgress) {
            _showProgressAsLastItem = showProgress;
            requestLayout();
            _parent->getMain()->update(true);
        }
    }

    virtual bool isAnimating() {
        return _showProgressAsLastItem && isItemVisible(_dir->itemCount());
    }

    virtual void animate(lUInt64 millisPassed) {
        _progressWidget->animate(millisPassed);
    }

    void calcCoverSize(int dx, int dy) {
        if (dx < dy) {
            // vertical
            _coverDx = dx / 6;
            _coverDy = _coverDx * 4 / 3;
        } else {
            // horizontal
            _coverDy = dy / 4;
            _coverDx = _coverDy * 3 / 4;
        }
    }

    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight) {
        calcCoverSize(baseWidth, baseHeight);
        setColCount(baseWidth > baseHeight ? 2 : 1);
        CRUIListWidget::measure(baseWidth, baseHeight);
    }

    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom) {
        CRUIListWidget::layout(left, top, right, bottom);
    }

    CRUIOpdsItemListWidget(CRUIOpdsBrowserWidget * parent) : CRUIListWidget(true), _dir(NULL), _parent(parent), _showProgressAsLastItem(false) {
        setId("FILE_LIST");
        setLayoutParams(FILL_PARENT, FILL_PARENT);
        //setBackground("tx_wood_v3.jpg");
        calcCoverSize(deviceInfo.shortSide, deviceInfo.longSide);
        _folderWidget = new CRUIOpdsDirItemWidget(_coverDx, _coverDy, "folder");
        _bookWidget = new CRUIOpdsBookItemWidget(_coverDx, _coverDy, parent->getMain(), parent);
        _progressWidget = new CRUIOpdsProgressItemWidget(_coverDx, _coverDy, "spinner_white_48");
        setStyle("FILE_LIST");
        setColCount(2);
    }

    virtual int getItemCount() {
        if (!_dir)
            return 0;
        //CRLog::trace("item count is %d", _dir->itemCount());
        return _dir->itemCount() + (_showProgressAsLastItem ? 1 : 0);
    }
    virtual CRUIWidget * getItemWidget(int index) {
        if (index >= _dir->itemCount()) {
            _progressWidget->update(_coverDx, _coverDy);
            return _progressWidget;
        }
        CRDirEntry * item = _dir->getItem(index);
        if (item->isDirectory()) {
            CRUIOpdsDirItemWidget * res = _folderWidget;
            res->setDir(item, _coverDx, _coverDy);
            res->_line1->setText(L"");
            res->_line2->setText(item->getTitle());
            res->_line3->setText(item->getDescription());
            //CRLog::trace("returning folder item");
            return res;
        } else {
            CRUIOpdsBookItemWidget * res = _bookWidget;
            res->setBook(item, _coverDx, _coverDy);
            lString16 text1;
            lString16 text2;
            lString16 text3;
            lString16 text4;
            text2 = item->getTitle();
            text1 = item->getAuthorNames(false);
//            if (book) {
//                text2 = Utf8ToUnicode(book->title.c_str());
//                text1 = item->getAuthorNames(false);
//                text3 = item->getSeriesName(true);
//                text4 = sizeToString(book->filesize);
//                text4 += " ";
//                text4 += LVDocFormatName(book->format);
//            } else {
//                text2 = Utf8ToUnicode(item->getFileName());
//            }
            if (text2.empty())
                text2 = Utf8ToUnicode(item->getFileName());
            res->_line1->setText(text1);
            res->_line2->setText(text2);
            res->_line3->setText(text3);
            res->_line4->setText(text4);
            //CRLog::trace("returning book item");
            return res;
        }
    }
    virtual void setDirectory(CRDirContentItem * dir)
    {
        _dir = dir;
        //dir->sort(BY_TITLE);
        requestLayout();
    }

    /// return true if drag operation is intercepted
    virtual bool startDragging(const CRUIMotionEvent * event, bool vertical) {
        return _parent->getMain()->startDragging(event, vertical);
    }

    /// returns true if all coverpages are available, false if background tasks are submitted
    virtual bool requestAllVisibleCoverpages() {
        coverPageManager->cancelAll();
        lvRect rc = _pos;
        applyMargin(rc);
        applyPadding(rc);

        lvRect clip = _pos;
        bool foundNotReady = false;
        for (int i=0; i<getItemCount() && i < _itemRects.length(); i++) {
            lvRect childRc = _itemRects[i];
            if (_vertical) {
                childRc.top -= _scrollOffset;
                childRc.bottom -= _scrollOffset;
            } else {
                childRc.left -= _scrollOffset;
                childRc.right -= _scrollOffset;
            }
            if (clip.intersects(childRc)) {
                CRDirEntry * item = _dir->getItem(i);
                if (!item->isDirectory()) {
                    lvPoint coverSize = _bookWidget->calcCoverSize(_coverDx, _coverDy);
                    LVDrawBuf * buf = coverPageManager->getIfReady(item, coverSize.x, coverSize.y);
                    if (buf) {
                        // image is ready
                    } else {
                        foundNotReady = true;
                        coverPageManager->prepare(item, coverSize.x, coverSize.y, NULL);
                    }
                }
            }
        }
        return !foundNotReady;
    }
};

void CRUIOpdsBrowserWidget::setDirectory(LVClonePtr<BookDBCatalog> & catalog, CRDirContentItem * dir)
{
    _catalog = catalog;

    if (_dir)
        _dir->unlock();
    _dir = (CROpdsCatalogsItem*)dir;
    if (_dir)
        _dir->lock();
    //_title->setTitle(makeDirName(dir, false));
    _title->setTitle(dir->getTitle());
    _fileList->setDirectory(_dir);
    //_fileList->setDirectory(dir);
	requestLayout();
}

class OPDSParser : public LVXMLParserCallback {
    BookDBCatalog * _catalog;
    lString8 protocol;
    lString8 host;
    lString8 url;
    lString8 serverurl;
    lString8 margin;
    int level;
    bool insideEntry;
    bool insideTitle;
    bool insideContent;
    bool insideLink;
    bool insideLanguage;
    bool insideFormat;
    bool insideAuthor;
    bool insideName;
    bool insideUri;
    bool insideId;
    lString8 entryTitle;
    lString8 entryContent;
    lString8 entryContentType;
    lString8 entryHref;
    lString8 linkHref;
    lString8 linkType;
    lString8 linkRel;
    lString8 linkTitle;
    lString8 language;
    lString8 format;
    lString8 authorName;
    lString8 authorUri;
    lString8 id;
    LVPtrVector<OPDSLink> links;
    LVPtrVector<OPDSAuthor> authors;
public:
    lString8 nextPartUrl;
    lString8 openSearchUrl;
    lString8 searchTermsUrl;

    LVPtrVector<CRDirEntry, false> _entries;

    OPDSParser(BookDBCatalog * catalog) : _catalog(catalog), level(0), insideEntry(false) {
        insideTitle = false;
        insideContent = false;
        insideLink = false;
        insideLanguage = false;
        insideFormat = false;
        insideAuthor = false;
        insideName = false;
        insideUri = false;
        insideId = false;
    }

    /// make absolute URL from relative
    lString8 makeLink(lString8 relativeLink) {
        if (relativeLink.startsWith("/")) {
            return serverurl + relativeLink.substr(1);
        }
        if (relativeLink.startsWith("http://") || relativeLink.startsWith("https://"))
            return relativeLink;
        if (url.endsWith("/"))
            return url + relativeLink;
        else
            return url + "/" + relativeLink;
    }

    /// called on parsing end
    virtual void OnStop() {

    }

    /// called on opening tag <
    virtual ldomNode * OnTagOpen( const lChar16 * nsname, const lChar16 * tagname) {
        CR_UNUSED(nsname);
        CRLog::trace("%s<%s>", margin.c_str(), LCSTR(lString16(tagname)));
        margin << "    ";
        level++;
        lString16 tag(tagname);
        if (tag == "entry") {
            entryTitle.clear();
            entryContent.clear();
            entryContentType.clear();
            linkHref.clear();
            linkType.clear();
            language.clear();
            format.clear();
            links.clear();
            authors.clear();
            id.clear();
            insideEntry = true;
        } else if (tag== "title")
            insideTitle = true;
        else if (tag == "content")
            insideContent = true;
        else if (tag == "language")
            insideLanguage = true;
        else if (tag == "format")
            insideFormat = true;
        else if (tag == "id")
            insideId = true;
        else if (tag == "author") {
            authorName.clear();
            authorUri.clear();
            insideAuthor = true;
        } else if (tag == "name")
            insideName = true;
        else if (tag == "uri") {
            insideUri = true;
        } else if (tag == "link") {
            insideLink = true;
            linkHref.clear();
            linkType.clear();
            linkRel.clear();
            linkTitle.clear();
        }
        return NULL;
    }

    /// called after > of opening tag (when entering tag body)
    virtual void OnTagBody() {
        // ignore
    }

    void addEntry() {
        if (!links.length())
            return;
        if (entryTitle.empty())
            return;
        OPDSLink * opdsLink = NULL;
        OPDSLink * acquisitionLink = NULL;
        OPDSLink * alternateLink = NULL;
        lString8 coverpageUrl;
        lString8 coverpageThumbUrl;
        for (int i = 0; i < links.length(); i++) {
            OPDSLink * link = links[i];
            if (link->type.startsWith("application/atom+xml") && (link->rel.empty() || link->rel == "subsection"))
                opdsLink = link;
            else if (link->rel.startsWith("http://opds-spec.org/acquisition")) {
                acquisitionLink = link;
            } else if (link->rel == "alternate" && (link->type == "text/html" || link->type == "text")) {
                alternateLink = link;
            } else if (link->rel == "http://opds-spec.org/image" || link->rel == "x-stanza-cover-image") {
                coverpageUrl = link->href;
            } else if (link->rel == "http://opds-spec.org/image/thumbnail" || link->rel == "x-stanza-cover-image-thumbnail") {
                coverpageThumbUrl = link->href;
            }

        }
        if (acquisitionLink || alternateLink) {
            // add book ref
            CROpdsCatalogsItem * item = new CROpdsCatalogsItem(_catalog, lString8(_catalog->url.c_str()));
            item->setTitle(Utf8ToUnicode(entryTitle));
            item->setAuthors(authors);
            item->setDescription(Utf8ToUnicode(entryContent));
            item->setDescriptionType(entryContentType);
            item->setIsBook();
            item->setLinks(links);
            item->setCoverThumbUrl(coverpageThumbUrl);
            item->setCoverUrl(coverpageUrl);
            item->setId(id);
            _entries.add(item);
        } else if (opdsLink) {
            // add catalog ref
            CROpdsCatalogsItem * item = new CROpdsCatalogsItem(_catalog, opdsLink->href);
            item->setId(id);
            item->setTitle(Utf8ToUnicode(entryTitle));
            item->setDescription(Utf8ToUnicode(entryContent));
            item->setDescriptionType(entryContentType);
            _entries.add(item);
        } else {
            CRLog::error("No valid links found");
            for (int i = 0; i < links.length(); i++) {
                OPDSLink * link = links[i];
                CRLog::trace("link %d href=%s type=%s rel=%s title=%s", i, link->href.c_str(), link->type.c_str(), link->rel.c_str(), link->title.c_str());
            }
        }
        //CRLog::trace("*** Entry: title=%s content=%s href=%s", title.c_str(), content.c_str(), href.c_str());
    }

    /// called on tag close
    virtual void OnTagClose( const lChar16 * nsname, const lChar16 * tagname ) {
        CR_UNUSED(nsname);
        if (margin.length() >= 4)
            margin.erase(margin.length() - 4, 4);
        CRLog::trace("%s</%s>", margin.c_str(), LCSTR(lString16(tagname)));
        level--;
        lString16 tag(tagname);
        if (tag == "entry") {
            if (!entryTitle.empty()) {
                addEntry();
            }
            insideEntry = false;
        } else if (tag == "title")
            insideTitle = false;
        else if (tag == "content")
            insideContent = false;
        else if (tag == "id")
            insideId = false;
        else if (tag == "language")
            insideLanguage = false;
        else if (tag == "format")
            insideFormat = false;
        else if (tag == "author") {
            if (!authorName.empty()) {
                OPDSAuthor * author = new OPDSAuthor();
                author->name = authorName;
                author->url = authorUri;
                authors.add(author);
            }
            insideAuthor = false;
        } else if (tag == "uri") {
            insideUri = false;
        } else if (tag == "name")
            insideName = false;
        else if (tag == "link") {
            if (insideEntry && !linkHref.empty() && !linkType.empty()) {
                bool alreadyExists = false;
                for (int i = 0; i <links.length(); i++) {
                    if (links[i]->href == linkHref)
                        alreadyExists = true;
                }
                if (!alreadyExists) {
                    OPDSLink * link = new OPDSLink();
                    link->type = linkType;
                    link->rel = linkRel;
                    link->title = linkTitle;
                    link->href = linkHref;
                    links.add(link);
                }
            } else if (linkRel == "next" && !linkHref.empty()) {
                nextPartUrl = linkHref;
            } else if (linkRel == "search" && !linkHref.empty()) {
                if (linkType.startsWith("application/atom+xml"))
                    searchTermsUrl = linkHref;
                else if (linkType.startsWith("application/opensearchdescription+xml"))
                    openSearchUrl = linkHref;
            }
            insideLink = false;
        }
    }

    /// called on element attribute
    virtual void OnAttribute( const lChar16 * nsname, const lChar16 * attrname, const lChar16 * attrvalue ) {
        CR_UNUSED(nsname);
        lString16 attr(attrname);
        lString8 value8 = UnicodeToUtf8(attrvalue);
        CRLog::trace("%s  attribute: %s = %s", margin.c_str(), LCSTR(attr), value8.c_str());
        value8.trim();
        if (insideLink) {
            if (attr == "type")
                linkType = value8;
            else if (attr == "href")
                linkHref = makeLink(value8);
            else if (attr == "rel")
                linkRel = value8;
            else if (attr == "title")
                linkTitle = value8;
        } else if (insideContent) {
            if (attr == "type")
                entryContentType = value8;
        }

    }

    /// called on text
    virtual void OnText( const lChar16 * text, int len, lUInt32 flags ) {
        CR_UNUSED(flags);
        lString16 txt(text, len);
        lString8 txt8 = UnicodeToUtf8(txt);
        CRLog::trace("%s  text: %s", margin.c_str(), txt8.c_str());
        if (insideEntry) {
            if (insideTitle)
                entryTitle = txt8;
            else if (insideContent)
                entryContent = txt8;
            else if (insideLanguage)
                language = txt8;
            else if (insideFormat)
                format = txt8;
            else if (insideId)
                id = txt8;
            else if (insideAuthor && insideName)
                authorName = txt8;
            else if (insideAuthor && insideUri)
                authorUri = txt8;
        }
    }

    /// add named BLOB data to document
    virtual bool OnBlob(lString16 name, const lUInt8 * data, int size) {
        CR_UNUSED3(name, data, size);
        return true;
    }

    bool parse(lString8 _url, LVStreamRef stream) {
        url = _url;
        int protocolEnd = url.pos("://");
        if (protocolEnd < 0)
            return false;
        protocol = url.substr(0, protocolEnd);
        int hostStart = protocolEnd + 3;
        int hostEnd = url.pos("/", hostStart);
        if (hostEnd < 0)
            return false;
        host = url.substr(hostStart, hostEnd - hostStart);
        serverurl = url.substr(0, hostEnd + 1);
        CRLog::trace("url=%s protocol=%s host=%s serverurl=%s", url.c_str(), protocol.c_str(), host.c_str(), serverurl.c_str());
        LVXMLParser parser(stream, this);
        if (parser.Parse()) {
            CRLog::trace("Parsed ok");
            return true;
        } else {
            return false;
        }
    }
};

/// download result
void CRUIOpdsBrowserWidget::onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream) {
    CRLog::trace("onDownloadResult task=%d url=%s result=%d resultMessage=%s, totalSize=%d", downloadTaskId, url.c_str(), result, resultMessage.c_str(), size);
    if (stream.isNull()) {
        CRLog::trace("No data received");
    } else {
        CRLog::trace("Stream size is %d", (int)stream->GetSize());
    }
    lString8 charset;
    if (_requestId == downloadTaskId) {
        _requestId = 0;
        // received OPDS data
        if (!result) {
            if (mimeType.startsWith("application/atom+xml") || mimeType.startsWith("text/xml")) {
                lString8Collection typeParams;
                typeParams.split(mimeType, lString8(";"));
                for (int i = 1; i < typeParams.length(); i++) {
                    if (typeParams[i].startsWith("charset="))
                        charset = typeParams[i].substr(8);
                }
                //
                OPDSParser parser(_catalog.get());
                if (parser.parse(url, stream)) {
                    CRLog::trace("Parsed ok, %d entries found", parser._entries.length());
                    if (parser._entries.length() == 0) {
                        lString16 errorMsg = lString16("No entries returned from OPDS catalog");
                        getMain()->showMessage(errorMsg, 2000);
                        _fileList->setProgressItemVisible(false);
                    } else {
                        if (_searchUrl.empty() && !parser.searchTermsUrl.empty())
                            _searchUrl = parser.searchTermsUrl;
                        for (int i = 0; i < parser._entries.length(); i++) {
                            _dir->addEntry(parser._entries[i]);
                        }
                        //_fileList->setDirectory(_dir);
                        if (!parser.nextPartUrl.empty()) {
                            _nextPartURL = parser.nextPartUrl;
                        } else {
                            _fileList->setProgressItemVisible(false);
                        }
                    }
                    requestLayout();
                    getMain()->update(true);
                } else {
                    lString16 errorMsg = lString16("Error while parsing OPDS catalog");
                    getMain()->showMessage(errorMsg, 2000);
                }
            } else {
                _nextPartURL.clear();
                lString16 errorMsg = lString16("Cannot parse OPDS catalog: unexpected content type ") + Utf8ToUnicode(mimeType);
                getMain()->showMessage(errorMsg, 4000);
                CRLog::error("Unexpected content type: %s", mimeType.c_str());
                OPDSParser parser(_catalog.get());
                if (parser.parse(url, stream)) {

                }
                _fileList->setProgressItemVisible(false);
            }
        } else {
            CRLog::error("Error %d %s", result, resultMessage.c_str());
            if (result == 204 || result == 401) {
                lString16 errorMsg = lString16("Authentication required. Please specify Login and Password in this catalog settings.");
                getMain()->showMessage(errorMsg, 4000);
            } else {
                lString16 errorMsg = lString16("ERROR ") + lString16::itoa(result) + L" : " + Utf8ToUnicode(resultMessage);
                getMain()->showMessage(errorMsg, 4000);
            }
            _nextPartURL.clear();
            _fileList->setProgressItemVisible(false);
        }
    } else if (downloadTaskId == _coverTaskId) {
        LVStreamRef nullref;
        coverPageManager->setExternalImage(_coverTaskBook, result == 0 ? stream : nullref);
        invalidate();
        _coverTaskId = 0;
        if (_coverTaskBook)
            delete _coverTaskBook;
        _coverTaskBook = NULL;
        if (_coversToLoad.length())
            fetchCover(_coversToLoad.pop());
    } else {
        CRLog::warn("Download finished from unknown downloadTaskId %d", downloadTaskId);
    }
}

/// download progress
void CRUIOpdsBrowserWidget::onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded) {
    CR_UNUSED3(result, resultMessage, mimeType);
    CRLog::trace("onDownloadProgress task=%d url=%s bytesRead=%d totalSize=%d", downloadTaskId, url.c_str(), sizeDownloaded, size);
}

void CRUIOpdsBrowserWidget::fetchNextPart() {
    if (!_nextPartURL.empty() && !_requestId) {
        //getMain()->showMessage(lString16("Opening ") + Utf8ToUnicode(_nextPartURL), 1000);
        _requestId = getMain()->openUrl(this, _nextPartURL, lString8("GET"), lString8(_catalog->login.c_str()), lString8(_catalog->password.c_str()), lString8());
        _fileList->setProgressItemVisible(true);
        _nextPartURL.clear();
    }
}


bool CRUIOpdsBrowserWidget::fetchCover(CRDirEntry * book) {
    _coverTaskBook = book;
    _coverTaskId = getMain()->openUrl(this, book->getCoverPathName(), lString8("GET"), lString8(_catalog->login.c_str()), lString8(_catalog->password.c_str()), lString8());
    return _coverTaskId != 0;
}

/// call to schedule download of image
bool CRUIOpdsBrowserWidget::onRequestImageDownload(CRDirEntry * book) {
    book = book->clone();
    CRLog::trace("onRequestImageDownload(%s)", book->getCoverPathName().c_str());
    if (!_coverTaskId) {
        fetchCover(book);
    } else {
        _coversToLoad.add(book);
    }
    return true;
}


void CRUIOpdsBrowserWidget::afterNavigationTo() {
    CRUIWindowWidget::afterNavigationTo();
    if (_dir && !_catalog.isNull() && _dir->itemCount() == 0) {
        //getMain()->showMessage(lString16("Opening ") + Utf8ToUnicode(_dir->getURL()), 1000);
        _requestId = getMain()->openUrl(this, _dir->getURL(), lString8("GET"), lString8(_catalog->login.c_str()), lString8(_catalog->password.c_str()), lString8());
        _fileList->setProgressItemVisible(_requestId != 0);
        requestLayout();
        _main->update(true);
        if (!_requestId) {
            _main->showMessage(lString16("Network is not available"), 2000);
        }
    }
    requestLayout();
}

void CRUIOpdsBrowserWidget::cancelDownloads() {
    if (_requestId) {
        _main->cancelDownload(_requestId);
        _requestId = 0;
    }
    if (_coverTaskId) {
        _main->cancelDownload(_coverTaskId);
        if (_coverTaskBook) {
            coverPageManager->cancel(_coverTaskBook, 0, 0);
            delete _coverTaskBook;
        }
        _coverTaskId = 0;
        _coverTaskBook = NULL;
        for (int i = 0; i < _coversToLoad.length(); i++)
            coverPageManager->cancel(_coversToLoad[i], 0, 0);
        _coversToLoad.clear();
    }
}

void CRUIOpdsBrowserWidget::afterNavigationFrom() {
    cancelDownloads();
}

CRUIOpdsBrowserWidget::CRUIOpdsBrowserWidget(CRUIMainWidget * main) : CRUIWindowWidget(main), _title(NULL)
//, _fileList(NULL),
  , _catalog(NULL), _dir(NULL), _requestId(0), _coverTaskId(0), _coverTaskBook(NULL)
{
    _title = new CRUITitleBarWidget(lString16("File list"), this, this, true);
	_body->addChild(_title);
    _fileList = new CRUIOpdsItemListWidget(this);
    _body->addChild(_fileList);
    _fileList->setOnItemClickListener(this);
    setDefaultWidget(_fileList);
    //_fileList = new CRUIOpdsItemListWidget(this);
	//_body->addChild(_fileList);
    //_fileList->setOnItemClickListener(this);
//	CRUIVerticalLayout * layout = new CRUIVerticalLayout();
//	layout->setLayoutParams(FILL_PARENT, FILL_PARENT);
//	CRUIImageWidget * image = new CRUIImageWidget("internet");
//	image->setPadding(PT_TO_PX(8));
//	image->setAlign(ALIGN_CENTER);
//	image->setLayoutParams(FILL_PARENT, FILL_PARENT);
//	layout->addChild(image);
//	CRUITextWidget * text1 = new CRUITextWidget();
//	text1->setAlign(ALIGN_CENTER);
//	text1->setPadding(PT_TO_PX(8));
//	text1->setMaxLines(2);
//	text1->setText("OPDS catalog access is not yet implemented.");
//	text1->setFontSize(FONT_SIZE_LARGE);
//	text1->setLayoutParams(FILL_PARENT, FILL_PARENT);
//	layout->setStyle("SETTINGS_ITEM_LIST");
//	layout->addChild(text1);
//	_body->addChild(layout);


}

bool CRUIOpdsBrowserWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    else if (widget->getId() == "MENU") {
        onAction(CMD_MENU);
    }
    return true;
}

bool CRUIOpdsBrowserWidget::onLongClick(CRUIWidget * widget) {
//    if (widget->getId() == "BACK") {
//        CRUIActionList actions;
//        lString8 path = _dir->getPathName();
//        lString8 lastPath = path;
//        for (;;) {
//            LVRemovePathDelimiter(path);
//            path = LVExtractPath(path);
//            if (path == lastPath)
//                break;
//            LVRemovePathDelimiter(path);
//            CRUIAction action(CMD_SHOW_FOLDER);
//            action.icon_res = "folder_icon";
//            action.name = Utf8ToUnicode(path);
//            action.sparam = path;
//            actions.add(&action);
//            lastPath = path;
//            if (path=="/" || path.endsWith(":\\") || path.endsWith("\\\\") || path == "@/" || path == "@\\")
//                break;
//        }
//        actions.add(ACTION_CURRENT_BOOK);
//        actions.add(ACTION_READER_HOME);
//        lvRect margins;
//        //margins.right = MIN_ITEM_PX * 120 / 100;
//        showMenu(actions, ALIGN_TOP, margins, false);
//    } else
    if (widget->getId() == "MENU") {
        onAction(CMD_SETTINGS);
    }
    return true;
}

/// handle menu or other action
bool CRUIOpdsBrowserWidget::onAction(const CRUIAction * action) {
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    case CMD_MENU:
    {
        CRUIActionList actions;
        actions.add(ACTION_BACK);
        if (!_searchUrl.empty())
            actions.add(ACTION_OPDS_CATALOG_SEARCH);
        if (_main->getSettings()->getBoolDef(PROP_NIGHT_MODE, false))
            actions.add(ACTION_DAY_MODE);
        else
            actions.add(ACTION_NIGHT_MODE);
        actions.add(ACTION_SETTINGS);
        actions.add(ACTION_READER_HOME);
        actions.add(ACTION_EXIT);
        lvRect margins;
        //margins.right = MIN_ITEM_PX * 120 / 100;
        showMenu(actions, ALIGN_TOP, margins, false);
        return true;
    }
    case CMD_OPDS_CATALOG_SEARCH:
        showSearchPopup();
        return true;
    case CMD_SETTINGS:
        _main->showSettings(lString8("@settings/browser"));
        return true;
    }
    return false;
}

class CRUISearchOpdsPopup : public CRUIHorizontalLayout, public CRUIOnClickListener, public CRUIOnReturnPressListener {
    CRUIOpdsBrowserWidget * _window;
    CRUIEditWidget * _editor;
    CRUIImageButton * _nextButton;
public:
    CRUISearchOpdsPopup(CRUIOpdsBrowserWidget * window) : _window(window) {
        setLayoutParams(FILL_PARENT, WRAP_CONTENT);
//        CRUIWidget * delimiter = new CRUIWidget();
//        delimiter->setBackground(0xC0000000);
//        delimiter->setMinHeight(PT_TO_PX(2));
//        delimiter->setMaxHeight(PT_TO_PX(2));
//        _scrollLayout->addChild(delimiter);
        setId("FINDTEXT");

        CRUIVerticalLayout * editlayout = new CRUIVerticalLayout();
        CRUIWidget * spacer1 = new CRUIWidget();
        spacer1->setLayoutParams(FILL_PARENT, FILL_PARENT);
        CRUIWidget * spacer2 = new CRUIWidget();
        spacer2->setLayoutParams(FILL_PARENT, FILL_PARENT);
        _editor = new CRUIEditWidget();
        _editor->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _editor->setBackgroundAlpha(0x80);
        _editor->setOnReturnPressedListener(this);
        //_editor->setPasswordChar('*');
        editlayout->addChild(spacer1);
        editlayout->addChild(_editor);
        editlayout->addChild(spacer2);
        editlayout->setLayoutParams(FILL_PARENT, FILL_PARENT);
        editlayout->setMaxHeight(MIN_ITEM_PX * 3 / 4);
        addChild(editlayout);

        // Buttons
        _nextButton = new CRUIImageButton("right_circular");
        _nextButton->setId("FIND_NEXT");
        addChild(_nextButton);
        _nextButton->setMaxHeight(MIN_ITEM_PX * 3 / 4);

        _nextButton->setBackgroundAlpha(0x80);
        setBackground("home_frame.9");

        _nextButton->setOnClickListener(this);
    }

    virtual bool onReturnPressed(CRUIWidget * widget) {
        CR_UNUSED(widget);
        lString16 text = _editor->getText();
        if (text.empty())
            return true;
        _window->openSearchResults(text);
        return true;
    }

    virtual bool onClick(CRUIWidget * widget) {
        lString16 text = _editor->getText();
        if (text.empty())
            return true;
        if (widget->getId() == "FIND_NEXT") {
            _window->openSearchResults(text);
        }
        return true;
    }

    /// call to set focus to appropriate child once widget appears on screen
    virtual bool initFocus() {
        CRUIEventManager::dispatchFocusChange(_editor);
        return true;
    }

    virtual ~CRUISearchOpdsPopup() {
        //CRLog::trace("~CRUIFindTextPopup()");
        //_window->getMain()->cancelTimer(GO_TO_PERCENT_REPEAT_TIMER_ID);
    }

//    /// handle timer event; return true to allow recurring timer event occur more times, false to stop
//    virtual bool onTimerEvent(lUInt32 timerId) {
////        if (_moveByPageDirection) {
////            moveByPage(_moveByPageDirection);
////            _window->getMain()->setTimer(GO_TO_PERCENT_REPEAT_TIMER_ID, this, GO_TO_PERCENT_REPEAT_TIMER_DELAY, false);
////        }
//        CR_UNUSED(timerId); return false;
//    }

};

lString8 encodeUrlParam(lString8 str) {
    lString8 res;
    for (int i = 0; i < str.length(); i++) {
        lUInt8 ch = str[i];
        if ((ch>='A' && ch <='Z') || (ch>='a' && ch <='z') || (ch>='0' && ch <='9')
                || ch == '_' || ch == '-')
        {
            res += ch;
        } else {
            char buf[10];
            sprintf(buf, "%%%02x", ch);
            res += buf;
        }
    }
    return res;
}

void CRUIOpdsBrowserWidget::openSearchResults(lString16 pattern) {
    _popupControl.close();
    lString8 url = _searchUrl;
    lString8 pattern8 = UnicodeToUtf8(pattern);
    pattern8.trim();
    if (pattern8.length()) {
        int pos = url.pos("{searchTerms}");
        lString8 encoded = encodeUrlParam(pattern8);
        url.replace(pos, 13, encoded);
        CRLog::info("Search url: %s", url.c_str());
        getMain()->showOpds(_dir->getCatalog(), url, lString16("Search results: " + pattern));
    } else {
        getMain()->showMessage(lString16("No pattern to search"), 2000);
    }
}

void CRUIOpdsBrowserWidget::showSearchPopup() {
    lvRect margins;
    CRUISearchOpdsPopup * popup = new CRUISearchOpdsPopup(this);
    preparePopup(popup, ALIGN_TOP, margins, 0x80, false, false);
}

bool CRUIOpdsBrowserWidget::onListItemClick(CRUIListWidget * widget, int index) {
    if (index < 0 || index >= _dir->itemCount())
        return false;
    CRDirEntry * entry = _dir->getItem(index);
    CROpdsCatalogsItem * item = (CROpdsCatalogsItem *)entry;
    if (entry->isDirectory()) {
        widget->setSelectedItem(index);
        getMain()->showOpds(item->getCatalog(), item->getURL(), item->getTitle());
    } else {
        // Book? open book
        widget->setSelectedItem(index);
        LVClonePtr<CROpdsCatalogsItem> book(item);
        getMain()->showOpdsBook(book);
    }
    return true;
}

CRUIOpdsBrowserWidget::~CRUIOpdsBrowserWidget()
{
    cancelDownloads();
    if (_dir)
        _dir->unlock();
}

/// returns true if all coverpages are available, false if background tasks are submitted
bool CRUIOpdsBrowserWidget::requestAllVisibleCoverpages() {
    //return _fileList->requestAllVisibleCoverpages();
	return false;
}

bool CRUIOpdsBrowserWidget::onKeyEvent(const CRUIKeyEvent * event) {
    int key = event->key();
    if (event->getType() == KEY_ACTION_PRESS) {
        if (key == CR_KEY_ESC || key == CR_KEY_BACK || key == CR_KEY_MENU) {
            return true;
        }
    } else if (event->getType() == KEY_ACTION_RELEASE) {
        if (key == CR_KEY_ESC || key == CR_KEY_BACK) {
            _main->back();
            return true;
        } else if (key == CR_KEY_MENU) {
            return onAction(CRUIActionByCode(CMD_MENU));
        	return true;
        }
    }
    return false;
}

/// motion event handler, returns true if it handled event
bool CRUIOpdsBrowserWidget::onTouchEvent(const CRUIMotionEvent * event) {
    int action = event->getAction();
    int delta = event->getX() - event->getStartX();
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
    switch (action) {
    case ACTION_DOWN:
        break;
    case ACTION_UP:
        break;
    case ACTION_FOCUS_IN:
        break;
    case ACTION_FOCUS_OUT:
        return false; // to continue tracking
        break;
    case ACTION_CANCEL:
        break;
    case ACTION_MOVE:
        if ((delta > DRAG_THRESHOLD_X) || (-delta > DRAG_THRESHOLD_X))
            getMain()->startDragging(event, false);
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
    }
    return true;
}
