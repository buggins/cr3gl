/*
 * cuihomewidget.cpp
 *
 *  Created on: Aug 20, 2013
 *      Author: vlopatin
 */

#include "cruihomewidget.h"
#include "cruicontrols.h"
#include "crui.h"

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
public:
	CRUINowReadingWidget() : CRUILinearLayout(false) {
		_coverImage = new CRUISolidFillImage(0xE0E0A0);
		_cover = new CRUIImageWidget(_coverImage);
		_cover->setMinWidth(deviceInfo.shortSide / 6);
		_cover->setMinHeight(deviceInfo.longSide / 6);
		addChild(_cover);
		_layout = new CRUILinearLayout(true);
		addChild(_layout);
		_captionLayout = new CRUILinearLayout(false);
		_menuButton = new CRUIButton(lString16::empty_str, "cancel");
		_caption = new CRUITextWidget(lString16(L"Now reading"));
		_captionLayout->addChild(_caption);
		_captionLayout->addChild(_menuButton);
		_layout->addChild(_captionLayout);
		_title = new CRUITextWidget(lString16(L"War and Peace"));
		_authors = new CRUITextWidget(lString16(L"Leo Tolstoy"));
		_info = new CRUITextWidget(lString16(L"fb2 3245K 1891"));
		_layout->addChild(_title);
		_layout->addChild(_authors);
		_layout->addChild(_info);
	}
};

class CRUIHomeItemListWidget : public CRUILinearLayout, public CRUIListAdapter {
	CRUITextWidget * _caption;
	CRUIListWidget * _list;
public:
	CRUIHomeItemListWidget(lString16 caption) : CRUILinearLayout(true) {
		_caption = new CRUITextWidget(caption);
		addChild(_caption);
		_list = new CRUIListWidget(false, this);
		addChild(_list);
	}

	virtual int getItemCount(CRUIListWidget * list) {
		return 0;
	}
	virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index) {
		return NULL;
	}
};

class CRUIFileSystemDirsWidget : public CRUIHomeItemListWidget {
public:
	CRUIFileSystemDirsWidget() : CRUIHomeItemListWidget(lString16(L"Browse file system")) {

	}
};

class CRUILibraryWidget : public CRUIHomeItemListWidget {
public:
	CRUILibraryWidget() : CRUIHomeItemListWidget(lString16(L"Browse file system")) {

	}
};

class CRUIOnlineCatalogsWidget : public CRUIHomeItemListWidget {
public:
	CRUIOnlineCatalogsWidget() : CRUIHomeItemListWidget(lString16(L"Browse file system")) {

	}
};

class CRUIRecentBooksListWidget : public CRUIHomeItemListWidget {
public:
	CRUIRecentBooksListWidget() : CRUIHomeItemListWidget(lString16(L"Recently read books")) {
	}

	virtual int getItemCount(CRUIListWidget * list) {
		return 0;
	}
	virtual CRUIWidget * getItemWidget(CRUIListWidget * list, int index) {
		return NULL;
	}
};

CRUIHomeWidget::CRUIHomeWidget() {
	_currentBook = new CRUINowReadingWidget();
	_recentBooksList = new CRUIRecentBooksListWidget();
	_fileSystem = new CRUIFileSystemDirsWidget();
	_library = new CRUILibraryWidget();
	_onlineCatalogsList = new CRUIOnlineCatalogsWidget();
	addChild(_currentBook);
	addChild(_recentBooksList);
	addChild(_fileSystem);
	addChild(_library);
	addChild(_onlineCatalogsList);
	setBackground(0x808080);
}

/// measure dimensions
void CRUIHomeWidget::measure(int baseWidth, int baseHeight)
{
	_measuredWidth = baseWidth;
	_measuredHeight = baseHeight;
	bool vertical = baseWidth < baseHeight;
	if (vertical) {
		int nowReadingH = baseHeight / 3;
		int otherH = (baseHeight - nowReadingH) / 4;
		_currentBook->measure(baseWidth, otherH);
		_recentBooksList->measure(baseWidth, otherH);
		_fileSystem->measure(baseWidth, otherH);
		_library->measure(baseWidth, otherH);
		_onlineCatalogsList->measure(baseWidth, otherH);
	} else {
		int nowReadingH = baseHeight / 3;
		int otherH = (baseHeight - nowReadingH) / 2;
		_currentBook->measure(baseWidth, otherH);
		_recentBooksList->measure(baseWidth / 2, otherH);
		_fileSystem->measure(baseWidth / 2, otherH);
		_library->measure(baseWidth / 2, otherH);
		_onlineCatalogsList->measure(baseWidth / 2, otherH);
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
		int nowReadingH = h / 3;
		int otherH = (h - nowReadingH) / 4;
		int y = top;
		_currentBook->layout(left, y, right, y + nowReadingH);
		y += nowReadingH;
		_recentBooksList->layout(left, y, right, y + otherH); y += otherH;
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

