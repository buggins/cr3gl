
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
    if (mime.startsWith("application/rtf"))
        return lString16(L"RTF");
    return lString16("Unknown");
}

enum {
    NOT_DOWNLOADED,
    DOWNLOADING,
    DOWNLOADED
};
class CRUIBookDownloadWidget : public CRUIVerticalLayout {
    CRUIOpdsBookWidget * _bookwidget;
    CROpdsCatalogsItem * _book;
    OPDSLink * _link;
    lString16 _formatName;
    CRUIButton * _button;
    CRUIProgressWidget * _progress;
    int _state;
public:
    int getBookState() { return _state; }
    void setBookState(int state) {
        _state = state;
        switch (_state) {
        default:
        case NOT_DOWNLOADED:
            _button->setText(lString16("Download ") + _formatName);
            _button->setIcon("downloads");
            break;
        case DOWNLOADED:
            _button->setText(lString16("Open ") + _formatName);
            _button->setIcon("book");
            break;
        case DOWNLOADING:
            _button->setText(lString16("Cancel download"));
            _button->setIcon("cancel");
            break;
        }
    }

    CRUIBookDownloadWidget(CRUIOpdsBookWidget * bookwidget, CROpdsCatalogsItem * book, OPDSLink * link) : _bookwidget(bookwidget), _book(book), _link(link), _state(NOT_DOWNLOADED) {
        _formatName = mimeToFormatName(link->type);
        _button = new CRUIButton(lString16("Download ") + _formatName, "downloads");
        _button->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _button->setMaxLines(2);
        _button->setMargin(lvRect(PT_TO_PX(2), PT_TO_PX(2), PT_TO_PX(2), PT_TO_PX(0)));
        addChild(_button);
        _progress = new CRUIProgressWidget();
        _progress->setMargin(lvRect(PT_TO_PX(4), PT_TO_PX(0), PT_TO_PX(4), PT_TO_PX(3)));
        _progress->setProgress(5000);
        addChild(_progress);
        setBookState(NOT_DOWNLOADED);
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
    CRLog::trace("onDownloadProgress task=%d url=%s result=%d resultMessage=%s, totalSize=%d", downloadTaskId, url.c_str(), result, resultMessage.c_str(), size);
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
    } else {
        CRLog::warn("Download finished from unknown downloadTaskId %d", downloadTaskId);
    }
}

/// download progress
void CRUIOpdsBookWidget::onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded) {
    CR_UNUSED3(result, resultMessage, mimeType);
    CRLog::trace("onDownloadProgress task=%d url=%s bytesRead=%d totalSize=%d", downloadTaskId, url.c_str(), sizeDownloaded, size);
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

void CRUIOpdsBookWidget::afterNavigationFrom() {
    cancelDownloads();
}


CRUIOpdsBookWidget::CRUIOpdsBookWidget(CRUIMainWidget * main, CROpdsCatalogsItem * book)
    : CRUIWindowWidget(main)
    , _title(NULL)
    , _book(NULL)
    , _coverTaskId(0)
    , _coverTaskBook(NULL)
{
    _book = new CROpdsCatalogsItem(*book);
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
    _description = new CRUIRichTextWidget();
    _description->setMargin(PT_TO_PX(3));
    _description->setFontSize(FONT_SIZE_SMALL);
    if (_book->getDescriptionType() == "text/html")
        _description->setHtml(_book->getDescription());
    else
        _description->setPlainText(_book->getDescription());
    scroll->addChild(toplayout);
    scroll->addChild(_description);
//    CRUIButton * button = new CRUIButton(lString16("Download EPUB"));
//    scroll->addChild(button);
//    button = new CRUIButton(lString16("Download FB2"));
//    scroll->addChild(button);
//    button = new CRUIButton(lString16("Download RTF"));
//    scroll->addChild(button);
    _body->addChild(scroll);

//    //_fileList = new CRUIOpdsItemListWidget(this);
//    //_body->addChild(_fileList);
//    //_fileList->setOnItemClickListener(this);
//    CRUITableLayout * layout = new CRUITableLayout(2);
//    layout->setLayoutParams(FILL_PARENT, FILL_PARENT);

//    // add edit boxes
//    CRUITextWidget * label = new CRUITextWidget(STR_OPDS_CATALOG_NAME);
//    _edTitle = new CRUIEditWidget();
//    label->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
//    _edTitle->setText(Utf8ToUnicode(_book->getTitle()));
//    _edTitle->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
//    layout->addChild(label);
//    layout->addChild(_edTitle);
//    label = new CRUITextWidget(STR_OPDS_CATALOG_URL);
//    label->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
//    _edUrl = new CRUIEditWidget();
//    _edUrl->setText(Utf8ToUnicode(_catalog->url.c_str()));
//    _edUrl->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
//    layout->addChild(label);
//    layout->addChild(_edUrl);
//    label = new CRUITextWidget(STR_OPDS_CATALOG_LOGIN);
//    label->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
//    _edLogin = new CRUIEditWidget();
//    _edLogin->setText(Utf8ToUnicode(_catalog->login.c_str()));
//    _edLogin->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
//    layout->addChild(label);
//    layout->addChild(_edLogin);
//    label = new CRUITextWidget(STR_OPDS_CATALOG_PASSWORD);
//    label->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
//    _edPassword = new CRUIEditWidget();
//    _edPassword->setText(Utf8ToUnicode(_catalog->password.c_str()));
//    _edPassword->setPasswordChar('*');
//    _edPassword->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
//    layout->addChild(label);
//    layout->addChild(_edPassword);
//    layout->setPadding(PT_TO_PX(3));

//    // add spacers
//    CRUIWidget * spacer = new CRUIWidget();
//    spacer->setLayoutParams(WRAP_CONTENT, FILL_PARENT);
//    layout->addChild(spacer);
//    spacer = new CRUIWidget();
//    spacer->setLayoutParams(FILL_PARENT, FILL_PARENT);
//    layout->addChild(spacer);

    //layout->

    setStyle("SETTINGS_ITEM_LIST");

    _buttonsTable = new CRUITableLayout(2);
    //_body->addChild(layout);
    for (int i = 0; i < _book->getLinks().length(); i++) {
        OPDSLink * link = _book->getLinks()[i];
        if (link->rel.startsWith("http://opds-spec.org/acquisition") ||
                (link->rel == "alternate" && (link->type == "text/html" || link->type == "text"))) {
            CRUIBookDownloadWidget * widget = new CRUIBookDownloadWidget(this, _book, link);
            _downloads.add(widget);
            _buttonsTable->addChild(widget);
        }
    }
    scroll->addChild(_buttonsTable);
}

void CRUIOpdsBookWidget::updateCoverSize(int baseHeight) {
    /// calculate cover widget size
    int coverH = baseHeight / 3;
    _cover->setSize(coverH * 3 / 4, coverH);
    int itemw = coverH; // + itempadding.left + itempadding.right + itemmargin.left +itemmargin.right;
    _cover->setMinWidth(itemw);
    _cover->setMaxWidth(itemw);
}

/// measure dimensions
void CRUIOpdsBookWidget::measure(int baseWidth, int baseHeight) {
    updateCoverSize(baseHeight);
    /// measure
    CRUIWindowWidget::measure(baseWidth, baseHeight);
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
    if (_book)
        delete _book;
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

