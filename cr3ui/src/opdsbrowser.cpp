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



void CRUIOpdsBrowserWidget::afterNavigationTo() {
    CRUIOnlineStoreWidget::afterNavigationTo();
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

CRUIOpdsBrowserWidget::CRUIOpdsBrowserWidget(CRUIMainWidget * main)
    : CRUIOnlineStoreWidget(main)
{
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
}

