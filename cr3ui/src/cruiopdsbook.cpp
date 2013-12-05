
#include "cruiopdsbook.h"

#include "crui.h"
#include "cruiopdsprops.h"
#include "cruilist.h"
#include "cruicontrols.h"
#include "crcoverpages.h"
#include "cruimain.h"
#include "cruicoverwidget.h"
#include "stringresource.h"
#include "cruiconfig.h"
#include "lvrend.h"

using namespace CRUI;

lString16 mimeToFormatName(lString8 mime) {
    if (mime.startsWith("application/fb2"))
        return lString16(L"FB2");
    if (mime.startsWith("application/epub"))
        return lString16(L"EPUB");
    if (mime.startsWith("application/x-mobipocket-ebook"))
        return lString16(L"MOBI");
    if (mime.startsWith("application/txt"))
        return lString16(L"TXT");
    if (mime.startsWith("application/html") || mime.startsWith("text/html"))
        return lString16(L"HTML");
    if (mime.startsWith("application/doc") || mime.startsWith("application/msword"))
        return lString16(L"DOC");
    if (mime.startsWith("application/rtf"))
        return lString16(L"RTF");
    return Utf8ToUnicode(mime);
}

lString8 normalizeFilename(lString8 fn) {
    lString8 res;
    for (int i = 0; i <fn.length(); i++) {
        int ch = (lUInt8)fn[i];
        if (ch <=32 || ch == '/' || ch == '\\' || ch == ':' || ch == ';' || ch == '@' || ch == '(' || ch == ')' || ch == '[' || ch == ']')
            res << "_";
        else
            res << ch;
    }
    return res;
}

lString8 mimeToExtension(lString8 mime) {
    if (mime.startsWith("application/fb2+zip"))
        return lString8(".fb2.zip");
    if (mime.startsWith("application/fb2"))
        return lString8(".fb2");
    if (mime.startsWith("application/epub+zip"))
        return lString8(".epub");
    if (mime.startsWith("application/x-mobipocket-ebook"))
        return lString8(".mobi");
    if (mime.startsWith("application/txt+zip"))
        return lString8(".txt.zip");
    if (mime.startsWith("application/txt"))
        return lString8(".txt");
    if (mime.startsWith("application/html+zip") || mime.startsWith("text/html+zip"))
        return lString8(".html.zip");
    if (mime.startsWith("application/html") || mime.startsWith("text/html"))
        return lString8(".html");
    if (mime.startsWith("application/rtf+zip"))
        return lString8(".rtf.zip");
    if (mime.startsWith("application/doc+zip") || mime.startsWith("application/msword+zip"))
        return lString8(".doc.zip");
    if (mime.startsWith("application/doc") || mime.startsWith("application/msword"))
        return lString8(".doc");
    if (mime.startsWith("application/rtf"))
        return lString8(".rtf");
    return lString8(".") + normalizeFilename(mime);
}

enum {
    NOT_DOWNLOADED,
    DOWNLOADING,
    DOWNLOADED
};


class CRUIBookDownloadWidget : public CRUIHorizontalLayout, public CRUIOnClickListener {
    CRUIOpdsBookWidget * _bookwidget;
    CROpdsCatalogsItem * _book;
    OPDSLink * _link;
    lString16 _formatName;
    CRUIImageButton * _button;
    CRUITextWidget * _caption;
    CRUIProgressWidget * _progress;
    int _state;
    lString8 _downloadDir;
    lString8 _downloadFilename;
    lString8 _downloadTmpFilename;
public:
    OPDSLink * getLink() { return _link; }

    bool createDownloadDir() {
        lString16 path = Utf8ToUnicode(_downloadDir);
        LVRemoveLastPathDelimiter(path);
        if (LVDirectoryExists(path))
            return LVDirectoryIsWritable(path);
        if (!LVCreateDirectory(path))
            return false;
        return LVDirectoryIsWritable(path);
    }

    bool deleteTempFile() {
        return LVDeleteFile(_downloadTmpFilename);
    }

    bool renameTempFileToBookFile() {
        return LVRenameFile(_downloadTmpFilename, _downloadFilename);
    }

    lString8 getDownloadDir() { return _downloadDir; }
    lString8 getDownloadFilename() { return _downloadFilename; }
    lString8 getDownloadTmpFilename() { return _downloadTmpFilename; }

    void downloadStarted(CRUIBookDownloadWidget * activeWidget) {
        if (activeWidget != this) {
            _caption->setState(CRUI::STATE_DISABLED, CRUI::STATE_DISABLED);
            _button->setState(CRUI::STATE_DISABLED, CRUI::STATE_DISABLED);
            _button->setBackgroundAlpha(0x80);
        } else {
            _progress->setProgress(0);
            setBookState(DOWNLOADING);
        }

    }

    void updateDirs(lString8 downloadsDir) {
        //
        lString8 subdir = UnicodeToUtf8(_book->getAuthorNames(false));
        subdir.trim();
        if (subdir.empty())
            subdir = "No Author";
        _downloadDir = downloadsDir + normalizeFilename(subdir);
        LVAppendPathDelimiter(_downloadDir);
        lString8 bookname = UnicodeToUtf8(_book->getTitle());
        bookname.trim();
        if (bookname.empty())
            bookname = _link->title;
        bookname.trim();
        if (bookname.empty())
            bookname = "noname"; // TODO: generate name
        bookname = normalizeFilename(bookname);
        lString8 extension = mimeToExtension(_link->type);
        bookname << '.';
        bookname << extension;
        _downloadFilename = _downloadDir + bookname;
        _downloadTmpFilename = _downloadDir + bookname + ".cr3.tmp";
        setBookState(LVFileExists(_downloadFilename) ? DOWNLOADED : NOT_DOWNLOADED);
    }

    void downloadCancelled(CRUIBookDownloadWidget * activeWidget) {
        if (activeWidget != this) {
            _caption->setState(0, CRUI::STATE_DISABLED);
            _button->setState(0, CRUI::STATE_DISABLED);
            _button->setBackgroundAlpha(0);
        } else {
            _progress->setProgress(-1);
            setBookState(NOT_DOWNLOADED);
            deleteTempFile();
        }
    }

    void downloadFinished(CRUIBookDownloadWidget * activeWidget) {
        if (activeWidget != this) {
            _caption->setState(0, CRUI::STATE_DISABLED);
            _button->setState(0, CRUI::STATE_DISABLED);
            _button->setBackgroundAlpha(0);
        } else {
            renameTempFileToBookFile();
            _progress->setProgress(-1);
            if (!LVFileExists(_downloadFilename))
                deleteTempFile();
            setBookState(LVFileExists(_downloadFilename) ? DOWNLOADED : NOT_DOWNLOADED);
        }
    }

    void downloadProgress(int progress) {
        _progress->setProgress(progress);
    }

    virtual bool onClick(CRUIWidget * widget) {
        CR_UNUSED(widget);
        if (_state == NOT_DOWNLOADED)
            _bookwidget->onDownloadButton(this);
        else if (_state == DOWNLOADING)
            _bookwidget->onCancelButton(this);
        else if (_state == DOWNLOADED)
            _bookwidget->onOpenButton(this);
        return true;
    }

    int getBookState() { return _state; }
    void setBookState(int state) {
        _state = state;
        switch (_state) {
        default:
        case NOT_DOWNLOADED:
            _caption->setText(_16(STR_OPDS_BOOK_DOWNLOAD) + L" " + _formatName);
            _button->setIcon("download");
            break;
        case DOWNLOADED:
            _caption->setText(_16(STR_OPDS_BOOK_OPEN) + L" " + _formatName);
            _button->setIcon("book");
            break;
        case DOWNLOADING:
            _caption->setText(_16(STR_OPDS_BOOK_DOWNLOADING) + L" " + _formatName + L"...");
            _button->setIcon("cancel");
            break;
        }
    }

    CRUIBookDownloadWidget(CRUIOpdsBookWidget * bookwidget, CROpdsCatalogsItem * book, OPDSLink * link) : _bookwidget(bookwidget), _book(book), _link(link), _state(NOT_DOWNLOADED) {
        _formatName = mimeToFormatName(link->type);
        _button = new CRUIImageButton("download", "BUTTON");
        _button->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        _button->setPadding(lvRect(PT_TO_PX(1), PT_TO_PX(1), PT_TO_PX(1), PT_TO_PX(1)));
        _button->setOnClickListener(this);
        _button->setMinHeight(MIN_ITEM_PX * 3 / 4);
        _button->setMinWidth(MIN_ITEM_PX * 3 / 4);
        _button->setMaxHeight(MIN_ITEM_PX);
        _button->setMaxWidth(MIN_ITEM_PX);
        _button->setAlign(ALIGN_CENTER);

        _caption = new CRUITextWidget(lString16("Download ") + _formatName);
        _caption->setMaxLines(2);
        _caption->setFontSize(FONT_SIZE_LARGE);
        _caption->setAlign(ALIGN_LEFT | ALIGN_VCENTER);
        _caption->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _caption->setMargin(PT_TO_PX(1));
        _caption->setStyle("BUTTON_CAPTION");
        CRUIProgressWidget * _progress0 = new CRUIProgressWidget();
        _progress = new CRUIProgressWidget();
        _progress->setMargin(PT_TO_PX(2));
        _progress0->setMargin(PT_TO_PX(2));
        //_progress->setProgress(5000);
        addChild(_button);
        CRUIVerticalLayout * layout = new CRUIVerticalLayout();
        CRUIWidget * spacer1 = new CRUIWidget(); spacer1->setLayoutParams(FILL_PARENT, FILL_PARENT);
        CRUIWidget * spacer2 = new CRUIWidget(); spacer2->setLayoutParams(FILL_PARENT, FILL_PARENT);
        layout->setMaxHeight(MIN_ITEM_PX);
        layout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        layout->addChild(spacer1);
        layout->addChild(_progress0);
        layout->addChild(_caption);
        layout->addChild(_progress);
        layout->addChild(spacer2);
        //layout->setBackground(0xC0808080);
        layout->setMargin(1);
        //_caption->setBackground(0xC0000080);
        addChild(layout);
        setBookState(LVFileExists(_downloadFilename) ? DOWNLOADED : NOT_DOWNLOADED);
        setBackgroundAlpha(0x40);
    }
};

lString16 convertDescription(lString8 description) {
    return Utf8ToUnicode(description);
}

class CRUIOPDSLinkWidget : public CRUIHorizontalLayout, public CRUIOnClickListener {
    CRUIOpdsBookWidget * _bookwidget;
    CROpdsCatalogsItem * _book;
    OPDSLink * _link;
    CRUIImageButton * _button;
    CRUITextWidget * _caption;
public:
    virtual bool onClick(CRUIWidget * widget) {
        CR_UNUSED(widget);
        _bookwidget->onAdditionalLinkButton(_link);
        return true;
    }


    CRUIOPDSLinkWidget(CRUIOpdsBookWidget * bookwidget, CROpdsCatalogsItem * book, OPDSLink * link) : _bookwidget(bookwidget), _book(book), _link(link) {
        _button = new CRUIImageButton("forward", "BUTTON");
        _button->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        _button->setPadding(lvRect(PT_TO_PX(1), PT_TO_PX(1), PT_TO_PX(1), PT_TO_PX(1)));
        _button->setOnClickListener(this);
        _button->setMinHeight(MIN_ITEM_PX * 3 / 4);
        _button->setMinWidth(MIN_ITEM_PX * 3 / 4);
        _button->setMaxHeight(MIN_ITEM_PX);
        _button->setMaxWidth(MIN_ITEM_PX);
        _button->setAlign(ALIGN_CENTER);

        _caption = new CRUITextWidget(convertDescription(_link->title));
        _caption->setMaxLines(2);
        _caption->setFontSize(FONT_SIZE_LARGE);
        _caption->setAlign(ALIGN_LEFT | ALIGN_VCENTER);
        _caption->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _caption->setMargin(PT_TO_PX(1));
        _caption->setStyle("BUTTON_CAPTION");
        addChild(_button);
        addChild(_caption);
        //layout->setBackground(0xC0808080);
        setBackgroundAlpha(0x40);
    }
};



class CRUIDocView;
class CRUIRichTextWidget : public CRUIWidget {
protected:
    CRUIDocView * _docview;
    lUInt32 _lastPropsHash;
    lvPoint _lastSize;
    LVRendPageList _pageList;
public:
    CRUIRichTextWidget() {
        _docview = new CRUIDocView();
        lString16 sample = _16(STR_SETTINGS_FONT_SAMPLE_TEXT);
        LVArray<int> fontSizes;
        for (int i = crconfig.minFontSize; i <= crconfig.maxFontSize; i++)
            fontSizes.add(i);
        _docview->setFontSizes(fontSizes, false);
        _docview->setViewMode(DVM_SCROLL, 1);
    }

    virtual ~CRUIRichTextWidget() {
        delete _docview;
    }

    void setPlainText(lString16 text) {
        _docview->createDefaultDocument(lString16(), text);
        requestLayout();
    }

    void setHtml(lString16 text) {
        _docview->createHtmlDocument(text);
        requestLayout();
    }

    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight) {
        lvRect padding;
        getPadding(padding);
        lvRect margins;
        getMargin(margins);
        if (baseHeight == UNSPECIFIED)
            baseHeight = baseWidth;
        int dx = baseWidth - padding.left - padding.right - margins.left - margins.right;
        int dy = baseHeight - padding.top - padding.bottom - margins.top - margins.bottom;
        _docview->setFontSize(currentTheme->getFontSize(getFontSize()));
        _docview->Resize(dx, dy);
        _docview->Render(dx, dy, &_pageList);
        defMeasure(baseWidth, baseHeight, dx, _docview->GetFullHeight());
    }

    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf) {
        CRUIWidget::draw(buf);
        lvRect rc = _pos;
        LVDrawStateSaver saver(*buf);
        CR_UNUSED(saver);
        setClipRect(buf, rc);
        applyMargin(rc);
        lvRect margins;
        getPadding(margins);
        _docview->setTextColor(getTextColor());
        _docview->setBackgroundColor(0xFF000000);
        buf->SetBackgroundColor(0xFF000000);
        buf->SetTextColor(getTextColor());
//        _docview->Draw(*buf, false);
//        _docview->drawPageTo(buf, *page, &rc, 1, 0);
        ldomMarkedRangeList marks;
        ldomMarkedRangeList bookmarks;
        DrawDocument(*buf, _docview->getDocument()->getRootNode(), rc.left, rc.top, rc.width(), rc.height(), 0, 0, 0, &marks,
                           &bookmarks);
    }

    void format() {
        _docview->Render(0, 0, &_pageList);
    }
};

/// download result
void CRUIOpdsBookWidget::onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream) {
    CRLog::trace("onDownloadResult task=%d url=%s result=%d resultMessage=%s, totalSize=%d", downloadTaskId, url.c_str(), result, result ? resultMessage.c_str() : "", size);
    CR_UNUSED(mimeType);
    if (stream.isNull()) {
        CRLog::trace("No data received");
    } else {
        CRLog::trace("Stream size is %d", (int)stream->GetSize());
    }
    if (downloadTaskId == _coverTaskId) {
        LVStreamRef nullref;
        coverPageManager->setExternalImage(_coverTaskBook, result == 0 ? stream : nullref);
        _coverTaskId = 0;
        if (_coverTaskBook)
            delete _coverTaskBook;
        _coverTaskBook = NULL;
    } else if (downloadTaskId == _currentDownloadTaskId) {
        CRLog::debug("Book download is finished with result=%d", result);
        _currentDownloadTaskId = 0;
        for (int i = 0; i < _downloads.length(); i++) {
            if (result == 0)
                _downloads[i]->downloadFinished(_currentDownload);
            else
                _downloads[i]->downloadCancelled(_currentDownload);
        }
        _currentDownload = NULL;
        _main->update(true);
    } else {
        CRLog::warn("Download finished from unknown downloadTaskId %d", downloadTaskId);
    }
}

/// download progress
void CRUIOpdsBookWidget::onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded) {
    CR_UNUSED3(result, resultMessage, mimeType);
    if (_currentDownload && downloadTaskId == _currentDownloadTaskId) {
        int progress = (int)(size > 0 ? (lInt64)sizeDownloaded * 10000 / size : 0);
        CRLog::trace("onDownloadProgress task=%d url=%s bytesRead=%d totalSize=%d   %d.%02d%%", downloadTaskId, url.c_str(), sizeDownloaded, size, progress / 100, progress % 100);
        _currentDownload->downloadProgress(progress);
        _main->update(true);
    }
}

/// call to schedule download of image
bool CRUIOpdsBookWidget::onRequestImageDownload(CRDirEntry * book) {
    book = book->clone();
    CRLog::trace("onRequestImageDownload(%s)", book->getCoverPathName().c_str());
    if (!_coverTaskId) {
        _coverTaskBook = book;
        _coverTaskId = getMain()->openUrl(this, book->getCoverPathName(), lString8("GET"), lString8(_book->getCatalog()->login.c_str()), lString8(_book->getCatalog()->password.c_str()), lString8());
        return _coverTaskId != 0;
    }
    return true;
}

void CRUIOpdsBookWidget::cancelDownloads() {
    if (_coverTaskId) {
        _main->cancelDownload(_coverTaskId);
        if (_coverTaskBook) {
            coverPageManager->cancel(_coverTaskBook, 0, 0);
            delete _coverTaskBook;
        }
        _coverTaskId = 0;
        _coverTaskBook = NULL;
    }
}

void CRUIOpdsBookWidget::afterNavigationTo() {
    _downloadFolder = bookDB->getDownloadsDir();
    if (_downloadFolder.empty()) {
        CRLog::warn("Download folder is not specified! Downloads disabled.");
    }
}

void CRUIOpdsBookWidget::afterNavigationFrom() {
    cancelDownloads();
}


void CRUIOpdsBookWidget::updateDirs() {
    _downloadFolder = bookDB->getDownloadsDir();
    LVAppendPathDelimiter(_downloadFolder);
    if (_downloadFolder.empty()) {
        CRLog::warn("Download folder is not specified! Downloads disabled.");
    }
    for (int i = 0; i < _downloads.length(); i++) {
        _downloads[i]->updateDirs(_downloadFolder);
    }
}

CRUIOpdsBookWidget::CRUIOpdsBookWidget(CRUIMainWidget * main, LVClonePtr<CROpdsCatalogsItem> & book)
    : CRUIWindowWidget(main)
    , _title(NULL)
    , _book(book)
    , _cover(NULL)
    , _caption(NULL)
    , _authors(NULL)
    , _description(NULL)
    , _buttonsTable(NULL)
    , _coverTaskId(0)
    , _coverTaskBook(NULL)
    , _currentDownload(NULL)
    , _currentDownloadTaskId(0)
{
    CRLog::trace("CRUIOpdsBookWidget::CRUIOpdsBookWidget %08x", (lUInt64)this);

    updateDirs();

    _title = new CRUITitleBarWidget(lString16(""), this, this, true);
    _title->setTitle(Utf8ToUnicode(_book->getCatalog()->name.c_str()));
    _body->addChild(_title);



    CRUIScrollWidget * scroll = new CRUIScrollWidget(true);
    scroll->setLayoutParams(FILL_PARENT, FILL_PARENT);

    CRUILinearLayout * toplayout = new CRUIHorizontalLayout();
    toplayout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    CRUILinearLayout * rlayout = new CRUIVerticalLayout();
    rlayout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _cover = new CRCoverWidget(getMain(), book->clone(), 200, 200, this);
    _caption = new CRUITextWidget(book->getTitle());
    _caption->setFontSize(FONT_SIZE_LARGE);
    _caption->setAlign(ALIGN_CENTER);
    _caption->setMaxLines(2);
    _authors = new CRUITextWidget(book->getAuthorNames(false));
    _authors->setFontSize(FONT_SIZE_LARGE);
    _authors->setAlign(ALIGN_CENTER);
    _authors->setMaxLines(2);
    rlayout->addChild(_authors);
    rlayout->addChild(_caption);
    rlayout->setMargin(PT_TO_PX(3));
    toplayout->addChild(_cover);
    toplayout->addChild(rlayout);
    toplayout->setMargin(PT_TO_PX(3));

    scroll->addChild(toplayout);

    _description = new CRUIRichTextWidget();
    _description->setMargin(PT_TO_PX(3));
    _description->setFontSize(FONT_SIZE_SMALL);
    if (_book->getDescriptionType() == "text/html")
        _description->setHtml(_book->getDescription());
    else
        _description->setPlainText(_book->getDescription());
    scroll->addChild(_description);




    _buttonsTable = new CRUITableLayout(2);
    for (int i = 0; i < _book->getLinks().length(); i++) {
        OPDSLink * link = _book->getLinks()[i];
        if (link->rel.startsWith("http://opds-spec.org/acquisition") ||
                (link->rel == "alternate" && (link->type == "text/html" || link->type == "text"))) {
            CRUIBookDownloadWidget * widget = new CRUIBookDownloadWidget(this, _book.get(), link);
            _downloads.add(widget);
            _buttonsTable->addChild(widget);
        }
    }

    scroll->addChild(_buttonsTable);

    for (int i = 0; i < _book->getLinks().length(); i++) {
        OPDSLink * link = _book->getLinks()[i];
        if (link->type.startsWith("application/atom+xml") && !link->title.empty()) {
            CRUIOPDSLinkWidget * widget = new CRUIOPDSLinkWidget(this, _book.get(), link);
            scroll->addChild(widget);
        }
    }

    updateDirs();

    _body->addChild(scroll);
    setStyle("SETTINGS_ITEM_LIST");
}

void CRUIOpdsBookWidget::updateCoverSize(int baseHeight) {
    /// calculate cover widget size
    if (!_cover)
        return;
    int coverH = baseHeight / 3;
    _cover->setSize(coverH * 3 / 4, coverH);
    int itemw = coverH; // + itempadding.left + itempadding.right + itemmargin.left +itemmargin.right;
    _cover->setMinWidth(itemw);
    _cover->setMaxWidth(itemw);
}

void CRUIOpdsBookWidget::draw(LVDrawBuf * buf) {
    //CRLog::trace("CRUIOpdsBookWidget::draw %08x", (lUInt64)this);
    CRUIWindowWidget::draw(buf);
}

/// measure dimensions
void CRUIOpdsBookWidget::measure(int baseWidth, int baseHeight) {
    //CRLog::trace("CRUIOpdsBookWidget::measure %08x", (lUInt64)this);
    updateCoverSize(baseHeight);
    /// measure
    CRUIWindowWidget::measure(baseWidth, baseHeight);
    //CRLog::trace("CRUIOpdsBookWidget::measure %08x exit", (lUInt64)this);
}

/// updates widget position based on specified rectangle
void CRUIOpdsBookWidget::layout(int left, int top, int right, int bottom) {
    //CRLog::trace("CRUIOpdsBookWidget::layout %08x", (lUInt64)this);
    CRUIWindowWidget::layout(left, top, right, bottom);
    //CRLog::trace("CRUIOpdsBookWidget::layout %08x exit", (lUInt64)this);
}

bool CRUIOpdsBookWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    else if (widget->getId() == "MENU") {
        onAction(CMD_MENU);
    }
    return true;
}


bool CRUIOpdsBookWidget::onLongClick(CRUIWidget * widget) {
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
bool CRUIOpdsBookWidget::onAction(const CRUIAction * action) {
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    case CMD_MENU:
    {
//        CRUIActionList actions;
//        actions.add(ACTION_OPDS_CATALOG_OPEN);
//        actions.add(ACTION_OPDS_CATALOG_REMOVE);
//        actions.add(ACTION_OPDS_CATALOG_CANCEL_CHANGES);
//        lvRect margins;
//        //margins.right = MIN_ITEM_PX * 120 / 100;
//        showMenu(actions, ALIGN_TOP, margins, false);
        return true;
    }
    }
    return false;
}

CRUIOpdsBookWidget::~CRUIOpdsBookWidget()
{
    cancelDownloads();
}

bool CRUIOpdsBookWidget::onKeyEvent(const CRUIKeyEvent * event) {
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

void CRUIOpdsBookWidget::beforeNavigationFrom() {
//    if (!_catalog)
//        return;
//    save();
}

/// motion event handler, returns true if it handled event
bool CRUIOpdsBookWidget::onTouchEvent(const CRUIMotionEvent * event) {
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

void CRUIOpdsBookWidget::onDownloadButton(CRUIBookDownloadWidget * control) {
    if (_currentDownload)
        return;

    if (_downloadFolder.empty()) {
        CRLog::warn("Download folder is not specified! Downloads disabled.");
        _main->showMessage(_16(STR_ERROR_DOWNLOADS_DIRECTORY_NOT_SET), 4000);
        return;
    }

    if (!control->createDownloadDir()) {
        CRLog::error("Cannot write to download directory.");
        _main->showMessage(_16(STR_ERROR_DOWNLOADS_DIRECTORY_NOT_WRITABLE), 4000);
        return;
    }

    _currentDownloadTaskId = _main->openUrl(this, control->getLink()->href, lString8("GET"),
            lString8(_book->getCatalog()->login.c_str()), lString8(_book->getCatalog()->password.c_str()), control->getDownloadTmpFilename());
    if (_currentDownloadTaskId) {
        _currentDownload = control;
        for (int i = 0; i < _downloads.length(); i++)
            _downloads[i]->downloadStarted(control);
    } else {
        CRLog::error("Cannot start download.");
        _main->showMessage(_16(STR_ERROR_DOWNLOADS_CANNOT_START_DOWNLOAD), 4000);
    }
}

void CRUIOpdsBookWidget::onCancelButton(CRUIBookDownloadWidget * control) {
    if (!_currentDownload)
        return;
    _main->cancelDownload(_currentDownloadTaskId);
    for (int i = 0; i < _downloads.length(); i++)
        _downloads[i]->downloadCancelled(control);
    _currentDownload = NULL;
    _currentDownloadTaskId = 0;
}

void CRUIOpdsBookWidget::onAdditionalLinkButton(OPDSLink * link) {
    _main->showOpds(_book->getCatalog(), link->href, Utf8ToUnicode(link->title));
}


void CRUIOpdsBookWidget::onOpenButton(CRUIBookDownloadWidget * control) {
    lString8 fn = control->getDownloadFilename();
    if (LVFileExists(fn)) {
        _main->openBookFromFile(fn);
    } else {
        _main->showMessage(lString16("Cannot open book file"), 3000);
    }
}
