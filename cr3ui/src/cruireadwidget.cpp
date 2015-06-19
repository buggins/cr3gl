/*
 * cruireadwidget.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

// uncomment to simulate slow render
//#define SLOW_RENDER_SIMULATION

#include "stringresource.h"
#include "cruireadwidget.h"
#include "crui.h"
#include "cruimain.h"
#include "gldrawbuf.h"
#include "fileinfo.h"
#include "cruiconfig.h"
#include "lvstsheet.h"
#include "hyphman.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace CRUI;

#define SELECTION_LONG_TAP_TIMER_ID 123001
#define SELECTION_LONG_TAP_DELAY_MILLIS 800
#define GO_TO_PERCENT_REPEAT_TIMER_ID 123002
#define GO_TO_PERCENT_REPEAT_TIMER_DELAY 250
#define SAVE_POSITION_TIMER_ID 123003
#define SAVE_POSITION_DELAY_MILLIS 1000

lUInt32 applyAlpha(lUInt32 cl1, lUInt32 cl2, int alpha) {
	if (alpha <=0)
		return cl1;
	else if (alpha >= 255)
		return cl2;
    lUInt32 opaque = 256 - alpha;
    lUInt32 n1 = (((cl2 & 0xFF00FF) * alpha + (cl1 & 0xFF00FF) * opaque) >> 8) & 0xFF00FF;
    lUInt32 n2 = (((cl2 >> 8) & 0xFF00FF) * alpha + ((cl1 >> 8) & 0xFF00FF) * opaque) & 0xFF00FF00;
    return n1 | n2;
}

CRUIDocView::CRUIDocView() : LVDocView() {
    //background = resourceResolver->getIcon("leather.jpg", true);
    _coverColor = 0xA04030;
    _showCover = true;
    _pageAnimationSupportsCoverFrame = true;
    background = CRUIImageRef(new CRUISolidFillImage(0xFFFFFF));
}

/// applies properties, returns list of not recognized properties
CRPropRef CRUIDocView::propsApply(CRPropRef props) {
    //CRPropRef oldSettings = propsGetCurrent();
    CRPropRef newSettings = propsGetCurrent() | props;
    CRPropRef forDocview = LVCreatePropsContainer();
    bool backgroundChanged = false;
    //bool needClearCache = false;
    for (int i = 0; i < props->getCount(); i++) {
        lString8 key(props->getName(i));
        //lString8 value(UnicodeToUtf8(props->getValue(i)));
        if (key == PROP_PAGE_MARGINS) {
            int marginPercent = props->getIntDef(key.c_str(), 5000);
            int hmargin = deviceInfo.shortSide * marginPercent / 10000;
            lvRect margins(hmargin, hmargin / 2, hmargin, hmargin / 2);
            if (propsGetCurrent()->getIntDef(PROP_PAGE_MARGIN_LEFT, 0) != margins.left)
                forDocview->setInt(PROP_PAGE_MARGIN_LEFT, margins.left);
            if (propsGetCurrent()->getIntDef(PROP_PAGE_MARGIN_RIGHT, 0) != margins.right)
                forDocview->setInt(PROP_PAGE_MARGIN_RIGHT, margins.right);
            if (propsGetCurrent()->getIntDef(PROP_PAGE_MARGIN_TOP, 0) != margins.top)
                forDocview->setInt(PROP_PAGE_MARGIN_TOP, margins.top);
            if (propsGetCurrent()->getIntDef(PROP_PAGE_MARGIN_BOTTOM, 0) != margins.bottom)
                forDocview->setInt(PROP_PAGE_MARGIN_BOTTOM, margins.bottom);
        } else if (key == PROP_APP_BOOK_COVER_VISIBLE) {
            _showCover = props->getBoolDef(key.c_str(), true);
        } else if (key == PROP_APP_BOOK_COVER_COLOR) {
            _coverColor = props->getColorDef(key.c_str(), 0xA04030);
        } else if (key == PROP_PAGE_VIEW_ANIMATION) {
            _pageAnimationSupportsCoverFrame =
                    props->getIntDef(PROP_PAGE_VIEW_ANIMATION, 0) == PAGE_ANIMATION_3D ||
                    props->getIntDef(PROP_PAGE_VIEW_ANIMATION, 0) == PAGE_ANIMATION_NONE ||
                    props->getIntDef(PROP_PAGE_VIEW_ANIMATION, 0) == PAGE_ANIMATION_FADE
                    ;
        } else if (key == PROP_FONT_ANTIALIASING) {
            int antialiasingMode = props->getIntDef(PROP_FONT_ANTIALIASING, 2);
            if (antialiasingMode == 1) {
                antialiasingMode = 2;
            }
            if (fontMan->GetAntialiasMode() != antialiasingMode) {
                fontMan->SetAntialiasMode(antialiasingMode);
            }
            requestRender();
        } else if (key == PROP_FONT_HINTING) {
            bool bytecode = props->getBoolDef(PROP_FONT_HINTING, 1);
            int hintingMode = bytecode ? HINTING_MODE_BYTECODE_INTERPRETOR : HINTING_MODE_AUTOHINT;
            if ((int)fontMan->GetHintingMode() != hintingMode && hintingMode >= 0 && hintingMode <= 2) {
                //CRLog::debug("Setting hinting mode to %d", mode);
                fontMan->SetHintingMode((hinting_mode_t)hintingMode);
            }
            requestRender();
        } else if (key == PROP_FONT_GAMMA_INDEX) {
            int gammaIndex = props->getIntDef(PROP_FONT_GAMMA_INDEX, 15);
            int oldGammaIndex = fontMan->GetGammaIndex();
            if (oldGammaIndex != gammaIndex) {
                fontMan->SetGammaIndex(gammaIndex);
            }
        } else {
            forDocview->setString(key.c_str(), props->getValue(i));
        }
        if (key == PROP_BACKGROUND_COLOR
                || key == PROP_BACKGROUND_IMAGE
                || key == PROP_BACKGROUND_IMAGE_ENABLED
                || key == PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS
                || key == PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST) {
            propsGetCurrent()->setString(key.c_str(), props->getValue(i));
            backgroundChanged = true;
            //needClearCache = true;
        }
    }
    CRPropRef res = LVDocView::propsApply(forDocview);
    if (backgroundChanged) {
        //setBackground(resourceResolver->getBackgroundImage(newSettings));
        setBackground(resourceResolver->getBackgroundImage(propsGetCurrent()));
    }
    return res;
}

/// clears page background
void CRUIDocView::drawPageBackground( LVDrawBuf & drawbuf, int offsetX, int offsetY, int alpha) {
//    	CRUIImageRef background = resourceResolver->getIcon("paper1.jpg", true);
//        CRUIImageRef backgroundScrollLeft = resourceResolver->getIcon("scroll-edge-left", true);
//        CRUIImageRef backgroundScrollRight = resourceResolver->getIcon("scroll-edge-right", true);
    lvRect rc(0, 0, drawbuf.GetWidth(), drawbuf.GetHeight());
    if (crconfig.einkMode) {
        drawbuf.FillRect(rc, 0xFFFFFF);
        return;
    }

    LVDrawStateSaver s(drawbuf);
    CR_UNUSED(s);
    drawbuf.setAlpha(alpha);
    background->draw(&drawbuf, rc, offsetX, offsetY);
}

void CRUIDocView::setBackground(CRUIImageRef img) {
    background = img;
}

lString16 CRUIDocView::getLink( int x, int y, int r )
{
    int step = 5;
    int n = r / step;
    r = n * step;
    lString16 lnk = getLink(x, y);
    if (r==0 || !lnk.empty())
        return lnk;
    lString16 link;
    for ( int xx = -r; xx<=r; xx+=step ) {
        link = getLink( x+xx, y-r );
        if ( !link.empty() )
            return link;
        link = getLink( x+xx, y+r );
        if ( !link.empty() )
            return link;
    }
    for ( int yy = -r + step; yy<=r - step; yy+=step ) {
        link = getLink( x+r, y+yy );
        if ( !link.empty() )
            return link;
        link = getLink( x-r, y+yy );
        if ( !link.empty() )
            return link;
    }
    return lString16::empty_str;
}

lvRect CRUIDocView::calcCoverFrameWidths(lvRect rc)
{
    // calc page count
    int pageCount = getPagesVisibleSetting();
    int fsz = getFontSize();
    if (rc.width() / 2 / fsz < 15)
        pageCount = 1;
    overrideVisiblePageCount(pageCount);

    lvRect res;
    if (!_showCover || !_pageAnimationSupportsCoverFrame)
        return res;

    int visiblePages = getVisiblePageCount();
    bool isPageMode = getViewMode() == DVM_PAGES;
    if (!isPageMode)
        return res;
    int fw = rc.minDimension() / 60 + PT_TO_PX(2);
    res.top = res.bottom = fw;
    res.right = fw * 3 / 2 + PT_TO_PX(1);
    if (visiblePages > 1)
        res.left = fw * 3 / 2 + PT_TO_PX(1);
    return res;
}

void CRUIDocView::drawCoverFrame(LVDrawBuf & drawbuf, lvRect outerRect, lvRect innerRect) {
    if (outerRect.top >= innerRect.top)
        return;
    drawbuf.FillRect(outerRect.left, outerRect.top, outerRect.right, innerRect.top, _coverColor);
    drawbuf.FillRect(outerRect.left, innerRect.bottom, outerRect.right, outerRect.bottom, _coverColor);
    drawbuf.FillRect(outerRect.left, innerRect.top, innerRect.left, innerRect.bottom, _coverColor);
    drawbuf.FillRect(innerRect.right, innerRect.top, outerRect.right, innerRect.bottom, _coverColor);
    int dw = (innerRect.top - outerRect.top) * 40 / 100;
    int maxalpha = 128;
    for (int i = 0; i < dw; i++ ) {

        lUInt32 cl = (255 - (dw - i) * maxalpha / dw) << 24;
        if (i == 0)
            cl = 0x40000000;
        if (dw > 5 && i == 1)
            cl = 0x60000000;
        lUInt32 topcl = cl | 0x202020;
        lvRect rc = outerRect;
        rc.top += i;
        rc.right -= i;
        rc.bottom -= i;
        if (outerRect.left != innerRect.left) {
            rc.left += i;
            drawbuf.FillRect(rc.left, rc.top + 1, rc.left + 1, rc.bottom - 1, topcl);
        }
        drawbuf.FillRect(rc.left, rc.top, rc.right, rc.top + 1, topcl);
        drawbuf.FillRect(rc.right - 1, rc.top + 1, rc.right, rc.bottom - 1, cl);
        drawbuf.FillRect(rc.left, rc.bottom - 1, rc.right, rc.bottom, cl);
    }
    outerRect.left = (outerRect.left + innerRect.left * 2) / 3;
    outerRect.right = (outerRect.right + innerRect.right * 2) / 3;
    outerRect.top = innerRect.top;
    outerRect.bottom = innerRect.bottom;
    lvRect rc = outerRect;
    rc.right = innerRect.left;
    if (rc.right > rc.left) {
        LVDrawStateSaver saver(drawbuf);
        CR_UNUSED(saver);
        drawbuf.SetClipRect(&rc);
        drawPageBackground(drawbuf, 0, 0, 0);
        drawbuf.GradientRect(rc.left, rc.top, rc.right, rc.bottom, 0xE0000000, 0xC0000000, 0xC0000000, 0xE0000000);
        drawbuf.FillRect(rc.right - 1, rc.top, rc.right, rc.bottom, 0xC0000000);
        rc.right = rc.left + rc.width() * 60 / 100;
        drawbuf.FillRect(rc.right - 1, rc.top, rc.right, rc.bottom, 0xC0000000);
        drawbuf.GradientRect(rc.left, rc.top, rc.right, rc.bottom, 0xE0000000, 0xA0000000, 0xA0000000, 0xE0000000);
        drawbuf.FillRect(rc.left, rc.top, rc.left + 1, rc.bottom, 0xC0000000);
        if (rc.width() > 3) {
            rc.right = rc.left + rc.width() * 50 / 100;
            drawbuf.FillRect(rc.right - 1, rc.top, rc.right, rc.bottom, 0xD0000000);
            drawbuf.GradientRect(rc.left, rc.top, rc.right, rc.bottom, 0xF0000000, 0xC0000000, 0xC0000000, 0xF0000000);
        }
    }
    rc = outerRect;
    rc.left = innerRect.right;
    {
        LVDrawStateSaver saver(drawbuf);
        CR_UNUSED(saver);
        drawbuf.SetClipRect(&rc);
        drawPageBackground(drawbuf, 0, 0, 0);
        drawbuf.GradientRect(rc.left, rc.top, rc.right, rc.bottom, 0xC0000000, 0xE0000000, 0xE0000000, 0xC0000000);
        drawbuf.FillRect(rc.left, rc.top, rc.left + 1, rc.bottom, 0xC0000000);
        rc.left = rc.right - rc.width() * 60 / 100;
        drawbuf.FillRect(rc.left, rc.top, rc.left + 1, rc.bottom, 0xC0000000);
        drawbuf.GradientRect(rc.left, rc.top, rc.right, rc.bottom, 0xA0000000, 0xE0000000, 0xE0000000, 0xA0000000);
        drawbuf.FillRect(rc.right - 1, rc.top, rc.right, rc.bottom, 0xC0000000);
        if (rc.width() > 3) {
            rc.left = rc.right - rc.width() * 50 / 100;
            drawbuf.FillRect(rc.left, rc.top, rc.left + 1, rc.bottom, 0xD0000000);
            drawbuf.GradientRect(rc.left, rc.top, rc.right, rc.bottom, 0xC0000000, 0xF0000000, 0xF0000000, 0xC0000000);
        }
    }
}

lString16 CRUIDocView::getLink( int x, int y )
{
    lvPoint pt(x, y);
//    if (!windowToDocPoint(pt))
//        return lString16();
    ldomXPointer p = getNodeByPoint(pt);
    if ( p.isNull() )
        return lString16::empty_str;
    lString16 href = p.getHRef();
    return href;
}

class CRUIReadMenu : public CRUIFrameLayout, CRUIOnClickListener, CRUIOnScrollPosCallback {
    CRUIReadWidget * _window;
    CRUIActionList _actionList;
    LVPtrVector<CRUIButton, false> _buttons;
    CRUILinearLayout * _scrollLayout;
    CRUITextWidget * _positionText;
    CRUISliderWidget * _scrollSlider;
    lvPoint _itemSize;
    int _btnCols;
    int _btnRows;
    int _maxRows;
    bool _labels;
    bool _vertical;
public:
    CRUIReadMenu(CRUIReadWidget * window, const CRUIActionList & actionList, bool progressControl = true, bool labels = true, int maxRows = 0) : _window(window), _actionList(actionList) {
        _maxRows = maxRows;
        _labels = labels;
        _vertical = false;
        setId("MAINMENU");
        for (int i = 0; i < _actionList.length(); i++) {
            const CRUIAction * action = _actionList[i];
            lString16 title = labels ? action->getName() : lString16::empty_str;
            CRUIButton * button = new CRUIButton(title, action->icon_res.c_str(), true);
            button->setId(lString8::itoa(action->id));
            button->setOnClickListener(this);
            button->setStyle("BUTTON_NOBACKGROUND");
            button->setPadding(lvRect(PT_TO_PX(2), PT_TO_PX(2), PT_TO_PX(2), PT_TO_PX(2)));
            button->setFontSize(FONT_SIZE_XSMALL);
            if (labels) {
                CRUITextWidget* caption = (CRUITextWidget*)button->childById("BUTTON_CAPTION");
                caption->setMaxLines(2)->setFontSize(FONT_SIZE_XSMALL);
            }
            //if (!labels)
            //    caption->setVisibility(CRUI::GONE);
            _buttons.add(button);
            addChild(button);
        }
        if (progressControl) {
            _scrollLayout = new CRUIVerticalLayout();
            _scrollLayout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    //        CRUIWidget * delimiter = new CRUIWidget();
    //        delimiter->setBackground(0xC0000000);
    //        delimiter->setMinHeight(PT_TO_PX(2));
    //        delimiter->setMaxHeight(PT_TO_PX(2));
    //        _scrollLayout->addChild(delimiter);
            _positionText = new CRUITextWidget();
            _positionText->setText(_window->getCurrentPositionDesc());
            _positionText->setPadding(lvRect(PT_TO_PX(8), MIN_ITEM_PX / 8, PT_TO_PX(2), 0));
            _positionText->setFontSize(FONT_SIZE_SMALL);
            _scrollLayout->addChild(_positionText);
            _scrollSlider = new CRUISliderWidget(0, 10000, _window->getCurrentPositionPercent());
            _scrollSlider->setScrollPosCallback(this);
            _scrollSlider->setMaxHeight(MIN_ITEM_PX * 3 / 4);
            _scrollLayout->addChild(_scrollSlider);
            _scrollLayout->setBackground("home_frame.9");
            addChild(_scrollLayout);
        } else {
            _scrollLayout = NULL;
            _scrollSlider = NULL;
            _positionText = NULL;
        }
    }
    virtual bool isVertical() {
        return _vertical;
    }

    virtual void setVertical(bool v) {
        if (_vertical != v) {
            _vertical = v;
            requestLayout();
        }
    }

    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight) {
        if (getVisibility() == GONE) {
            _measuredWidth = 0;
            _measuredHeight = 0;
            return;
        }
        CRUIImageRef icon = resourceResolver->getIcon(_actionList[0]->icon_res.c_str());
        LVFontRef font = currentTheme->getFontForSize(FONT_SIZE_XSMALL);
        int iconh = icon->originalHeight();
        int iconw = icon->originalWidth();
        int texth = _labels ? font->getHeight() * 2 : 0;
        _itemSize.y = iconh + texth * 170 / 100 + PT_TO_PX(2);
        _itemSize.x = iconw * 120 / 100 + PT_TO_PX(4);
        if (_itemSize.y < MIN_ITEM_PX)
        	_itemSize.y = MIN_ITEM_PX;
        if (_itemSize.x < MIN_ITEM_PX)
        	_itemSize.x = MIN_ITEM_PX;
        int count = _actionList.length();
        if (_vertical) {
            int rows = baseHeight / _itemSize.y;
            if (rows < 1)
                rows = 1;
            if (rows > _buttons.length())
                rows = _buttons.length();
            int cols = 1;
            _btnCols = cols;
            _btnRows = rows; // + 1;
            if (_scrollLayout)
                _scrollLayout->measure(baseWidth, baseHeight);
            int height = baseHeight;
            int width = _btnCols * _itemSize.x + PT_TO_PX(3);
            defMeasure(baseWidth, baseHeight, width, height);
        } else {
            int cols = baseWidth / _itemSize.x;
            if (cols < 1)
                cols = 1;
            int rows = (count + (cols - 1)) / cols;
            if (_maxRows == 0) {
                while (cols > 2 && (count + (cols - 1 - 1)) / (cols - 1) == rows)
                    cols--;
            } else {
                if (rows > _maxRows)
                    rows = _maxRows;
            }
            _btnCols = cols;
            _btnRows = rows; // + 1;
            if (_scrollLayout)
                _scrollLayout->measure(baseWidth, baseHeight);
            int width = baseWidth;
            int height = _btnRows * _itemSize.y + PT_TO_PX(3) + (_scrollLayout ? _scrollLayout->getMeasuredHeight() : 0);
            defMeasure(baseWidth, baseHeight, width, height);
        }
    }

    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom) {
        CRUIWidget::layout(left, top, right, bottom);
        int count = _actionList.length();
        lvRect rc = _pos;
        applyMargin(rc);
        applyPadding(rc);
        if (_vertical) {
            int rowh = rc.height() / _buttons.length();
            if (rowh < _itemSize.y)
                rowh = _itemSize.y;
            if (rowh > _itemSize.y * 130 / 100)
                rowh = _itemSize.y * 130 / 100;
            lvRect btnrc = rc;
            for (int y = 0; y < _buttons.length(); y++) {
                btnrc.bottom = btnrc.top + rowh;
                CRUIButton * button = _buttons[y];
                if (btnrc.bottom < rc.bottom) {
                    button->setVisibility(CRUI::VISIBLE);
                    button->measure(btnrc.width(), btnrc.height());
                    button->layout(btnrc.left, btnrc.top, btnrc.right, btnrc.bottom);
                } else {
                    button->setVisibility(CRUI::GONE);
                }
                btnrc.top += rowh;
            }
        } else if (_maxRows == 1 && !_scrollLayout) {
            int colw = rc.width() / _buttons.length();
            if (colw < _itemSize.x)
                colw = _itemSize.x;
            if (colw > _itemSize.x * 130 / 100)
                colw = _itemSize.x * 130 / 100;
            lvRect btnrc = rc;
            for (int y = 0; y < _buttons.length(); y++) {
                btnrc.right = btnrc.left + colw;
                CRUIButton * button = _buttons[y];
                if (btnrc.right <= rc.right) {
                    button->setVisibility(CRUI::VISIBLE);
                    button->measure(btnrc.width(), btnrc.height());
                    button->layout(btnrc.left, btnrc.top, btnrc.right, btnrc.bottom);
                } else {
                    button->setVisibility(CRUI::GONE);
                }
                btnrc.left += colw;
            }
        } else {
            if (_scrollLayout) {
                _scrollLayout->layout(rc.left, rc.bottom - _scrollLayout->getMeasuredHeight(), rc.right, rc.bottom);
                rc.bottom -= _scrollLayout->getMeasuredHeight();
            }
            int rowh = rc.height() / _btnRows;
            lvRect rowrc = rc;
            for (int y = 0; y < _btnRows; y++) {
                int i0 = _btnCols * y;
                int rowlen = count - i0;
                if (rowlen > _btnCols)
                    rowlen = _btnCols;
                rowrc.bottom = rowrc.top + rowh;
                lvRect btnrc = rowrc;
                int itemw = rowrc.width() / rowlen;
                for (int x = 0; x < rowlen; x++) {
                    btnrc.right = btnrc.left + itemw;
                    CRUIButton * button = _buttons[i0 + x];
                    button->measure(btnrc.width(), btnrc.height());
                    button->layout(btnrc.left, btnrc.top, btnrc.right, btnrc.bottom);
                    btnrc.left += itemw;
                }
                rowrc.top += rowh;
            }
        }
    }

    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
        CR_UNUSED(widget);
        if (!manual)
            return false;
        _window->goToPercent(pos);
        _positionText->setText(_window->getCurrentPositionDesc());
        return true;
    }

    virtual bool onClick(CRUIWidget * widget) {
        int id = widget->getId().atoi();
        const CRUIAction * action = NULL;
        for (int i = 0; i < _actionList.length(); i++) {
            if (id == _actionList[i]->id) {
                action = _actionList[i];
            }
        }
        if (action) {
            _window->onMenuItemAction(action);
        }
        return true;
    }
};


class CRUIGoToPercentPopup : public CRUIVerticalLayout, CRUIOnScrollPosCallback {
    CRUIReadWidget * _window;
    CRUITextWidget * _positionText;
    CRUISliderWidget * _scrollSlider;
    int _moveByPageDirection;
public:
    CRUIGoToPercentPopup(CRUIReadWidget * window) : _window(window) {
        setLayoutParams(FILL_PARENT, WRAP_CONTENT);
//        CRUIWidget * delimiter = new CRUIWidget();
//        delimiter->setBackground(0xC0000000);
//        delimiter->setMinHeight(PT_TO_PX(2));
//        delimiter->setMaxHeight(PT_TO_PX(2));
//        _scrollLayout->addChild(delimiter);
        setId("GOTOPERCENT");
        _positionText = new CRUITextWidget();
        _positionText->setText(_window->getCurrentPositionDesc());
        _positionText->setPadding(lvRect(PT_TO_PX(8), MIN_ITEM_PX / 8, PT_TO_PX(2), 0));
        _positionText->setFontSize(FONT_SIZE_SMALL);
        addChild(_positionText);
        _scrollSlider = new CRUISliderWidget(0, 10000, _window->getCurrentPositionPercent());
        _scrollSlider->setScrollPosCallback(this);
        _scrollSlider->setMaxHeight(MIN_ITEM_PX * 5 / 8);
        addChild(_scrollSlider);
        setBackground("home_frame.9");
        _moveByPageDirection = 0;
    }

    virtual ~CRUIGoToPercentPopup() {
        CRUIEventManager::cancelTimer(GO_TO_PERCENT_REPEAT_TIMER_ID);
    }

    virtual bool onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
        CR_UNUSED(widget);
        if (!manual)
            return false;
        _window->goToPercent(pos);
        _positionText->setText(_window->getCurrentPositionDesc());
        return true;
    }

    void moveByPage(int direction) {
        _window->moveByPage(direction);
        _positionText->setText(_window->getCurrentPositionDesc());
        _scrollSlider->setScrollPos(_window->getCurrentPositionPercent());
    }

    /// handle timer event; return true to allow recurring timer event occur more times, false to stop
    virtual bool onTimerEvent(lUInt32 timerId) {
        if (_moveByPageDirection) {
            moveByPage(_moveByPageDirection);
            CRUIEventManager::setTimer(GO_TO_PERCENT_REPEAT_TIMER_ID, this, GO_TO_PERCENT_REPEAT_TIMER_DELAY, false);
        }
        CR_UNUSED(timerId); return false;
    }

    /// motion event handler, returns true if it handled event
    bool onTouchEvent(const CRUIMotionEvent * event) {
        lvPoint pt(event->getX(), event->getY());
        if (!_pos.isPointInside(pt)) {
            if (event->getAction() == ACTION_DOWN) {
                if (event->getX() < _pos.width() / 5) {
                    _moveByPageDirection = -1;
                } else if (event->getX() > _pos.width() * 4 / 5) {
                    _moveByPageDirection = 1;
                }
                if (_moveByPageDirection) {
                    moveByPage(_moveByPageDirection);
                    CRUIEventManager::setTimer(GO_TO_PERCENT_REPEAT_TIMER_ID, this, GO_TO_PERCENT_REPEAT_TIMER_DELAY, false);
                    return true;
                }
            } else if (event->getAction() == ACTION_UP || event->getAction() == ACTION_CANCEL) {
                if (_moveByPageDirection) {
                    _moveByPageDirection = 0;
                    return true;
                }
            }
        }
        return false;
    }

};

class CRUIFindTextPopup : public CRUIHorizontalLayout, public CRUIOnClickListener, public CRUIOnReturnPressListener {
    CRUIReadWidget * _window;
    CRUIEditWidget * _editor;
    CRUIImageButton * _prevButton;
    CRUIImageButton * _nextButton;
public:
    CRUIFindTextPopup(CRUIReadWidget * window, lString16 pattern) : _window(window) {
        setLayoutParams(FILL_PARENT, WRAP_CONTENT);
//        CRUIWidget * delimiter = new CRUIWidget();
//        delimiter->setBackground(0xC0000000);
//        delimiter->setMinHeight(PT_TO_PX(2));
//        delimiter->setMaxHeight(PT_TO_PX(2));
//        _scrollLayout->addChild(delimiter);
        setId("FINDTEXT");

        CRUIVerticalLayout * editlayout = new CRUIVerticalLayout();
        CRUIWidget * spacer1 = new CRUIWidget();
        spacer1->setLayoutParams(FILL_PARENT, FILL_PARENT);
        CRUIWidget * spacer2 = new CRUIWidget();
        spacer2->setLayoutParams(FILL_PARENT, FILL_PARENT);
        _editor = new CRUIEditWidget();
        _editor->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        _editor->setBackgroundAlpha(0x80);
        _editor->setText(pattern);
        _editor->setOnReturnPressedListener(this);
        //_editor->setPasswordChar('*');
        editlayout->addChild(spacer1);
        editlayout->addChild(_editor);
        editlayout->addChild(spacer2);
        editlayout->setLayoutParams(FILL_PARENT, FILL_PARENT);
        editlayout->setMaxHeight(MIN_ITEM_PX * 3 / 4);
        addChild(editlayout);

        // Buttons
        _prevButton = new CRUIImageButton("left_circular");
        _prevButton->setId("FIND_PREV");
        addChild(_prevButton);
        _nextButton = new CRUIImageButton("right_circular");
        _nextButton->setId("FIND_NEXT");
        addChild(_nextButton);
        _prevButton->setMaxHeight(MIN_ITEM_PX * 3 / 4);
        _nextButton->setMaxHeight(MIN_ITEM_PX * 3 / 4);

        _prevButton->setBackgroundAlpha(0x80);
        _nextButton->setBackgroundAlpha(0x80);
        setBackground("home_frame.9");

        _nextButton->setOnClickListener(this);
        _prevButton->setOnClickListener(this);
    }

    virtual bool onReturnPressed(CRUIWidget * widget) {
        CR_UNUSED(widget);
        lString16 text = _editor->getText();
        if (text.empty())
            return true;
        if (!_window->findText(text, 1, false, true))
            _window->findText(text, -1, false, true);
        return true;
    }

    virtual bool onClick(CRUIWidget * widget) {
        lString16 text = _editor->getText();
        if (text.empty())
            return true;
        if (widget->getId() == "FIND_NEXT") {
            if (!_window->findText(text, 1, false, true))
                _window->findText(text, -1, false, true);

        } else if (widget->getId() == "FIND_PREV") {
            if (!_window->findText(text, 1, true, true))
                _window->findText(text, -1, true, true);
        }
        return true;
    }

    /// call to set focus to appropriate child once widget appears on screen
    virtual bool initFocus() {
        CRUIEventManager::dispatchFocusChange(_editor);
        return true;
    }

    virtual ~CRUIFindTextPopup() {
        //CRLog::trace("~CRUIFindTextPopup()");
        //_window->getMain()->cancelTimer(GO_TO_PERCENT_REPEAT_TIMER_ID);
    }

//    /// handle timer event; return true to allow recurring timer event occur more times, false to stop
//    virtual bool onTimerEvent(lUInt32 timerId) {
////        if (_moveByPageDirection) {
////            moveByPage(_moveByPageDirection);
////            _window->getMain()->setTimer(GO_TO_PERCENT_REPEAT_TIMER_ID, this, GO_TO_PERCENT_REPEAT_TIMER_DELAY, false);
////        }
//        CR_UNUSED(timerId); return false;
//    }

};

bool CRUIReadWidget::findText(lString16 pattern, int origin, bool reverse, bool caseInsensitive) {
    if (pattern.empty())
        return false;
    if (pattern != _lastSearchPattern && origin == 1)
        origin = 0;
    _lastSearchPattern = pattern;
    LVArray<ldomWord> words;
    lvRect rc;
    _docview->GetPos( rc );
    int pageHeight = rc.height();
    int start = -1;
    int end = -1;
    if ( reverse ) {
        // reverse
        if ( origin == 0 ) {
            // from end current page to first page
            end = rc.bottom;
        } else if ( origin == -1 ) {
            // from last page to end of current page
            start = rc.bottom;
        } else { // origin == 1
            // from prev page to first page
            end = rc.top;
        }
    } else {
        // forward
        if ( origin == 0 ) {
            // from current page to last page
            start = rc.top;
        } else if ( origin == -1 ) {
            // from first page to current page
            end = rc.top;
        } else { // origin == 1
            // from next page to last
            start = rc.bottom;
        }
    }
    CRLog::debug("CRViewDialog::findText: Current page: %d .. %d", rc.top, rc.bottom);
    CRLog::debug("CRViewDialog::findText: searching for text '%s' from %d to %d origin %d", LCSTR(pattern), start, end, origin );
    if ( _docview->getDocument()->findText( pattern, caseInsensitive, reverse, start, end, words, 200, pageHeight ) ) {
        CRLog::debug("CRViewDialog::findText: pattern found");
        _docview->clearSelection();
        _docview->selectWords( words );
        ldomMarkedRangeList * ranges = _docview->getMarkedRanges();
        if ( ranges ) {
            if ( ranges->length()>0 ) {
                int pos = ranges->get(0)->start.y;
                _docview->SetPos(pos);
            }
        }
        clearImageCaches();
        return true;
    }
    CRLog::debug("CRViewDialog::findText: pattern not found");
    _docview->clearSelection();
    clearImageCaches();
    return false;
}

static bool isDocViewProp(const lString8 & key) {
    return key == PROP_FONT_FACE
            || key == PROP_FONT_COLOR
            || key == PROP_FONT_WEIGHT_EMBOLDEN
            || key == PROP_FONT_SIZE
            || key == PROP_STATUS_FONT_SIZE
            || key == PROP_FONT_FACE
            || key == PROP_STATUS_FONT_FACE
            || key == PROP_STATUS_FONT_COLOR
            || key == PROP_BACKGROUND_COLOR
            || key == PROP_INTERLINE_SPACE
            || key == PROP_HIGHLIGHT_COMMENT_BOOKMARKS
            || key == PROP_HIGHLIGHT_SELECTION_COLOR
            || key == PROP_HIGHLIGHT_BOOKMARK_COLOR_COMMENT
            || key == PROP_HIGHLIGHT_BOOKMARK_COLOR_CORRECTION
            || key == PROP_FLOATING_PUNCTUATION
            || key == PROP_BACKGROUND_IMAGE
            || key == PROP_BACKGROUND_IMAGE_ENABLED
            || key == PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS
            || key == PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST
            || key == PROP_FONT_GAMMA_INDEX
            || key == PROP_FONT_ANTIALIASING
            || key == PROP_FONT_WEIGHT_EMBOLDEN
            || key == PROP_PAGE_MARGINS
            || key == PROP_FONT_HINTING;
}

void drawVGradient(LVDrawBuf * buf, lvRect & rc, lUInt32 colorTop, lUInt32 colorBottom) {
    buf->GradientRect(rc.left, rc.top, rc.right, rc.bottom, colorTop, colorTop, colorBottom, colorBottom);
}

static CRUIDocView * createDocView() {
	CRUIDocView * _docview = new CRUIDocView();
    _docview->setViewMode(DVM_SCROLL, 1);
    LVArray<int> sizes;
    for (int i = deviceInfo.shortSide / 40; i < deviceInfo.shortSide / 10 && i < 200; i++)
    	sizes.add(i);
	_docview->setFontSizes(sizes, false);
	_docview->setFontSize(deviceInfo.shortSide / 20);
	lvRect margins(deviceInfo.shortSide / 20, deviceInfo.shortSide / 20, deviceInfo.shortSide / 20, deviceInfo.shortSide / 20);
	_docview->setPageMargins(margins);
	return _docview;
}

CRUIReadWidget::CRUIReadWidget(CRUIMainWidget * main)
    : CRUIWindowWidget(main)
    , _pinchSettingPreview(NULL)
	, _isDragging(false)
	, _dragStartOffset(0)
    , _viewMode(DVM_PAGES)
    , _pageAnimation(PAGE_ANIMATION_SLIDE)
    , _locked(false)
	, _fileItem(NULL)
	, _lastPosition(NULL)
	, _startPositionIsUpdated(false)
    , _ttsInProgress(false)
	, _pinchOp(PINCH_OP_NONE)
    , _toolbar(NULL)
    , _toolbarPosition(READER_TOOLBAR_OFF)
    , _scrollbar(NULL)
    , _volumeKeysEnabled(false)
{
    setId("READ");
    _docview = createDocView();
    _docview->setCallback(this);
    _docview->setViewMode(DVM_PAGES);
    _docview->setVisiblePageCount(2);
    _docview->setStatusFontSize(deviceInfo.shortSide / 25);
    _docview->setStatusMode(0, false, true, false, true, false, true, true);
    _popupControl.setOwner(this);
}

CRUIReadWidget::~CRUIReadWidget() {
    if (_fileItem)
        delete _fileItem;
    if (_lastPosition)
        delete _lastPosition;
    if (_toolbar)
        delete _toolbar;
    if (_scrollbar)
        delete _scrollbar;
}

bool CRUIReadWidget::isToolbarVertical(int baseWidth, int baseHeight) {
    if (_toolbarPosition == READER_TOOLBAR_LEFT)
        return true;
    else if (_toolbarPosition == READER_TOOLBAR_TOP)
        return false;
    else if (baseWidth > baseHeight)
        return _toolbarPosition == READER_TOOLBAR_SHORT_SIDE;
    else
        return _toolbarPosition == READER_TOOLBAR_LONG_SIDE;
}

/// measure dimensions
void CRUIReadWidget::measure(int baseWidth, int baseHeight) {
    _measuredWidth = baseWidth;
    _measuredHeight = baseHeight;
    if (_toolbar) {
        bool toolbarVertical = isToolbarVertical(baseWidth, baseHeight);
        _toolbar->setVertical(toolbarVertical);
        _toolbar->measure(baseWidth, baseHeight);
    }
    if (_scrollbar) {
        _scrollbar->measure(baseWidth, baseHeight);
    }
}

/// updates widget position based on specified rectangle
void CRUIReadWidget::layout(int left, int top, int right, int bottom) {
    _clientRect = lvRect(left, top, right, bottom);
    bool toolbarVertical = isToolbarVertical(_clientRect.width(), _clientRect.height());
    //_clientRect.left += 50;
    int toolbarHeight = 0;
    int toolbarWidth = 0;
    int scrollbarWidth = 0;
    if (_toolbar) {
        _toolbar->setVertical(toolbarVertical);
        _toolbar->measure(_clientRect.width(), _clientRect.height());
        if (toolbarVertical)
            toolbarWidth = _toolbar->getMeasuredWidth();
        else
            toolbarHeight = _toolbar->getMeasuredHeight();
        _clientRect.top += toolbarHeight;
        _clientRect.left += toolbarWidth;
    }
    if (_scrollbar) {
        _scrollbar->measure(_clientRect.width(), _clientRect.height());
        scrollbarWidth = _scrollbar->getMeasuredWidth();
        _clientRect.right -= scrollbarWidth;
        updateScrollbar();
    }
    _bookRect = _clientRect;

    lvRect frame = _docview->calcCoverFrameWidths(_clientRect);
    _clientRect.shrinkBy(frame);
    CRUIReadMenu * saved = _toolbar;
    CRUIScrollBar * savedsb = _scrollbar;
    _toolbar = NULL;
    _scrollbar = NULL;
    CRUIWindowWidget::layout(left, top, right, bottom);
    _toolbar = saved;
    _scrollbar = savedsb;
    if (_toolbar) {
        if (toolbarVertical)
            _toolbar->layout(left, top, left + toolbarWidth, bottom);
        else
            _toolbar->layout(left, top, right, top + toolbarHeight);
    }
    if (_scrollbar) {
        _scrollbar->layout(_bookRect.right, _bookRect.top, right, _bookRect.bottom);
    }
    if (!_locked) {
        if (_docview->GetWidth() != _clientRect.width() || _docview->GetHeight() != _clientRect.height()) {
            _docview->Resize(_clientRect.width(), _clientRect.height());
        }
    }
}

void CRUIReadWidget::prepareScroll(int direction) {
    if (renderIfNecessary()) {
        CRLog::trace("CRUIReadWidget::prepareScroll(%d)", direction);
        if (_viewMode == DVM_PAGES)
            _pagedCache.prepare(_docview, _docview->getCurPage(), _measuredWidth, _measuredHeight, direction, true, _pageAnimation);
        else
            _scrollCache.prepare(_docview, _docview->GetPos(), _measuredWidth, _measuredHeight, direction, true);
        CRLog::trace("CRUIReadWidget::prepareScroll(%d) - done", direction);
    }
}

/// draws widget with its children to specified surface
void CRUIReadWidget::draw(LVDrawBuf * buf) {
    _popupControl.updateLayout(_clientRect);
    if (_pinchOp && _pinchSettingPreview) {
    	_pinchSettingPreview->SetPos(0, false);
        buf->SetTextColor(_pinchSettingPreview->getTextColor());
        buf->SetBackgroundColor(_pinchSettingPreview->getBackgroundColor());
        _pinchSettingPreview->Draw(*buf, false);
    	_drawRequested = false;
    	return;
    }
    if (renderIfNecessary()) {
        //CRLog::trace("Document is ready, drawing");
//    	if (crconfig.einkModeSettingsSupported)
//    		_main->getPlatform()->prepareEinkController(true);
    	if (_viewMode == DVM_PAGES) {
            int direction = 0;
            int progress = 0;
            int startx = -1;
            int currx = -1;
            //bool singlePage3dPatch = false;
            if (_scroll.isActive()) {
                direction = _scroll.dir() > 0 ? 1 : -1;
                progress = _scroll.progress();
                if (progress < 0)
                    progress = 0;
                else if (progress > 10000)
                    progress = 10000;
//                if (_pageAnimation == PAGE_ANIMATION_3D && _docview->getVisiblePageCount() == 1) {
//                    //singlePage3dPatch = true;
//                    if (direction < 0) {
//                        startx = 0;
//                        currx = _pos.width() * 2 * progress / 10000;
//                    } else if (direction > 0) {
//                        startx = _pos.width();
//                        currx = _pos.width() - 2 * _pos.width() * progress / 10000;
//                    }
//                }
            } else if (_isDragging) {
                direction = _pagedCache.dir() > 0 ? 1 : -1;
                progress = (- (_dragPos.x - _dragStart.x) * direction) * 10000 / _clientRect.width();
                startx = _dragStart.x;
                currx = _dragPos.x;
            }
            //CRLog::trace("preparing");
            _pagedCache.prepare(_docview, _docview->getCurPage(), _clientRect.width(), _clientRect.height(), direction, false, _pageAnimation);
            //CRLog::trace("drawing");
            LVDrawStateSaver s(*buf);
            CR_UNUSED(s);
            if (_viewMode == DVM_PAGES) {
                buf->SetClipRect(&_bookRect);
                _docview->drawCoverFrame(*buf, _bookRect, _clientRect);
            }
            buf->SetClipRect(&_clientRect);
            _pagedCache.draw(buf, _docview->getCurPage(), direction, progress, _clientRect.left, _clientRect.top, startx, currx);
            //CRLog::trace("drawing done");
        } else {
            _scrollCache.prepare(_docview, _docview->GetPos(), _clientRect.width(), _clientRect.height(), 0, false);
            _scrollCache.draw(buf, _docview->GetPos(), _clientRect.left, _clientRect.top);
        }
    } else {
        // document render in progress; draw just page background
        //CRLog::trace("Document is locked, just drawing background");
        LVDrawStateSaver s(*buf);
        CR_UNUSED(s);
        if (_viewMode == DVM_PAGES) {
            buf->SetClipRect(&_bookRect);
            _docview->drawCoverFrame(*buf, _bookRect, _clientRect);
        }
        buf->SetClipRect(&_clientRect);
        _docview->drawPageBackground(*buf, _clientRect.left, _clientRect.top);
    }
    // scroll bottom and top gradients
    if (_viewMode != DVM_PAGES) {
        lvRect top = _clientRect;
        top.bottom = top.top + deviceInfo.shortSide / 60;
        lvRect top2 = _clientRect;
        top2.top = top.bottom;
        top2.bottom = top.top + deviceInfo.shortSide / 30;
        lvRect bottom = _clientRect;
        bottom.top = bottom.bottom - deviceInfo.shortSide / 60;
        lvRect bottom2 = _clientRect;
        bottom2.bottom = bottom.top;
        bottom2.top = bottom.bottom - deviceInfo.shortSide / 30;
        drawVGradient(buf, top, 0xA0000000, 0xE0000000);
        drawVGradient(buf, top2, 0xE0000000, 0xFF000000);
        drawVGradient(buf, bottom2, 0xFF000000, 0xE0000000);
        drawVGradient(buf, bottom, 0xE0000000, 0xA0000000);
    }
    // toolbar support
    if (_toolbar) {
        _toolbar->draw(buf);
    }
    // scrollbar support
    if (_scrollbar) {
        updateScrollbar();
        _scrollbar->draw(buf);
    }
    // popup support
    if (_popupControl.popupBackground)
        _popupControl.popupBackground->draw(buf);
    if (_popupControl.popup)
        _popupControl.popup->draw(buf);
	_drawRequested = false;
}

class BookLoadedNotificationTask : public CRRunnable {
    lString8 pathname;
    CRDocumentLoadCallback * callback;
    CRDocumentLoadCallback * callback2;
    bool success;
public:
    BookLoadedNotificationTask(lString8 _pathname, bool _success, CRDocumentLoadCallback * _callback, CRDocumentLoadCallback * _callback2) {
        pathname = _pathname;
        pathname.modify();
        callback = _callback;
        callback2 = _callback2;
        success = _success;
    }
    virtual void run() {
        CRLog::trace("BookLoadedNotificationTask.run()");
        callback2->onDocumentLoadFinished(pathname, success);
        callback->onDocumentLoadFinished(pathname, success);
    }
};

class BookRenderedNotificationTask : public CRRunnable {
    lString8 pathname;
    CRDocumentRenderCallback * callback;
    CRDocumentRenderCallback * callback2;
public:
    BookRenderedNotificationTask(lString8 _pathname, CRDocumentRenderCallback * _callback, CRDocumentRenderCallback * _callback2) {
        pathname = _pathname;
        pathname.modify();
        callback = _callback;
        callback2 = _callback2;
    }
    virtual void run() {
        CRLog::trace("BookRenderedNotificationTask.run()");
        callback2->onDocumentRenderFinished(pathname);
        callback->onDocumentRenderFinished(pathname);
    }
};

class OpenBookTask : public CRRunnable {
    lString8 _pathname;
    CRUIMainWidget * _main;
    CRUIReadWidget * _read;
public:
    OpenBookTask(lString16 pathname, CRUIMainWidget * main, CRUIReadWidget * read) : _main(main), _read(read) {
        _pathname = UnicodeToUtf8(pathname);
    }
    virtual void run() {
    	{
			CRENGINE_GUARD;
			CRLog::info("Loading book in background thread");
			bool success = _read->getDocView()->LoadDocument(Utf8ToUnicode(_pathname).c_str()) != 0;
			CRLog::info("Loading is finished %s", success ? "successfully" : "with error");
	#ifdef SLOW_RENDER_SIMULATION
			concurrencyProvider->sleepMs(3000);
	#endif
			if (!success) {
				_read->getDocView()->createDefaultDocument(lString16("Cannot open document"), lString16("Error occured while trying to open document"));
			}
			concurrencyProvider->executeGui(new BookLoadedNotificationTask(_pathname, success, _main, _read));
			CRLog::info("Rendering book in background thread");
			_read->getDocView()->Render();
			_read->restorePosition();
			_read->getDocView()->updateCache();
	#ifdef SLOW_RENDER_SIMULATION
			concurrencyProvider->sleepMs(3000);
	#endif
			CRLog::info("Render is finished");
    	}
        concurrencyProvider->executeGui(new BookRenderedNotificationTask(_pathname, _main, _read));
    }
};

class RenderBookTask : public CRRunnable {
    lString8 _pathname;
    CRUIMainWidget * _main;
    CRUIReadWidget * _read;
public:
    RenderBookTask(lString16 pathname, CRUIMainWidget * main, CRUIReadWidget * read) : _main(main), _read(read) {
        _pathname = UnicodeToUtf8(pathname);
    }
    virtual void run() {
    	{
    	    CRENGINE_GUARD;
			CRLog::info("Rendering in background thread");
			_read->getDocView()->Render();
			_read->getDocView()->updateCache();
	#ifdef SLOW_RENDER_SIMULATION
			concurrencyProvider->sleepMs(3000);
	#endif
			CRLog::info("Render in background thread is finished");
    	}
        concurrencyProvider->executeGui(new BookRenderedNotificationTask(_pathname, _main, _read));
    }
};

void CRUIReadWidget::closeBook() {
    updatePosition();
    clearImageCaches();
    if (_fileItem)
        delete _fileItem;
    if (_lastPosition)
        delete _lastPosition;
    _fileItem = NULL;
    _lastPosition = NULL;
    _bookmarks.clear();
    CRENGINE_GUARD;
    _docview->close();
}

bool CRUIReadWidget::restorePosition() {
    if (!_fileItem || !_fileItem->getBook())
        return false;
    BookDBBookmark * bmk = dirCache->loadLastPosition(_fileItem->getBook());
    if (bmk) {
        // found position
        ldomXPointer bm = _docview->getDocument()->createXPointer(lString16(bmk->startPos.c_str()));
        _docview->goToBookmark(bm);
        if (!_lastPosition)
            _lastPosition = bmk;
        else
            delete bmk;
        return true;
    }
    return false;
}

void CRUIReadWidget::afterNavigationFrom() {
    updateVolumeControls();
}

void CRUIReadWidget::afterNavigationTo() {
    updateVolumeControls();
}

void CRUIReadWidget::beforeNavigationFrom() {
    updatePosition();
}

void CRUIReadWidget::cancelPositionUpdateTimer() {
    CRUIEventManager::cancelTimer(SAVE_POSITION_TIMER_ID);
}

/// prepare next image for fast page flip
void CRUIReadWidget::prepareNextPage() {
    if (renderIfNecessary()) {
        //CRLog::trace("CRUIReadWidget::prepareScroll(%d)", direction);
        if (_viewMode == DVM_PAGES) {
            int dir = _pagedCache.getLastDirection();
            _pagedCache.prepare(_docview, _docview->getCurPage(), _measuredWidth, _measuredHeight, dir, true, _pageAnimation);
        } else {
            int dir = _scrollCache.getLastDirection();
            _scrollCache.prepare(_docview, _docview->GetPos(), _measuredWidth, _measuredHeight, dir, true);
        }
        //CRLog::trace("CRUIReadWidget::prepareScroll(%d) - done", direction);
    }
}

void CRUIReadWidget::updatePosition() {
    cancelPositionUpdateTimer();
    concurrencyProvider->executeGui(NULL, 0);
    CRLog::trace("CRUIReadWidget::updatePosition()");
    if (!_fileItem || !_fileItem->getBook())
        return;
    {
		CRENGINE_GUARD;
		ldomXPointer ptr = _docview->getBookmark();
		if ( ptr.isNull() )
			return;
		CRBookmark bm(ptr);
		lString16 comment;
		lString16 titleText;
		lString16 posText;
		bm.setType( bmkt_lastpos );
		if ( _docview->getBookmarkPosText( ptr, titleText, posText ) ) {
			 bm.setTitleText( titleText );
			 bm.setPosText( posText );
		}
		bm.setStartPos( ptr.toString() );
		int pos = ptr.toPoint().y;
		int fh = _docview->getDocument()->getFullHeight();
		int percent = fh > 0 ? (int)(pos * (lInt64)10000 / fh) : 0;
		if ( percent<0 )
			percent = 0;
		if ( percent>10000 )
			percent = 10000;
		bm.setPercent( percent );
		bm.setCommentText( comment );
		if (!_lastPosition)
			_lastPosition = new BookDBBookmark();
		_lastPosition->bookId = _fileItem->getBook()->id;
		_lastPosition->type = bm.getType();
		_lastPosition->percent = bm.getPercent();
		_lastPosition->shortcut = bm.getShortcut();
		_lastPosition->timestamp = GetCurrentTimeMillis();
		_lastPosition->startPos = UnicodeToUtf8(bm.getStartPos()).c_str();
		_lastPosition->endPos = UnicodeToUtf8(bm.getEndPos()).c_str();
		_lastPosition->titleText = UnicodeToUtf8(bm.getTitleText()).c_str();
		_lastPosition->posText = UnicodeToUtf8(bm.getPosText()).c_str();
		_lastPosition->commentText = UnicodeToUtf8(bm.getCommentText()).c_str();
		_lastPosition->startPos = UnicodeToUtf8(bm.getStartPos()).c_str();
		dirCache->saveLastPosition(_fileItem->getBook(), _lastPosition);
    }
    prepareNextPage();
}

lString8 lastBookLang;
lString8 lastSettingsLang;
bool setHyph(lString8 bookLang, lString8 settingsLang) {
    if (bookLang == lastBookLang && settingsLang == lastSettingsLang) // don't set duplicate
        return false;
    lastBookLang = bookLang;
    lastSettingsLang = settingsLang;
    return crconfig.setHyphenationDictionary(bookLang, settingsLang);
}

bool CRUIReadWidget::openBook(const CRFileItem * file) {
    if (_locked)
        return false;
    if (!file)
        return false;
    closeBook();
    _locked = true;
    clearImageCaches();
    _main->showSlowOperationPopup();
    _fileItem = static_cast<CRFileItem*>(file->clone());
    BookDBBook * book = file->getBook();
    if (!book) {
        CRLog::error("Book entry is not found in FileInfo %s", _fileItem->getPathName().c_str());
        return false;
    }
    _lastPosition = bookDB->loadLastPosition(file->getBook());
    bookDB->loadBookmarks(file->getBook(), _bookmarks);
    lString8 bookLang(_fileItem->getBook() ? _fileItem->getBook()->language.c_str() : "");
    lString8 settingsHyph = UnicodeToUtf8(_main->getSettings()->getStringDef(PROP_HYPHENATION_DICT, crconfig.systemLanguage.c_str()));
    //lString8 systemLang = crconfig.systemLanguage;
    setHyph(bookLang, settingsHyph);
    _main->executeBackground(new OpenBookTask(Utf8ToUnicode(getPathName()), _main, this));
    return true;
}

void CRUIReadWidget::onDocumentLoadFinished(lString8 pathname, bool success) {
    CR_UNUSED(pathname);
    if (!success) {
        if (_fileItem)
            delete _fileItem;
        if (_lastPosition)
            delete _lastPosition;
        _fileItem = NULL;
        _lastPosition = NULL;
    }
    // force update reading position - to refresh timestamp
    _startPositionIsUpdated = false;
}

void CRUIReadWidget::onDocumentRenderFinished(lString8 pathname) {
    CR_UNUSED(pathname);
    CRLog::trace("Render is finished - unlocking document");
    _locked = false;
    invalidate();
    clearImageCaches();
    if (!_startPositionIsUpdated) {
        // call update position to refresh last access timestamp
        _startPositionIsUpdated = true;
        postUpdatePosition();
    }
    updateBookmarks();
    _main->update(true);
}

/// returns true if document is ready, false if background rendering is in progress
bool CRUIReadWidget::renderIfNecessary() {
    if (_locked) {
        CRLog::trace("Document is locked");
        return false;
    }
    if (_docview->GetWidth() != _clientRect.width() || _docview->GetHeight() != _clientRect.height()) {
        CRLog::trace("Changing docview size to %dx%d", _clientRect.width(), _clientRect.height());
        _docview->Resize(_clientRect.width(), _clientRect.height());
    }
    if (_docview->IsRendered())
        return true;
    CRLog::info("Render is required! Starting render task");
    _locked = true;
    clearImageCaches();
    _main->showSlowOperationPopup();
    _main->executeBackground(new RenderBookTask(Utf8ToUnicode(getPathName()), _main, this));
    return false;
}

#define SCROLL_SPEED_CALC_INTERVAL 2000
#define SCROLL_MIN_SPEED 3
#define SCROLL_FRICTION 13

/// overriden to treat popup as first child
int CRUIReadWidget::getChildCount() {
    int cnt = 0;
    if (_popupControl.popup)
        cnt++;
    if (_popupControl.popupBackground)
        cnt++;
    if (_toolbar)
        cnt++;
    if (_scrollbar)
        cnt++;
    return cnt;
}

/// overriden to treat popup as first child
CRUIWidget * CRUIReadWidget::getChild(int index) {
    CRUIWidget * children[4];
    int i = 0;
    if (_popupControl.popupBackground)
        children[i++] = _popupControl.popupBackground;
    if (_popupControl.popup)
        children[i++] = _popupControl.popup;
    if (_toolbar)
        children[i++] = _toolbar;
    if (_scrollbar)
        children[i++] = _scrollbar;
    if (index < i) {
        return children[index];
    }
    return NULL;
}

void CRUIReadWidget::onScrollAnimationStop() {
    if (_viewMode == DVM_PAGES && !_scroll.isCancelled()) {
        //CRLog::trace("flip stopped, old page: %d, new page: %d", _docview->getCurPage(), _pagedCache.getNewPage());
        _docview->goToPage(_pagedCache.getNewPage(), true);
//            if (_scroll.dir() > 0)
//                _docview->doCommand(DCMD_PAGEDOWN, 1);
//            else if (_scroll.dir() < 0)
//                _docview->doCommand(DCMD_PAGEUP, 1);
    }
    postUpdatePosition();
}

void CRUIReadWidget::animate(lUInt64 millisPassed) {
    if (_locked) {
        if (_scroll.isActive())
            _scroll.stop();
        return;
    }
    bool scrollWasActive = _scroll.isActive();
    CRUIWindowWidget::animate(millisPassed);
//    if (_viewMode == DVM_PAGES && scrollWasActive) {
//        if (_scroll.pos() >= _pos.width() - 1) {
//            CRLog::trace("Stopping page animation: stop position reached");
//            _scroll.stop();
//        }
//    }
    bool changed = scrollWasActive && _scroll.animate(millisPassed);
    if (changed) {
        if (_viewMode == DVM_PAGES) {
            if (_scroll.pos() >= _clientRect.width() - 1) {
                //CRLog::trace("Stopping page animation: stop position reached");
                _scroll.stop();
            }
        } else {
            int oldpos = _docview->GetPos();
            //CRLog::trace("scroll animation: new position %d", _scroll.pos());
            _docview->SetPos(_scroll.pos(), false);
            if (oldpos == _docview->GetPos()) {
                //CRLog::trace("scroll animation - stopping at %d since set position not changed position", _scroll.pos());
                // stopping: bounds
                _scroll.stop();
            }
        }
    }
    if (scrollWasActive && !_scroll.isActive()) {
        onScrollAnimationStop();
    }
}

//class UpdatePositionEvent : public CRRunnable {
//    CRUIReadWidget * _widget;
//public:
//    UpdatePositionEvent(CRUIReadWidget * widget) : _widget(widget) { }
//    virtual void run() {
//        _widget->updatePosition();
//    }
//};

void CRUIReadWidget::postUpdatePosition(int delay) {
    CRLog::trace("CRUIReadWidget::postUpdatePosition(%d)", delay);
    CRUIEventManager::setTimer(SAVE_POSITION_TIMER_ID, this, delay, false);
    //concurrencyProvider->executeGui(new UpdatePositionEvent(this), delay);
}

bool CRUIReadWidget::isAnimating() {
    return _scroll.isActive() || CRUIWindowWidget::isAnimating();
}

void CRUIReadWidget::animateScrollTo(int newpos, int speed) {
    if (_locked)
        return;
    CRLog::trace("animateScrollTo( %d -> %d )", _docview->GetPos(), newpos);
    int delta = newpos - _docview->GetPos();
    prepareScroll(delta);
    _scroll.start(_docview->GetPos(), newpos, speed, SCROLL_FRICTION);
    invalidate();
    _main->update(true);
}

void CRUIReadWidget::animatePageFlip(int newpage, int speed) {
    if (_locked)
        return;
    int page = _docview->getCurPage();
    CRLog::trace("animatePageFlip( %d -> %d )", page, newpage);
    //_pagedCache.prepare(_docview->);
    int dir = newpage > page ? 1 : -1;
    if (_pageAnimation == PAGE_ANIMATION_NONE) {
        _docview->goToPage(newpage);
        invalidate();
        return;
    }
    prepareScroll(dir);
    _scroll.setDirection(dir);
    _scroll.start(0, _clientRect.width(), speed, SCROLL_FRICTION);
    invalidate();
    //_main->update(true);
}

bool CRUIReadWidget::doCommand(int cmd, int param) {
    if (_locked)
        return false;
    int pos = _docview->GetPos();
    int newpos = pos;
    int speed = 0;
    int page = _docview->getCurPage();
    int newpage = page;
    if (_viewMode == DVM_PAGES) {
        if (cmd == DCMD_LINEUP)
            cmd = DCMD_PAGEUP;
        else if (cmd == DCMD_LINEDOWN)
            cmd = DCMD_PAGEDOWN;
    }
    switch (cmd) {
    case DCMD_PAGEUP:
        if (param <= 0)
            param = 1;
        if (_viewMode == DVM_PAGES) {
            if (_pageAnimation == PAGE_ANIMATION_NONE || param == 10) {
                _docview->doCommand((LVDocCmd)cmd, param);
            } else {
                newpage = page - _docview->getVisiblePageCount();
                speed = _clientRect.width() * 2;
            }
        } else {
            newpos = pos - _clientRect.height() * (param - 1) - _clientRect.height() * 9 / 10;
            speed = _clientRect.height() * 2 * param;
        }
        invalidate();
        break;
    case DCMD_PAGEDOWN:
        if (param <= 0)
            param = 1;
        if (_viewMode == DVM_PAGES) {
            if (_pageAnimation == PAGE_ANIMATION_NONE || param == 10) {
                _docview->doCommand((LVDocCmd)cmd, param);
            } else {
                newpage = page + _docview->getVisiblePageCount();
                speed = _clientRect.width() * 4;
            }
        } else {
            newpos = pos + _clientRect.height() * (param - 1) + _clientRect.height() * 9 / 10;
            speed = _clientRect.height() * 2 * param;
        }
        invalidate();
        break;
    case DCMD_LINEUP:
        newpos = pos - _docview->getFontSize() * 3 * (param > 0 ? param : 1);
        speed = _clientRect.height();
        invalidate();
        break;
    case DCMD_LINEDOWN:
        newpos = pos + _docview->getFontSize() * 3 * (param > 0 ? param : 1);
        speed = _clientRect.height();
        invalidate();
        break;
    case DCMD_SELECT_FIRST_SENTENCE:
    case DCMD_SELECT_PREV_SENTENCE:
    case DCMD_SELECT_NEXT_SENTENCE:
        _docview->doCommand((LVDocCmd)cmd, param);
        clearImageCaches();
        invalidate();
        //_main->update(false);
        return true;
    default:
        return _docview->doCommand((LVDocCmd)cmd, param) != 0;
    }
    if (_viewMode == DVM_PAGES) {
        if (page != newpage && newpage >= 0 && newpage < _docview->getPageCount() + _docview->getVisiblePageCount() - 1) {
            animatePageFlip(newpage, speed);
        }
    } else {
        if (pos != newpos) {
            animateScrollTo(newpos, speed);
        }
    }
    return true;
}

void CRUIReadWidget::clearImageCaches() {
	_scrollCache.clear();
    _pagedCache.clear();
}

bool CRUIReadWidget::onKeyEvent(const CRUIKeyEvent * event) {
    if (_locked)
        return false;
    int key = event->key();
	//CRLog::trace("CRUIReadWidget::onKeyEvent(%d  0x%x  popup.closing=%s   popup.progress=%d)", key, key, _popupControl.closing ? "yes" : "no", _popupControl.progress);
    if (_popupControl.popup) {
        CRLog::trace("Popup is active - transferring key to window");
    	return CRUIWindowWidget::onKeyEvent(event);
    }

    if (_ttsInProgress) {
        if (event->getType() != KEY_ACTION_RELEASE || key == CR_KEY_F5)
            return true;
        stopReadAloud();
        return true;
    }

    bool longPress = event->getDownDuration() > 500;
    if (event->getType() == KEY_ACTION_RELEASE) {
        switch(key) {
        case CR_KEY_VOLUME_UP:
        case CR_KEY_VOLUME_DOWN:
            return _volumeKeysEnabled;
        case CR_KEY_PGDOWN:
        case CR_KEY_SPACE:
        case CR_KEY_PGUP:
        case CR_KEY_HOME:
        case CR_KEY_END:
        case CR_KEY_UP:
        case CR_KEY_DOWN:
        case CR_KEY_LEFT:
        case CR_KEY_RIGHT:
        case CR_KEY_Q:
        case CR_KEY_W:
        case CR_KEY_E:
        case CR_KEY_F9:
            return true;
        case CR_KEY_F5:
            onAction(ACTION_TTS_PLAY);
            return true;
        case CR_KEY_F3:
            //onAction(ACTION_SHOW_FOLDER);
            return true;
        default:
            break;
        }
        if (_scroll.isActive()) {
            _scroll.stop();
            onScrollAnimationStop();
        }
        switch(key) {
        case CR_KEY_F10:
        case CR_KEY_MENU:
            if (longPress)
                _main->showSettings(lString8("@settings/reader"));
            else
                showReaderMenu();
            invalidate();
            return true;
        case CR_KEY_BACK:
        case CR_KEY_ESC:
            if (!longPress && _docview->canGoBack())
                onAction(CMD_LINK_BACK);
            else
                onAction(CMD_BACK);
            invalidate();
            return true;
        default:
        	break;
        }
    }

    if (event->getType() == KEY_ACTION_PRESS) {
        if (_scroll.isActive()) {
            _scroll.stop();
            onScrollAnimationStop();
        }
        //CRLog::trace("keyDown(0x%04x) oldpos=%d", key,  _docview->GetPos());
        switch(key) {
        case CR_KEY_F5:
        case CR_KEY_F3:
            return true;
        case CR_KEY_F9:
            _main->showSettings(lString8("@settings/reader"));
            invalidate();
            return true;
#ifdef _DEBUG
        case CR_KEY_Q:
            doCommand(DCMD_SELECT_FIRST_SENTENCE);
            return true;
        case CR_KEY_W:
            doCommand(DCMD_SELECT_NEXT_SENTENCE);
            return true;
        case CR_KEY_E:
            doCommand(DCMD_SELECT_PREV_SENTENCE);
            return true;
#endif
        case CR_KEY_VOLUME_DOWN:
            if (!_volumeKeysEnabled)
                return false;
            // fall down
        case CR_KEY_PGDOWN:
        case CR_KEY_SPACE:
            doCommand(DCMD_PAGEDOWN);
            invalidate();
            return true;
        case CR_KEY_VOLUME_UP:
            if (!_volumeKeysEnabled)
                return false;
            // fall down
        case CR_KEY_PGUP:
            doCommand(DCMD_PAGEUP);
            invalidate();
            return true;
        case CR_KEY_HOME:
            doCommand(DCMD_BEGIN);
            invalidate();
            return true;
        case CR_KEY_END:
            doCommand(DCMD_END);
            invalidate();
            return true;
        case CR_KEY_LEFT:
        case CR_KEY_UP:
            doCommand(DCMD_LINEUP, 1);
            invalidate();
            return true;
        case CR_KEY_DOWN:
        case CR_KEY_RIGHT:
            doCommand(DCMD_LINEDOWN, 1);
            invalidate();
            return true;
//        case CR_KEY_ESC:
//        case CR_KEY_BACK:
//            _main->back();
//            invalidate();
//            return true;
        case CR_KEY_MENU:
        //case CR_KEY_RETURN:
            return true;
        default:
            break;
        }
    }
    //CRLog::trace("new pos=%d", _docview->GetPos());
    return CRUIWindowWidget::onKeyEvent(event);
}

int CRUIReadWidget::pointToTapZone(int x, int y) {
    y -= _clientRect.top - _pos.top;
    x -= _clientRect.left - _pos.left;
    int x0 = x / ((_clientRect.width() + 2) / 3);
    int y0 = y / ((_clientRect.height() + 2) / 3);
    if (x0 > 2) x0 = 2;
    if (x0 < 0) x0 = 0;
    if (y0 > 2) y0 = 2;
    if (y0 < 0) y0 = 0;
    return y0 * 3 + x0 + 1;
}

bool CRUIReadWidget::onTapZone(int zone, bool additionalAction) {
    lString8 settingName;
    if (additionalAction)
        settingName = PROP_APP_TAP_ZONE_ACTION_DOUBLE;
    else
        settingName = PROP_APP_TAP_ZONE_ACTION_NORMAL;
    settingName += lString8::itoa(zone);
    lString8 action = UnicodeToUtf8(_main->getSettings()->getStringDef(settingName.c_str()));
    if (!action.empty()) {
        const CRUIAction * a = CRUIActionByName(action.c_str());
        if (a != NULL) {
            return onAction(a);
        }
    }
    return false;
}

void CRUIReadWidget::startPinchOp(int op, int dx, int dy) {
	if (_pinchOp)
		return;
	_pinchOp = op;
	_pinchOpStartDx = dx;
	_pinchOpStartDy = dy;
    _pinchOpCurrentDx = dx;
    _pinchOpCurrentDy = dy;
    _pinchSettingPreview = createDocView();
    CRPropRef changed = _main->getSettings();
    CRPropRef docviewprops = LVCreatePropsContainer();
    //bool backgroundChanged = false;
    for (int i = 0; i < changed->getCount(); i++) {
        lString8 key(changed->getName(i));
        lString8 value(UnicodeToUtf8(changed->getValue(i)));
        if (isDocViewProp(key)) {
            docviewprops->setString(key.c_str(), value.c_str());
            if (key == PROP_FONT_COLOR) {
            	_pinchSettingPreview->setTextColor(changed->getColorDef(PROP_FONT_COLOR, 0));
            }
        }
    }
    _pinchSettingPreview->propsApply(docviewprops);
    lString16 title;
    switch(_pinchOp) {
    case PINCH_OP_HORIZONTAL:
    	title = _16(STR_PINCH_CHANGING_PAGE_MARGINS);
    	break;
    case PINCH_OP_VERTICAL:
    	title = _16(STR_PINCH_CHANGING_INTERLINE_SPACING);
    	break;
    case PINCH_OP_DIAGONAL:
    	title = _16(STR_PINCH_CHANGING_FONT_SIZE);
    	_pinchOpSettingValue = _docview->getFontSize();
    	break;
    }
    lString16 sampleText = _16(STR_SETTINGS_FONT_SAMPLE_TEXT);
    sampleText = sampleText + " " + sampleText;
    _pinchSettingPreview->createDefaultDocument(title, sampleText + "\n"
    		+ sampleText + "\n" + sampleText + "\n" + sampleText
    		+ "\n" + sampleText + "\n" + sampleText + "\n" + sampleText + "\n" + sampleText);
    _pinchSettingPreview->Resize(_clientRect.width(), _clientRect.height());
    CRLog::trace("startPinchOp %d   %d %d", _pinchOp, dx, dy);
    invalidate();
}

void CRUIReadWidget::updatePinchOp(int dx, int dy) {
	if (!_pinchOp)
		return;
    _pinchOpCurrentDx = dx;
    _pinchOpCurrentDy = dy;
    int delta = 0;
    int startSettingValue = 0;
    int newSettingValue = 0;
    switch(_pinchOp) {
    case PINCH_OP_HORIZONTAL:
		{
			startSettingValue = _main->getSettings()->getIntDef(PROP_PAGE_MARGINS, 100);
			delta = (dx) - (_pinchOpStartDx);
			int maxdiff = 2000 - 100;
			newSettingValue = startSettingValue + maxdiff * (-delta) * 120 / 100 / deviceInfo.shortSide;
			if (newSettingValue < 100)
				newSettingValue = 100;
			if (newSettingValue > 2000)
				newSettingValue = 2000;
			CRPropRef props = LVCreatePropsContainer();
			props->setInt(PROP_PAGE_MARGINS, newSettingValue);
			_pinchOpSettingValue = newSettingValue;
			_pinchSettingPreview->propsApply(props);
			invalidate();
		}
    	break;
    case PINCH_OP_VERTICAL:
		{
			startSettingValue = _main->getSettings()->getIntDef(PROP_INTERLINE_SPACE, 100);
			delta = (dy) - (_pinchOpStartDy);
			int maxdiff = 200 - 80;
			newSettingValue = startSettingValue + maxdiff * delta * 120 / 100 / deviceInfo.shortSide;
			if (newSettingValue < 80)
				newSettingValue = 80;
			if (newSettingValue > 200)
				newSettingValue = 200;
			CRPropRef props = LVCreatePropsContainer();
			props->setInt(PROP_INTERLINE_SPACE, newSettingValue);
			_pinchOpSettingValue = newSettingValue;
			_pinchSettingPreview->propsApply(props);
			invalidate();
		}
    	break;
    case PINCH_OP_DIAGONAL:
		{
			delta = (dx + dy) - (_pinchOpStartDx + _pinchOpStartDy);
			int maxdiff = crconfig.maxFontSize - crconfig.minFontSize;
			startSettingValue = _docview->getFontSize();

			if (delta > 0) {
				newSettingValue = startSettingValue + maxdiff * delta * 120 / 100 / deviceInfo.shortSide;
				CRLog::trace("Zoom in %d -> %d", startSettingValue, newSettingValue);
			} else {
				newSettingValue = startSettingValue + maxdiff * delta * 120 / 100  / deviceInfo.shortSide;
				//newSettingValue = startSettingValue - startSettingValue * ((-delta) * 2 / deviceInfo.shortSide);
				CRLog::trace("Zoom out %d -> %d", startSettingValue, newSettingValue);
			}
			if (newSettingValue < crconfig.minFontSize)
				newSettingValue = crconfig.minFontSize;
			if (newSettingValue > crconfig.maxFontSize)
				newSettingValue = crconfig.maxFontSize;
			if (_pinchSettingPreview->getFontSize() != newSettingValue) {
				_pinchOpSettingValue = newSettingValue;
				_pinchSettingPreview->setFontSize(newSettingValue);
				invalidate();
			}
			break;
		}
    }
    CRLog::trace("updatePinchOp %d   %d %d", _pinchOp, dx, dy);
}

void CRUIReadWidget::endPinchOp(int dx, int dy, bool cancel) {
	if (!_pinchOp)
		return;
    CRLog::trace("endPinchOp %d   %d %d", _pinchOp, dx, dy);
	if (_pinchSettingPreview) {
		delete _pinchSettingPreview;
		_pinchSettingPreview = NULL;
	}
	if (!cancel) {
		switch(_pinchOp) {
		case PINCH_OP_HORIZONTAL:
			_main->initNewSettings()->setInt(PROP_PAGE_MARGINS, _pinchOpSettingValue);
			_main->applySettings();
			break;
		case PINCH_OP_VERTICAL:
			_main->initNewSettings()->setInt(PROP_INTERLINE_SPACE, _pinchOpSettingValue);
			_main->applySettings();
			break;
		case PINCH_OP_DIAGONAL:
			_main->initNewSettings()->setInt(PROP_FONT_SIZE, _pinchOpSettingValue);
			_main->applySettings();
			break;
		}
	}
	_pinchOp = PINCH_OP_NONE;
    invalidate();
}

/// returns true to allow parent intercept this widget which is currently handled by this widget
bool CRUIReadWidget::allowInterceptTouchEvent(const CRUIMotionEvent * event) {
    CR_UNUSED(event);
	if (_isDragging || _pinchOp)
		return false;
	return true;
}

void CRUIReadWidget::removeBookmark(lInt64 id) {
    if (!_fileItem->getBook())
        return;
    for (int i = 0; i < _bookmarks.length(); i++) {
        BookDBBookmark * item = _bookmarks[i];
        if (item->id == id) {
            bookDB->removeBookmark(_fileItem->getBook(), item);
            _bookmarks.remove(i);
            delete item;
        }
    }
}

void CRUIReadWidget::addSelectionBookmark() {
    if (_selectionBookmark.isNull())
        return;
    bookDB->saveBookmark(_fileItem->getBook(), _selectionBookmark.get());
    _bookmarks.add(_selectionBookmark->clone());
    _selectionBookmark.clear();
    updateBookmarks();
    cancelSelection();
}

void CRUIReadWidget::updateBookmarks() {
    LVPtrVector<CRBookmark> bookmarks;
    for (int i=0; i<_bookmarks.length(); i++) {
        BookDBBookmark * bm = _bookmarks[i];
        CRBookmark * bookmark = new CRBookmark(lString16(bm->startPos.c_str()), lString16(bm->endPos.c_str()));
        bookmark->setType(bm->type);
        bookmarks.add(bookmark);
    }
    _docview->setBookmarkList(bookmarks);
    clearImageCaches();
    invalidate();
}

void CRUIReadWidget::onPopupClosing(CRUIWidget * popup) {
    CR_UNUSED(popup);
    if (popup->getId() == "FINDTEXT") {
        _docview->clearSelection();
        clearImageCaches();
    }
    if (popup->getId() == "MAINMENU" || popup->getId() == "GOTOPERCENT" || popup->getId() == "FINDTEXT") {
        // add position to navigation history, if necessary
        if (_lastPosition == NULL)
            return;
        lString16 lastPos(_lastPosition->startPos.c_str());
        lString16 currPos(_docview->getBookmark().toString());
        if (lastPos != currPos) {
            _docview->savePosToNavigationHistory(lastPos);
            //_docview->savePosToNavigationHistory(currPos);
            postUpdatePosition();
        }
    }
    cancelSelection();
}

void CRUIReadWidget::updateSelectionBookmark()
{
    if (!_fileItem->getBook())
        return;
    ldomXPointer p0 = _docview->getDocument()->createXPointer(_selection.startPos);
    ldomXPointer p1 = _docview->getDocument()->createXPointer(_selection.endPos);
    if (!p1.isNull() && !p0.isNull()) {
        ldomXRange r(p0, p1);
        r.sort();
        if ( r.isNull() )
            return;
        BookDBBookmark * bookmark = new BookDBBookmark();
        bookmark->type = bmkt_comment;
        bookmark->startPos = DBString(UnicodeToUtf8(_selection.startPos).c_str());
        bookmark->endPos = DBString(UnicodeToUtf8(_selection.endPos).c_str());
        bookmark->posText = DBString(UnicodeToUtf8(r.getRangeText()).c_str());
        lvRect rc;
        if (r.getStart().getRect(rc)) {
            bookmark->percent = rc.top * 10000 / _docview->GetFullHeight();
        }
        lString16 titleText;
        lString16 posText;
        _docview->getBookmarkPosText(p0, titleText, posText);
        bookmark->titleText = DBString(UnicodeToUtf8(titleText).c_str());
        _selectionBookmark = bookmark;
    }
}

void CRUIReadWidget::selectionDone(int x, int y) {
    updateSelection(x, y);
    ldomXPointer p0 = _docview->getDocument()->createXPointer(_selection.startPos);
    ldomXPointer p1 = _docview->getDocument()->createXPointer(_selection.endPos);
    if (!p1.isNull() && !p0.isNull()) {
        ldomXRange r(p0, p1);
        if ( !r.getStart().isVisibleWordStart() )
            r.getStart().prevVisibleWordStart();
        if ( !r.getEnd().isVisibleWordEnd() )
            r.getEnd().nextVisibleWordEnd();
        r.sort();
        if ( r.isNull() )
            return;
        _selection.startPos = r.getStart().toString();
        _selection.endPos = r.getEnd().toString();
        _selection.startCursorPos.clear();
        _selection.endCursorPos.clear();
        _docview->getCursorRect(r.getStart(), _selection.startCursorPos, false);
        _docview->getCursorRect(r.getEnd(), _selection.endCursorPos, false);
        _selection.selectionText = r.getRangeText();
        updateSelectionBookmark();
        _selection.popupActive = true;
        r.setFlags(1);
        _docview->selectRange(r);
        clearImageCaches();
        invalidate();
        CRLog::trace("show selection menu");
        CRUIActionList actions;
        actions.add(ACTION_SELECTION_COPY);
        actions.add(ACTION_SELECTION_ADD_BOOKMARK);
        if (_main->getPlatform()->getTextToSpeech())
            actions.add(ACTION_TTS_PLAY);
        lvRect margins;
        CRUIReadMenu * menu = new CRUIReadMenu(this, actions, false);
        CRLog::trace("showing popup");

        int pos = ALIGN_CENTER;
        if (_selection.startCursorPos.top > _clientRect.height() / 3) {
            pos = ALIGN_TOP;
            menu->setAlign(ALIGN_TOP|ALIGN_HCENTER);
        } else if (_selection.endCursorPos.bottom < 2 * _clientRect.height() / 3) {
            pos = ALIGN_BOTTOM;
            menu->setAlign(ALIGN_BOTTOM|ALIGN_HCENTER);
        } else {
            menu->setAlign(ALIGN_CENTER);
        }
        menu->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        preparePopup(menu, pos, margins, 0x20);
        CRLog::trace("Show selection toolbar popup");
    } else {
        cancelSelection();
    }
}

void CRUIReadWidget::updateSelection(int x, int y) {
    y -= _clientRect.top - _pos.top;
    x -= _clientRect.left - _pos.left;
    if (!_selection.selecting)
        return;
    lvPoint pt(x, y);
    ldomXPointer p0 = _docview->getDocument()->createXPointer(_selection.startPos);
    ldomXPointer p = _docview->getNodeByPoint(pt);
    if (!p.isNull() && !p0.isNull()) {
        _selection.endPos = p.toString();
        ldomXRange r(p0, p);
        if ( !r.getStart().isVisibleWordStart() )
            r.getStart().prevVisibleWordStart();
        if ( !r.getEnd().isVisibleWordEnd() )
            r.getEnd().nextVisibleWordEnd();
        r.sort();
        if ( r.isNull() )
            return;
        r.setFlags(1);
        _docview->selectRange(r);
        clearImageCaches();
        invalidate();
        _main->update(true);
    }
}

void CRUIReadWidget::startSelectionTimer(int x, int y) {
    y -= _clientRect.top - _pos.top;
    x -= _clientRect.left - _pos.left;
    cancelSelection();
    lvPoint pt(x, y);
    ldomXPointer p = _docview->getNodeByPoint(pt);
    if (!p.isNull()) {
        ldomXRange r(p, p);
        if ( !r.getStart().isVisibleWordStart() )
            r.getStart().prevVisibleWordStart();
        if ( !r.getEnd().isVisibleWordEnd() )
            r.getEnd().nextVisibleWordEnd();
        if ( r.isNull() )
            return;
        _selection.startPos = r.getStart().toString();
        _selection.endPos = r.getEnd().toString();
        CRLog::trace("Starting selection timer");
        CRUIEventManager::setTimer(SELECTION_LONG_TAP_TIMER_ID, this, SELECTION_LONG_TAP_DELAY_MILLIS, false);
        _selection.timerStarted = true;
    }
}

/// handle timer event; return true to allow recurring timer event occur more times, false to stop
bool CRUIReadWidget::onTimerEvent(lUInt32 timerId) {
    if (timerId == SELECTION_LONG_TAP_TIMER_ID) {
        CRLog::trace("onTimerEvent(SELECTION_LONG_TAP_TIMER_ID)");
        if (_selection.timerStarted) {
            _selection.timerStarted = false;
            ldomXPointer pt1 = _docview->getDocument()->createXPointer(_selection.startPos);
            ldomXPointer pt2 = _docview->getDocument()->createXPointer(_selection.endPos);
            if (!pt1.isNull() && !pt2.isNull()) {
                ldomXRange r(pt1, pt2);
                if ( !r.getStart().isVisibleWordStart() )
                    r.getStart().prevVisibleWordStart();
                if ( !r.getEnd().isVisibleWordEnd() )
                    r.getEnd().nextVisibleWordEnd();
                r.sort();
                if (!r.isNull()) {
                    r.setFlags(1);
                    _docview->selectRange(r);
                    _selection.selecting = true;
                    CRLog::trace("Creating selection");
                    clearImageCaches();
                    invalidate();
                    _main->update(true);
                }
            }
        }
        return false;
    } else if (timerId == SAVE_POSITION_TIMER_ID) {
        updatePosition();
        return false;
    }
    return false;
}

void CRUIReadWidget::cancelSelection() {
    if (_selection.timerStarted) {
        CRUIEventManager::cancelTimer(SELECTION_LONG_TAP_TIMER_ID);
        _selection.timerStarted = false;
    }
    if (_selection.selecting) {
        _selection.selecting = false;
        _selection.startPos.clear();
        _selection.endPos.clear();
        _docview->clearSelection();
        clearImageCaches();
    }
    _selection.startCursorPos.clear();
    _selection.endCursorPos.clear();
    _selection.selectionText.clear();
    if (_selection.popupActive) {
        // TODO
        _selection.popupActive = false;
    }
}

/// motion event handler, returns true if it handled event
bool CRUIReadWidget::onTouchEvent(const CRUIMotionEvent * event) {
    if (_locked)
        return false;
    int action = event->getAction();

    if (_ttsInProgress) {
        if (action == ACTION_UP) {
            stopReadAloud();
        }
        return true;
    }

    if (action != ACTION_MOVE && event->count() > 1)
    	CRLog::trace("CRUIReadWidget::onTouchEvent multitouch %d pointers action = %d", event->count(), action);
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
    bool insideClient = _clientRect.isPointInside(lvPoint(event->getX(), event->getY()));
    int dx = event->getX() - event->getStartX();
    int dy = event->getY() - event->getStartY();
    int pinchDx = event->getPinchDx();
    int pinchDy = event->getPinchDy();
    int delta = dy; //isVertical() ? dy : dx;
    int delta2 = dx; //isVertical() ? dy : dx;
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
//    if (event->isCancelRequested())
//        return true;
    switch (action) {
    case ACTION_WHEEL:
        {
            int delta = event->getWheelDelta();
            int cmd = DCMD_PAGEDOWN;
            int param = 1;
            if (_viewMode == DVM_PAGES) {
                if (_scroll.isActive())
                    return true;
                if (delta < 0)
                    cmd = DCMD_PAGEDOWN;
                else
                    cmd = DCMD_PAGEUP;
            } else {
                if (delta < 0)
                    cmd = DCMD_LINEDOWN;
                else
                    cmd = DCMD_LINEUP;
                param = 4;
            }
            doCommand(cmd, param);
        }
        break;
    case ACTION_DOWN:
        cancelPositionUpdateTimer();
        if (_scroll.isActive()) {
            _scroll.stop();
            onScrollAnimationStop();
//            if (_viewMode == DVM_PAGES && !_scroll.isCancelled())
//                _docview->goToPage(_pagedCache.getNewPage());
            if (_isDragging) {
                event->cancelAllPointers();
                _isDragging = false;
                invalidate();
                return true;
            }
        }
        _isDragging = false;
        if (insideClient) {
            _dragStart.x = event->getX();
            _dragStart.y = event->getY();
            _dragPos = _dragStart;
            if (_viewMode == DVM_PAGES)
                _dragStartOffset = _dragStart.x;
            else
                _dragStartOffset = _docview->GetPos();
        }
        if (_scroll.isActive())
            _scroll.stop();
        if (event->count() == 1 && insideClient) {
            startSelectionTimer(event->getX(), event->getY());
        } else
            cancelSelection();

        //invalidate();
        //CRLog::trace("list DOWN");
        break;
    case ACTION_UP:
        {
            cancelPositionUpdateTimer();
            //invalidate();
            if (!_selection.selecting)
                cancelSelection();
            if (_selection.selecting) {
                // update selection end
                selectionDone(event->getX(), event->getY());
            } else if (_pinchOp) {
            	endPinchOp(pinchDx, pinchDy, false);
            	event->cancelAllPointers();
            } else if (_isDragging) {
                lvPoint speed = event->getSpeed(SCROLL_SPEED_CALC_INTERVAL);
                if (_viewMode == DVM_PAGES) {
                    int w = _clientRect.width();
                    int xx = 0;
                    int progress = 0; //myAbs(_dragPos.x - _dragStart.x);
                    _pagedCache.calcDragPositionProgress(_dragStart.x, _dragPos.x, _pagedCache.dir(), progress, xx);
                    progress = _clientRect.width() * progress / 10000;
                    int spd = myAbs(speed.x);
                    bool cancelling = ((speed.x > 0 && _pagedCache.dir() > 0) || (speed.x < 0 && _pagedCache.dir() < 0)) && (spd > SCROLL_MIN_SPEED * 2);
                    if ((progress < _clientRect.width() / 7) && (spd < SCROLL_MIN_SPEED * 3))
                        cancelling = true; // cancel if too small progress and too small speed
                    _scroll.setDirection(_pagedCache.dir());
                    if (spd < w)
                        spd = w;
                    int pmin = 0;
                    cancelPositionUpdateTimer();
                    _scroll.start(pmin, w, spd, SCROLL_FRICTION);
                    _scroll.setPos(progress);
                    // cancel if UP event occured during moving in opposite direction
                    if (cancelling) {
                        _scroll.cancel();
                        _pagedCache.setNewPage(_docview->getCurPage());
                    }
                    CRLog::trace("Starting page flip with speed %d", _scroll.speed());
                    _isDragging = false;
                } else {
                    if (speed.y < -SCROLL_MIN_SPEED || speed.y > SCROLL_MIN_SPEED) {
                        _scroll.start(_docview->GetPos(), -speed.y, SCROLL_FRICTION);
                        CRLog::trace("Starting scroll with speed %d", _scroll.speed());
                    }
                }
            	event->cancelAllPointers();
            } else {
            	int x = event->getX();
            	int y = event->getY();
                x -= _clientRect.left - _pos.left;
                y -= _clientRect.top - _pos.top;
                lString16 link = _docview->getLink(x, y, 0);
                if (link.empty())
                    link = _docview->getLink(x, y, 6);
                if (link.empty())
                    link = _docview->getLink(x, y, MIN_ITEM_PX / 10);
                bool longTap = (event->getDownDuration() > 500);
                bool twoFinigersTap = false;
                if (event->count() == 2) {
                	int dx1 = myAbs(event->getX(0) - event->getStartX(0));
                	int dy1 = myAbs(event->getY(0) - event->getStartY(0));
                	int dx2 = myAbs(event->getX(1) - event->getStartX(1));
                	int dy2 = myAbs(event->getY(1) - event->getStartY(1));
                	if (dx1 < DRAG_THRESHOLD && dy1 < DRAG_THRESHOLD && dx2 < DRAG_THRESHOLD && dy2 < DRAG_THRESHOLD) {
                		twoFinigersTap = true;
                		x = (x + event->getX(1)) / 2;
                		y = (y + event->getY(1)) / 2;
                	}
                }
                int zone = pointToTapZone(event->getX(), event->getY());
                event->cancelAllPointers();

                if (!link.empty()) {
                    CRLog::info("Link pressed: %s", LCSTR(link));
                    if (link.startsWith("http://") || link.startsWith("https://")) {
                        // TODO: support external links
                        if (!_main->getPlatform()->openLinkInExternalBrowser(UnicodeToUtf8(link))) {
                            _main->showMessage(lString16("Failed to open link ") + link, 3000);
                        }
                    } else {
                        _docview->goLink(link);
                        invalidate();
                    }
                } else {
                    //bool twoFingersTap = (event->count() == 2) && event->get
                    //onTapZone(zone, twoFinigersTap);
                    onTapZone(zone, longTap || twoFinigersTap);
                }
            }
            _dragStartOffset = 0; //NO_DRAG;
            _isDragging = false;
//            setScrollOffset(_scrollOffset);
//            if (itemIndex != -1) {
//                //CRLog::trace("UP ts=%lld downTs=%lld downDuration=%lld", event->getEventTimestamp(), event->getDownEventTimestamp(), event->getDownDuration());
//                bool isLong = event->getDownDuration() > LONG_TOUCH_THRESHOLD; // 0.5 seconds threshold
//                if (isLong && onItemLongClickEvent(itemIndex))
//                    return true;
//                onItemClickEvent(itemIndex);
//            }
        }
        // fire onclick
        //CRLog::trace("list UP");
        break;
    case ACTION_FOCUS_IN:
//        if (isDragging)
//            setScrollOffset(_dragStartOffset - delta);
//        else
//            _selectedItem = index;
        //invalidate();
        //CRLog::trace("list FOCUS IN");
        break;
    case ACTION_FOCUS_OUT:
//        if (isDragging)
//            setScrollOffset(_dragStartOffset - delta);
//        else
//            _selectedItem = -1;
        //invalidate();
        return false; // to continue tracking
        //CRLog::trace("list FOCUS OUT");
        break;
    case ACTION_CANCEL:
        if (_pinchOp) {
        	endPinchOp(pinchDx, pinchDy, true);
        }
        _isDragging = false;
        cancelSelection();
        //setScrollOffset(_scrollOffset);
        //CRLog::trace("list CANCEL");
        break;
    case ACTION_MOVE:
        if (_selection.selecting) {
            // update selection
            if (insideClient)
                updateSelection(event->getX(), event->getY());
        } else if (_pinchOp) {
            updatePinchOp(pinchDx, pinchDy);
    	} else if (!_isDragging && event->count() == 2) {
            cancelSelection();
			int ddx0 = myAbs(event->getStartX(0) - event->getStartX(1));
			int ddy0 = myAbs(event->getStartY(0) - event->getStartY(1));
			int ddx1 = myAbs(event->getX(0) - event->getX(1));
			int ddy1 = myAbs(event->getY(0) - event->getY(1));
			int op0, op1;
			if (ddx0 > ddy0 * 3)
				op0 = PINCH_OP_HORIZONTAL;
			else if (ddy0 > ddx0 * 3)
				op0 = PINCH_OP_VERTICAL;
			else
				op0 = PINCH_OP_DIAGONAL;
			if (ddx1 > ddy1 * 3)
				op1 = PINCH_OP_HORIZONTAL;
			else if (ddy1 > ddx1 * 3)
				op1 = PINCH_OP_VERTICAL;
			else
				op1 = PINCH_OP_DIAGONAL;
			int ddd = myAbs(pinchDx) + myAbs(pinchDy);
			if (op0 == op1 && ddd > DRAG_THRESHOLD_X * 2 / 3) {
				startPinchOp(op0, pinchDx, pinchDy);
			}
        } else if (_viewMode != DVM_PAGES && !_isDragging && ((delta > DRAG_THRESHOLD) || (-delta > DRAG_THRESHOLD))) {
            cancelSelection();
            if (insideClient) {
                _isDragging = true;
                _docview->SetPos(_dragStartOffset - delta, false);
                prepareScroll(-delta);
                invalidate();
            }
            //_main->update(true);
        } else if (_viewMode == DVM_PAGES && !_isDragging && ((delta2 > DRAG_THRESHOLD) || (-delta2 > DRAG_THRESHOLD))) {
            if (insideClient) {
                if (_pageAnimation == PAGE_ANIMATION_NONE) {
                    if (delta2 < 0)
                        _docview->doCommand(DCMD_PAGEDOWN, 1);
                    else
                        _docview->doCommand(DCMD_PAGEUP, 1);
                    event->cancelAllPointers();
                    cancelSelection();
                    invalidate();
                    return true;
                }
                _isDragging = true;
                cancelSelection();
                prepareScroll(-delta2);
                invalidate();
                //_main->update(true);
            }
        } else if (_isDragging) {
            if (insideClient) {
                _dragPos.x = event->getX();
                _dragPos.y = event->getY();
                if (_viewMode == DVM_PAGES) {
                    // will be handled in Draw
                } else {
                    _docview->SetPos(_dragStartOffset - delta, false);
                }
                cancelSelection();
                invalidate();
            }
            //_main->update(true);
        } else if (!_isDragging) {
            if (insideClient) {
                if (event->count() == 2) {
                    int ddx0 = myAbs(event->getStartX(0) - event->getStartX(1));
                    int ddy0 = myAbs(event->getStartY(0) - event->getStartY(1));
                    int ddx1 = myAbs(event->getX(0) - event->getX(1));
                    int ddy1 = myAbs(event->getY(0) - event->getY(1));
                    int op0, op1;
                    op0 = -1;
                    op1 = -2;
                    if (ddx0 > ddy0 / 2)
                        op0 = PINCH_OP_HORIZONTAL;
                    else if (ddy0 > ddx0 / 2)
                        op0 = PINCH_OP_VERTICAL;
                    else
                        op0 = PINCH_OP_DIAGONAL;
                    if (ddx1 > ddy1 / 2)
                        op1 = PINCH_OP_HORIZONTAL;
                    else if (ddy1 > ddx1 / 2)
                        op1 = PINCH_OP_VERTICAL;
                    else
                        op0 = PINCH_OP_DIAGONAL;
                    int ddd = myAbs(pinchDx) + myAbs(pinchDy);
                    if (op0 == op1 && ddd > DRAG_THRESHOLD_X * 2 / 3) {
                        cancelSelection();
                        startPinchOp(op0, pinchDx, pinchDy);
                    }
                } else {
                    if ((delta2 > DRAG_THRESHOLD_X) || (-delta2 > DRAG_THRESHOLD_X)) {
                        cancelSelection();
                        _main->startDragging(event, false);
                    }
                }
            }
        }
        // ignore
        //CRLog::trace("list MOVE");
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
    }
    return true;
}

void CRUIReadWidget::goToPosition(lString16 path) {
    ldomXPointer pt = _docview->getDocument()->createXPointer(path);
    _docview->goToBookmark(pt);
    clearImageCaches();
}

// formats percent value 0..10000  as  XXX.XX%
static lString16 formatPercent(int percent) {
    char s[100];
    sprintf(s, "%d.%02d%%", percent / 100, percent % 100);
    return Utf8ToUnicode(s);
}

lString16 CRUIReadWidget::getCurrentPositionDesc() {
    int pos = getCurrentPositionPercent();
    lString16 str;
    str += formatPercent(pos);
    str += "  ";
    ldomXPointer ptr = _docview->getBookmark();
    if (!ptr.isNull()) {
        lString16 titleText;
        lString16 posText;
        if ( _docview->getBookmarkPosText( ptr, titleText, posText ) ) {
            str += titleText;
        }
    }
    return str;
}

int CRUIReadWidget::getCurrentPositionPercent() {
    return _docview->getPosPercent();
}

/// move by page w/o animation
void CRUIReadWidget::moveByPage(int direction) {
    if (direction > 0)
        _docview->doCommand(DCMD_PAGEDOWN, 1);
    else
        _docview->doCommand(DCMD_PAGEUP, 1);
    if (_viewMode == DVM_PAGES) {
        _pagedCache.prepare(_docview, _docview->getCurPage(), _measuredWidth, _measuredHeight, 0, false, _pageAnimation);
    } else {
        _scrollCache.prepare(_docview, _docview->GetPos(), _clientRect.width(), _clientRect.height(), 0, false);
    }
    invalidate();
    //_main->update(true);
}

void CRUIReadWidget::goToPage(int page) {
    if (_viewMode == DVM_PAGES) {
        _docview->goToPage(page, false);
        _pagedCache.prepare(_docview, _docview->getCurPage(), _measuredWidth, _measuredHeight, 0, false, _pageAnimation);
    }
}

void CRUIReadWidget::goToPercent(int percent) {
    int maxpos = _docview->GetFullHeight() - _docview->GetHeight();
    if (maxpos < 0)
        maxpos = 0;
    int p = (int)(percent * (lInt64)maxpos / 10000);
    _docview->SetPos(p, false);
    if (_viewMode == DVM_PAGES)
        _pagedCache.prepare(_docview, _docview->getCurPage(), _measuredWidth, _measuredHeight, 0, false, _pageAnimation);
    else
        _scrollCache.prepare(_docview, p, _clientRect.width(), _clientRect.height(), 0, false);
    invalidate();
}

bool CRUIReadWidget::hasTOC() {
    LVTocItem * toc = _docview->getToc();
    return toc && toc->getChildCount();
}

void CRUIReadWidget::showTOC() {
    if (!hasTOC())
        return;
    CRUITOCWidget * widget = new CRUITOCWidget(_main, this);
    _main->showTOC(widget);
}

void CRUIReadWidget::showBookmarks() {
    CRUIBookmarksWidget * widget = new CRUIBookmarksWidget(_main, this);
    _main->showBookmarks(widget);
}

void CRUIReadWidget::showFindTextPopup() {
    lvRect margins;
    CRUIFindTextPopup * popup = new CRUIFindTextPopup(this, _lastSearchPattern);
    preparePopup(popup, ALIGN_TOP, margins, 0x80, false, false);
}

void CRUIReadWidget::showGoToPercentPopup() {
    lvRect margins;
    CRUIGoToPercentPopup * popup = new CRUIGoToPercentPopup(this);
    preparePopup(popup, ALIGN_BOTTOM, margins, 0x30, false, true);
}

CRUIReadMenu * CRUIReadWidget::createReaderMenu(bool forToolbar) {
    CRUIActionList actions;
    actions.add(ACTION_BACK);
    if (_docview->canGoBack())
        actions.add(ACTION_LINK_BACK);
    if (_docview->canGoForward())
        actions.add(ACTION_LINK_FORWARD);
    actions.add(ACTION_SETTINGS);
    if (!crconfig.einkMode) {
        if (_main->getSettings()->getBoolDef(PROP_NIGHT_MODE, false))
            actions.add(ACTION_DAY_MODE);
        else
            actions.add(ACTION_NIGHT_MODE);
    }
    //actions.add(ACTION_GOTO_PERCENT);
    CRLog::trace("checking TOC");
    //if (hasTOC())
        actions.add(ACTION_TOC);
    actions.add(ACTION_GOTO_PERCENT);
    actions.add(ACTION_BOOKMARKS);
    actions.add(ACTION_FIND_TEXT);
    if (_main->getPlatform()->getTextToSpeech())
        actions.add(ACTION_TTS_PLAY);
    if (forToolbar) {
        actions.add(ACTION_PAGE_UP);
        actions.add(ACTION_PAGE_DOWN);
    }
    actions.add(ACTION_HELP);
    if (_main->getPlatform()->supportsFullscreen())
        actions.add(ACTION_TOGGLE_FULLSCREEN);
    if (!forToolbar)
        actions.add(ACTION_EXIT);
    CRUIReadMenu * menu = new CRUIReadMenu(this, actions, !forToolbar, !forToolbar, forToolbar ? 1 : 0);
    if (forToolbar)
        menu->setStyle("TOOL_BAR");
    return menu;
}

void CRUIReadWidget::showReaderMenu() {
    CRLog::trace("showReaderMenu");
    CRUIReadMenu * menu = createReaderMenu(false);
    CRLog::trace("showing popup");
    lvRect margins;
    preparePopup(menu, ALIGN_BOTTOM, margins, 0x20);
}

void CRUIReadWidget::onSentenceFinished() {
    if (!_ttsInProgress) {
        _docview->clearSelection();
        clearImageCaches();
        invalidate();
        _main->update(false);
        return;
    }
    CRLog::trace("CRUIReadWidget::onSentenceFinished()");
    if (!_docview->onSelectionCommand(DCMD_SELECT_NEXT_SENTENCE, 1)) {
        _ttsInProgress = false;
        return;
    }
    updateReadingPosition();
    _main->getPlatform()->getTextToSpeech()->setTextToSpeechCallback(this);
    CRLog::trace("CRUIReadWidget::onSentenceFinished() tell %s", UnicodeToUtf8(_selection.selectionText).c_str());
    _main->getPlatform()->getTextToSpeech()->tell(_selection.selectionText);
}

void CRUIReadWidget::setScrollBarVisible(bool v) {
    bool currentlyVisible = _scrollbar != NULL;
    if (currentlyVisible == v)
        return;
    if (v) {
        _scrollbar = new CRUIScrollBar(true, 0, 100, 0, 1);
        _scrollbar->setStyle("TOOL_BAR");
        _scrollbar->setScrollPosCallback(this);
    } else {
        if (_scrollbar)
            delete _scrollbar;
        _scrollbar = NULL;
    }
    requestLayout();
}

void CRUIReadWidget::updateScrollbar() {
    if (!_scrollbar)
        return;
    //if (!_docview->)
    const LVScrollInfo * si = _docview->getScrollInfo();
    _scrollbar->setMaxScrollPos(si->maxpos + si->pagesize);
    _scrollbar->setPageSize(si->pagesize);
    _scrollbar->setScrollPos(si->pos);
}

bool CRUIReadWidget::onScrollPosChange(CRUIScrollBase * widget, int pos, bool manual) {
    CR_UNUSED3(widget, pos, manual);
    if (manual) {
        if (_viewMode == DVM_PAGES)
            goToPage(pos * _docview->getVisiblePageCount());
        else
            goToPercent(_scrollbar->getScrollPosPercent());
    }
    return true;
}

void CRUIReadWidget::setToolbarPosition(int toolbarPosition) {
    if (toolbarPosition == _toolbarPosition)
        return;
    _toolbarPosition = toolbarPosition;
    if (_toolbarPosition) {
        if (_toolbar)
            delete _toolbar;
        _toolbar = createReaderMenu(true);
        requestLayout();
    } else {
        if (_toolbar) {
            delete _toolbar;
            _toolbar = NULL;
            requestLayout();
        }
    }
}

bool CRUIReadWidget::updateReadingPosition() {
    ldomXRangeList & sel = _docview->getDocument()->getSelections();
    ldomXRange currSel;
    if ( sel.length()>0 )
        currSel = *sel[0];
    if (currSel.isNull()) {
        // no selection
        return false;
    }
    _selection.startPos = currSel.getStart().toString();
    _selection.endPos = currSel.getEnd().toString();
    _selection.startCursorPos.clear();
    _selection.endCursorPos.clear();
    _docview->getCursorRect(currSel.getStart(), _selection.startCursorPos, false);
    _docview->getCursorRect(currSel.getEnd(), _selection.endCursorPos, false);
    _selection.selectionText = currSel.getRangeText();
    updateSelectionBookmark();
    clearImageCaches();
    invalidate();
    _main->update(false);
    return true;
}

void CRUIReadWidget::stopReadAloud() {
    if (_ttsInProgress) {
        _ttsInProgress = false;
        if (_main->getPlatform()->getTextToSpeech())
            _main->getPlatform()->getTextToSpeech()->stop();
        _docview->clearSelection();
        clearImageCaches();
        invalidate();
        _main->update(false);
        updateVolumeControls();
    }
}

void CRUIReadWidget::updateVolumeControls() {
    bool volumeControlsEnabled = _volumeKeysEnabled;
    if (_ttsInProgress)
        volumeControlsEnabled = false;
    if (_main->getMode() != MODE_READ)
        volumeControlsEnabled = false;
    _main->getPlatform()->setVolumeKeysEnabled(volumeControlsEnabled);
}

void CRUIReadWidget::startReadAloud() {
    if (_ttsInProgress)
        return;
    CRLog::trace("CRUIReadWidget::startReadAloud()");
    if (_main->getPlatform()->getTextToSpeech()) {
        _main->showMessage(_16(STR_TTS_PLAY_IN_PROGRESS), 4000);
        ldomXRangeList & sel = _docview->getDocument()->getSelections();
        ldomXRange currSel;
        if ( sel.length()>0 )
            currSel = *sel[0];
        else {
            _docview->onSelectionCommand(DCMD_SELECT_FIRST_SENTENCE, 1);
            ldomXRangeList & sel = _docview->getDocument()->getSelections();
            if ( sel.length()>0 )
                currSel = *sel[0];
        }
        if (currSel.isNull()) {
            // no selection
            return;
        }
        updateReadingPosition();
        _ttsInProgress = true;
        _main->getPlatform()->getTextToSpeech()->setTextToSpeechCallback(this);
        _main->getPlatform()->getTextToSpeech()->tell(_selection.selectionText);
        updateVolumeControls();
    }
    invalidate();
}

/// override to handle menu or other action
bool CRUIReadWidget::onAction(const CRUIAction * action) {
    if (!action)
        return false;
    if (action->cmd) {
        doCommand(action->cmd, action->param);
        return true;
    }
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    case CMD_SELECTION_COPY:
        _main->getPlatform()->copyToClipboard(_selection.selectionText);
        cancelSelection();
        return true;
    case CMD_SELECTION_ADD_BOOKMARK:
        addSelectionBookmark();
        cancelSelection();
        return true;
    case CMD_LINK_BACK:
        if (_docview->canGoBack()) {
            _docview->goBack();
            postUpdatePosition();
            _main->update(true);
        } else
            _main->back();
        return true;
    case CMD_LINK_FORWARD:
        _docview->goForward();
        return true;
    case CMD_TOC:
        showTOC();
        return true;
    case CMD_BOOKMARKS:
        showBookmarks();
        return true;
    case CMD_MENU:
        showReaderMenu();
        return true;
    case CMD_GOTO_PERCENT:
        showGoToPercentPopup();
        return true;
    case CMD_FIND_TEXT:
        showFindTextPopup();
        return true;
    case CMD_TTS_PLAY:
        startReadAloud();
        return true;
    case CMD_SETTINGS:
        _main->showSettings(lString8("@settings/reader"));
        return true;
    default:
        return CRUIWindowWidget::onAction(action);
    }
    return false;
}

// apply changed settings
void CRUIReadWidget::applySettings(CRPropRef changed, CRPropRef oldSettings, CRPropRef newSettings) {
    CR_UNUSED2(oldSettings, newSettings);
    CRPropRef docviewprops = LVCreatePropsContainer();
    //bool backgroundChanged = false;
    //bool needClearCache = false;
    for (int i = 0; i < changed->getCount(); i++) {
        lString8 key(changed->getName(i));
        lString8 value(UnicodeToUtf8(changed->getValue(i)));
        //CRLog::trace("%s = %s", key.c_str(), value.c_str());
        if (isDocViewProp(key)) {
            docviewprops->setString(key.c_str(), value.c_str());
            if (key == PROP_FONT_COLOR) {
                _docview->setTextColor(changed->getColorDef(PROP_FONT_COLOR, 0));
                docviewprops->setString(PROP_STATUS_FONT_COLOR, value.c_str());
                //needClearCache = true;
            }
            if (key == PROP_FONT_SIZE) {
                int sz = changed->getIntDef(PROP_FONT_SIZE, 22);
                docviewprops->setInt(PROP_STATUS_FONT_SIZE, sz * 75 / 100);
            }
        }
        if (key == PROP_BACKGROUND_COLOR
                || key == PROP_BACKGROUND_IMAGE
                || key == PROP_BACKGROUND_IMAGE_ENABLED
                || key == PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS
                || key == PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST) {
            //backgroundChanged = true;
            //needClearCache = true;
        }
        if (key == PROP_APP_READER_SHOW_TOOLBAR) {
            int n = changed->getIntDef(PROP_APP_READER_SHOW_TOOLBAR, 0);
            if (n != _toolbarPosition) {
                setToolbarPosition(n);
            }
        }
        if (key == PROP_APP_CONTROLS_VOLUME_KEYS) {
            _volumeKeysEnabled = changed->getBoolDef(PROP_APP_CONTROLS_VOLUME_KEYS, false);
            updateVolumeControls();
        }
        if (key == PROP_APP_READER_SHOW_SCROLLBAR) {
            bool flg = changed->getBoolDef(PROP_APP_READER_SHOW_SCROLLBAR, 0);
            setScrollBarVisible(flg);
        }
        if (key == PROP_APP_BOOK_COVER_VISIBLE || key == PROP_PAGE_VIEW_ANIMATION) {
            docviewprops->setString(key.c_str(), value.c_str());
        } else if (key == PROP_APP_BOOK_COVER_COLOR) {
            docviewprops->setString(key.c_str(), value.c_str());
        }
        if (key == PROP_HYPHENATION_DICT) {
            setHyph(lastBookLang, value);
            _docview->requestRender();
            //needClearCache = true;
            invalidate();
        }
        if (key == PROP_PAGE_VIEW_MODE) {
            int n = value.atoi();
            if (n == 0)
                _docview->setViewMode(DVM_SCROLL);
            else if (n == 1)
                _docview->setViewMode(DVM_PAGES, 1);
            else
                _docview->setViewMode(DVM_PAGES, 2);
            _viewMode = _docview->getViewMode();
        }
        if (key == PROP_PAGE_VIEW_ANIMATION) {
            _pageAnimation = (PageFlipAnimation)value.atoi();
            if (_pageAnimation < PAGE_ANIMATION_NONE || _pageAnimation > PAGE_ANIMATION_3D)
                _pageAnimation = PAGE_ANIMATION_SLIDE;
        }
    }
//    if (backgroundChanged) {
//        _docview->setBackground(resourceResolver->getBackgroundImage(newSettings));
//    }
    //if (needClearCache) {
    clearImageCaches();
    //}
    if (docviewprops->getCount())
        _docview->propsApply(docviewprops);
}

/// on starting file loading
void CRUIReadWidget::OnLoadFileStart( lString16 filename ) {
    CR_UNUSED(filename);
}

/// format detection finished
void CRUIReadWidget::OnLoadFileFormatDetected(doc_format_t fileFormat) {
    lString8 cssFile = crconfig.cssDir + LVDocFormatCssFileName(fileFormat);

    CRLog::trace("OnLoadFileFormatDetected cssFile=%s", cssFile.c_str());
    lString8 css;
    if (!LVLoadStylesheetFile(Utf8ToUnicode(cssFile), css)) {
        // todo: fallback
        CRLog::error("LVLoadStylesheetFile %s is failed", cssFile.c_str());
    }
    _docview->setStyleSheet(css);
}

/// file loading is finished successfully - drawCoveTo() may be called there
void CRUIReadWidget::OnLoadFileEnd() {

}

/// first page is loaded from file an can be formatted for preview
void CRUIReadWidget::OnLoadFileFirstPagesReady() {

}

/// file progress indicator, called with values 0..100
void CRUIReadWidget::OnLoadFileProgress( int percent) {
    CR_UNUSED(percent);
}

/// document formatting started
void CRUIReadWidget::OnFormatStart() {

}

/// document formatting finished
void CRUIReadWidget::OnFormatEnd() {
	invalidate();
}

/// format progress, called with values 0..100
void CRUIReadWidget::OnFormatProgress(int percent) {
    CR_UNUSED(percent);
}

/// format progress, called with values 0..100
void CRUIReadWidget::OnExportProgress(int percent) {
    CR_UNUSED(percent);
}

/// file load finiished with error
void CRUIReadWidget::OnLoadFileError(lString16 message) {
    CR_UNUSED(message);
}

/// Override to handle external links
void CRUIReadWidget::OnExternalLink(lString16 url, ldomNode * node) {
    CR_UNUSED2(url, node);
}

/// Called when page images should be invalidated (clearImageCache() called in LVDocView)
void CRUIReadWidget::OnImageCacheClear() {
//    class ClearCache : public CRRunnable {
//        CRUIReadWidget * _widget;
//    public:
//        ClearCache(CRUIReadWidget * widget) : _widget(widget) {}
//        virtual void run() {
//            _widget->clearImageCaches();
//        }
//    };
//    concurrencyProvider->executeGui(new ClearCache(this));
}

/// return true if reload will be processed by external code, false to let internal code process it
bool CRUIReadWidget::OnRequestReload() {
    return false;
}




//================================================================
// Scroll Mode page image cache

CRUIReadWidget::ScrollModePageCache::ScrollModePageCache() : minpos(0), maxpos(0), dx(0), dy(0), tdx(0), tdy(0) {

}

#define MIN_TEX_SIZE 64
#define MAX_TEX_SIZE 4096
static int nearestPOT(int n) {
	for (int i = MIN_TEX_SIZE; i <= MAX_TEX_SIZE; i++) {
		if (n <= i)
			return i;
	}
	return MIN_TEX_SIZE;
}

LVDrawBuf * CRUIReadWidget::ScrollModePageCache::createBuf() {
    return CRUICreateDrawBuf(dx, tdy, 32);
}

void CRUIReadWidget::ScrollModePageCache::setSize(int _dx, int _dy) {
    if (dx != _dx || dy != _dy) {
        clear();
        dx = _dx;
        dy = _dy;
        tdx = nearestPOT(dx);
        tdy = nearestPOT(dy);
    }
}

/// ensure images are prepared
void CRUIReadWidget::ScrollModePageCache::prepare(LVDocView * _docview, int _pos, int _dx, int _dy, int direction, bool force) {
    CRENGINE_GUARD;
    setSize(_dx, _dy);
    if (_pos >= minpos && _pos + dy <= maxpos && !force)
        return; // already prepared
    int y0 = direction == 0 ? _pos : (direction > 0 ? (_pos - dy / 4) : (_pos - dy * 5 / 4));
    int y1 = direction == 0 ? _pos : (direction > 0 ? (_pos + dy + dy * 5 / 4) : (_pos + dy + dy / 4));
    if (y0 < 0)
    	y0 = 0;
    if (y1 > _docview->GetFullHeight() + dy)
        y1 = _docview->GetFullHeight() + dy;
    int pos0 = y0 / tdy * tdy;
    int pos1 = (y1 + tdy - 1) / tdy * tdy;
    int pageCount = (pos1 - pos0) / tdy + 1;
    for (int i = pages.length() - 1; i >= 0; i--) {
        ScrollModePage * p = pages[i];
        if (!p->intersects(y0, y1)) {
            pages.remove(i);
            delete p;
        }
    }
    for (int i = 0; i < pageCount; i++) {
        int pos = pos0 + i * tdy;
        bool found = false;
        for (int k = pages.length() - 1; k >= 0; k--) {
            if (pages[k]->pos == pos) {
                found = true;
                break;
            }
        }
        if (!found) {
            ScrollModePage * page = new ScrollModePage();
            page->dx = dx;
            page->dy = tdy;
            page->pos = pos;
            page->drawbuf = createBuf();
            LVDrawBuf * buf = page->drawbuf; //dynamic_cast<GLDrawBuf*>(page->drawbuf);
            buf->beforeDrawing();
            int oldpos = _docview->GetPos();
            _docview->SetPos(pos, false, true);
            buf->SetTextColor(_docview->getTextColor());
            buf->SetBackgroundColor(_docview->getBackgroundColor());
            _docview->Draw(*buf, false);
            _docview->SetPos(oldpos, false, true);
            buf->afterDrawing();
            pages.add(page);
            CRLog::trace("new page cache item %d..%d", page->pos, page->pos + page->dy);
        }
    }
    minpos = maxpos = -1;
    for (int k = 0; k < pages.length(); k++) {
        //CRLog::trace("page cache item [%d] %d..%d", k, pages[k]->pos, pages[k]->pos + pages[k]->dy);
        if (minpos == -1 || minpos > pages[k]->pos) {
            minpos = pages[k]->pos;
        }
        if (maxpos == -1 || maxpos < pages[k]->pos + pages[k]->dy) {
            maxpos = pages[k]->pos + pages[k]->dy;
        }
    }
}

void CRUIReadWidget::ScrollModePageCache::draw(LVDrawBuf * dst, int pos, int x, int y) {
	CRLog::trace("ScrollModePageCache::draw()");
    // workaround for no-rtti builds
    LVDrawBuf * glbuf = dst; //->asGLDrawBuf(); //dynamic_cast<GLDrawBuf*>(buf);
    if (glbuf) {
        //glbuf->beforeDrawing();
        for (int k = pages.length() - 1; k >= 0; k--) {
            if (pages[k]->intersects(pos, pos + dy)) {
                // draw fragment
                int y0 = pages[k]->pos - pos;
                CRUIDrawTo(pages[k]->drawbuf, glbuf, x, y + y0);
                //pages[k]->drawbuf->DrawTo(glbuf, x, y + y0, 0, NULL);
            }
        }
        //glbuf->afterDrawing();
    }
}

void CRUIReadWidget::ScrollModePageCache::clear() {
    pages.clear();
    minpos = 0;
    maxpos = 0;
}


//=============================================================================
//  Paged mode

CRUIReadWidget::PagedModePageCache::PagedModePageCache() : numPages(0), pageCount(0), dx(0), dy(0), tdx(0), tdy(0), direction(0), newPage(0) {

}

void CRUIReadWidget::PagedModePageCache::clear() {
	//CRLog::trace("CRUIReadWidget::PagedModePageCache::clear");
    pages.clear();
}

LVDrawBuf * CRUIReadWidget::PagedModePageCache::createBuf() {
    return CRUICreateDrawBuf(dx, tdy, 32);
}

void CRUIReadWidget::PagedModePageCache::setSize(int _dx, int _dy, int _numPages, int _pageCount) {
    if (dx != _dx || dy != _dy || numPages != _numPages || pageCount != _pageCount) {
        clear();
        numPages = _numPages;
        pageCount = _pageCount;
        dx = _dx;
        dy = _dy;
        tdx = nearestPOT(dx);
        tdy = nearestPOT(dy);
    }
}

CRUIReadWidget::PagedModePage * CRUIReadWidget::PagedModePageCache::findPage(int page) {
	for (int i = 0; i < pages.length(); i++) {
        if (pages[i]->pageNumber == page && !pages[i]->back)
			return pages[i];
	}
	return NULL;
}

CRUIReadWidget::PagedModePage * CRUIReadWidget::PagedModePageCache::findPageBack(int page) {
    for (int i = 0; i < pages.length(); i++) {
        if (pages[i]->pageNumber == page && pages[i]->back)
            return pages[i];
    }
    // fallback
    //return findPage(page);
    return NULL;
}

void CRUIReadWidget::PagedModePageCache::clearExcept(int page1, int page2) {
	for (int i = pages.length() - 1; i >= 0; i--) {
		if (pages[i]->pageNumber != page1 && pages[i]->pageNumber != page2) {
			//CRLog::trace("Clearing page image %d", pages[i]->pageNumber);
			delete pages.remove(i);
		}
	}
}

void CRUIReadWidget::PagedModePageCache::preparePage(LVDocView * _docview, int pageNumber, bool back) {
	if (pageNumber < 0)
		return;
    if (back && findPageBack(pageNumber))
        return; // already prepared
    if (!back && findPage(pageNumber))
		return; // already prepared
    PagedModePage * page = new PagedModePage();
    page->dx = dx;
    page->dy = dy;
    page->tdx = tdx;
    page->tdy = tdy;
    page->pageNumber = pageNumber;
    page->numPages = numPages;
    page->drawbuf = createBuf();
	//CRLog::trace("Created page %08x", (lUInt32) page->drawbuf);
    page->back = back;
    LVDrawBuf * buf = page->drawbuf; //dynamic_cast<GLDrawBuf*>(page->drawbuf);
    //CRLog::trace("CRUIReadWidget::PagedModePageCache::preparePage Preparing page image for page %d ; buf = %08x", pageNumber, (lUInt32)buf);
    buf->beforeDrawing();
    buf->SetTextColor(_docview->getTextColor());
    buf->SetBackgroundColor(_docview->getBackgroundColor());
    lvRect rc(0, 0, dx, dy);
    int oldPage = _docview->getCurPage();
    if (oldPage != pageNumber)
        _docview->goToPage(pageNumber);
    //_docview->Draw(*buf, -1, pageNumber, false, false);
    //CRLog::trace("_docview->Draw calling");
    _docview->Draw(*buf, false);
    //CRLog::trace("_docview->Draw done");
//    if ((pageNumber & 1))
//    	buf->FillRect(100, 10, 110, 20, 0x800000FF);
//    else
//    	buf->FillRect(120, 10, 130, 20, 0x800000FF);
    if (back)
        _docview->drawPageBackground(*buf, 0, 0, 0x60); // semitransparent background above page image
    if (oldPage != pageNumber)
        _docview->goToPage(oldPage);
    if (!crconfig.einkMode) {
        //CRLog::trace("drawing page gradients");
        int sdx = dx / 10 / _docview->getVisiblePageCount();
        lUInt32 cl1 = 0xE0000000;
        lUInt32 cl2 = 0xFF000000;
        buf->GradientRect(0, 0, sdx, dy, cl1, cl2, cl2, cl1);
        buf->GradientRect(dx - sdx, 0, dx, dy, cl2, cl1, cl1, cl2);
        if (_docview->getVisiblePageCount() == 2) {
            buf->GradientRect(dx / 2, 0, dx / 2 + sdx, dy, cl1, cl2, cl2, cl1);
            buf->GradientRect(dx / 2 - sdx, 0, dx / 2, dy, cl2, cl1, cl1, cl2);
        }
        //CRLog::trace("drawing page gradients done");
    }
    if (_docview->getVisiblePageCount() == 2) {
        lvRect rc1 = rc;
        if (crconfig.einkMode) {
            rc1.left = dx / 2 + 1;
            rc1.right = rc1.left + 1;
            buf->FillRect(rc1, 0xAAAAAA);
        } else {
            rc1.right = dx / 2 + 1;
            buf->DrawFrame(rc1, 0xC0404040, 1);
            rc1.shrink(1);
            buf->DrawFrame(rc1, 0xE0404040, 1);
            rc1 = rc;
            rc1.left = dx / 2;
            buf->DrawFrame(rc1, 0xC0404040, 1);
            rc1.shrink(1);
            buf->DrawFrame(rc1, 0xE0404040, 1);
        }
    } else {
        buf->DrawFrame(rc, 0xC0404040, 1);
        rc.shrink(1);
        buf->DrawFrame(rc, 0xE0404040, 1);
    }
    //CRLog::trace("calling buf->afterDrawing()");
    buf->afterDrawing();
    //CRLog::trace("done buf->afterDrawing()");
    pages.add(page);
    CRLog::trace("CRUIReadWidget::PagedModePageCache::preparePage page is prepared");
}

/// ensure images are prepared
void CRUIReadWidget::PagedModePageCache::prepare(LVDocView * _docview, int _page, int _dx, int _dy, int _direction, bool force, int _pageAnimation) {
    CR_UNUSED(force);
    CRENGINE_GUARD;
    setSize(_dx, _dy, _docview->getVisiblePageCount(), _docview->getPageCount());
    pageAnimation = _pageAnimation;
    if (_direction)
        direction = _direction;
    int thisPage = _page; // current page
    if (numPages == 2)
    	thisPage = thisPage & ~1;
    int nextPage = -1;
    if (_direction > 0) {
    	nextPage = thisPage + numPages;
        if (nextPage >= pageCount + numPages - 1)
    		nextPage = -1;
    } else if (_direction < 0) {
    	nextPage = thisPage - numPages;
    	if (nextPage < 0)
    		nextPage = -1; // no page
    }
    if (nextPage >= 0)
        newPage = nextPage;
    if (findPage(thisPage) && (nextPage == -1 || findPage(nextPage)))
    	return; // already prepared
    //CRLog::trace("clearExcept(%d, %d) dir = %d", thisPage, nextPage, _direction);
    clearExcept(thisPage, nextPage);
    //CRLog::trace("preparePage(_docview, thisPage)");
    preparePage(_docview, thisPage);
    if (nextPage != -1) {
        //CRLog::trace("preparePage(_docview, nextPage)");
        preparePage(_docview, nextPage);
    }
    if (pageAnimation == PAGE_ANIMATION_3D && direction > 0 && numPages == 1) {
        preparePage(_docview, thisPage, true);
        if (nextPage != -1)
            preparePage(_docview, nextPage, true);
    }
    //CRLog::trace("prepare done");
}

void CRUIReadWidget::PagedModePageCache::drawFolded(LVDrawBuf * buf, PagedModePage * page, int srcx1, int srcx2, int dstx1, int dstx2, float angle1, float angle2, int y) {
    lUInt32 shadowAlpha = 64;
    float dangle = (angle2 - angle1);
    if (dangle < 0)
        dangle = -dangle;
    if (dangle < 0.01f) {
        buf->DrawFragment(page->drawbuf, srcx1, 0, srcx2 - srcx1, dy, dstx1, y, dstx2 - dstx1, dy, 0);
    } else {
        // TODO
        int steps = (int)(dangle / 0.15f + 1);
        float sa1 = (float)sin(angle1);
        float sa2 = (float)sin(angle2);
        for (int step = 0; step < steps; step++) {
            float a1 = angle1 + (angle2 - angle1) * step / steps;
            float a2 = angle1 + (angle2 - angle1) * (step + 1) / steps;
            int alpha1 = 255 - (int)(shadowAlpha * sin(a1));
            int alpha2 = 255 - (int)(shadowAlpha * sin(a2));
            int sx1 = srcx1 + (srcx2 - srcx1) * step / steps;
            int sx2 = srcx1 + (srcx2 - srcx1) * (step + 1) / steps;
            int dx1 = (int)(dstx1 + (dstx2 - dstx1) * ((float)sin(a1) - sa1) / (sa2 - sa1) + 0.5f);
            int dx2 = (int)(dstx1 + (dstx2 - dstx1) * ((float)sin(a2) - sa1) / (sa2 - sa1) + 0.5f);
            buf->DrawFragment(page->drawbuf, sx1, 0, sx2 - sx1, dy, dx1, y, dx2 - dx1, dy, 0);
            lUInt32 shadowcl1 = (alpha1 << 24) | 0x000000;
            lUInt32 shadowcl2 = (alpha2 << 24) | 0x000000;
            buf->GradientRect(dx1, y, dx2, y + dy, shadowcl1, shadowcl2, shadowcl2, shadowcl1);
        }
    }
}

// solve equation a - sin(a) == n
float solve_a_minus_sina_eq_n(float n) {
    int s = 1;
    if (n < 0) {
        s = -1;
        n = -n;
    }
    float a1 = 0;
    float a2 = (float)M_PI / 2;
    for (;;) {
        float a = (a1 + a2) / 2;
        float err = n - (a - (float)sin(a));
        if (err < 0.0001f && err > -0.0001f)
            return a * s;
        if (err < 0)
            a2 = a;
        else
            a1 = a;
    }
}

// solve equation cos(a) - a == n
float solve_cosa_minus_a_eq_n(float n) {
    float a1 = 0;
    float a2 = (float)M_PI / 2;
    for (;;) {
        float a = (a1 + a2) / 2;
        float err = n - ((float)cos(a) - a);
        if (err < 0.00001f && err > -0.00001f)
            return a;
        if (err > 0)
            a2 = a;
        else
            a1 = a;
    }
}

// solve equation a + cos(a) == n
float solve_cosa_plus_a_eq_n(float n) {
    float a1 = 0;
    float a2 = (float)M_PI / 2;
    for (;;) {
        float a = (a1 + a2) / 2;
        float err = n - ((float)cos(a) - a);
        if (err < 0.00001f && err > -0.00001f)
            return a;
        if (err > 0)
            a2 = a;
        else
            a1 = a;
    }
}

void CRUIReadWidget::PagedModePageCache::drawFolded(LVDrawBuf * buf, PagedModePage * page1, PagedModePage * page1back, PagedModePage * page2, int xx, int diam, int x, int y) {
    bool twoPages = numPages > 1;
    float m_pi_2 = (float)M_PI / 2;
    float fdiam = (float)diam;
    float fradius = fdiam / 2;
    float halfc = m_pi_2 * fdiam;
    float quarterc = halfc / 2;
    int maxdx = dx;
    if (twoPages)
        maxdx = dx / 2;
    float bx = 0;
    float ba = 0;
    float b = 0;
    float cx = 0;
    float ca = 0;
    float c = 0;
    float d = 0;
    float downx = 0;
    //int xx = progress * dx / 10000;
    int shadowdx = (int)(fradius);
    lUInt32 shadowcl1 = 0xD0000000;
    lUInt32 shadowcl2 = 0xFF000000;
    int shadowx = -1000;
    int shadowdarkness = 100;
    if (xx < quarterc - fradius) {
        ba = solve_a_minus_sina_eq_n(xx / fradius);
        b = (float)sin(ba) * fradius;
        bx = ba * fradius;
        downx = xx + b;

        int aa = (int)(dx - downx + 0.5f);
        int bb = (int)(dx - xx + 0.5f);
        drawFolded(buf, page1, 0, aa, 0 + x, aa + x, 0, 0, y);
        if (xx > 0)
            drawFolded(buf, page2, bb, dx, bb + x, dx + x, 0, 0, y);
        drawFolded(buf, page1, aa, dx, aa + x, dx - xx + x, 0, ba, y);

        shadowx = bb;
        shadowdx = xx;
        shadowdarkness = (int)(100 * ba / m_pi_2);
    } else if (xx < halfc) {
        b = fradius;
        ba = m_pi_2;
        bx = quarterc;
        ca = solve_cosa_minus_a_eq_n(1 - (xx - (bx - b)) / fradius);
        cx = fradius * ca;
        c = fradius - fradius * (float)cos(ca);
        //c = xx - (quarterc - fradius);
        downx = xx - c + fradius;
//            ca = (float)acos((fradius - c) / fradius);
//            cx = ca * fradius;

        int aa = (int)(dx - downx + 0.5f);
        int bb = (int)(aa + fradius + 0.5f);
        int sbb = (int)(aa + bx + 0.5f);
        int cc = (int)(bb - c + 0.5f);
        int icx = (int)(cx + 0.5f);
        drawFolded(buf, page1, 0, aa, 0 + x, aa + x, 0, 0, y); // left flat part
        drawFolded(buf, page2, bb, dx, bb + x, dx + x, 0, 0, y); // right flat part
        drawFolded(buf, page1, aa, sbb, aa + x, bb + x, 0, ba, y); // bottom bent
        if (twoPages)
            drawFolded(buf, page1back, 0, icx, cc + x, bb + x, m_pi_2 - ca, m_pi_2, y); // top bent
        else
            drawFolded(buf, page1back, dx, dx - icx, cc + x, bb + x, m_pi_2 - ca, m_pi_2, y); // top bent

        shadowx = bb;
        //shadowdx = xx;
        //shadowdarkness = (int)(100 * ba / m_pi_2);
    } else if (xx - (xx - halfc) / 2 < maxdx) { //xx < 2 * maxdx + halfc
        ba = m_pi_2;
        b = fradius;
        bx = quarterc;
        ca = m_pi_2;
        c = fradius;
        cx = quarterc;
        d = (xx - cx - bx) / 2;
        downx = xx - d;
        int bb = (int)(dx - downx + 0.5f);
        int aa = (int)(dx - (d + downx) + 0.5f);
        int cc = (int)(bb + c + 0.5f);
        int sd0 = 0;
        int sd1 = (int)(d + 0.5f);
        int sc1 = (int)(d + cx + 0.5f);
        if (!twoPages) {
            sd0 = dx - sd0 - 1;
            sd1 = dx - sd1 - 1;
            sc1 = dx - sc1 - 1;
        }

        drawFolded(buf, page1, 0, aa, 0 + x, aa + x, 0, 0, y); // left flat part
        drawFolded(buf, page2, cc, dx, cc + x, dx + x, 0, 0, y); // right flat part
        drawFolded(buf, page1back, sd1, sc1, bb + x, cc + x, 0, m_pi_2, y); // top bent
        drawFolded(buf, page1back, sd0, sd1, aa + x, bb + x, 0, 0, y); // left flat bent

        shadowx = cc;
        //shadowdx = xx;
        //shadowdarkness = (int)(100 * ba / m_pi_2);
    } else {
        int exx = 2 * maxdx - xx;
        if (exx > quarterc - fradius) {
            //
            cx = quarterc;
            c = fradius;
            ca = m_pi_2;
            ba = solve_cosa_plus_a_eq_n(1 - (exx + c - cx) / fradius);
            bx = ba * fradius;
            b = fradius - fradius * (float)cos(ba);
            d = (maxdx - bx - cx);
            //downx = maxdx + b;
            int sd0 = 0;
            int sd1 = (int)(d + 0.5f);
            int sc1 = (int)(d + cx + 0.5f);
            if (!twoPages) {
                sd0 = dx - sd0 - 1;
                sd1 = dx - sd1 - 1;
                sc1 = dx - sc1 - 1;
            }
            int aa = (int)exx;
            int bb = (int)(aa + d + 0.5f);
            int cc = (int)(bb + c + 0.5f);
            if (!twoPages) {
                aa -= dx;
                bb -= dx;
                cc -=dx;
            }
            drawFolded(buf, page2, dx - maxdx, dx, dx - maxdx + x, dx + x, 0, 0, y); // right flat part
            if (aa > 0)
                drawFolded(buf, page1, 0, aa, 0 + x, aa + x, 0, 0, y); // left flat part bottom
            if (cc > 0)
                drawFolded(buf, page1back, sd1, sc1, bb + x, cc + x, 0, m_pi_2, y); // top bent
            if (bb > 0)
                drawFolded(buf, page1back, sd0, sd1, aa + x, bb + x, 0, 0, y); // left flat bent

            shadowx = cc;
        } else if (exx >= 0) {
            //
            ca = solve_a_minus_sina_eq_n(exx / fradius);
            cx = fradius * ca;
            c = fradius * (float)sin(ca);
            d = maxdx - cx;
            int aa = (int)exx;
            int bb = (int)(aa + d + 0.5f);
            int cc = maxdx;
            int sd0 = 0;
            int sd1 = (int)(d + 0.5f);
            int sc1 = (int)(d + cx + 0.5f);
            if (!twoPages) {
                sd0 = dx - sd0 - 1;
                sd1 = dx - sd1 - 1;
                sc1 = dx - sc1 - 1;
            }
            drawFolded(buf, page2, dx - maxdx, dx, dx - maxdx + x, dx + x, 0, 0, y); // right flat part
            if (twoPages) {
                if (exx > 0)
                    drawFolded(buf, page1, 0, exx, 0 + x, exx + x, 0, 0, y); // left flat part bottom
                drawFolded(buf, page1back, sd1, sc1, bb + x, cc + x, 0, ca, y); // top bent
                drawFolded(buf, page1back, sd0, sd1, aa + x, bb + x, 0, 0, y); // left flat bent
            }

            shadowx = dx - maxdx;
            shadowdarkness = (int)(100 * ca / m_pi_2);
        }
    }

    if (shadowdarkness > 1 && shadowdarkness <= 100) {
        shadowcl1 = (255 - shadowdarkness * 0x50 / 100) << 24;
        buf->GradientRect(x + shadowx, y, x + shadowx + shadowdx, y + dy, shadowcl1, shadowcl2, shadowcl2, shadowcl1);
    }
}

void CRUIReadWidget::PagedModePageCache::calcDragPositionProgress(int startx, int currx, int direction, int & progress, int & xx) {
    if (pageAnimation == PAGE_ANIMATION_3D) {
        if (startx != -1 || currx != -1) {
            if (direction < 0 && currx < startx)
                currx = startx;
            if (direction > 0 && currx > startx)
                currx = startx;
            xx = dx - currx;
            int srcxx = direction < 0 ? dx : 0;
            int delta = currx - startx;
            if (delta < 0)
                delta = -delta;
            int correctiondx = dx / 10;
            if (delta > 0 && delta < correctiondx) {
                xx = (srcxx + (xx - srcxx) * delta / correctiondx);
            }
            if (direction > 0) {
                if (numPages == 1) {
                    progress = xx * 10000 / dx / 2;
                } else {
                    progress = xx * 10000 / dx;
                }
            } else if (direction < 0) {
                if (numPages == 1) {
                    progress = 10000 - xx * 10000 / dx / 2;
                } else {
                    progress = 10000 - xx * 10000 / dx;
                }
            } else {
                progress = 0;
            }
        } else {
            // calculate xx from progress
            if (numPages == 1) {
                if (direction > 0)
                    xx = dx * 2 * progress / 10000;
                else if (direction < 0)
                    xx = dx * 2 * (10000 - progress) / 10000;
            } else {
                if (direction > 0)
                    xx = dx * progress / 10000;
                else if (direction < 0)
                    xx = dx * (10000 - progress) / 10000;
            }
        }
    } else {
        xx = 0;
        if (startx != -1 || currx != -1) {
            if (direction > 0)
                progress = 10000 * (startx - currx) / dx;
            else if (direction < 0)
                progress = 10000 * (currx - startx) / dx;
            else
                progress = 0;
        }
    }
    if (progress < 0)
        progress = 0;
    if (progress > 10000)
        progress = 10000;
}

/// draw
void CRUIReadWidget::PagedModePageCache::draw(LVDrawBuf * dst, int pageNumber, int direction, int progress, int x, int y, int startx, int currx) {
    CR_UNUSED2(direction, progress);
    //CRLog::trace("PagedModePageCache::draw(page=%d, numPages=%d, progress=%d dir=%d)", pageNumber, numPages, progress, direction);
    // workaround for no-rtti builds
    LVDrawBuf * glbuf = dst; //dst->asGLDrawBuf(); //dynamic_cast<GLDrawBuf*>(buf);
    if (glbuf) {
        int diam = (numPages == 1 ? 50 : 25) * dx / 100;
        int xx = 0;
        calcDragPositionProgress(startx, currx, direction, progress, xx);
        //CRLog::trace("PagedModePageCache::draw(page=%d, progress=%d dir=%d xx=%d)", pageNumber, progress, direction, xx);
        //glbuf->beforeDrawing();
        int nextPage = pageNumber;
        if (direction > 0)
            nextPage += numPages;
        else if (direction < 0)
            nextPage -= numPages;
        if (nextPage >= pageCount + numPages - 1)
            nextPage = pageNumber;
        if (nextPage < 0)
            nextPage = pageNumber;
        CRUIReadWidget::PagedModePage * page = findPage(pageNumber);
        CRUIReadWidget::PagedModePage * page2 = ((nextPage != pageNumber) && (pageAnimation != PAGE_ANIMATION_NONE)) ? findPage(nextPage) : NULL;
        if (page2 && page) {
            // animation
            int ddx = dx * progress / 10000;
            int shadowdx = dx / 20;
            lUInt32 shadowcl1 = 0xD0000000;
            lUInt32 shadowcl2 = 0xFF000000;
            int alpha = 255 - 255 * progress / 10000;
            if (alpha < 0)
                alpha = 0;
            else if (alpha > 255)
                alpha = 255;
            if (direction > 0) {
                //
                if (pageAnimation == PAGE_ANIMATION_SLIDE) {
                    CRUIDrawTo(page2->drawbuf, glbuf, x + 0, y);
                    CRUIDrawTo(page->drawbuf, glbuf, x + 0 - ddx, y);
                    glbuf->GradientRect(x + dx - ddx, y, x + dx - ddx + shadowdx, y + dy, shadowcl1, shadowcl2, shadowcl2, shadowcl1);
                } else if (pageAnimation == PAGE_ANIMATION_SLIDE2) {
                    CRUIDrawTo(page2->drawbuf, glbuf, x + 0 + dx - ddx, y);
                    CRUIDrawTo(page->drawbuf, glbuf, x + 0 - ddx, y);
                } else if (pageAnimation == PAGE_ANIMATION_FADE) {
                    CRLog::trace("Fade animation: alpha %d", alpha);
                    CRUIDrawTo(page->drawbuf, glbuf, x + 0, y);
                    CRUIDrawTo(page2->drawbuf, glbuf, x + 0, y, alpha);
                } else if (pageAnimation == PAGE_ANIMATION_3D) {
                    CRUIReadWidget::PagedModePage * page_back = numPages == 1 ? findPageBack(pageNumber) : page2;
                    if (!page_back)
                        page_back = page;
                    drawFolded(glbuf, page, page_back, page2, xx, diam, x, y);
                }
            } else if (direction < 0) {
                //
                if (pageAnimation == PAGE_ANIMATION_SLIDE) {
                    CRUIDrawTo(page->drawbuf, glbuf, x + 0, y);
                    CRUIDrawTo(page2->drawbuf, glbuf, x + 0 - dx + ddx, y);
                    if (ddx < shadowdx)
                        shadowcl1 = (0xFF - ddx * 0x3F / shadowdx) << 24;
                    glbuf->GradientRect(x + 0 + ddx, y, x + 0 + ddx + shadowdx, y + dy, shadowcl1, shadowcl2, shadowcl2, shadowcl1);
                } else if (pageAnimation == PAGE_ANIMATION_SLIDE2) {
                    CRUIDrawTo(page->drawbuf, glbuf, x + 0 + ddx, y);
                    CRUIDrawTo(page2->drawbuf, glbuf, x + 0 - dx + ddx, y);
                } else if (pageAnimation == PAGE_ANIMATION_FADE) {
                    CRLog::trace("Fade animation: alpha %d", alpha);
                    CRUIDrawTo(page->drawbuf, glbuf, x + 0, y);
                    CRUIDrawTo(page2->drawbuf, glbuf, x + 0, y, alpha);
                } else if (pageAnimation == PAGE_ANIMATION_3D) {
                    CRUIReadWidget::PagedModePage * page_back = numPages == 1 ? findPageBack(nextPage) : page;
                    if (!page_back)
                        page_back = page2;
                    drawFolded(glbuf, page2, page_back, page, xx, diam, x, y);
                }
            }
        } else {
            // no animation
            if (page) {
                // simple draw current page
                CRUIDrawTo(page->drawbuf, glbuf, x, y);
            }
        }
        //glbuf->afterDrawing();
    }
}





CRUIBookmarksWidget::CRUIBookmarksWidget(CRUIMainWidget * main, CRUIReadWidget * read)  : CRUIWindowWidget(main), _readWidget(read) {
    for (int i = 0; i < _readWidget->getBookmarks().length(); i++) {
        _bookmarks.add(_readWidget->getBookmarks()[i]->clone());
    }
    _selectedItem = NULL;
    _title = new CRUITitleBarWidget(lString16(), this, this, false);
    _title->setTitle(STR_READER_BOOKMARKS);
    _body->addChild(_title);
    _list = new CRUIListWidget(true, this);
    _list->setOnItemClickListener(this);
    _list->setOnItemLongClickListener(this);
    _list->setStyle("SETTINGS_ITEM_LIST");
    _body->addChild(_list);
    _itemWidget = new CRUIHorizontalLayout();
    _itemWidget->setMinHeight(MIN_ITEM_PX * 2 / 3);
    _itemWidget->setPadding(PT_TO_PX(2));
    _chapter = new CRUITextWidget();
    _page = new CRUITextWidget();
    _itemWidget->addChild(_chapter);
    _itemWidget->addChild(_page);
    _page->setMargin(lvRect(PT_TO_PX(3), 0, PT_TO_PX(2), 0));
    _page->setAlign(ALIGN_RIGHT|ALIGN_VCENTER);
    _chapter->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
    _chapter->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _itemWidget->setStyle("LIST_ITEM");
}

// list adapter methods
int CRUIBookmarksWidget::getItemCount(CRUIListWidget * list) {
    CR_UNUSED(list);
    return _bookmarks.length();
}

CRUIWidget * CRUIBookmarksWidget::getItemWidget(CRUIListWidget * list, int index) {
    CR_UNUSED(list);
    BookDBBookmark * item = _bookmarks[index];
    _chapter->setText(Utf8ToUnicode(item->posText.c_str()));
    _page->setText(formatPercent(item->percent));
    return _itemWidget;
}

// list item click
bool CRUIBookmarksWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    BookDBBookmark * item = _bookmarks[itemIndex];
    _readWidget->goToPosition(Utf8ToUnicode(item->startPos.c_str()));
    return onAction(CMD_BACK);
}

// list item click
bool CRUIBookmarksWidget::onListItemLongClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    _selectedItem = _bookmarks[itemIndex];
    CRUIActionList actions;
    actions.add(ACTION_BOOKMARK_GOTO);
    actions.add(ACTION_BOOKMARK_REMOVE);
    lvRect margins;
    //margins.right = MIN_ITEM_PX * 120 / 100;
    showMenu(actions, ALIGN_TOP, margins, false);
    return true;
}

// on click
bool CRUIBookmarksWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    return true;
}

bool CRUIBookmarksWidget::onLongClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    return true;
}

/// override to handle menu or other action
bool CRUIBookmarksWidget::onAction(const CRUIAction * action) {
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    case CMD_BOOKMARK_GOTO:
        if (_selectedItem)
            _readWidget->goToPosition(Utf8ToUnicode(_selectedItem->startPos.c_str()));
        return onAction(CMD_BACK);
    case CMD_BOOKMARK_REMOVE:
        if (_selectedItem)
            _readWidget->removeBookmark(_selectedItem->id);
        return onAction(CMD_BACK);
    default:
        break;
    }
    return false;
}


static void addTocItems(LVPtrVector<LVTocItem, false> & toc, LVTocItem * item) {
    if (item->getParent())
        toc.add(item);
    for (int i = 0; i < item->getChildCount(); i++)
        addTocItems(toc, item->getChild(i));
}

CRUITOCWidget::CRUITOCWidget(CRUIMainWidget * main, CRUIReadWidget * read) : CRUIWindowWidget(main), _readWidget(read) {
    _title = new CRUITitleBarWidget(lString16(), this, this, false);
    _title->setTitle(STR_READER_TOC);
    _body->addChild(_title);
    _list = new CRUIListWidget(true, this);
    _list->setOnItemClickListener(this);
    _list->setStyle("SETTINGS_ITEM_LIST");
    _body->addChild(_list);
    addTocItems(_toc, read->getDocView()->getToc());
    _itemWidget = new CRUIHorizontalLayout();
    _itemWidget->setMinHeight(MIN_ITEM_PX * 2 / 3);
    _itemWidget->setPadding(PT_TO_PX(2));
    _chapter = new CRUITextWidget();
    _page = new CRUITextWidget();
    _itemWidget->addChild(_chapter);
    _itemWidget->addChild(_page);
    _page->setAlign(ALIGN_RIGHT|ALIGN_VCENTER);
    _chapter->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
    _chapter->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _itemWidget->setStyle("LIST_ITEM");
}

int CRUITOCWidget::getItemCount(CRUIListWidget * list) {
    CR_UNUSED(list);
    return _toc.length();
}

CRUIWidget * CRUITOCWidget::getItemWidget(CRUIListWidget * list, int index) {
    CR_UNUSED(list);
    LVTocItem * item = _toc[index];
    _chapter->setText(item->getName());
    _page->setText(formatPercent(item->getPercent()));
    lvRect padding;
    padding.left = (item->getLevel() - 1) * MIN_ITEM_PX / 3;
    _chapter->setPadding(padding);
    return _itemWidget;
}

// list item click
bool CRUITOCWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    LVTocItem * item = _toc[itemIndex];
    _readWidget->goToPosition(item->getPath());
    onAction(CMD_BACK);
    return true;
}

bool CRUITOCWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    return true;
}

bool CRUITOCWidget::onLongClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    return true;
}

/// handle menu or other action
bool CRUITOCWidget::onAction(const CRUIAction * action) {
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    default:
        break;
    }
    return false;
}

