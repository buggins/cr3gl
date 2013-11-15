/*
 * cruiwidget.h
 *
 *  Created on: Aug 15, 2013
 *      Author: vlopatin
 */

#ifndef CRUIWIDGET_H_
#define CRUIWIDGET_H_

#include "cruievent.h"
#include "cri18n.h"

#define LONG_TOUCH_THRESHOLD 500
#define DRAG_THRESHOLD PT_TO_PX(3)
#define DRAG_THRESHOLD_X PT_TO_PX(10)

namespace CRUI {
    enum Visibility {
        VISIBLE,   // Visible on screen; the default value
        INVISIBLE, // Not displayed, but taken into account during layout (space is left for it).
        GONE       // Completely hidden, as if the view had not been added.
    };
}

/// base class for all UI elements
class CRUIWidget {
protected:
	lString8 _id;
	lString8 _styleId;
	lUInt32 _state;
	lvRect _pos;
	lvRect _margin;
	lvRect _padding;
	int _layoutWidth;
	int _layoutHeight;
	int _minWidth;
	int _maxWidth;
	int _minHeight;
	int _maxHeight;
	int _measuredWidth;
	int _measuredHeight;
	CRUIWidget * _parent;
	lString8 _background;
	lString8 _background2;
	bool _backgroundTiled;
	bool _background2Tiled;
	lUInt32 _backgroundColor;
	lUInt32 _background2Color;
    int _backgroundAlpha;
//	CRUIImageRef _background;
//    CRUIImageRef _background2;
    bool _layoutRequested;
	bool _drawRequested;
	LVFontRef _font;
	lUInt8 _fontSize;
	lUInt32 _textColor;
	lUInt32 _align;
	int _layoutWeight;
    CRUI::Visibility _visibility;
	CRUIOnTouchEventListener * _onTouchListener;
    CRUIOnKeyEventListener * _onKeyListener;
    CRUIOnClickListener * _onClickListener;
	CRUIOnLongClickListener * _onLongClickListener;

	/// measure dimensions
	virtual void defMeasure(int baseWidth, int baseHeight, int contentWidth, int contentHeight);

	bool setClipRect(LVDrawBuf * buf, lvRect & rc);

public:

    virtual void onThemeChanged() {}

    /// correct rectangle bounds according to alignment
    virtual void applyAlign(lvRect & rc, int contentWidth, int contentHeight);

	CRUIWidget();
	virtual ~CRUIWidget();

	/// returns true if point is inside control (excluding margins)
	virtual bool isPointInside(int x, int y);
	/// returns true if widget is child of this
	virtual bool isChild(CRUIWidget * widget);

    /// motion event handler - before children, returns true if it handled event
    virtual bool onTouchEventPreProcess(const CRUIMotionEvent * event);
    /// motion event handler, returns true if it handled event
	virtual bool onTouchEvent(const CRUIMotionEvent * event);
    /// key event handler - before children, returns true if it handled event
    virtual bool onKeyEventPreProcess(const CRUIKeyEvent * event);
    /// key event handler, returns true if it handled event
    virtual bool onKeyEvent(const CRUIKeyEvent * event);
    /// click handler, returns true if it handled event
	virtual bool onClickEvent();
	/// long click handler, returns true if it handled event
	virtual bool onLongClickEvent();

	/// returns true to allow parent intercept this widget which is currently handled by this widget
	virtual bool allowInterceptTouchEvent(const CRUIMotionEvent * event) { CR_UNUSED(event); return true; }

	virtual CRUIOnTouchEventListener * setOnTouchListener(CRUIOnTouchEventListener * listener);
	virtual CRUIOnTouchEventListener * getOnTouchListener() { return _onTouchListener; }
    virtual CRUIOnKeyEventListener * setOnKeyListener(CRUIOnKeyEventListener * listener);
    virtual CRUIOnKeyEventListener * getOnKeyListener() { return _onKeyListener; }
    virtual CRUIOnClickListener * setOnClickListener(CRUIOnClickListener * listener);
	virtual CRUIOnClickListener * getOnClickListener() { return _onClickListener; }
	virtual CRUIOnLongClickListener * setOnLongClickListener(CRUIOnLongClickListener * listener);
	virtual CRUIOnLongClickListener * getOnLongClickListener() { return _onLongClickListener; }

	virtual lvPoint getTileOffset() const { return lvPoint(); }
	const lString8 & getId() { return _id; }
	CRUIWidget * setId(const lString8 & id) { _id = id; return this; }
    CRUIWidget * setId(const char * id) { _id = id; return this; }
    lUInt32 getState() { return _state; }
	lUInt32 getState(lUInt32 mask) { return _state & mask; }
	CRUIWidget * setState(lUInt32 state) { if (_state != state) { _state = state; invalidate(); } return this; }
	CRUIWidget * setState(lUInt32 state, lUInt32 mask) { return setState((_state & ~mask) | (state & mask)); }

    virtual const lvRect & getPos() { return _pos; }
	virtual lUInt32 getAlign();
	virtual lUInt32 getHAlign() { return getAlign() & CRUI::ALIGN_MASK_HORIZONTAL; }
	virtual lUInt32 getVAlign() { return getAlign() & CRUI::ALIGN_MASK_VERTICAL; }
	virtual CRUIWidget * setAlign(lUInt32 align) { _align = align; requestLayout(); return this; }

	int getLayoutWidth() { return _layoutWidth; }
	int getLayoutHeight() { return _layoutHeight; }
	int getLayoutWeight() { return _layoutWeight; }
	CRUIWidget * setLayoutParams(int width, int height) { _layoutWidth = width; _layoutHeight = height; requestLayout(); return this; }
	CRUIWidget * setLayoutWeight(int weight) { _layoutWeight = weight; requestLayout(); return this; }

	CRUIWidget * setPadding(int w) { _padding.left = _padding.top = _padding.right = _padding.bottom = w; requestLayout(); return this; }
	CRUIWidget * setMargin(int w) { _margin.left = _margin.top = _margin.right = _margin.bottom = w; requestLayout(); return this; }
	CRUIWidget * setPadding(const lvRect & rc) { _padding = rc; requestLayout(); return this; }
	CRUIWidget * setMargin(const lvRect & rc) { _margin = rc; requestLayout(); return this; }
	CRUIWidget * setMinWidth(int v) { _minWidth = v; requestLayout(); return this; }
	CRUIWidget * setMaxWidth(int v) { _maxWidth = v; requestLayout(); return this; }
	CRUIWidget * setMinHeight(int v) { _minHeight = v; requestLayout(); return this; }
	CRUIWidget * setMaxHeight(int v) { _maxHeight = v; requestLayout(); return this; }

	virtual void getMargin(lvRect & rc);
	virtual void getPadding(lvRect & rc);
	virtual void applyPadding(lvRect & rc);
	virtual void applyMargin(lvRect & rc);
	virtual const lvRect & getPadding();
	virtual const lvRect & getMargin();
	virtual int getMinHeight();
	virtual int getMaxHeight();
	virtual int getMaxWidth();
	virtual int getMinWidth();

	CRUIWidget * setStyle(lString8 styleId) { _styleId = styleId; return this; }
	CRUIWidget * setStyle(const char * styleId) { _styleId = lString8(styleId); return this; }
	CRUIStyle * getStyle(bool forState = false);

    virtual CRUIWidget * setText(lString16 text) { CR_UNUSED(text); return this; }
    virtual CRUIWidget * setText(lString8 textResourceId) { CR_UNUSED(textResourceId); return this; }
    virtual CRUIWidget * setText(const wchar_t * text) { CR_UNUSED(text); return this; }
    virtual CRUIWidget * setText(const char * textResourceId) { CR_UNUSED(textResourceId); return this; }
	virtual lString16 getText() { return lString16::empty_str; }

	virtual CRUIWidget * setFont(LVFontRef font) { _font = font; requestLayout(); return this; }
	virtual CRUIWidget * setFontSize(lUInt8 sz) { _fontSize = sz; requestLayout(); return this; }
	virtual CRUIWidget * setTextColor(lUInt32 color) { _textColor = color; requestLayout(); return this; }
    /// main (lower) layer of background
    virtual CRUIWidget * setBackground(const char * resourceName, bool tiled = false) { _background = resourceName; _backgroundTiled = tiled; _backgroundColor = COLOR_NONE; requestLayout(); return this; }
	//virtual CRUIWidget * setBackground(CRUIImageRef background) { _background = background; requestLayout(); return this; }
	virtual CRUIWidget * setBackground(lUInt32 color) { _backgroundColor = color; _background.clear(); requestLayout(); return this; }
	virtual CRUIImageRef getBackground();
    /// additional (upper) layer of background
    virtual CRUIWidget * setBackground2(const char * resourceName, bool tiled = false) { _background2 = resourceName; _background2Tiled = tiled; _background2Color = COLOR_NONE; requestLayout(); return this; }
    //virtual CRUIWidget * setBackground2(CRUIImageRef background) { _background2 = background; requestLayout(); return this; }
    virtual CRUIWidget * setBackground2(lUInt32 color) { _background2Color = color; _background2.clear(); requestLayout(); return this; }
    virtual CRUIImageRef getBackground2();

    /// returns background alpha 0..255 (0==opaque, 255 fully transparent)
    virtual int getBackgroundAlpha() { return _backgroundAlpha; }
    /// set background alpha, 0..255 (0==opaque, 255 fully transparent)
    virtual void setBackgroundAlpha(int alpha) { _backgroundAlpha = alpha; }

    virtual LVFontRef getFont();
	virtual lUInt32 getTextColor();
	virtual lUInt8 getFontSize() { return _fontSize; }

    /// returns visibility of widget (VISIBLE, INVISIBLE, GONE)
    virtual CRUI::Visibility getVisibility() { return _visibility; }
    /// sets new visibility for widget (VISIBLE, INVISIBLE, GONE)
    virtual CRUIWidget * setVisibility(CRUI::Visibility visibility);



	virtual int getChildCount() { return 0; }
	virtual CRUIWidget * childById(const lString8 & id);
	virtual CRUIWidget * childById(const char * id);
    virtual CRUIWidget * getChild(int index) { CR_UNUSED(index); return NULL; }
    virtual CRUIWidget * addChild(CRUIWidget * child) { CR_UNUSED(child); return NULL; }
    virtual CRUIWidget * removeChild(int index) { CR_UNUSED(index); return NULL; }
	virtual CRUIWidget * setParent(CRUIWidget * parent) { _parent = parent; return this; }
	/// returns parent widget pointer, NULL if it's top level widget
	virtual CRUIWidget * getParent() { return _parent; }



    virtual bool isLayoutRequested() { return getVisibility() != CRUI::GONE && _layoutRequested; }
    virtual void requestLayout(bool updateParent = false) {
		_layoutRequested = true;
		if (updateParent && _parent)
			_parent->requestLayout(true);
	}

    virtual void animate(lUInt64 millisPassed);
    virtual bool isAnimating() { return false; }
    virtual bool isAnimatingRecursive();
    virtual bool isDrawRequested() { return getVisibility() != CRUI::GONE && _drawRequested; }
	virtual void invalidate() {
		_drawRequested = true;
	}


    int getMeasuredWidth() { return _measuredWidth; }
    int getMeasuredHeight() { return _measuredHeight; }
    void setMeasured(int width, int height) { _measuredWidth = width; _measuredHeight = height; }

	/// measure dimensions
	virtual void measure(int baseWidth, int baseHeight);
	/// updates widget position based on specified rectangle
	virtual void layout(int left, int top, int right, int bottom);
	/// draws widget with its children to specified surface
	virtual void draw(LVDrawBuf * buf);
};

#define SCROLL_STOP_THRESHOLD 5

/// Scroll control utils
struct ScrollControl {
private:
    bool active;
    int  speed1000;
    int  startspeed;
    int  friction;
    lInt64 startpos1000;
    lInt64 pos1000; // position * 1000
    lInt64 dstpos1000; // position * 1000
    bool manual;
    int direction;
    bool cancelling;
public:
    void setDirection(int dir) { direction = dir; }
    int dir() { return direction; }
    int progress() { return (int)((pos1000 - startpos1000) * 10000 / (dstpos1000 - startpos1000)); }
    int startpos() { return (int)(startpos1000 / 1000); }
    void setPos(int p) { pos1000 = p * 1000; }
    int pos() { return (int)(pos1000 / 1000); }
    int speed() { return (int)(speed1000 / 1000); }
    bool isActive() { return active; }
    ScrollControl() : active(false), speed1000(0), friction(0), pos1000(0), manual(false) {}
    void stop() { active = false; speed1000 = 0; }
    /// animate cancel: scroll to previous position
    void cancel() { cancelling = true; }
    bool isCancelled() { return cancelling; }
    void start(int _pos, int _speed, int _friction);
    void start(int _pos, int _pos2, int _speed, int _friction);
    // returns true if position changed
    bool animate(lUInt64 millisPassed);
};


/// will set needLayout to true if any widget in tree starting from specified requires layout, set needRedraw if any widget is invalidated
void CRUICheckUpdateOptions(CRUIWidget * widget, bool & needLayout, bool & needRedraw, bool & animating);

#endif /* CRUIWIDGET_H_ */
