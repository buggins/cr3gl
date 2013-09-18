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


class CRUINowReadingWidget : public CRUILinearLayout {
    CRCoverWidget * _cover;
	CRUILinearLayout * _captionLayout;
	CRUILinearLayout * _layout;
	CRUIImageRef _coverImage;
	CRUIButton * _menuButton;
	CRUITextWidget * _caption;
	CRUITextWidget * _title;
	CRUITextWidget * _authors;
	CRUITextWidget * _info;
    CRUIHomeWidget * _home;
public:

    CRUINowReadingWidget(CRUIHomeWidget * home) : CRUILinearLayout(false), _home(home) {
        _coverImage = CRUIImageRef(); //resourceResolver->getIcon("cr3_logo");//new CRUISolidFillImage(0xE0E0A0);
        //int coverSize = deviceInfo.shortSide / 4;
        _cover = new CRCoverWidget(_home->getMain(), NULL, 75, 100);
        _cover->setMargin(PT_TO_PX(4));
        //_cover->setBackground(0xC0808000);
        //_cover->setBackground("home_frame.9.png");
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
        _captionLayout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);

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
	}

    void onThemeChanged() {
        _cover->setMargin(PT_TO_PX(4));
        _caption->setPadding(PT_TO_PX(4));
        lvRect pad(PT_TO_PX(4), 0, PT_TO_PX(4), 0);
        _title->setPadding(pad);
        _authors->setPadding(pad);
        _info->setPadding(pad);
        requestLayout();
    }

    void updateCoverSize(int baseHeight) {
        lvRect margin = getMargin();
        lvRect padding;
        getPadding(padding);
        int coverDy = baseHeight - margin.top - margin.bottom - padding.top - padding.bottom;
        int coverDx = coverDy * 3 / 4;
        _cover->setSize(coverDx, coverDy);
    }

    /// measure dimensions
    void measure(int baseWidth, int baseHeight) {
        updateLastBook(getLastBook());
        updateCoverSize(baseHeight);
        CRUILinearLayout::measure(baseWidth, baseHeight);
    }

    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom) {
        updateCoverSize(bottom - top);
        CRUILinearLayout::layout(left, top, right, bottom);
    }

    virtual ~CRUINowReadingWidget() {
    }

    void updateLastBook(const CRDirEntry * lastBook) {
        if (lastBook) {
            _title->setText(lastBook->getTitle());
            _authors->setText(lastBook->getAuthorNames(false));
            _info->setText(lastBook->getSeriesName(true));
        } else {
            _title->setText(lString16());
            _authors->setText(lString16());
            _info->setText(lString16());
        }
        _cover->setBook(lastBook);
        invalidate();
    }

    const CRFileItem * getLastBook() {
        CRDirContentItem * dir = dirCache->find(lString8(RECENT_DIR_TAG));
        CRFileItem * item = dir ? static_cast<CRFileItem*>(dir->getItem(0)) : NULL;
        return item;
    }

    virtual bool onClickEvent() {
        const CRFileItem * lastBook = getLastBook();
        if (lastBook) {
            _home->getMain()->openBook(lastBook);
            return true;
        }
        return false;
    }

    /// motion event handler, returns true if it handled event
    bool onTouchEvent(const CRUIMotionEvent * event) {
        int action = event->getAction();
        int delta = event->getX() - event->getStartX();
        //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
        switch (action) {
        case ACTION_DOWN:
            break;
        case ACTION_UP:
            if ((delta < DRAG_THRESHOLD_X * 2) && (-delta < DRAG_THRESHOLD_X * 2)) {
                bool isLong = event->getDownDuration() < LONG_TOUCH_THRESHOLD;
                if (isLong) {
                    if (onLongClickEvent())
                        return true;
                }
                onClickEvent();
            }
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
                _home->getMain()->startDragging(event, false);
            break;
        default:
            return CRUIWidget::onTouchEvent(event);
        }
        return true;
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
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex) {
        CR_UNUSED(widget);
        CR_UNUSED(itemIndex);
        return false;
    }
    CRUIHomeItemListWidget(CRUIHomeWidget * home, const char * captionResourceId) : CRUILinearLayout(true), _home(home) {
		_caption = new CRUITextWidget(captionResourceId);
		_caption->setLayoutParams(CRUI::FILL_PARENT, CRUI::WRAP_CONTENT);
		_caption->setPadding(lvRect(3,3,1,3));
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
		_list->setPadding(lvRect(PT_TO_PX(1), 0, PT_TO_PX(1), 0));
        _list->setOnItemClickListener(this);
        addChild(_list);

		_itemImage = new CRUIImageWidget(NULL);
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
		_itemWidget->setPadding(PT_TO_PX(1));
		_itemWidget->setMaxWidth(deviceInfo.shortSide / 5);
		_itemWidget->setMinWidth(deviceInfo.minListItemSize * 3 / 2);
		_itemWidget->setStyle("LIST_ITEM");
        setMargin(lvRect(PT_TO_PX(1), 0, PT_TO_PX(2), 0));
	}

    void onThemeChanged() {
        _caption->setPadding(3);
        _list->setPadding(PT_TO_PX(3));
        _itemImage->setPadding(PT_TO_PX(1));
        _textWidget->setPadding(PT_TO_PX(1));
        _itemWidget->setPadding(PT_TO_PX(2));
        _itemWidget->setMaxWidth(deviceInfo.shortSide / 5);
        _itemWidget->setMinWidth(deviceInfo.minListItemSize * 3 / 2);
        setMargin(lvRect(PT_TO_PX(2), 0, PT_TO_PX(2), 0));
        requestLayout();
    }

    virtual int getItemCount(CRUIListWidget * list) {
        CR_UNUSED(list);
        return 10;
	}
	virtual lString16 getItemText(int index) {
		char s[100];
		sprintf(s, "item%d", index);
		return lString16(s);
	}
	virtual lString8 getItemIcon(int index) {
        CR_UNUSED(index);
        return lString8("folder");
	}
	virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index) {
        CR_UNUSED(list);
        lString8 icon = getItemIcon(index);
		lString16 text = getItemText(index);
		_textWidget->setText(text);
		_itemImage->setImage(icon);
		return _itemWidget;
	}
};

class CRUIFileSystemDirsWidget : public CRUIHomeItemListWidget {
public:
    virtual int getItemCount(CRUIListWidget * list) {
        CR_UNUSED(list);
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
	virtual lString8 getItemIcon(int index) {
        CRTopDirItem * item = deviceInfo.topDirs.getItem(index);
        switch(item->getDirType()) {
        case DIR_TYPE_INTERNAL_STORAGE:
            return lString8("integrated_circuit");
        case DIR_TYPE_SD_CARD:
            return lString8("sd");
        case DIR_TYPE_FS_ROOT:
            return lString8("folder");
        case DIR_TYPE_DEFAULT_BOOKS_DIR:
            return lString8("likes");
        case DIR_TYPE_CURRENT_BOOK_DIR:
            return lString8("documents");
        case DIR_TYPE_DOWNLOADS:
            return lString8("downloads");
        case DIR_TYPE_FAVORITE:
            return lString8("likes");
        case DIR_TYPE_NORMAL:
            return lString8("folder");
        default:
            return lString8("folder");
        }
    }
    CRUIFileSystemDirsWidget(CRUIHomeWidget * home) : CRUIHomeItemListWidget(home, STR_BROWSE_FILESYSTEM) {
        deviceInfo.topDirs.sort(0);
    }
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex) {
        CR_UNUSED(widget);
        CRTopDirItem * item = deviceInfo.topDirs.getItem(itemIndex);
        _home->getMain()->showFolder(item->getPathName(), true);
        return true;
    }
};

class CRUILibraryWidget : public CRUIHomeItemListWidget {
    LVPtrVector<CRDirEntry> _entries;
public:
    const BookDBBook * getLastBook() {
        CRDirContentItem * dir = dirCache->find(lString8(RECENT_DIR_TAG));
        CRFileItem * item = dir ? static_cast<CRFileItem*>(dir->getItem(0)) : NULL;
        return item ? item->getBook() : NULL;
    }

    void init() {
        const BookDBBook * currentBook = getLastBook();
        _entries.clear();
        _entries.add(new CRDirItem(lString8(BOOKS_BY_AUTHOR_TAG), false));
        _entries.add(new CRDirItem(lString8(BOOKS_BY_TITLE_TAG), false));
        _entries.add(new CRDirItem(lString8(BOOKS_BY_SERIES_TAG), false));
        _entries.add(new CRDirItem(lString8(BOOKS_BY_FILENAME_TAG), false));
        _entries.add(new CRDirItem(lString8(SEARCH_RESULTS_TAG), false));
        if (currentBook) {
            for (int i = 0; i < currentBook->authors.length(); i++) {
                lString8 author(currentBook->authors[i]->fileAs.c_str());
                if (author.length()) {
                    _entries.add(new CRDirItem(lString8(BOOKS_BY_AUTHOR_TAG) + author, false));
                }
            }
            if (!currentBook->series.isNull()) {
                lString8 series(currentBook->series->name.c_str());
                if (series.length()) {
                    _entries.add(new CRDirItem(lString8(BOOKS_BY_SERIES_TAG) + series, false));
                }
            }
        }
    }

    virtual int getItemCount(CRUIListWidget * list) {
        CR_UNUSED(list);
        return _entries.length();
    }
    virtual lString16 getItemText(int index) {
        if (index < 0 || index >= _entries.length())
            return lString16();
        CRDirEntry * item = _entries[index];
        lString16 pattern = item->getFilterString();
        if (pattern.length())
            return pattern;
        switch(item->getDirType()) {
        case DIR_TYPE_BOOKS_BY_AUTHOR:
            return _16(STR_BOOKS_BY_AUTHOR);
        case DIR_TYPE_BOOKS_BY_TITLE:
            return _16(STR_BOOKS_BY_TITLE);
        case DIR_TYPE_BOOKS_BY_FILENAME:
            return _16(STR_BOOKS_BY_FILENAME);
        case DIR_TYPE_BOOKS_BY_SERIES:
            return _16(STR_BOOKS_BY_SERIES);
        case DIR_TYPE_BOOKS_SEARCH_RESULT:
            return _16(STR_BOOKS_SEARCH);
        default:
            return Utf8ToUnicode(item->getPathName());
        }
    }
    virtual lString8 getItemIcon(int index) {
        if (index < 0 || index >= _entries.length())
            return lString8();
        CRDirEntry * item = _entries[index];
        switch(item->getDirType()) {
        case DIR_TYPE_BOOKS_BY_AUTHOR:
            return lString8("book_stack");
        case DIR_TYPE_BOOKS_BY_TITLE:
            return lString8("book_stack");
        case DIR_TYPE_BOOKS_BY_FILENAME:
            return lString8("book_stack");
        case DIR_TYPE_BOOKS_BY_SERIES:
            return lString8("book_stack");
        case DIR_TYPE_BOOKS_SEARCH_RESULT:
            return lString8("opera_glasses");
        default:
            return lString8("folder");
        }
    }
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight) {
        init();
        CRUILinearLayout::measure(baseWidth, baseHeight);
    }

    CRUILibraryWidget(CRUIHomeWidget * home) : CRUIHomeItemListWidget(home, STR_BROWSE_LIBRARY) {
        init();
    }
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex) {
        CR_UNUSED(widget);
        if (itemIndex < 0 || itemIndex >= _entries.length())
            return false;
        CRDirEntry * item = _entries[itemIndex];
        _home->getMain()->showFolder(item->getPathName(), true);
        return true;
    }
};

class CRUIOnlineCatalogsWidget : public CRUIHomeItemListWidget {
public:
    CRUIOnlineCatalogsWidget(CRUIHomeWidget * home) : CRUIHomeItemListWidget(home, STR_ONLINE_CATALOGS) {

	}
    virtual lString8 getItemIcon(int index) {
        CR_UNUSED(index);
        return lString8("internet");
    }
};

class CRUIRecentBooksListWidget : public CRUILinearLayout, public CRUIListAdapter, public CRUIOnListItemClickListener {
protected:
    CRUITextWidget * _caption;
    CRUIListWidget * _list;
    CRUILinearLayout * _itemWidget;
    CRCoverWidget  * _itemImage;
    CRUITextWidget * _textWidget;
    CRUIHomeWidget * _home;
public:
    virtual bool onListItemClick(CRUIListWidget * widget, int itemIndex) {
        CR_UNUSED(widget);
        CRDirContentItem * dir = dirCache->find(lString8(RECENT_DIR_TAG));
        CRFileItem * item = dir ? static_cast<CRFileItem*>(dir->getItem(itemIndex + 1)) : NULL;
        if (item) {
            _home->getMain()->openBook(item);
            return true;
        }
        return false;
    }

    CRUIRecentBooksListWidget(CRUIHomeWidget * home) : CRUILinearLayout(true), _home(home) {
        _caption = new CRUITextWidget(STR_RECENT_BOOKS);
        _caption->setLayoutParams(CRUI::FILL_PARENT, CRUI::WRAP_CONTENT);
        _caption->setPadding(3);
        _caption->setStyle("HOME_LIST_CAPTION");
        addChild(_caption);
        _list = new CRUIListWidget(false, this);
        _list->setLayoutParams(CRUI::FILL_PARENT, CRUI::FILL_PARENT);
        _list->setBackground("home_frame.9.png");
        _list->setPadding(PT_TO_PX(1));
        _list->setOnItemClickListener(this);
        addChild(_list);

        _itemImage = new CRCoverWidget(home->getMain(), NULL, 75, 100);
        _itemImage->setAlign(CRUI::ALIGN_CENTER);
        _itemImage->setLayoutParams(CRUI::WRAP_CONTENT, CRUI::FILL_PARENT);
        //_itemImage->setLayoutParams(CRUI::WRAP_CONTENT, CRUI::WRAP_CONTENT);
        _itemImage->setPadding(PT_TO_PX(1));
        //_itemImage->setBackground(0x0000C0C0);

        _textWidget = new CRUITextWidget(lString16());
        _textWidget->setAlign(CRUI::ALIGN_TOP | CRUI::ALIGN_HCENTER);
        _textWidget->setFontSize(CRUI::FONT_SIZE_XSMALL);
        _textWidget->setPadding(PT_TO_PX(1));
        _textWidget->setMaxLines(2);
        _textWidget->setEllipsisMode(ELLIPSIS_MIDDLE);
        //_textWidget->setBackground(0xC0C000C0);
        _textWidget->setLayoutParams(CRUI::WRAP_CONTENT, CRUI::WRAP_CONTENT);

        _itemWidget = new CRUILinearLayout(true);
        _itemWidget->addChild(_itemImage);
        _itemWidget->addChild(_textWidget);
        _itemWidget->setPadding(PT_TO_PX(1));
//        _itemWidget->setMaxWidth(deviceInfo.shortSide / 5);
//        _itemWidget->setMinWidth(deviceInfo.minListItemSize * 3 / 2);
        _itemWidget->setStyle("LIST_ITEM");

        setMargin(lvRect(PT_TO_PX(2), 0, PT_TO_PX(2), 0));
    }

    void onThemeChanged() {
        _caption->setPadding(3);
        _list->setPadding(PT_TO_PX(3));
        _itemImage->setPadding(PT_TO_PX(1));
        _textWidget->setPadding(PT_TO_PX(1));
        _itemWidget->setPadding(PT_TO_PX(2));
        setMargin(lvRect(PT_TO_PX(2), 0, PT_TO_PX(2), 0));
        requestLayout();
    }

    virtual int getItemCount(CRUIListWidget * list) {
        CR_UNUSED(list);
        CRDirContentItem * dir = dirCache->find(lString8(RECENT_DIR_TAG));
        if (!dir)
            return 0;
        int count = dir->itemCount() - 1;
        return count >= 0 ? count : 0;
    }

    virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index) {
        CR_UNUSED(list);
        CRDirContentItem * dir = dirCache->find(lString8(RECENT_DIR_TAG));
        CRFileItem * item = dir ? static_cast<CRFileItem*>(dir->getItem(index + 1)) : NULL;
        if (item) {
            lString16 text = item->getTitle();
            if (text.empty())
                text = Utf8ToUnicode(item->getFileName());
            _textWidget->setText(text);
            _itemImage->setBook(item);
        } else {
            _textWidget->setText(lString16("?"));
            _itemImage->setBook(NULL);
        }
        return _itemWidget;
    }

    virtual void updateCoverSize(int baseHeight) {
        /// calculate cover widget size
        lvRect itempadding;
        _itemWidget->getPadding(itempadding);
        lvRect itemmargin = _itemWidget->getMargin();
        lvRect textpadding;
        _textWidget->getPadding(textpadding);
        lvRect textmargin = _textWidget->getMargin();
        int textH = _textWidget->getFont()->getHeight() * 2;
        int coverH = baseHeight - textH - textpadding.top - textpadding.bottom - itempadding.top - itempadding.bottom -
                textmargin.top - textmargin.bottom - itemmargin.top - itemmargin.bottom;
        _itemImage->setSize(coverH * 3 / 4, coverH);

        //_textWidget->setBackground(0xC060C0A0);

        /// widget size constraints
        //int coverW = coverH * 3 / 4 + itempadding.left + itempadding.right + itemmargin.left + itemmargin.right;

        //CRLog::trace("Recent books cover size %d x %d  h=%d  texth=%d", coverW, coverH, baseHeight, textH);

        int itemw = coverH; // + itempadding.left + itempadding.right + itemmargin.left +itemmargin.right;
        _itemWidget->setMinWidth(itemw);
        // TODO: get it working
        _itemWidget->setMaxWidth(itemw);
    }

    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight) {
        _caption->measure(baseWidth, baseHeight);
        lvRect listmargin = _list->getMargin();
        lvRect listpadding;
        _list->getPadding(listpadding);

        int h = baseHeight - _caption->getMeasuredHeight() - listmargin.top - listmargin.bottom - listpadding.top - listpadding.bottom;
        //CRLog::trace("Recent list measure: %d", h);

        updateCoverSize(h);
        /// measure
        CRUILinearLayout::measure(baseWidth, baseHeight);
    }
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom) {
        //CRLog::trace("Recent list layout: %d", bottom - top);
        //updateCoverSize(bottom - top);
        CRUILinearLayout::layout(left, top, right, bottom);
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
    _measuredWidth = baseWidth;
	_measuredHeight = baseHeight;
    bool vertical = baseWidth < baseHeight * 85 / 100;
	if (vertical) {
        int nowReadingH = baseHeight * 20 / 100;
        int recentH = baseHeight * 25 / 100;
		int otherH = (baseHeight - nowReadingH - recentH) / 3;
		_currentBook->measure(baseWidth, nowReadingH);
		_recentBooksList->measure(baseWidth, recentH);
		_fileSystem->measure(baseWidth, otherH);
		_library->measure(baseWidth, otherH);
		_onlineCatalogsList->measure(baseWidth, otherH);
	} else {
        int nowReadingH = baseHeight * 30 / 100;
        int recentH = baseHeight * 40 / 100;
        int otherH = (baseHeight - nowReadingH - recentH);
		_currentBook->measure(baseWidth, nowReadingH);
        _recentBooksList->measure(baseWidth * 3 / 5, recentH);
        _fileSystem->measure(baseWidth * 2 / 5, recentH);
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
    bool vertical = w < h * 85 / 100;
	if (vertical) {
        int nowReadingH = h * 20 / 100;
        int recentH = h * 25 / 100;
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
        int nowReadingH = h * 30 / 100;
        int recentH = h * 40 / 100;
        int otherH = (h - nowReadingH - recentH);
//		int nowReadingH = h / 3;
//		int otherH = (h - nowReadingH) / 2;
		int y = top;
		_currentBook->layout(left, y, right, y + nowReadingH);
		y += nowReadingH;
        _recentBooksList->layout(left, y, (left + right) / 2, y + recentH);
        _fileSystem->layout((left + right) / 2, y, right, y + recentH);
        y += recentH;
		_library->layout(left, y, (left + right) / 2, y + otherH);
		_onlineCatalogsList->layout((left + right) / 2, y, right, y + otherH);
	}
}

bool CRUIHomeWidget::onKeyEvent(const CRUIKeyEvent * event) {
	if (event->getType() == KEY_ACTION_RELEASE && (event->key() == CR_KEY_BACK || event->key() == CR_KEY_ESC)) {
		if (getMain()->getPlatform() != NULL)
			getMain()->getPlatform()->exitApp();
		return true;
	}
	if (event->getType() == KEY_ACTION_PRESS && (event->key() == CR_KEY_BACK || event->key() == CR_KEY_ESC)) {
		return true;
	}
	return false;
}

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



void CRCoverWidget::setSize(int width, int height) {
    _dx = width;
    _dy = height;
    setMinWidth(width);
    setMaxWidth(width);
    setMinHeight(height);
    setMaxHeight(height);
    requestLayout();
}

void CRCoverWidget::setBook(const CRDirEntry * book) {
    if (_book)
        delete _book;
    _book = book ? book->clone() : NULL;
}

/// measure dimensions
void CRCoverWidget::measure(int baseWidth, int baseHeight) {
    CR_UNUSED2(baseWidth, baseHeight);
    _measuredWidth = _dx;
    _measuredHeight = _dy;
}

/// updates widget position based on specified rectangle
void CRCoverWidget::layout(int left, int top, int right, int bottom) {
    _pos.left = left;
    _pos.top = top;
    _pos.right = right;
    _pos.bottom = bottom;
}
/// draws widget with its children to specified surface
void CRCoverWidget::draw(LVDrawBuf * buf) {
    CRUIWidget::draw(buf);
    if (!_book)
        return;
    LVDrawStateSaver saver(*buf);
    lvRect rc = _pos;
    applyMargin(rc);
    setClipRect(buf, rc);
    applyPadding(rc);

    int width = rc.width();
    int height = rc.height();
    // fix proportions
    if (width > height * 8 / 10)
        width = height * 8 / 10;
    else if (height > width * 10 / 8)
        height = width * 10 / 8;
    if (width < 30 || height < 40)
        return; // too small

    LVDrawBuf * cover = coverPageManager->getIfReady(_book, width, height);
    if (!cover) {
        coverPageManager->prepare(_book, width, height, _main->createUpdateCallback());
        return;
    }
    applyAlign(rc, width, height);
    // don't scale
    // draw
    buf->DrawRescaled(cover, rc.left, rc.top, width, height, 0);
}

CRCoverWidget::CRCoverWidget(CRUIMainWidget * main, CRDirEntry * book, int dx, int dy) : _main(main), _book(book) {
    setSize(dx, dy);
    setAlign(CRUI::ALIGN_CENTER);
    setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
}

lvPoint CRCoverWidget::calcCoverSize(int w, int h) {
    lvRect padding;
    getPadding(padding);
    lvRect margin = getMargin();
    int width = w - padding.left - padding.right - margin.left - margin.right;
    int height = h - padding.top - padding.bottom - margin.top - margin.bottom;
    // fix proportions
    if (width > height * 8 / 10)
        width = height * 8 / 10;
    else if (height > width * 10 / 8)
        height = width * 10 / 8;
    return lvPoint(width, height);
}
