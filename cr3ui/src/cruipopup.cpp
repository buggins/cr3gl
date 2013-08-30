#include "cruipopup.h"
#include "crui.h"

bool CRUIPopupWindow::isAnimating() {
    lUInt64 ts = GetCurrentTimeMillis();
    int interval = (int)(ts - startTimestamp);
    return (appearanceDelay && interval < 2 * appearanceDelay);
}

/// draws widget with its children to specified surface
void CRUIPopupWindow::draw(LVDrawBuf * buf) {
    lUInt64 ts = GetCurrentTimeMillis();
    int interval = (int)(ts - startTimestamp);
    if (appearanceDelay && interval < appearanceDelay) {
        CRLog::trace("popup is still invisible");
        return; // invisible so far
    }
    lUInt32 cl = fadeColor;
    int animationDelay = appearanceDelay * 2;
    if (appearanceDelay && interval > 0 && interval < appearanceDelay + animationDelay) {
        CRLog::trace("popup fading animation in progress");
        int fadePercent = 100;
        int delta = interval - appearanceDelay;
        fadePercent = delta * 100 / animationDelay;
        if (fadePercent < 0)
            fadePercent = 0;
        if (fadePercent > 100)
            fadePercent = 100;
        int startAlpha = 255;
        int endAlpha = (fadeColor >> 24) & 255;
        int alpha = startAlpha + (endAlpha - startAlpha) * fadePercent / 100;
        cl = (cl & 0xFFFFFF) | ((alpha & 255) << 24);
    }
    buf->FillRect(_pos, cl);
    //CRUIWidget::draw(buf);
    CRUIWidget * child = getContentWidget();
    if (!child)
        return;
    LVDrawStateSaver saver(*buf);
    lvRect rc = _pos;
    applyMargin(rc);
    setClipRect(buf, rc);
    applyPadding(rc);
    child->draw(buf);
}

/// measure dimensions
void CRUIPopupWindow::measure(int baseWidth, int baseHeight)
{
    CRUIWidget * child = getContentWidget();
    _measuredWidth = baseWidth;
    _measuredHeight = baseHeight;
    if (!child) {
        return;
    }
    child->measure(baseWidth, baseHeight);
}

/// updates widget position based on specified rectangle
void CRUIPopupWindow::layout(int left, int top, int right, int bottom)
{
    _pos.left = left;
    _pos.top = top;
    _pos.right = right;
    _pos.bottom = bottom;
    CRUIWidget * child = getContentWidget();
    if (!child)
        return;
    lvRect rc = _pos;
    applyMargin(rc);
    applyPadding(rc);
    child->applyAlign(rc, child->getMeasuredWidth(), child->getMeasuredHeight());
    rc.right = rc.left + child->getMeasuredWidth();
    rc.bottom = rc.top + child->getMeasuredHeight();
    child->layout(rc.left, rc.top, rc.right, rc.bottom);
}

CRUIWidget * CRUIPopupWindow::setContentWidget(CRUIWidget * widget) {
    if (getContentWidget() == widget)
        return widget;
    _children.clear();
    _children.add(widget);
    requestLayout();
    return widget;
}

CRUIPopupWindow::CRUIPopupWindow(int _appearanceDelay, lUInt32 _fadeColor) : appearanceDelay(_appearanceDelay), fadeColor(_fadeColor) {
    startTimestamp = GetCurrentTimeMillis();
}

CRUIPopupWindow::CRUIPopupWindow(CRUIWidget * contentWidget, int _appearanceDelay, lUInt32 _fadeColor) : appearanceDelay(_appearanceDelay), fadeColor(_fadeColor) {
    startTimestamp = GetCurrentTimeMillis();
    setContentWidget(contentWidget);
}
