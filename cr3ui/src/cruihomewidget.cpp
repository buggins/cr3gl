/*
 * cuihomewidget.cpp
 *
 *  Created on: Aug 20, 2013
 *      Author: vlopatin
 */

#include "cruihomewidget.h"
#include "cruicontrols.h"
#include "crui.h"
#include "cruitheme.h"
#include "stringresource.h"
#include "cruimain.h"
#include "crcoverpages.h"

using namespace CRUI;

class CRUIMainWidget;

class MainWidgetUpdateCallback : public CRRunnable {
    CRUIMainWidget * _main;
public:
    MainWidgetUpdateCallback(CRUIMainWidget * main) : _main(main) {}
    virtual void run() {
        _main->requestLayout();
        _main->update();
    }
};


class CRUINowReadingWidget : public CRUILinearLayout {
	CRUIImageWidget * _cover;
	CRUILinearLayout * _captionLayout;
	CRUILinearLayout * _layout;
	CRUIImageRef _coverImage;
	CRUIButton * _menuButton;
	CRUITextWidget * _caption;
	CRUITextWidget * _title;
	CRUITextWidget * _authors;
	CRUITextWidget * _info;
    CRUIHomeWidget * _home;
    CRDirEntry * _lastBook;
    int _coverDx;
    int _coverDy;
public:

    CRUINowReadingWidget(CRUIHomeWidget * home) : CRUILinearLayout(false), _home(home), _lastBook(NULL) {
        _coverImage = CRUIImageRef(); //resourceResolver->getIcon("cr3_logo");//new CRUISolidFillImage(0xE0E0A0);
		_cover = new CRUIImageWidget(_coverImage);
		int coverSize = deviceInfo.shortSide / 4;
		_cover->setMargin(PT_TO_PX(4));
        _coverDx = coverSize * 3 / 4;
        _coverDy = coverSize;
        _cover->setMinWidth(_coverDx);
        _cover->setMaxWidth(_coverDx);
        _cover->setMinHeight(_coverDy);
        _cover->setMaxHeight(_coverDy);
        //_cover->setBackground(0xC0808000);
        _cover->setBackground("home_frame.9.png");
        _cover->setAlign(ALIGN_CENTER);
		addChild(_cover);
		_layout = new CRUILinearLayout(true);
		addChild(_layout);
		_captionLayout = new CRUILinearLayout(false);
		_menuButton = new CRUIImageButton("ic_menu_more"); //moreicon
		_caption = new CRUITextWidget(STR_NOW_READING);
		_caption->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
		_caption->setFontSize(FONT_SIZE_SMALL);
        //_caption->setBackground(0xE0404040);
		_caption->setAlign(ALIGN_LEFT|ALIGN_TOP);
		_caption->setPadding(PT_TO_PX(4));
        _caption->setStyle("HOME_LIST_CAPTION");
		_captionLayout->addChild(_caption);
		_captionLayout->addChild(_menuButton);

		_layout->addChild(_captionLayout);
		lvRect pad(PT_TO_PX(4), 0, PT_TO_PX(4), 0);
		_title = new CRUITextWidget(lString16(L"War and Peace"));
		_title->setFontSize(FONT_SIZE_MEDIUM);
		_title->setPadding(pad);
		_authors = new CRUITextWidget(lString16(L"Leo Tolstoy"));
		_authors->setFontSize(FONT_SIZE_SMALL);
		_authors->setPadding(pad);
		_info = new CRUITextWidget(lString16(L"fb2 3245K 1891"));
		_info->setFontSize(FONT_SIZE_SMALL);
		_info->setPadding(pad);

//		CRUIButton * testButton = new CRUIButton(lString16(), "ic_menu_more");
//		testButton->setMinWidth(100);

        CRUIWidget * spacer1 = new CRUIWidget();
        spacer1->setLayoutParams(FILL_PARENT, FILL_PARENT)->setLayoutWeight(1);
        CRUIWidget * spacer2 = new CRUIWidget();
        spacer2->setLayoutParams(FILL_PARENT, FILL_PARENT)->setLayoutWeight(2);
        _layout->addChild(spacer1);
        _layout->addChild(_authors);
		_layout->addChild(_title);
		_layout->addChild(_info);
        _layout->addChild(spacer2);

//		_layout->addChild(testButton);

		_layout->setLayoutParams(CRUI::FILL_PARENT, CRUI::FILL_PARENT);

        setLastBook(NULL);
	}

    ~CRUINowReadingWidget() {
        if (_lastBook)
            delete _lastBook;
    }

    void updateCover() {
        if (_lastBook) {
            LVDrawBuf * cover = coverPageManager->getIfReady(_lastBook, _coverDx, _coverDy);
            if (cover) {
                _coverImage = CRUIImageRef(new CRUIDrawBufImage(cover)) ; //resourceResolver->getIcon("cr3_logo");//new CRUISolidFillImage(0xE0E0A0);
            } else {
                _coverImage = CRUIImageRef(); //resourceResolver->getIcon("cr3_logo");//new CRUISolidFillImage(0xE0E0A0);
                coverPageManager->prepare(_lastBook, _coverDx, _coverDy, new MainWidgetUpdateCallback(_home->getMain()));
            }
        } else {
            _coverImage = CRUIImageRef();
        }
        _cover->setImage(_coverImage);
    }

    void setLastBook(CRDirEntry * lastBook) {
        if (_lastBook)
            delete _lastBook;
        _lastBook = lastBook ? lastBook->clone() : NULL;
        if (_lastBook) {
            _title->setText(_lastBook->getTitle());
            _authors->setText(_lastBook->getAuthorNames(false));
            _info->setText(_lastBook->getSeriesName(true));
        } else {
            _title->setText(lString16());
            _authors->setText(lString16());
            _info->setText(lString16());
        }
        updateCover();
        invalidate();
    }

    const CRDirEntry * getLastBook() {
        return _lastBook;
    }

};

class CRUIHomeItemListWidget : public CRUILinearLayout, public CRUIListAdapter, public CRUIOnListItemClickListener {
protected:
	CRUITextWidget * _caption;
	CRUIListWidget * _list;
	CRUILinearLayout * _itemWidget;
	CRUIImageWidget * _itemImage;
	CRUITextWidget * _textWidget;
    CRUIHomeWidget * _home;
public:
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex) { return false; }
    CRUIHomeItemListWidget(CRUIHomeWidget * home, const char * captionResourceId) : CRUILinearLayout(true), _home(home) {
		_caption = new CRUITextWidget(captionResourceId);
		_caption->setLayoutParams(CRUI::FILL_PARENT, CRUI::WRAP_CONTENT);
		_caption->setPadding(3);
        //_caption->setFontSize(CRUI::FONT_SIZE_SMALL);
        _caption->setStyle("HOME_LIST_CAPTION");
//		lvRect rc;
//		_caption->getMargin(rc);
//		CRLog::trace("list caption margin: %d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
//		_caption->getPadding(rc);
//		CRLog::trace("list caption padding: %d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
//		CRUIImageRef bg = _caption->getBackground();
//		const CR9PatchInfo * nine = bg.isNull() ? NULL : bg->getNinePatchInfo();
//		if (nine) {
//			rc = nine->padding;
//			CRLog::trace("list caption nine patch: %d,%d,%d,%d", rc.left, rc.top, rc.right, rc.bottom);
//		}

		addChild(_caption);
		_list = new CRUIListWidget(false, this);
		_list->setLayoutParams(CRUI::FILL_PARENT, CRUI::FILL_PARENT);
        _list->setBackground("home_frame.9.png");
		_list->setPadding(PT_TO_PX(3));
        _list->setOnItemClickListener(this);
        addChild(_list);

		_itemImage = new CRUIImageWidget(CRUIImageRef());
		_itemImage->setAlign(CRUI::ALIGN_CENTER);
		_itemImage->setLayoutParams(CRUI::WRAP_CONTENT, CRUI::FILL_PARENT);
		_itemImage->setPadding(PT_TO_PX(1));

		_textWidget = new CRUITextWidget(lString16());
		_textWidget->setAlign(CRUI::ALIGN_TOP | CRUI::ALIGN_HCENTER);
        _textWidget->setFontSize(CRUI::FONT_SIZE_XSMALL);
		_textWidget->setPadding(PT_TO_PX(1));
        _textWidget->setMaxLines(2);
        _textWidget->setEllipsisMode(ELLIPSIS_MIDDLE);

		_itemWidget = new CRUILinearLayout(true);
		_itemWidget->addChild(_itemImage);
		_itemWidget->addChild(_textWidget);
		_itemWidget->setPadding(PT_TO_PX(2));
		_itemWidget->setMaxWidth(deviceInfo.shortSide / 5);
		_itemWidget->setMinWidth(deviceInfo.minListItemSize * 3 / 2);
		_itemWidget->setStyle("LIST_ITEM");
        setMargin(lvRect(PT_TO_PX(2), 0, PT_TO_PX(2), 0));
	}

	virtual int getItemCount(CRUIListWidget * list) {
		return 10;
	}
	virtual lString16 getItemText(int index) {
		char s[100];
		sprintf(s, "item%d", index);
		return lString16(s);
	}
	virtual CRUIImageRef getItemIcon(int index) {
		return resourceResolver->getIcon("folder_blue");
	}
	virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index) {
		CRUIImageRef icon = getItemIcon(index);
		lString16 text = getItemText(index);
		_textWidget->setText(text);
		_itemImage->setImage(icon);
		return _itemWidget;
	}
};

class CRUIFileSystemDirsWidget : public CRUIHomeItemListWidget {
public:
    virtual int getItemCount(CRUIListWidget * list) {
        return deviceInfo.topDirs.itemCount();
    }
    virtual lString16 getItemText(int index) {
        CRTopDirItem * item = deviceInfo.topDirs.getItem(index);
        switch(item->getDirType()) {
        case DIR_TYPE_INTERNAL_STORAGE:
            return _16(STR_INTERNAL_STORAGE_DIR);
        case DIR_TYPE_SD_CARD:
            return _16(STR_SD_CARD_DIR);
        case DIR_TYPE_FS_ROOT:
            return Utf8ToUnicode(item->getPathName());
        case DIR_TYPE_DEFAULT_BOOKS_DIR:
            return Utf8ToUnicode(item->getPathName());
        case DIR_TYPE_CURRENT_BOOK_DIR:
            return Utf8ToUnicode(item->getPathName());
        case DIR_TYPE_DOWNLOADS:
            return Utf8ToUnicode(item->getPathName());
        case DIR_TYPE_FAVORITE:
            return Utf8ToUnicode(item->getPathName());
        case DIR_TYPE_NORMAL:
            return Utf8ToUnicode(item->getFileName());
        default:
            return Utf8ToUnicode(item->getPathName());
        }
    }
    virtual CRUIImageRef getItemIcon(int index) {
        CRTopDirItem * item = deviceInfo.topDirs.getItem(index);
        switch(item->getDirType()) {
        case DIR_TYPE_INTERNAL_STORAGE:
            return resourceResolver->getIcon("media_flash_sd_mmc");
        case DIR_TYPE_SD_CARD:
            return resourceResolver->getIcon("media_flash_sd_mmc");
        case DIR_TYPE_FS_ROOT:
            return resourceResolver->getIcon("folder_blue");
        case DIR_TYPE_DEFAULT_BOOKS_DIR:
            return resourceResolver->getIcon("folder_bookmark");
        case DIR_TYPE_CURRENT_BOOK_DIR:
            return resourceResolver->getIcon("folder_bookmark");
        case DIR_TYPE_DOWNLOADS:
            return resourceResolver->getIcon("folder_blue");
        case DIR_TYPE_FAVORITE:
            return resourceResolver->getIcon("folder_bookmark");
        case DIR_TYPE_NORMAL:
            return resourceResolver->getIcon("folder_blue");
        default:
            return resourceResolver->getIcon("folder_blue");
        }
    }
    CRUIFileSystemDirsWidget(CRUIHomeWidget * home) : CRUIHomeItemListWidget(home, STR_BROWSE_FILESYSTEM) {
        deviceInfo.topDirs.sort(0);
    }
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex) {
        CRTopDirItem * item = deviceInfo.topDirs.getItem(itemIndex);
        _home->getMain()->showFolder(item->getPathName(), true);
        return true;
    }
};

class CRUILibraryWidget : public CRUIHomeItemListWidget {
public:
    CRUILibraryWidget(CRUIHomeWidget * home) : CRUIHomeItemListWidget(home, STR_BROWSE_LIBRARY) {

	}
};

class CRUIOnlineCatalogsWidget : public CRUIHomeItemListWidget {
public:
    CRUIOnlineCatalogsWidget(CRUIHomeWidget * home) : CRUIHomeItemListWidget(home, STR_ONLINE_CATALOGS) {

	}
};

class CRUIRecentBooksListWidget : public CRUIHomeItemListWidget {
public:
    CRUIRecentBooksListWidget(CRUIHomeWidget * home) : CRUIHomeItemListWidget(home, STR_RECENT_BOOKS) {
	}
};

CRUIHomeWidget::CRUIHomeWidget(CRUIMainWidget * main) : _main(main){
    _currentBook = new CRUINowReadingWidget(this);
    _recentBooksList = new CRUIRecentBooksListWidget(this);
    _fileSystem = new CRUIFileSystemDirsWidget(this);
    _library = new CRUILibraryWidget(this);
    _onlineCatalogsList = new CRUIOnlineCatalogsWidget(this);
	addChild(_currentBook);
	addChild(_recentBooksList);
	addChild(_fileSystem);
	addChild(_library);
	addChild(_onlineCatalogsList);
	setStyle("HOME_WIDGET");
}

/// measure dimensions
void CRUIHomeWidget::measure(int baseWidth, int baseHeight)
{
    _currentBook->updateCover();
    _measuredWidth = baseWidth;
	_measuredHeight = baseHeight;
	bool vertical = baseWidth < baseHeight;
	if (vertical) {
		int nowReadingH = baseHeight / 4;
		int recentH = baseHeight / 5;
		int otherH = (baseHeight - nowReadingH - recentH) / 3;
		_currentBook->measure(baseWidth, nowReadingH);
		_recentBooksList->measure(baseWidth, recentH);
		_fileSystem->measure(baseWidth, otherH);
		_library->measure(baseWidth, otherH);
		_onlineCatalogsList->measure(baseWidth, otherH);
	} else {
		int nowReadingH = baseHeight / 3;
		int otherH = (baseHeight - nowReadingH) / 2;
		_currentBook->measure(baseWidth, nowReadingH);
        _recentBooksList->measure(baseWidth * 3 / 5, otherH);
        _fileSystem->measure(baseWidth * 2 / 5, otherH);
        _library->measure(baseWidth * 3 / 5, otherH);
        _onlineCatalogsList->measure(baseWidth * 2 / 5, otherH);
	}
}

/// updates widget position based on specified rectangle
void CRUIHomeWidget::layout(int left, int top, int right, int bottom)
{
	CRUIWidget::layout(left, top, right, bottom);
	int w = (right - left);
	int h = (bottom - top);
	bool vertical = w < h;
	if (vertical) {
		int nowReadingH = h / 4;
		int recentH = h / 5;
		int otherH = (h - nowReadingH - recentH) / 3;
		int y = top;
		_currentBook->layout(left, y, right, y + nowReadingH);
		y += nowReadingH;
		_recentBooksList->layout(left, y, right, y + recentH);
		y += recentH;
		_fileSystem->layout(left, y, right, y + otherH); y += otherH;
		_library->layout(left, y, right, y + otherH); y += otherH;
		_onlineCatalogsList->layout(left, y, right, y + otherH);
	} else {
		int nowReadingH = h / 3;
		int otherH = (h - nowReadingH) / 2;
		int y = top;
		_currentBook->layout(left, y, right, y + nowReadingH);
		y += nowReadingH;
		_recentBooksList->layout(left, y, (left + right) / 2, y + otherH);
		_fileSystem->layout((left + right) / 2, y, right, y + otherH);
		y += otherH;
		_library->layout(left, y, (left + right) / 2, y + otherH);
		_onlineCatalogsList->layout((left + right) / 2, y, right, y + otherH);
	}
}

#define DRAG_THRESHOLD_X 15

/// motion event handler, returns true if it handled event
bool CRUIHomeWidget::onTouchEvent(const CRUIMotionEvent * event) {
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

void CRUIHomeWidget::setLastBook(CRDirEntry * lastBook) {
    _currentBook->setLastBook(lastBook);
}

const CRDirEntry * CRUIHomeWidget::getLastBook() {
    return _currentBook->getLastBook();
}
