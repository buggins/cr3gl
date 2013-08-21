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
		_backButton = new CRUIImageButton("cancel");
		_caption = new CRUITextWidget(title);
		_caption->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
		_caption->setFontSize(FONT_SIZE_MEDIUM);
		_caption->setBackground(0xa0404040);
		_caption->setAlign(ALIGN_LEFT|ALIGN_CENTER);
		_caption->setPadding(PT_TO_PX(2));
		addChild(_backButton);
		addChild(_caption);
		addChild(_menuButton);
		setMinHeight(MIN_ITEM_PX);
		setBackground("tx_wood_v3.jpg");
	}
};

class CRUIFileItemWidget : public CRUILinearLayout {
public:
	CRUIImageWidget * _icon;
	CRUILinearLayout * _layout;
	CRUITextWidget * _line1;
	CRUITextWidget * _line2;
	CRUITextWidget * _line3;
	CRUIFileItemWidget(const char * iconRes) : CRUILinearLayout(false) {
		_icon = new CRUIImageWidget(iconRes);
		_icon->setMinWidth(MIN_ITEM_PX);
		_icon->setMinHeight(MIN_ITEM_PX);
		_icon->setAlign(ALIGN_CENTER);
		_icon->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
		addChild(_icon);
		_layout = new CRUILinearLayout(true);
		_line1 = new CRUITextWidget();
		_line1->setFontSize(FONT_SIZE_SMALL);
		_line2 = new CRUITextWidget();
		_line2->setFontSize(FONT_SIZE_MEDIUM);
		_line3 = new CRUITextWidget();
		_line3->setFontSize(FONT_SIZE_SMALL);
		_layout->addChild(_line1);
		_layout->addChild(_line2);
		_layout->addChild(_line3);
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
			if (book) {
				text2 = Utf8ToUnicode(book->title.c_str());
				text1 = lString16("Author name");
				text3 = lString16("Moonlight, #1; FB2 1024K");
			} else {
				text2 = Utf8ToUnicode(item->getFileName());
			}
			if (text2.empty())
				text2 = Utf8ToUnicode(item->getFileName());
			res->_line1->setText(text1);
			res->_line2->setText(text2);
			res->_line3->setText(text3);
			//CRLog::trace("returning book item");
			return res;
		}
	}
	virtual void setDirectory(CRDirCacheItem * dir)
	{
		_dir = dir;
		requestLayout();
		if (dir)
			CRLog::trace("setDirectory(%s)", dir->getPathName().c_str());
		else
			CRLog::trace("setDirectory(NULL)");
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
