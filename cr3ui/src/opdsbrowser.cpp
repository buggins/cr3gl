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


//class CRUIOpdsItemListWidget : public CRUIListWidget {
//protected:
//    CRDirContentItem * _dir;
//    CRUIDirItemWidget * _folderWidget;
//    CRUIBookItemWidget * _bookWidget;
//    CRUIOpdsBrowserWidget * _parent;
//    int _coverDx;
//    int _coverDy;
//public:
//    void calcCoverSize(int dx, int dy) {
//        if (dx < dy) {
//            // vertical
//            _coverDx = dx / 6;
//            _coverDy = _coverDx * 4 / 3;
//        } else {
//            // horizontal
//            _coverDy = dy / 4;
//            _coverDx = _coverDy * 3 / 4;
//        }
//    }
//
//    /// measure dimensions
//    virtual void measure(int baseWidth, int baseHeight) {
//        calcCoverSize(baseWidth, baseHeight);
//        setColCount(baseWidth > baseHeight ? 2 : 1);
//        CRUIListWidget::measure(baseWidth, baseHeight);
//    }
//
//    /// updates widget position based on specified rectangle
//    virtual void layout(int left, int top, int right, int bottom) {
//        CRUIListWidget::layout(left, top, right, bottom);
//    }
//
//    CRUIOpdsItemListWidget(CRUIOpdsBrowserWidget * parent) : CRUIListWidget(true), _dir(NULL), _parent(parent) {
//		setLayoutParams(FILL_PARENT, FILL_PARENT);
//		//setBackground("tx_wood_v3.jpg");
//        calcCoverSize(deviceInfo.shortSide, deviceInfo.longSide);
//        _folderWidget = new CRUIDirItemWidget(_coverDx, _coverDy, "folder");
//        _bookWidget = new CRUIBookItemWidget(_coverDx, _coverDy, parent->getMain());
//		setStyle("FILE_LIST");
//        setColCount(2);
//	}
//	virtual int getItemCount() {
//		if (!_dir)
//			return 0;
//		//CRLog::trace("item count is %d", _dir->itemCount());
//		return _dir->itemCount();
//	}
//	virtual CRUIWidget * getItemWidget(int index) {
//		CRDirEntry * item = _dir->getItem(index);
//		if (item->isDirectory()) {
//            CRUIDirItemWidget * res = _folderWidget;
//            res->setDir(item, _coverDx, _coverDy);
//            res->_line1->setText(L"");
//            res->_line2->setText(makeDirName(item, true));
//            if (item->getBookCount()) {
//                lString16 info = _16(STR_BOOK_COUNT);
//                res->_line3->setText(info + lString16::itoa(item->getBookCount()));
//            } else {
//                res->_line3->setText(L"");
//            }
//			//CRLog::trace("returning folder item");
//			return res;
//		} else {
//            CRUIBookItemWidget * res = _bookWidget;
//            res->setBook(item, _coverDx, _coverDy);
//			BookDBBook * book = item->getBook();
//			lString16 text1;
//			lString16 text2;
//			lString16 text3;
//			lString16 text4;
//			if (book) {
//				text2 = Utf8ToUnicode(book->title.c_str());
//                text1 = item->getAuthorNames(false);
//                text3 = item->getSeriesName(true);
//				text4 = sizeToString(book->filesize);
//				text4 += " ";
//				text4 += LVDocFormatName(book->format);
//			} else {
//				text2 = Utf8ToUnicode(item->getFileName());
//			}
//			if (text2.empty())
//				text2 = Utf8ToUnicode(item->getFileName());
//			res->_line1->setText(text1);
//			res->_line2->setText(text2);
//			res->_line3->setText(text3);
//			res->_line4->setText(text4);
//			//CRLog::trace("returning book item");
//			return res;
//		}
//	}
//    virtual void setDirectory(CRDirContentItem * dir)
//	{
//		_dir = dir;
//		dir->sort(BY_TITLE);
//		requestLayout();
//	}
//
//    /// return true if drag operation is intercepted
//    virtual bool startDragging(const CRUIMotionEvent * event, bool vertical) {
//        return _parent->getMain()->startDragging(event, vertical);
//    }
//
//    /// returns true if all coverpages are available, false if background tasks are submitted
//    virtual bool requestAllVisibleCoverpages() {
//        coverPageManager->cancelAll();
//        lvRect rc = _pos;
//        applyMargin(rc);
//        applyPadding(rc);
//
//        lvRect clip = _pos;
//        bool foundNotReady = false;
//        for (int i=0; i<getItemCount() && i < _itemRects.length(); i++) {
//            lvRect childRc = _itemRects[i];
//            if (_vertical) {
//                childRc.top -= _scrollOffset;
//                childRc.bottom -= _scrollOffset;
//            } else {
//                childRc.left -= _scrollOffset;
//                childRc.right -= _scrollOffset;
//            }
//            if (clip.intersects(childRc)) {
//                CRDirEntry * item = _dir->getItem(i);
//                if (!item->isDirectory()) {
//                    lvPoint coverSize = _bookWidget->calcCoverSize(_coverDx, _coverDy);
//                    LVDrawBuf * buf = coverPageManager->getIfReady(item, coverSize.x, coverSize.y);
//                    if (buf) {
//                        // image is ready
//                    } else {
//                        foundNotReady = true;
//                        coverPageManager->prepare(item, coverSize.x, coverSize.y, NULL);
//                    }
//                }
//            }
//        }
//        return !foundNotReady;
//    }
//};

void CRUIOpdsBrowserWidget::setDirectory(CRDirContentItem * dir)
{
    if (_dir)
        _dir->unlock();
	_dir = dir;
    if (_dir)
        _dir->lock();
    //_title->setTitle(makeDirName(dir, false));
    _title->setTitle(Utf8ToUnicode(dir->getPathName()));
	//_fileList->setDirectory(dir);
	requestLayout();
}

CRUIOpdsBrowserWidget::CRUIOpdsBrowserWidget(CRUIMainWidget * main) : CRUIWindowWidget(main), _title(NULL)
//, _fileList(NULL),
	, _dir(NULL)
{
    _title = new CRUITitleBarWidget(lString16("File list"), this, this, false);
	_body->addChild(_title);
    //_fileList = new CRUIOpdsItemListWidget(this);
	//_body->addChild(_fileList);
    //_fileList->setOnItemClickListener(this);
	CRUIVerticalLayout * layout = new CRUIVerticalLayout();
	layout->setLayoutParams(FILL_PARENT, FILL_PARENT);
	CRUIImageWidget * image = new CRUIImageWidget("internet");
	image->setPadding(PT_TO_PX(8));
	image->setAlign(ALIGN_CENTER);
	image->setLayoutParams(FILL_PARENT, FILL_PARENT);
	layout->addChild(image);
	CRUITextWidget * text1 = new CRUITextWidget();
	text1->setAlign(ALIGN_CENTER);
	text1->setPadding(PT_TO_PX(8));
	text1->setMaxLines(2);
	text1->setText("OPDS catalog access is not yet implemented.");
	text1->setFontSize(FONT_SIZE_LARGE);
	text1->setLayoutParams(FILL_PARENT, FILL_PARENT);
	layout->setStyle("SETTINGS_ITEM_LIST");
	layout->addChild(text1);
	_body->addChild(layout);
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
    return false;
}

bool CRUIOpdsBrowserWidget::onListItemClick(CRUIListWidget * widget, int index) {
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

CRUIOpdsBrowserWidget::~CRUIOpdsBrowserWidget()
{
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
