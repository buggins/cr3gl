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
    CRUIWidget * _callbackWidget;
    CRUIFileItemWidget(int iconDx, int iconDy, const char * iconRes, CRUIWidget * callbackWidget) : CRUILinearLayout(false), _iconDx(iconDx), _iconDy(iconDy), _entry(NULL), _callbackWidget(callbackWidget) {
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
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf) {
        //CRLog::trace("CRUIFileItemWidget draw enter");
        if (_entry) {
            //CRLog::trace("Book entry");
            LVDrawBuf * buf = coverPageManager->getIfReady(_entry, _iconDx, _iconDy);
            if (buf) {
                //CRLog::trace("Found existing cover");
                CRUIImageRef img(new CRUIDrawBufImage(buf));
                _icon->setImage(img);
            } else {
                //CRLog::trace("Requesting new cover");
                coverPageManager->prepare(_entry, _iconDx, _iconDy, new CoverReadyCallback(_callbackWidget));
                //CRLog::trace("Clearing coverpage");
                _icon->setImage(CRUIImageRef());
            }
        }
        //CRLog::trace("CRUIFileItemWidget draw - calling CRUILinearLayout::draw");
        CRUILinearLayout::draw(buf);
        //CRLog::trace("CRUIFileItemWidget draw exit");
    }
    void setBook(CRDirEntry * entry, int iconDx, int iconDy) {
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
        CRUIListWidget::measure(baseWidth, baseHeight);
    }

    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom) {
        CRUIListWidget::layout(left, top, right, bottom);
    }

    CRUIFileListWidget() : CRUIListWidget(true) {
		setLayoutParams(FILL_PARENT, FILL_PARENT);
		//setBackground("tx_wood_v3.jpg");
        calcCoverSize(deviceInfo.shortSide, deviceInfo.longSide);
        _folderWidget = new CRUIFileItemWidget(_coverDx, _coverDy, "folder_blue", this);
        _bookWidget = new CRUIFileItemWidget(_coverDx, _coverDy, "cr3_logo", this);
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
            res->setBook(NULL, _coverDx, _coverDy);
            res->_line1->setText(L"");
			res->_line2->setText(Utf8ToUnicode(item->getFileName()));
			res->_line3->setText(L"");
			//CRLog::trace("returning folder item");
			return res;
		} else {
			CRUIFileItemWidget * res = _bookWidget;
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
};

void CRUIFolderWidget::setDirectory(CRDirCacheItem * dir)
{
	_dir = dir;
	_fileList->setDirectory(dir);
	requestLayout();
}

CRUIFolderWidget::CRUIFolderWidget(CRUIMainWidget * main) : CRUILinearLayout(true), 	_title(NULL), _fileList(NULL), _dir(NULL), _main(main)
{
	_title = new CRUITitleBarWidget(lString16("File list"));
	addChild(_title);
	_fileList = new CRUIFileListWidget();
	addChild(_fileList);
}

CRUIFolderWidget::~CRUIFolderWidget()
{

}
