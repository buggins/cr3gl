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
		int coverSize = deviceInfo.shortSide / 4;
		_cover->setMinWidth(coverSize);
		_cover->setMinHeight(coverSize * 4 / 3);
		_cover->setBackground(0xC0808000);
		addChild(_cover);
		_layout = new CRUILinearLayout(true);
		addChild(_layout);
		_captionLayout = new CRUILinearLayout(false);
		CRUIImageRef img = resourceResolver->getIcon("moreicon");
		if (img.isNull()) {
			CRLog::trace("cannot load moreicon image");
			img = resourceResolver->getIcon("cancel");
		}
		if (!img.isNull())
			CRLog::trace("img size %d x %d", img->originalWidth(), img->originalHeight());
		_menuButton = new CRUIButton(lString16::empty_str, img); //moreicon
		_menuButton->setStyle("BUTTON_NOBACKGROUND");
		_caption = new CRUITextWidget(lString16(L"Now reading"));
		_caption->setLayoutParams(CRUI::FILL_PARENT, CRUI::WRAP_CONTENT);
		_caption->setBackground(0xE0404040);
		_captionLayout->addChild(_caption);
		_captionLayout->addChild(_menuButton);
		_layout->addChild(_captionLayout);
		_title = new CRUITextWidget(lString16(L"War and Peace"));
		_authors = new CRUITextWidget(lString16(L"Leo Tolstoy"));
		_info = new CRUITextWidget(lString16(L"fb2 3245K 1891"));
		_layout->addChild(_title);
		_layout->addChild(_authors);
		_layout->addChild(_info);
		_layout->setLayoutParams(CRUI::FILL_PARENT, CRUI::FILL_PARENT);
	}
};

class CRUIHomeItemListWidget : public CRUILinearLayout, public CRUIListAdapter {
	CRUITextWidget * _caption;
	CRUIListWidget * _list;
	CRUILinearLayout * _itemWidget;
	CRUIImageWidget * _itemImage;
	CRUITextWidget * _textWidget;
public:
	CRUIHomeItemListWidget(lString16 caption) : CRUILinearLayout(true) {
		_caption = new CRUITextWidget(caption);
		_caption->setLayoutParams(CRUI::FILL_PARENT, CRUI::WRAP_CONTENT);
		_caption->setPadding(3);
		_caption->setFont(currentTheme->getFontForSize(CRUI::FONT_SIZE_SMALL));
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
		_list->setBackground(0xE0000000);
		_list->setPadding(4);
		addChild(_list);

		_itemImage = new CRUIImageWidget(CRUIImageRef());
		_itemImage->setAlign(CRUI::ALIGN_HCENTER | CRUI::ALIGN_TOP);
		_itemImage->setLayoutParams(CRUI::WRAP_CONTENT, CRUI::FILL_PARENT);

		_textWidget = new CRUITextWidget(lString16());
		_textWidget->setAlign(CRUI::ALIGN_TOP | CRUI::ALIGN_HCENTER);
		_textWidget->setFont(currentTheme->getFontForSize(CRUI::FONT_SIZE_XSMALL));

		_itemWidget = new CRUILinearLayout(true);
		_itemWidget->addChild(_itemImage);
		_itemWidget->addChild(_textWidget);
	}

	virtual int getItemCount(CRUIListWidget * list) {
		return 20;
	}
	virtual lString16 getItemText(int index) {
		char s[100];
		sprintf(s, "item%d", index);
		return lString16(s);
	}
	virtual CRUIImageRef getItemIcon(int index) {
		return resourceResolver->getIcon("cancel");
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
	CRUIFileSystemDirsWidget() : CRUIHomeItemListWidget(lString16(L"Browse file system")) {

	}
};

class CRUILibraryWidget : public CRUIHomeItemListWidget {
public:
	CRUILibraryWidget() : CRUIHomeItemListWidget(lString16(L"Library")) {

	}
};

class CRUIOnlineCatalogsWidget : public CRUIHomeItemListWidget {
public:
	CRUIOnlineCatalogsWidget() : CRUIHomeItemListWidget(lString16(L"Online Catalogs")) {

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
	setStyle("HOME_WIDGET");
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

