/*
 * cruicontrols.cpp
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#include "cruicontrols.h"

using namespace CRUI;

//=================================================================================

/// measure dimensions
void CRUITextWidget::measure(int baseWidth, int baseHeight) {
	int width = getFont()->getTextWidth(_text.c_str(), _text.length());
	int height = getFont()->getHeight();
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
	CRUIWidget::draw(buf);
	LVDrawStateSaver saver(*buf);
	lvRect rc = _pos;
	applyMargin(rc);
	setClipRect(buf, rc);
	applyPadding(rc);
	buf->SetTextColor(getTextColor());
	int width = getFont()->getTextWidth(_text.c_str(), _text.length());
	int height = getFont()->getHeight();
	applyAlign(rc, width, height);
	getFont()->DrawTextString(buf, rc.left, rc.top,
            _text.c_str(), _text.length(),
            '?');
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





