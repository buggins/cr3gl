/*
 * cruicontrols.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruicontrols.h"
#include "crui.h"
#include "gldrawbuf.h"

using namespace CRUI;

//=================================================================================

lString16 CRUITextWidget::getText() {
	if (!_text.empty())
		return _text;
	if (!_textResourceId.empty())
		return _16(_textResourceId.c_str());
	return _text;
}

static lString16 setEllipsis(const lString16 & text, int mode, int count, const lString16 & ellipsis) {
    if (count == 0)
        return text;
    if (count >= text.length())
        return lString16("");
//    if (ellipsis.empty())
//        return text;
    if (mode == ELLIPSIS_LEFT) {
        return ellipsis + text.substr(count);
    } else if (mode == ELLIPSIS_MIDDLE) {
        int p = (text.length() - count - 1) / 2;
        if (p < 0)
            p = 0;
        return text.substr(0, p) + ellipsis + text.substr(p + count);
    } else {
        // ELLIPSIS_RIGHT
        return text.substr(0, text.length() - count) + ellipsis;
    }
}

lString16 CRUITextWidget::applyEllipsis(lString16 text, int maxWidth, int mode, const lString16 & ellipsis) {
    LVFontRef font = getFont();
    for (int count = 0; count < text.length(); count++) {
        lString16 s = count > 0 ? setEllipsis(text, mode, count, count > 0 ? ellipsis : lString16::empty_str) : text;
        int w = font->getTextWidth(s.c_str(), s.length());
        if (w <= maxWidth)
            return s;
    }
    return lString16();
}

static int findBestSplitPosition(const lString16 & text, int startPos, int dir) {
    for (int i = startPos; i >= 0 && i < text.length(); i += dir) {
        lChar16 ch = text[i];
        if (ch == ' ' || ch == '/' || ch == '\\' || ch == '.' || ch == ',')
            return i;
    }
    return -1; // not found
}

void CRUITextWidget::layoutText(lString16 text, int maxWidth, lString16 & line1, lString16 & line2, int & width, int & height) {
    lvRect pad = getPadding();
    lvRect margin = getMargin();
    maxWidth -= pad.left + pad.right + margin.left + margin.right;
    width = getFont()->getTextWidth(text.c_str(), text.length());
    height = getFont()->getHeight();
    if (width <= maxWidth) {
        line1 = text;
        return;
    }
    lString16 ellipsis("...");
    if (_maxLines <= 1) {
        line1 = applyEllipsis(text, maxWidth, _ellipsisMode, ellipsis);
        width = getFont()->getTextWidth(line1.c_str(), line1.length());
        return;
    }
    lString16 s1 = applyEllipsis(text, maxWidth, ELLIPSIS_RIGHT, lString16::empty_str);
    lString16 s2 = applyEllipsis(text, maxWidth, ELLIPSIS_LEFT, lString16::empty_str);
    height = height * 2;
    if (_ellipsisMode == ELLIPSIS_LEFT) {
        int p = findBestSplitPosition(text, text.length() - s2.length(), 1);
        if (text.length() - p >= s2.length() / 7) {
            line1 = text.substr(0, p + 1).trim();
            line2 = text.substr(p + 1).trim();
            line1 = applyEllipsis(line1, maxWidth, ELLIPSIS_LEFT, ellipsis);
        } else {
            line1 = s1.trim();
            line2 = text.substr(s1.length()).trim();
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_RIGHT, ellipsis);
        }
    } else if (_ellipsisMode == ELLIPSIS_MIDDLE) {
        int p = findBestSplitPosition(text, s1.length() - 1, -1);
        if (p > s1.length() / 7) {
            line1 = text.substr(0, p + 1).trim();
            line2 = text.substr(p + 1).trim();
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_LEFT, ellipsis);
        } else {
            line1 = s1.trim();
            line2 = text.substr(s1.length()).trim();
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_LEFT, ellipsis);
        }
    } else {
        // ELLIPSIS_RIGHT
        int p = findBestSplitPosition(text, s1.length() - 1, -1);
        if (p > s1.length() / 7) {
            line1 = text.substr(0, p + 1).trim();
            line2 = text.substr(p + 1).trim();
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_RIGHT, ellipsis);
        } else {
            line1 = s1.trim();
            line2 = text.substr(s1.length()).trim();
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_RIGHT, ellipsis);
        }
    }
    //CRLog::trace("Split: %d [%s] [%s]", height, LCSTR(line1), LCSTR(line2));
    // calc width
    int w1 = getFont()->getTextWidth(line1.c_str(), line1.length());
    int w2 = getFont()->getTextWidth(line2.c_str(), line2.length());
    if (w1 > w2)
        width = w1;
    else
        width = w2;
}

/// measure dimensions
void CRUITextWidget::measure(int baseWidth, int baseHeight) {
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
    lString16 text = getText();
    lString16 line1, line2;
    int width, height;
    layoutText(text, baseWidth, line1, line2, width, height);
	defMeasure(baseWidth, baseHeight, width, height);
    //CRLog::trace("Measure: %d %d [%s]", _measuredWidth, _measuredHeight, LCSTR(text));
}

/// updates widget position based on specified rectangle
void CRUITextWidget::layout(int left, int top, int right, int bottom) {
    CRUIWidget::layout(left, top, right, bottom);
    //lString16 text = getText();
    //CRLog::trace("Layout: %d %d [%s]", right - left, bottom - top, LCSTR(text));
}

/// draws widget with its children to specified surface
void CRUITextWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    lString16 text = getText();
    lString16 line1, line2;
    int width, height;
    layoutText(text, _pos.width(), line1, line2, width, height);
    CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	buf->SetTextColor(getTextColor());
	//CRLog::trace("rc=%d,%d %dx%d align=%d w=%d h=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign(), width, height);
    if (line2.empty()) {
        // single line
        applyAlign(rc, width, height);
        getFont()->DrawTextString(buf, rc.left, rc.top,
                line1.c_str(), line1.length(),
                '?');
    } else {
        // two lines
        int h = getFont()->getHeight();
        int fh = line2.empty() ? h : h * 2;
        int w1 = getFont()->getTextWidth(line1.c_str(), line1.length());
        int w2 = getFont()->getTextWidth(line2.c_str(), line2.length());
        lvRect rc1 = rc;
        //rc1.bottom = rc1.bottom - h;
        applyAlign(rc1, w1, fh);
        lvRect rc2 = rc;
        applyAlign(rc2, w2, fh);
        rc2.top = rc2.top + h;
        getFont()->DrawTextString(buf, rc1.left, rc1.top,
                line1.c_str(), line1.length(),
                '?');
        getFont()->DrawTextString(buf, rc2.left, rc2.top,
                line2.c_str(), line2.length(),
                '?');
        //CRLog::trace("Draw: %d [%s] [%s]", rc.height(), LCSTR(line1), LCSTR(line2));
    }
}



//=============================================================================
// Image Widget
/// measure dimensions
void CRUIImageWidget::measure(int baseWidth, int baseHeight) {
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
    CRUIImageRef image = getImage();
    int width = !image ? 0 : image->originalWidth();
    int height = !image ? 0 : image->originalHeight();
	defMeasure(baseWidth, baseHeight, width, height);
}

/// updates widget position based on specified rectangle
void CRUIImageWidget::layout(int left, int top, int right, int bottom) {
    CRUIWidget::layout(left, top, right, bottom);
}
/// draws widget with its children to specified surface
void CRUIImageWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
    if (getBackgroundAlpha())
        buf->setAlpha(getBackgroundAlpha());
    CRUIImageRef image = getImage();
    int width = !image ? 0 : image->originalWidth();
    int height = !image ? 0 : image->originalHeight();
    if (!image.isNull()) {
		//CRLog::trace("rc=%d,%d %dx%d align=%d w=%d h=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign(), _image->originalWidth(), _image->originalHeight());
        applyAlign(rc, width, height);
		// don't scale
        rc.right = rc.left + width;
        rc.bottom = rc.top + height;
		//CRLog::trace("aligned %d,%d %dx%d align=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign());
		// draw
        image->draw(buf, rc);
	}
}



/// measure dimensions
void CRUIProgressWidget::measure(int baseWidth, int baseHeight) {
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
    int height = MIN_ITEM_PX / 6;
    if (height < 6)
        height = 6;
    int width = height;
    defMeasure(baseWidth, baseHeight, width, height);
}

/// updates widget position based on specified rectangle
void CRUIProgressWidget::layout(int left, int top, int right, int bottom) {
    CRUIWidget::layout(left, top, right, bottom);
}

/// draws widget with its children to specified surface
void CRUIProgressWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    if (_progress < 0)
        return;
    CRUIWidget::draw(buf);
    LVDrawStateSaver saver(*buf);
    lvRect rc = _pos;
    applyMargin(rc);
    setClipRect(buf, rc);
    applyPadding(rc);
    if (getBackgroundAlpha())
        buf->setAlpha(getBackgroundAlpha());
    buf->Rect(rc, 0x40808080);
    rc.shrink(2);
    rc.right = rc.left + rc.width() * _progress / 10000;
    buf->FillRect(rc, 0x204040FF);
}




void CRUISpinnerWidget::animate(lUInt64 millisPassed) {
    _angle += millisPassed * _speed;
    _angle = _angle % 360000;
}

bool CRUISpinnerWidget::isAnimating() {
    return true;
}

/// draws widget with its children to specified surface
void CRUISpinnerWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    LVDrawStateSaver saver(*buf);
    lvRect rc = _pos;
    applyMargin(rc);
    setClipRect(buf, rc);
    applyPadding(rc);
    CRUIImageRef image = getImage();
    if (!image.isNull()) {
        //CRLog::trace("rc=%d,%d %dx%d align=%d w=%d h=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign(), _image->originalWidth(), _image->originalHeight());
        applyAlign(rc, image->originalWidth(), image->originalHeight());
        // don't scale
        rc.right = rc.left + image->originalWidth();
        rc.bottom = rc.top + image->originalHeight();
        //CRLog::trace("aligned %d,%d %dx%d align=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign());
        // draw
        image->drawRotated(buf, rc, 360 - _angle / 1000);
        //_image->draw(buf, rc);
    }
}





CRUIImageButton::CRUIImageButton(const char * imageResource, const char * styleName) : CRUIButton(lString16::empty_str, imageResource, true)
{
	if (styleName)
		setStyle(styleName);
	setMinWidth(deviceInfo.minListItemSize);
	setMinHeight(deviceInfo.minListItemSize);
	setAlign(CRUI::ALIGN_CENTER);
}

void CRUIImageButton::onThemeChanged() {
    setMinWidth(deviceInfo.minListItemSize);
    setMinHeight(deviceInfo.minListItemSize);
}

CRUIButton::CRUIButton(lString16 text, const char * imageRes, bool vertical) : CRUILinearLayout(vertical), _icon(NULL), _label(NULL) {
    init(text, imageRes, vertical);
}

void CRUIButton::init(lString16 text, const char * imageRes, bool vertical) {
	_styleId = "BUTTON";
    bool hasImage = imageRes && imageRes[0];
    if (hasImage) {
        _icon = new CRUIImageWidget(imageRes);
        _icon->setId("BUTTON_ICON");
		if (text.empty()) {
			_icon->setAlign(ALIGN_CENTER);
			//_icon->setLayoutParams(FILL_PARENT, FILL_PARENT);
		} else if (vertical)
			_icon->setAlign(ALIGN_HCENTER | ALIGN_TOP);
		else
			_icon->setAlign(ALIGN_LEFT | ALIGN_VCENTER);
		addChild(_icon);
	}
	if (!text.empty()) {
		_label = new CRUITextWidget(text);
        _label->setId("BUTTON_CAPTION");
        if (!hasImage) {
			_label->setAlign(ALIGN_CENTER);
			//_label->setLayoutParams(FILL_PARENT, FILL_PARENT);
		} else if (vertical)
			_label->setAlign(ALIGN_TOP | ALIGN_HCENTER);
		else
			_label->setAlign(ALIGN_LEFT | ALIGN_VCENTER);
        if (hasImage) {
			lvRect padding;
			getPadding(padding);
			lvRect lblPadding;
			_label->getPadding(lblPadding);
			if (vertical) {
				if (!lblPadding.top)
					lblPadding.top = padding.top * 2 / 3;
			} else {
				if (!lblPadding.left)
					lblPadding.left = padding.left * 2 / 3;
			}
			_label->setPadding(lblPadding);
		}
		addChild(_label);
	}
}

//CRUIButton::CRUIButton(lString16 text, CRUIImageRef image, bool vertical)
//: CRUILinearLayout(vertical), _icon(NULL), _label(NULL)
//{
//	init(text, image, vertical);
//}

/// motion event handler, returns true if it handled event
bool CRUIButton::onTouchEvent(const CRUIMotionEvent * event) {
    if (getState(STATE_DISABLED))
        return false;
    int action = event->getAction();
    //CRLog::trace("CRUIButton::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
	switch (action) {
	case ACTION_DOWN:
		setState(STATE_PRESSED, STATE_PRESSED);
		//CRLog::trace("button DOWN");
        onClickDownEvent();
        break;
	case ACTION_UP:
		{
            setState(0, STATE_PRESSED);
            bool isLong = event->getDownDuration() > 500; // 0.5 seconds threshold
            if (isLong && onLongClickEvent())
                return true;
            onClickEvent();
		}
		// fire onclick
		//CRLog::trace("button UP");
		break;
	case ACTION_FOCUS_IN:
        setState(STATE_PRESSED, STATE_PRESSED);
		//CRLog::trace("button FOCUS IN");
		break;
	case ACTION_FOCUS_OUT:
		setState(0, STATE_PRESSED);
		//CRLog::trace("button FOCUS OUT");
		break;
	case ACTION_CANCEL:
		setState(0, STATE_PRESSED);
		//CRLog::trace("button CANCEL");
		break;
	case ACTION_MOVE:
		// ignore
		//CRLog::trace("button MOVE");
		break;
	default:
		return CRUIWidget::onTouchEvent(event);
	}
	return true;
}





/// measure dimensions
void CRUISliderWidget::measure(int baseWidth, int baseHeight) {
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
    int width = baseWidth * 9 / 10;
    int height = PT_TO_PX(30);
    defMeasure(baseWidth, baseHeight, width, height);
}

/// updates widget position based on specified rectangle
void CRUISliderWidget::layout(int left, int top, int right, int bottom) {
    CRUIWidget::layout(left, top, right, bottom);
}

/// draws widget with its children to specified surface
void CRUISliderWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    CRUIWidget::draw(buf);
    LVDrawStateSaver saver(*buf);
    CR_UNUSED(saver);
    lvRect rc = _pos;
    applyMargin(rc);
    setClipRect(buf, rc);
    applyPadding(rc);

    //CRUIImageRef handle = resourceResolver->getIcon("btn_radio_off");
    CRUIImageRef handle = resourceResolver->getIcon("00_slider_handle");
    CRUIImageRef line = resourceResolver->getIcon("home_frame.9");


    int cx = handle.isNull() ? MIN_ITEM_PX / 3 : handle->originalWidth();
    int cy = handle.isNull() ? MIN_ITEM_PX * 2 / 3 : handle->originalHeight();
    int lh = MIN_ITEM_PX / 6;
    int y0 = (rc.top + rc.bottom) / 2;
    lvRect linerc = rc;
    linerc.left += cx / 2;
    linerc.right -= cx / 2;
    linerc.top = y0 - lh/2;
    linerc.bottom = linerc.top + lh;
    bool hasGradient = _color1 != 0xFFFFFFFF || _color2 != 0xFFFFFFFF;
    if (line.isNull()) {
        buf->FillRect(linerc, currentTheme->getColor(COLOR_ID_SLIDER_LINE_COLOR_OUTER));
        linerc.shrink(lh / 4);
        buf->FillRect(linerc, currentTheme->getColor(COLOR_ID_SLIDER_LINE_COLOR_INNER));
    } else {
        if (hasGradient) {
            lvRect rc = linerc;
//            const CR9PatchInfo * ninePatch = line->getNinePatchInfo();
//            if (ninePatch) {
//                rc.shrinkBy(ninePatch->padding);
//            } else {
//                rc.shrink(lh / 4);
//            }
            buf->GradientRect(rc.left, rc.top, rc.right, rc.bottom, _color1, _color2, _color2, _color1);
        }
        line->draw(buf, linerc);
    }

    int x0 = linerc.left + linerc.width() * (_value - _minValue) / (_maxValue - _minValue);
    lvRect crc(x0 - cx / 2, y0 - cy / 2, x0 + cx / 2, y0 + cy / 2);
    if (handle.isNull()) {
        buf->FillRect(crc, currentTheme->getColor(COLOR_ID_SLIDER_POINTER_COLOR_OUTER));
        crc.shrink(lh / 3);
        buf->FillRect(crc, currentTheme->getColor(COLOR_ID_SLIDER_POINTER_COLOR_INNER));
    } else {
        handle->draw(buf, crc);
    }
}

void CRUISliderWidget::updatePos(int pos) {
    int oldpos = _value;
    setScrollPos(pos);
    if (_value != oldpos) {
        if (_callback)
            _callback->onScrollPosChange(this, _value, true);
        invalidate();
    }
}

/// motion event handler, returns true if it handled event
bool CRUISliderWidget::onTouchEvent(const CRUIMotionEvent * event) {
    int action = event->getAction();

    lvRect rc = _pos;
    applyMargin(rc);
    applyPadding(rc);
    int cx = PT_TO_PX(9);
    rc.left += cx/2;
    rc.right -= cx/2;
    int x = event->getX();
    //int y = event->getY();
    if (x < rc.left)
        x = rc.left;
    if (x > rc.right)
        x = rc.right;
    int pos = _minValue + ((x - rc.left) * (_maxValue - _minValue) + rc.width() / 2) / rc.width();
    switch (action) {
    case ACTION_DOWN:
        updatePos(pos);
        break;
    case ACTION_UP:
        {
            updatePos(pos);
        }
        // fire onclick
        //CRLog::trace("list UP");
        break;
    case ACTION_FOCUS_IN:
        updatePos(pos);
        //CRLog::trace("list FOCUS IN");
        break;
    case ACTION_FOCUS_OUT:
        return false; // to continue tracking
        //CRLog::trace("list FOCUS OUT");
        break;
    case ACTION_CANCEL:
        break;
    case ACTION_MOVE:
        updatePos(pos);
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
    }
    return true;
}



CRUIEditWidget::~CRUIEditWidget() {
    //CRLog::trace("~CRUIEditWidget()");
    if (CRUIEventManager::getFocusedWidget() == this) {
        CRUIEventManager::dispatchFocusChange(NULL);
        if (CRUIEventManager::isVirtualKeyboardShown())
            CRUIEventManager::hideVirtualKeyboard();
    }
}

CRUIEditWidget::CRUIEditWidget() : _cursorPos(0), _scrollx(0), _lastEnteredCharPos(-1), _scrollDirection(0), _passwordChar(0), _onReturnPressedListener(NULL) {
    //_text = "Editor test sample";
    //_cursorPos = 3;
    setStyle("EDITBOX");
}

lString16 CRUIEditWidget::getTextToShow() {
    if (!_passwordChar)
        return _text;
    lString16 res;
    res.reserve(_text.length());
    for (int i = 0; i < _text.length(); i++) {
        if (i + 1 == _lastEnteredCharPos)
            res << _text[i];
        else
            res << _passwordChar;
    }
    return res;
}

#define EDIT_WIDGET_HIDE_PASSWORD_TIMER_ID 123013
#define EDIT_WIDGET_SCROLL_TIMER_ID 123014

bool CRUIEditWidget::onTimerEvent(lUInt32 timerId) {
    if (timerId == EDIT_WIDGET_HIDE_PASSWORD_TIMER_ID) {
        if (_lastEnteredCharPos >=0 ) {
            _lastEnteredCharPos = -1;
            updateCursor(_cursorPos, false);
            invalidate();
            CRUIEventManager::requestScreenUpdate(false);
        }
    } else if (timerId == EDIT_WIDGET_SCROLL_TIMER_ID) {
        scrollByTimer();
        return true;
    }
    return false;
}

void CRUIEditWidget::setScrollTimer(int direction) {
    _scrollDirection = direction;
    CRUIEventManager::setTimer(EDIT_WIDGET_SCROLL_TIMER_ID, this, 500, true);
}

void CRUIEditWidget::cancelScrollTimer() {
    if (!_scrollDirection)
        return;
    _scrollDirection = 0;
    CRUIEventManager::cancelTimer(EDIT_WIDGET_SCROLL_TIMER_ID);
}

void CRUIEditWidget::scrollByTimer() {
    if (!_scrollDirection)
        return;
    int prevpos = _cursorPos;
    updateCursor(_cursorPos, true, true);
    if (_cursorPos != prevpos) {
        invalidate();
        CRUIEventManager::requestScreenUpdate();
    } else {
        cancelScrollTimer();
    }
}

CRUIWidget * CRUIEditWidget::setText(lString16 txt) {
    _text = txt;
    _lastEnteredCharPos = -1;
    updateCursor(_text.length());
    invalidate();
    return this;
}

void CRUIEditWidget::setPasswordChar(lChar16 ch) {
    _passwordChar = ch;
    updateCursor(_cursorPos);
    invalidate();
}

/// measure dimensions
void CRUIEditWidget::measure(int baseWidth, int baseHeight) {
    if (getVisibility() == GONE) {
        _measuredWidth = 0;
        _measuredHeight = 0;
        return;
    }
    LVFontRef font = getFont();
    int width = font->getTextWidth(_text.c_str(), _text.length());
    int height = font->getHeight();
    defMeasure(baseWidth, baseHeight, width, height);
}

/// updates widget position based on specified rectangle
void CRUIEditWidget::layout(int left, int top, int right, int bottom) {
    CRUIWidget::layout(left, top, right, bottom);
    updateCursor(_cursorPos, false);
}

struct MeasuredText {
    lString16 _text;
    LVArray<lUInt16> _widths;
    LVArray<lUInt8> _flags;
    int _width;
    MeasuredText(LVFontRef font, lString16 text) : _text(text), _width(0) {
        if (_text.length()) {
            _widths.addSpace(_text.length() + 5);
            _flags.addSpace(_text.length() + 5);
            font->measureText(_text.c_str(), _text.length(), _widths.get(), _flags.get(), 10000, '?', 0, false);
            _width = _widths[_text.length() - 1];
        }
    }
    int getWidth() {
        return _width;
    }

    int getOffset(int index) {
        if (index <= 0)
            return 0;
        if (index > _text.length())
            return _widths[_text.length() - 1];
        return _widths[index - 1];
    }

    int offsetToIndex(int offset) {
        int prevx = 0;
        for (int i = 0; i < _text.length(); i++) {
            int x = _widths[i];
            if (offset < (x + prevx) / 2)
                return i;
            prevx = x;
        }
        return _text.length();
    }
};

/// draws widget with its children to specified surface
void CRUIEditWidget::draw(LVDrawBuf * buf) {
    if (getVisibility() != VISIBLE) {
        return;
    }
    CRUIWidget::draw(buf);
    LVDrawStateSaver saver(*buf);
    CR_UNUSED(saver);
    lvRect rc = _pos;
    applyMargin(rc);
    applyPadding(rc);
    rc.left--;
    setClipRect(buf, rc);
    rc.left++;
    LVFontRef font = getFont();
    lString16 text = getTextToShow();
    MeasuredText measured(font, text);
    buf->SetTextColor(getStyle()->getTextColor());
    int yoffset = (rc.height() - font->getHeight()) / 2;
    if (isFocused()) {
        updateCursor(_cursorPos, false);
        int cursorx = measured.getOffset(_cursorPos) - _scrollx;
        buf->FillRect(rc.left + cursorx - 1, rc.top + yoffset - 2, rc.left + cursorx + 1, rc.top + yoffset + font->getHeight() + 2, 0x6060FF);
    }
    font->DrawTextString(buf, rc.left - _scrollx, rc.top + yoffset, text.c_str(), text.length(), '?');
}

/// motion event handler, returns true if it handled event
bool CRUIEditWidget::onTouchEvent(const CRUIMotionEvent * event) {
    int action = event->getAction();
    if (!isFocused()) {
        if (action == ACTION_UP) {
            CRUIEventManager::showVirtualKeyboard(0, getText(), false);
            CRUIEventManager::dispatchFocusChange(this);
        }
        return true;
    }
    if (!CRUIEventManager::isVirtualKeyboardShown()) {
    	if (action == ACTION_UP)
    		CRUIEventManager::showVirtualKeyboard(0, getText(), false);
    	return true;
    }
    lvRect rc = _pos;
    applyMargin(rc);
    applyPadding(rc);
    LVFontRef font = getFont();
    lString16 text = getTextToShow();
    MeasuredText measured(font, text);
    int x = event->getX();
    if (x < rc.left)
        x = rc.left;
    if (x >= rc.right)
        x = rc.right - 1;
    int newcurpos = measured.offsetToIndex(x - rc.left + _scrollx);
    lvPoint pt(event->getX(), event->getY());
    bool inside = _pos.isPointInside(pt);
    switch (action) {
    case ACTION_DOWN:
        if (!CRUIEventManager::isVirtualKeyboardShown())
            CRUIEventManager::showVirtualKeyboard(0, getText(), false);
        if (inside) {
            _lastEnteredCharPos = -1;
            updateCursor(newcurpos, false);
            if (pt.x >= rc.right - rc.width() / 20)
                setScrollTimer(1);
            else if (pt.x < rc.left + rc.width() / 20)
                setScrollTimer(-1);
            else
                cancelScrollTimer();
        }
        break;
    case ACTION_UP:
        if (inside) {
            _lastEnteredCharPos = -1;
            updateCursor(newcurpos, false);
        }
        cancelScrollTimer();
        break;
    case ACTION_MOVE:
        if (inside) {
            if (pt.x >= rc.right - rc.width() / 20) {
                if (!_scrollDirection)
                    setScrollTimer(1);
            } else if (pt.x < rc.left + rc.width() / 20) {
                //
                if (!_scrollDirection)
                    setScrollTimer(-1);
            } else
                cancelScrollTimer();
            _lastEnteredCharPos = -1;
            updateCursor(newcurpos, false);
        }
        break;
    case ACTION_CANCEL:
        _lastEnteredCharPos = -1;
        cancelScrollTimer();
        break;
    default:
        break;
    }
    return true;
}

void CRUIEditWidget::updateCursor(int pos, bool scrollIfNearBounds, bool changeCursorPositionAfterScroll) {
    _cursorPos = pos;
    lString16 text = getTextToShow();
    if (_cursorPos < 0)
        _cursorPos = 0;
    if (_cursorPos > text.length())
        _cursorPos = text.length();
    lvRect rc = _pos;
    applyMargin(rc);
    applyPadding(rc);
    LVFontRef font = getFont();
    MeasuredText measured(font, text);
    if (measured.getWidth() < rc.width()) {
        _scrollx = 0;
        return;
    }
    if (_scrollx > measured.getWidth() - rc.width())
        _scrollx = measured.getWidth() - rc.width();
    if (_scrollx < 0)
        _scrollx = 0;
    int cursoroffset = measured.getOffset(_cursorPos);
    int oldcursorx = cursoroffset - _scrollx;
    int scrollThreshold = scrollIfNearBounds ? rc.width() / 20 : 0;
    bool scrolled = false;
    if (cursoroffset < _scrollx + scrollThreshold) {
        _scrollx = cursoroffset - rc.width() / 10;
        if (_scrollx < 0)
            _scrollx = 0;
        scrolled = true;
    } else if (cursoroffset > _scrollx + rc.width() - scrollThreshold) {
        _scrollx = cursoroffset - rc.width() + rc.width() / 10;
        if (_scrollx > measured.getWidth() - rc.width())
            _scrollx = measured.getWidth() - rc.width();
        scrolled = true;
    }
    if (scrolled && changeCursorPositionAfterScroll && oldcursorx >= 0 && oldcursorx < rc.width()) {
        // change
        _cursorPos = measured.offsetToIndex(_scrollx + oldcursorx);
        if (_cursorPos < 0)
            _cursorPos = 0;
        if (_cursorPos > text.length())
            _cursorPos = text.length();
    }
    invalidate();
}

bool CRUIEditWidget::onFocusChange(bool focused) {
    invalidate();
    cancelScrollTimer();
    if (focused) {
        updateCursor(_text.length());
        CRUIEventManager::showVirtualKeyboard(0, getText(), false);
    } else {
        _scrollx = 0;
        CRUIEventManager::hideVirtualKeyboard();
    }
    return true;
}

/// key event handler, returns true if it handled event
bool CRUIEditWidget::onKeyEvent(const CRUIKeyEvent * event) {
    invalidate();
    cancelScrollTimer();
    if (event->getType() == KEY_ACTION_PRESS) {
        switch(event->key()) {
        case CR_KEY_BACKSPACE:
            if (_cursorPos > 0) {
                _lastEnteredCharPos = -1;
                _text.erase(_cursorPos - 1, 1);
                updateCursor(_cursorPos - 1);
            }
            return true;
        case CR_KEY_LEFT:
            if (_cursorPos > 0) {
                _lastEnteredCharPos = -1;
                updateCursor(_cursorPos - 1);
            }
            return true;
        case CR_KEY_RIGHT:
            if (_cursorPos < _text.length()) {
                _lastEnteredCharPos = -1;
                updateCursor(_cursorPos + 1);
            }
            return true;
        case CR_KEY_HOME:
            _lastEnteredCharPos = -1;
            updateCursor(0);
            return true;
        case CR_KEY_END:
            _lastEnteredCharPos = -1;
            updateCursor(_text.length());
            return true;
        case CR_KEY_RETURN:
            return true;
        default:
            break;
        }
    }
    if (event->getType() == KEY_ACTION_RELEASE) {
        switch(event->key()) {
        case CR_KEY_BACKSPACE:
            return true;
        case CR_KEY_LEFT:
            return true;
        case CR_KEY_RIGHT:
            return true;
        case CR_KEY_HOME:
            return true;
        case CR_KEY_END:
            return true;
        case CR_KEY_RETURN:
            CRUIEventManager::hideVirtualKeyboard();
            if (_onReturnPressedListener)
                return _onReturnPressedListener->onReturnPressed(this);
            return true;
        default:
            break;
        }
    }
    lString16 eventText = event->text();
    if (eventText.length()) {
        if (eventText[0] >= 32) {
            if (event->getType() == KEY_ACTION_PRESS) {
                _text.insert(_cursorPos, eventText);
                _lastEnteredCharPos = _cursorPos + eventText.length();
                updateCursor(_cursorPos + eventText.length());
                if (_passwordChar)
                    CRUIEventManager::setTimer(EDIT_WIDGET_HIDE_PASSWORD_TIMER_ID, this, 500, false);
                invalidate();
            }
            return true;
        }
    }
    return true;
}
