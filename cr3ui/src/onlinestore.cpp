#include "onlinestore.h"

#include "crui.h"
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




//================================================================================================
// online store book list
//================================================================================================

void CRUIOpdsItemListWidget::setScrollOffset(int offset) {
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

void CRUIOpdsItemListWidget::setProgressItemVisible(bool showProgress) {
    if (_showProgressAsLastItem != showProgress) {
        _showProgressAsLastItem = showProgress;
        requestLayout();
        _parent->getMain()->update(true);
    }
}

bool CRUIOpdsItemListWidget::isAnimating() {
    return _showProgressAsLastItem && isItemVisible(_dir->itemCount());
}

void CRUIOpdsItemListWidget::animate(lUInt64 millisPassed) {
    _progressWidget->animate(millisPassed);
}

void CRUIOpdsItemListWidget::calcCoverSize(int dx, int dy) {
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
void CRUIOpdsItemListWidget::measure(int baseWidth, int baseHeight) {
    calcCoverSize(baseWidth, baseHeight);
    setColCount(baseWidth > baseHeight ? 2 : 1);
    CRUIListWidget::measure(baseWidth, baseHeight);
}

/// updates widget position based on specified rectangle
void CRUIOpdsItemListWidget::layout(int left, int top, int right, int bottom) {
    CRUIListWidget::layout(left, top, right, bottom);
}

CRUIOpdsItemListWidget::CRUIOpdsItemListWidget(CRUIOnlineStoreWidget * parent) : CRUIListWidget(true), _dir(NULL), _parent(parent), _showProgressAsLastItem(false) {
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

int CRUIOpdsItemListWidget::getItemCount() {
    if (!_dir)
        return 0;
    //CRLog::trace("item count is %d", _dir->itemCount());
    return _dir->itemCount() + (_showProgressAsLastItem ? 1 : 0);
}

CRUIWidget * CRUIOpdsItemListWidget::getItemWidget(int index) {
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

void CRUIOpdsItemListWidget::setDirectory(CRDirContentItem * dir)
{
    _dir = dir;
    //dir->sort(BY_TITLE);
    requestLayout();
}

/// return true if drag operation is intercepted
bool CRUIOpdsItemListWidget::startDragging(const CRUIMotionEvent * event, bool vertical) {
    return _parent->getMain()->startDragging(event, vertical);
}

/// returns true if all coverpages are available, false if background tasks are submitted
bool CRUIOpdsItemListWidget::requestAllVisibleCoverpages() {
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




//================================================================================================
// online store window
//================================================================================================


CRUIOnlineStoreWidget::CRUIOnlineStoreWidget(CRUIMainWidget * main)
    : CRUIWindowWidget(main), _title(NULL)
  , _catalog(NULL), _dir(NULL), _requestId(0), _coverTaskId(0), _coverTaskBook(NULL)
{
    _title = new CRUITitleBarWidget(lString16("File list"), this, this, true);
    _body->addChild(_title);
    _fileList = new CRUIOpdsItemListWidget(this);
    _body->addChild(_fileList);
    _fileList->setOnItemClickListener(this);
    setDefaultWidget(_fileList);
}

CRUIOnlineStoreWidget::~CRUIOnlineStoreWidget() {
    cancelDownloads();
    if (_dir)
        _dir->unlock();
}

/// returns true if all coverpages are available, false if background tasks are submitted
bool CRUIOnlineStoreWidget::requestAllVisibleCoverpages() {
    return false;
}

/// motion event handler, returns true if it handled event
bool CRUIOnlineStoreWidget::onTouchEvent(const CRUIMotionEvent * event) {
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

bool CRUIOnlineStoreWidget::onKeyEvent(const CRUIKeyEvent * event) {
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

bool CRUIOnlineStoreWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    else if (widget->getId() == "MENU") {
        onAction(CMD_MENU);
    }
    return true;
}

bool CRUIOnlineStoreWidget::onLongClick(CRUIWidget * widget) {
    if (widget->getId() == "MENU") {
        onAction(CMD_SETTINGS);
    }
    return true;
}

/// call to schedule download of image
bool CRUIOnlineStoreWidget::onRequestImageDownload(CRDirEntry * book) {
    book = book->clone();
    CRLog::trace("onRequestImageDownload(%s)", book->getCoverPathName().c_str());
    if (!_coverTaskId) {
        fetchCover(book);
    } else {
        _coversToLoad.add(book);
    }
    return true;
}

void CRUIOnlineStoreWidget::cancelDownloads() {
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

void CRUIOnlineStoreWidget::afterNavigationFrom() {
    cancelDownloads();
}

void CRUIOnlineStoreWidget::afterNavigationTo() {
    CRUIWindowWidget::afterNavigationTo();
}

bool CRUIOnlineStoreWidget::fetchCover(CRDirEntry * book) {
    _coverTaskBook = book;
    _coverTaskId = getMain()->openUrl(this, book->getCoverPathName(), lString8("GET"), lString8(_catalog->login.c_str()), lString8(_catalog->password.c_str()), lString8());
    return _coverTaskId != 0;
}

void CRUIOnlineStoreWidget::fetchNextPart() {
    // override to implement
}

/// download result
void CRUIOnlineStoreWidget::onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream) {

}

/// download progress
void CRUIOnlineStoreWidget::onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded) {

}

/// handle menu or other action
bool CRUIOnlineStoreWidget::onAction(const CRUIAction * action) {
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
    return CRUIWindowWidget::onAction(action);
}

void CRUIOnlineStoreWidget::setDirectory(LVClonePtr<BookDBCatalog> & catalog, CRDirContentItem * dir)
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

bool CRUIOnlineStoreWidget::onListItemClick(CRUIListWidget * widget, int index) {
    CR_UNUSED2(widget, index);
    return false;

}

void CRUIOnlineStoreWidget::openSearchResults(lString16 pattern) {
    CR_UNUSED(pattern);
}

void CRUIOnlineStoreWidget::showSearchPopup() {
}

