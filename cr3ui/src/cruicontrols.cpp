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
        lString16 s = setEllipsis(text, mode, count, count > 0 ? ellipsis : lString16::empty_str);
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
        if (text.length() - p >= s2.length() / 3) {
            line1 = text.substr(0, p + 1);
            line2 = text.substr(p + 1);
            line1 = applyEllipsis(line1, maxWidth, ELLIPSIS_LEFT, ellipsis);
        } else {
            line1 = s1;
            line2 = text.substr(s1.length());
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_RIGHT, ellipsis);
        }
    } else if (_ellipsisMode == ELLIPSIS_MIDDLE) {
        int p = findBestSplitPosition(text, s1.length() - 1, -1);
        if (p > s1.length() / 3) {
            line1 = text.substr(0, p + 1);
            line2 = text.substr(p + 1);
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_LEFT, ellipsis);
        } else {
            line1 = s1;
            line2 = text.substr(s1.length());
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_LEFT, ellipsis);
        }
    } else {
        // ELLIPSIS_RIGHT
        int p = findBestSplitPosition(text, s1.length() - 1, -1);
        if (p > s1.length() / 3) {
            line1 = text.substr(0, p + 1);
            line2 = text.substr(p + 1);
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_RIGHT, ellipsis);
        } else {
            line1 = s1;
            line2 = text.substr(s1.length());
            line2 = applyEllipsis(line2, maxWidth, ELLIPSIS_RIGHT, ellipsis);
        }
    }
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
    lString16 text = getText();
    lString16 line1, line2;
    int width, height;
    layoutText(text, baseWidth, line1, line2, width, height);
	defMeasure(baseWidth, baseHeight, width, height);
}

/// updates widget position based on specified rectangle
void CRUITextWidget::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
	_layoutRequested = false;
}

/// draws widget with its children to specified surface
void CRUITextWidget::draw(LVDrawBuf * buf) {
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
        int w1 = getFont()->getTextWidth(line1.c_str(), line1.length());
        int w2 = getFont()->getTextWidth(line2.c_str(), line2.length());
        lvRect rc1 = rc;
        rc1.bottom = rc1.bottom - h;
        applyAlign(rc1, w1, h);
        lvRect rc2 = rc;
        rc2.top = rc2.top + h;
        applyAlign(rc2, w2, h);
        getFont()->DrawTextString(buf, rc1.left, rc1.top,
                line1.c_str(), line1.length(),
                '?');
        getFont()->DrawTextString(buf, rc2.left, rc2.top,
                line2.c_str(), line2.length(),
                '?');
    }
}



//=============================================================================
// Image Widget
/// measure dimensions
void CRUIImageWidget::measure(int baseWidth, int baseHeight) {
	int width = !_image ? 0 : _image->originalWidth();
	int height = !_image ? 0 : _image->originalHeight();
	defMeasure(baseWidth, baseHeight, width, height);
}

/// updates widget position based on specified rectangle
void CRUIImageWidget::layout(int left, int top, int right, int bottom) {
	_pos.left = left;
	_pos.top = top;
	_pos.right = right;
	_pos.bottom = bottom;
}
/// draws widget with its children to specified surface
void CRUIImageWidget::draw(LVDrawBuf * buf) {
	CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	if (!_image.isNull()) {
		//CRLog::trace("rc=%d,%d %dx%d align=%d w=%d h=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign(), _image->originalWidth(), _image->originalHeight());
		applyAlign(rc, _image->originalWidth(), _image->originalHeight());
		// don't scale
		rc.right = rc.left + _image->originalWidth();
		rc.bottom = rc.top + _image->originalHeight();
		//CRLog::trace("aligned %d,%d %dx%d align=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign());
		// draw
		_image->draw(buf, rc);
	}
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
    LVDrawStateSaver saver(*buf);
    lvRect rc = _pos;
    applyMargin(rc);
    setClipRect(buf, rc);
    applyPadding(rc);
    if (!_image.isNull()) {
        //CRLog::trace("rc=%d,%d %dx%d align=%d w=%d h=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign(), _image->originalWidth(), _image->originalHeight());
        applyAlign(rc, _image->originalWidth(), _image->originalHeight());
        // don't scale
        rc.right = rc.left + _image->originalWidth();
        rc.bottom = rc.top + _image->originalHeight();
        //CRLog::trace("aligned %d,%d %dx%d align=%d", rc.left, rc.top, rc.width(), rc.height(), getAlign());
        // draw
        _image->drawRotated(buf, rc, 360 - _angle / 1000);
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

CRUIButton::CRUIButton(lString16 text, const char * imageRes, bool vertical) : CRUILinearLayout(vertical), _icon(NULL), _label(NULL) {
	CRUIImageRef image;
	if (imageRes && imageRes[0])
		image = resourceResolver->getIcon(imageRes);
	init(text, image, vertical);
}

void CRUIButton::init(lString16 text, CRUIImageRef image, bool vertical) {
	_styleId = "BUTTON";
	if (!image.isNull()) {
		_icon = new CRUIImageWidget(image);
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
		if (image.isNull()) {
			_label->setAlign(ALIGN_CENTER);
			//_label->setLayoutParams(FILL_PARENT, FILL_PARENT);
		} else if (vertical)
			_label->setAlign(ALIGN_TOP | ALIGN_HCENTER);
		else
			_label->setAlign(ALIGN_LEFT | ALIGN_VCENTER);
		if (!image.isNull()) {
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

CRUIButton::CRUIButton(lString16 text, CRUIImageRef image, bool vertical)
: CRUILinearLayout(vertical), _icon(NULL), _label(NULL)
{
	init(text, image, vertical);
}

/// motion event handler, returns true if it handled event
bool CRUIButton::onTouchEvent(const CRUIMotionEvent * event) {
	int action = event->getAction();
	CRLog::trace("CRUIButton::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
	switch (action) {
	case ACTION_DOWN:
		setState(STATE_PRESSED, STATE_PRESSED);
		//CRLog::trace("button DOWN");
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





