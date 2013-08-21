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

using namespace CRUI;

class CRUITitleBarWidget : public CRUILinearLayout {
	CRUIButton * _backButton;
	CRUIButton * _menuButton;
	CRUITextWidget * _caption;
public:
	CRUITitleBarWidget(lString16 title) : CRUILinearLayout(false) {
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
		setBackground("tx_wood_v3.jpg");
		//_caption->setBackground(0xC0C0C040);
	}
};

class CRUIFileItemWidget : public CRUILinearLayout {
public:
	CRUIImageWidget * _icon;
	CRUILinearLayout * _layout;
	CRUILinearLayout * _infolayout;
	CRUITextWidget * _line1;
	CRUITextWidget * _line2;
	CRUITextWidget * _line3;
	CRUITextWidget * _line4;
	CRUIFileItemWidget(const char * iconRes) : CRUILinearLayout(false) {
		_icon = new CRUIImageWidget(iconRes);
		_icon->setMinWidth(MIN_ITEM_PX);
		_icon->setMinHeight(MIN_ITEM_PX);
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
		_line4->setAlign(ALIGN_RIGHT | ALIGN_BOTTOM);
		_layout->addChild(_line1);
		_layout->addChild(_line2);
		_infolayout->addChild(_line3);
		_infolayout->addChild(_line4);
		_line3->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
		_layout->addChild(_infolayout);
		_layout->setPadding(lvRect(PT_TO_PX(2), 0, PT_TO_PX(1), 0));
		_layout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
		//_layout->setBackground(0x80C0C0C0);
		addChild(_layout);
		setMinWidth(MIN_ITEM_PX);
		setMinHeight(MIN_ITEM_PX);
		setMargin(PT_TO_PX(1));
		setStyle("LIST_ITEM");
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
	CRUIFileItemWidget * _folderWidget;
	CRUIFileItemWidget * _bookWidget;
public:
	CRUIFileListWidget() : CRUIListWidget(true) {
		setLayoutParams(FILL_PARENT, FILL_PARENT);
		//setBackground("tx_wood_v3.jpg");
		_folderWidget = new CRUIFileItemWidget("folder_blue");
		_bookWidget = new CRUIFileItemWidget("cr3_logo");
		setStyle("FILE_LIST");
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
			CRUIFileItemWidget * res = _folderWidget;
			res->_line1->setText(L"");
			res->_line2->setText(Utf8ToUnicode(item->getFileName()));
			res->_line3->setText(L"");
			//CRLog::trace("returning folder item");
			return res;
		} else {
			CRUIFileItemWidget * res = _bookWidget;
			BookDBBook * book = item->getBook();
			lString16 text1;
			lString16 text2;
			lString16 text3;
			lString16 text4;
			if (book) {
				text2 = Utf8ToUnicode(book->title.c_str());
				for (int i = 0; i<book->authors.length(); i++) {
					if (text1.length())
						text1 += L", ";
					text1 += Utf8ToUnicode(book->authors[i]->name.c_str());
				}
				if (!book->series.isNull()) {
					text3 += Utf8ToUnicode(book->series->name.c_str());
					if (book->seriesNumber) {
						if (text3.length())
							text3 += " ";
						text3 += "#";
						text3 += lString16::itoa(book->seriesNumber);
					}
				}
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
};

void CRUIFolderWidget::setDirectory(CRDirCacheItem * dir)
{
	_dir = dir;
	_fileList->setDirectory(dir);
	requestLayout();
}

CRUIFolderWidget::CRUIFolderWidget() : CRUILinearLayout(true), 	_title(NULL), _fileList(NULL), _dir(NULL)
{
	_title = new CRUITitleBarWidget(lString16("File list"));
	addChild(_title);
	_fileList = new CRUIFileListWidget();
	addChild(_fileList);
}

CRUIFolderWidget::~CRUIFolderWidget()
{

}
