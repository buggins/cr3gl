/*
 * cruifolderwidget.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */


#include "crui.h"
#include "cruifolderwidget.h"
#include "cruilist.h"
#include "cruicontrols.h"
#include "crcoverpages.h"
#include "cruimain.h"
#include "cruicoverwidget.h"
#include "stringresource.h"

using namespace CRUI;


lString16 makeDirName(const CRDirEntry * entry, bool shortForm) {
    DIR_TYPE type = entry->getDirType();
    if (type == DIR_TYPE_BOOKS_BY_AUTHOR || type == DIR_TYPE_BOOKS_BY_FILENAME
             || type == DIR_TYPE_BOOKS_BY_TITLE || type == DIR_TYPE_BOOKS_BY_SERIES) {
        lString16 pattern = entry->getFilterString();
        lString16 title;
        switch(type) {
        case DIR_TYPE_BOOKS_BY_TITLE:
            title = _16(STR_BOOKS_BY_TITLE);
            break;
        case DIR_TYPE_BOOKS_BY_FILENAME:
            title = _16(STR_BOOKS_BY_FILENAME);
            break;
        case DIR_TYPE_BOOKS_BY_SERIES:
            title = _16(STR_BOOKS_BY_SERIES);
            break;
        case DIR_TYPE_BOOKS_BY_AUTHOR:
        default:
            title = _16(STR_BOOKS_BY_AUTHOR);
            break;
        }
        lString16 suffix;
        if (pattern.endsWith("%")) {
            suffix = L"...";
            pattern = pattern.substr(0, pattern.length() - 1);
        }
        if (shortForm && !pattern.empty())
            return pattern + suffix;
        if (!pattern.empty()) {
            title += L": ";
            title += pattern + suffix;
        }
        return title;
    } else {
        return Utf8ToUnicode(shortForm ? entry->getFileName() : entry->getPathName());
    }
}

class CoverReadyCallback : public CRRunnable {
    CRUIWidget * _callbackWidget;
public:
    CoverReadyCallback(CRUIWidget * callbackWidget) : _callbackWidget(callbackWidget) {}
    virtual void run() {
        CRLog::trace("Cover page ready callback is called");
        // TODO: safety fix
        _callbackWidget->invalidate();
    }
};

class CRUIDirItemWidget : public CRUILinearLayout {
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
    CRUIDirItemWidget(int iconDx, int iconDy, const char * iconRes) : CRUILinearLayout(false), _iconDx(iconDx), _iconDy(iconDy), _entry(NULL) {
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

class CRUIBookItemWidget : public CRUILinearLayout {
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
    CRUIBookItemWidget(int iconDx, int iconDy, CRUIMainWidget * callback) : CRUILinearLayout(false), _iconDx(iconDx), _iconDy(iconDy), _entry(NULL) {
        _icon = new CRCoverWidget(callback, NULL, iconDx, iconDy);
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

static lString16 sizeToString(int size) {
	if (size < 4096)
		return lString16::itoa(size);
	else if (size < 4096*1024)
		return lString16::itoa(size / 1024) + L"K";
	else
		return lString16::itoa(size / 1024 / 1024) + L"M";
}

class CRUIFileListWidget : public CRUIListWidget {
protected:
    CRDirContentItem * _dir;
    CRUIDirItemWidget * _folderWidget;
    CRUIBookItemWidget * _bookWidget;
    CRUIFolderWidget * _parent;
    int _coverDx;
    int _coverDy;
public:
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

    CRUIFileListWidget(CRUIFolderWidget * parent) : CRUIListWidget(true), _dir(NULL), _parent(parent) {
        setId("FILE_LIST");
		setLayoutParams(FILL_PARENT, FILL_PARENT);
		//setBackground("tx_wood_v3.jpg");
        calcCoverSize(deviceInfo.shortSide, deviceInfo.longSide);
        _folderWidget = new CRUIDirItemWidget(_coverDx, _coverDy, "folder");
        _bookWidget = new CRUIBookItemWidget(_coverDx, _coverDy, parent->getMain());
		setStyle("FILE_LIST");
        setColCount(2);
	}
	virtual int getItemCount() {
		if (!_dir)
			return 0;
		//CRLog::trace("item count is %d", _dir->itemCount());
		return _dir->itemCount();
	}
	virtual CRUIWidget * getItemWidget(int index) {
		CRDirEntry * item = _dir->getItem(index);
		if (item->isDirectory()) {
            CRUIDirItemWidget * res = _folderWidget;
            res->setDir(item, _coverDx, _coverDy);
            res->_line1->setText(L"");
            res->_line2->setText(makeDirName(item, true));
            if (item->getBookCount()) {
                lString16 info = _16(STR_BOOK_COUNT);
                res->_line3->setText(info + lString16::itoa(item->getBookCount()));
            } else {
                res->_line3->setText(L"");
            }
			//CRLog::trace("returning folder item");
			return res;
		} else {
            CRUIBookItemWidget * res = _bookWidget;
            res->setBook(item, _coverDx, _coverDy);
			BookDBBook * book = item->getBook();
			lString16 text1;
			lString16 text2;
			lString16 text3;
			lString16 text4;
			if (book) {
				text2 = Utf8ToUnicode(book->title.c_str());
                text1 = item->getAuthorNames(false);
                text3 = item->getSeriesName(true);
				text4 = sizeToString(book->filesize);
				text4 += " ";
				text4 += LVDocFormatName(book->format);
			} else {
				text2 = Utf8ToUnicode(item->getFileName());
			}
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
		dir->sort(BY_TITLE);
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

void CRUIFolderWidget::setDirectory(CRDirContentItem * dir)
{
    if (_dir)
        _dir->unlock();
	_dir = dir;
    if (_dir)
        _dir->lock();
    _title->setTitle(makeDirName(dir, false));
	_fileList->setDirectory(dir);
	requestLayout();
}

CRUIFolderWidget::CRUIFolderWidget(CRUIMainWidget * main) : CRUIWindowWidget(main), _title(NULL), _fileList(NULL), _dir(NULL)
{
    _title = new CRUITitleBarWidget(lString16("File list"), this, this, true);
	_body->addChild(_title);
    _fileList = new CRUIFileListWidget(this);
	_body->addChild(_fileList);
    _fileList->setOnItemClickListener(this);
    setDefaultWidget(_fileList);
}

bool CRUIFolderWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    else if (widget->getId() == "MENU") {
        onAction(CMD_MENU);
    }
    return true;
}

bool CRUIFolderWidget::onLongClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK") {
        CRUIActionList actions;
        lString8 path = _dir->getPathName();
        lString8 lastPath = path;
        for (;;) {
            LVRemovePathDelimiter(path);
            path = LVExtractPath(path);
            if (path == lastPath)
                break;
            LVRemovePathDelimiter(path);
            CRUIAction action(CMD_SHOW_FOLDER);
            action.icon_res = "folder_icon";
            action.name = Utf8ToUnicode(path);
            action.sparam = path;
            actions.add(&action);
            lastPath = path;
            if (path=="/" || path.endsWith(":\\") || path.endsWith("\\\\") || path == "@/" || path == "@\\")
                break;
        }
        actions.add(ACTION_CURRENT_BOOK);
        actions.add(ACTION_READER_HOME);
        lvRect margins;
        //margins.right = MIN_ITEM_PX * 120 / 100;
        showMenu(actions, ALIGN_TOP, margins, false);
    } else if (widget->getId() == "MENU") {
        onAction(CMD_SETTINGS);
    }
    return true;
}

/// handle menu or other action
bool CRUIFolderWidget::onAction(const CRUIAction * action) {
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    case CMD_FOLDER_BOOKMARK_USE_FOR_DOWNLOADS:
    {
        lString8 path = _dir->getPathName();
        bookDB->setDownloadsDir(path);
        _main->updateFolderBookmarks();
        return true;
    }
    case CMD_FOLDER_BOOKMARK_ADD:
    {
        lString8 path = _dir->getPathName();
        bookDB->addFolderBookmark(path);
        _main->updateFolderBookmarks();
        return true;
    }
    case CMD_FOLDER_BOOKMARK_REMOVE:
    {
        lString8 path = _dir->getPathName();
        bookDB->removeFolderBookmark(path);
        _main->updateFolderBookmarks();
        return true;
    }
    case CMD_MENU:
    {
        CRUIActionList actions;
        actions.add(ACTION_BACK);
        lString8 path = _dir->getPathName();
        lString8 currentDownloadsFolder = bookDB->getDownloadsDir();
        //ACTION_FOLDER_BOOKMARK_USE_FOR_DOWNLOADS
        if (currentDownloadsFolder != path && !_dir->isArchive()) // TODO: check if CAN WRITE
            actions.add(ACTION_FOLDER_BOOKMARK_USE_FOR_DOWNLOADS);
        if (bookDB->isFolderBookmarked(path) && currentDownloadsFolder != path)
            actions.add(ACTION_FOLDER_BOOKMARK_REMOVE);
        else if (!deviceInfo.isTopDir(path))
            actions.add(ACTION_FOLDER_BOOKMARK_ADD);
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
    case CMD_SETTINGS:
        _main->showSettings(lString8("@settings/browser"));
        return true;
    }
    return CRUIWindowWidget::onAction(action);
}

bool CRUIFolderWidget::onListItemClick(CRUIListWidget * widget, int index) {
    if (index < 0 || index > _dir->itemCount())
        return false;
    CRDirEntry * entry = _dir->getItem(index);
    if (entry->isDirectory()) {
        widget->setSelectedItem(index);
        getMain()->showFolder(entry->getPathName(), true);
    } else {
        // Book? open book
        getMain()->openBook(static_cast<CRFileItem *>(entry));
    }
    return true;
}

CRUIFolderWidget::~CRUIFolderWidget()
{
    if (_dir)
        _dir->unlock();
}

/// returns true if all coverpages are available, false if background tasks are submitted
bool CRUIFolderWidget::requestAllVisibleCoverpages() {
    return _fileList->requestAllVisibleCoverpages();
}

bool CRUIFolderWidget::onKeyEvent(const CRUIKeyEvent * event) {
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
    return CRUIWindowWidget::onKeyEvent(event);
}

/// motion event handler, returns true if it handled event
bool CRUIFolderWidget::onTouchEvent(const CRUIMotionEvent * event) {
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
