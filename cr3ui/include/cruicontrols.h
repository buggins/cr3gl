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
	int _maxLines;
public:
	virtual CRUIWidget * setMaxLines(int maxLines) { _maxLines = maxLines; requestLayout(); return this; }
	virtual CRUIWidget * setText(lString16 text) { _text = text; requestLayout(); return this; }

	CRUITextWidget(lString16 text, int maxLines = 1) : _text(text), _maxLines(maxLines) {}
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
public:
	CRUIImageWidget(CRUIImageRef image) : _image(image) { }
	virtual void setImage(CRUIImageRef image) { _image = image; requestLayout(); }
	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

class CRUIButton : public CRUILinearLayout {
protected:
	CRUIImageWidget * _icon;
	CRUITextWidget * _label;
	void init(lString16 text, CRUIImageRef image, bool vertical);
public:
	/// motion event handler, returns true if it handled event
	virtual bool onTouchEvent(const CRUIMotionEvent * event);
	CRUIButton(lString16 text, CRUIImageRef image = CRUIImageRef(), bool vertical = false);
	CRUIButton(lString16 text, const char * imageRes, bool vertical = false);
};




#endif /* CRUICONTROLS_H_ */
