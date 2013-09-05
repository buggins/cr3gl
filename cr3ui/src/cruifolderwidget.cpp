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
#include "cruihomewidget.h"

using namespace CRUI;

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

#define DRAG_THRESHOLD_X 15

class CRUITitleBarWidget : public CRUILinearLayout {
	CRUIButton * _backButton;
	CRUIButton * _menuButton;
	CRUITextWidget * _caption;
public:
    CRUITitleBarWidget(lString16 title, CRUIOnClickListener * buttonListener) : CRUILinearLayout(false) {
        setStyle("TOOL_BAR");
        setLayoutParams(FILL_PARENT, WRAP_CONTENT);
		_menuButton = new CRUIImageButton("ic_menu_more");
		_backButton = new CRUIImageButton("ic_menu_back");
		_caption = new CRUITextWidget(title);
		_caption->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
		_caption->setFontSize(FONT_SIZE_MEDIUM);
		_caption->setAlign(ALIGN_HCENTER | ALIGN_VCENTER);
		_caption->setPadding(PT_TO_PX(2));
		addChild(_backButton);
		addChild(_caption);
		addChild(_menuButton);
		setMinHeight(MIN_ITEM_PX);
		//_caption->setBackground(0xC0C0C040);
        _menuButton->setId(lString8("MENU"));
        _backButton->setId(lString8("BACK"));
        _backButton->setOnClickListener(buttonListener);
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
        _icon->setMinWidth(iconDx);
        _icon->setMinHeight(iconDy);
        _icon->setMaxWidth(iconDx);
        _icon->setMaxHeight(iconDy);
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
        _layout->setPadding(lvRect(PT_TO_PX(2), 0, PT_TO_PX(1), 0));
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

    void setDir(CRDirEntry * entry, int iconDx, int iconDy) {
        _iconDx = iconDx;
        _iconDy = iconDy;
        _icon->setMinWidth(iconDx);
        _icon->setMinHeight(iconDy);
        _icon->setMaxWidth(iconDx);
        _icon->setMaxHeight(iconDy);
        _layout->setMaxHeight(_iconDy);
        _layout->setMinHeight(_iconDy);
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
        _layout->setPadding(lvRect(PT_TO_PX(2), 0, PT_TO_PX(1), 0));
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
	CRDirCacheItem * _dir;
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
		setLayoutParams(FILL_PARENT, FILL_PARENT);
		//setBackground("tx_wood_v3.jpg");
        calcCoverSize(deviceInfo.shortSide, deviceInfo.longSide);
        _folderWidget = new CRUIDirItemWidget(_coverDx, _coverDy, "folder_blue");
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
			res->_line2->setText(Utf8ToUnicode(item->getFileName()));
			res->_line3->setText(L"");
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
	virtual void setDirectory(CRDirCacheItem * dir)
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

void CRUIFolderWidget::setDirectory(CRDirCacheItem * dir)
{
	_dir = dir;
	_fileList->setDirectory(dir);
	requestLayout();
}


CRUIFolderWidget::CRUIFolderWidget(CRUIMainWidget * main) : CRUILinearLayout(true), 	_title(NULL), _fileList(NULL), _dir(NULL), _main(main)
{
    _title = new CRUITitleBarWidget(lString16("File list"), this);
	addChild(_title);
    _fileList = new CRUIFileListWidget(this);
	addChild(_fileList);
    _fileList->setOnItemClickListener(this);
}

bool CRUIFolderWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        getMain()->back();
    else if (widget->getId() == "MENU") {
        // TODO
    }
    return true;
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
        getMain()->openBook(dynamic_cast<CRFileItem *>(entry));
    }
}

CRUIFolderWidget::~CRUIFolderWidget()
{

}

/// returns true if all coverpages are available, false if background tasks are submitted
bool CRUIFolderWidget::requestAllVisibleCoverpages() {
    return _fileList->requestAllVisibleCoverpages();
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
