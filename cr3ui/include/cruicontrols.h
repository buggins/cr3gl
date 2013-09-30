/*
 * cruicontrols.h
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#ifndef CRUICONTROLS_H_
#define CRUICONTROLS_H_

#include "cruilayout.h"

class CRUITextWidget : public CRUIWidget {
protected:
	lString16 _text;
	lString8 _textResourceId;
	int _maxLines;
    int _ellipsisMode;
    void layoutText(lString16 text, int maxWidth, lString16 & line1, lString16 & line2, int & width, int & height);
    lString16 applyEllipsis(lString16 text, int maxWidth, int mode, const lString16 & ellipsis);
public:
    virtual int getEllipsisMode() { return _ellipsisMode; }
    virtual int getMaxLines() { return _maxLines; }
    virtual CRUITextWidget * setEllipsisMode(int mode) { _ellipsisMode = mode; requestLayout(); return this; }
    virtual CRUITextWidget * setMaxLines(int maxLines) { _maxLines = maxLines; requestLayout(); return this; }
	virtual CRUIWidget * setText(lString16 text) { _text = text; requestLayout(); return this; }
	virtual CRUIWidget * setText(const wchar_t * text) { _text = lString16(text); requestLayout(); return this; }
	virtual CRUIWidget * setText(lString8 textResourceId) { _textResourceId = textResourceId; requestLayout(); return this; }
	virtual CRUIWidget * setText(const char * textResourceId) { _textResourceId = lString8(textResourceId); requestLayout(); return this; }
	virtual lString16 getText();

    CRUITextWidget(lString16 text, int maxLines = 1) : _text(text), _maxLines(maxLines), _ellipsisMode(CRUI::ELLIPSIS_RIGHT) {}
    CRUITextWidget(const char * textResourceId, int maxLines = 1) : _textResourceId(textResourceId), _maxLines(maxLines), _ellipsisMode(CRUI::ELLIPSIS_RIGHT) {}
    CRUITextWidget() : _maxLines(1), _ellipsisMode(CRUI::ELLIPSIS_RIGHT) {}
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

class CRUIImageWidget : public CRUIWidget {
protected:
    CRUIImageRef _image;
    lString8 _imageRes;
public:
    CRUIImageWidget(const char * imageRes = NULL) : _imageRes(imageRes) { }
    virtual void setImage(const char * imageRes) { _image.Clear(); _imageRes = imageRes; requestLayout(); }
    virtual void setImage(const lString8 & imageRes) { _image.Clear(); _imageRes = imageRes; requestLayout(); }
    virtual void setImage(CRUIImageRef img) { _image = img; _imageRes.clear(); requestLayout(); }
    CRUIImageRef getImage() { return _image.isNull() ? (!_imageRes.empty() ? resourceResolver->getIcon(_imageRes.c_str()) : CRUIImageRef()) : _image; }
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

class CRUISpinnerWidget : public CRUIImageWidget {
protected:
    int _angle;
    int _speed;
public:
    CRUISpinnerWidget(const char * imageRes, int speed = 180) : CRUIImageWidget(imageRes), _angle(0), _speed(speed) { }
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    virtual void animate(lUInt64 millisPassed);
    virtual bool isAnimating();
};

class CRUIButton : public CRUILinearLayout {
protected:
	CRUIImageWidget * _icon;
	CRUITextWidget * _label;
    void init(lString16 text, const char * imageRes, bool vertical);
public:
	/// motion event handler, returns true if it handled event
	virtual bool onTouchEvent(const CRUIMotionEvent * event);
    //CRUIButton(lString16 text, CRUIImageRef image = CRUIImageRef(), bool vertical = false);
	CRUIButton(lString16 text, const char * imageRes, bool vertical = false);
};

class CRUIImageButton : public CRUIButton {
protected:
public:
	CRUIImageButton(const char * imageResource, const char * styleName = "BUTTON_NOBACKGROUND");
    void onThemeChanged();
};

class CRUISliderWidget : public CRUIWidget {
protected:
    CRUIImageRef sliderResource;
    CRUIImageRef lineResource;
    int _minValue;
    int _maxValue;
    int _value;
    CRUIOnScrollPosCallback * _callback;
    void updatePos(int pos);
public:
    int getScrollPos() { return _value; }
    void setScrollPos(int value) { _value = value < _minValue ? _minValue : (value > _maxValue ? _maxValue : value); }
    int getMinScrollPos() { return _minValue; }
    int getMaxScrollPos() { return _maxValue; }
    void setMinScrollPos(int value) { _minValue = value; }
    void setMaxScrollPos(int value) { _maxValue = value; }
    void setScrollPosCallback(CRUIOnScrollPosCallback * callback) { _callback = callback; }
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);

    CRUISliderWidget(int minValue, int maxValue, int currentValue) :  _minValue(minValue), _maxValue(maxValue), _value(currentValue), _callback(NULL) {
        setScrollPos(currentValue);
    }
    virtual ~CRUISliderWidget() {}
};

#endif /* CRUICONTROLS_H_ */
