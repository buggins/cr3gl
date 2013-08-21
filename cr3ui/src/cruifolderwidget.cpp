/*
 * cruifolderwidget.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */


#include "crui.h"
#include "cruifolderwidget.h"
#include "cruilist.h"

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
		_caption->setFontSize(FONT_SIZE_SMALL);
		_caption->setBackground(0xE0404040);
		_caption->setAlign(ALIGN_LEFT|ALIGN_TOP);
		addChild(_backButton);
		addChild(_caption);
		addChild(_menuButton);
		setMinHeight(MIN_ITEM_PX);
		setBackground("tx_wood_v3.jpg");
	}
};

class CRUIFileListWidget : public CRUIListWidget {
protected:
	CRDirCacheItem * _dir;
public:
	CRUIFileListWidget() {
		setLayoutParams(FILL_PARENT, FILL_PARENT);
	}
	virtual int getItemCount() {
		return 0;
	}
	virtual CRUIWidget * getItemWidget(int index) {
		return NULL;
	}
	virtual void setDirectory(CRDirCacheItem * dir)
	{
		_dir = dir;
		requestLayout();
	}
};

void CRUIFolderWidget::setDirectory(CRDirCacheItem * dir)
{
	_dir = dir;
	_fileList->setDirectory(dir);
	requestLayout();
}

CRUIFolderWidget::CRUIFolderWidget() : _title(NULL), _fileList(NULL), _dir(NULL)
{
	_title = new CRUITitleBarWidget(lString16("File list"));
	addChild(_title);
	_fileList = new CRUIFileListWidget();
	addChild(_fileList);
}

CRUIFolderWidget::~CRUIFolderWidget()
{

}
