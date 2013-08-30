#include "cruipopup.h"

/// draws widget with its children to specified surface
void CRUIPopupWindow::draw(LVDrawBuf * buf) {
    CRUIWidget::draw(buf);
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
    applyAlign(rc, child->getMeasuredWidth(), child->getMeasuredHeight());
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

CRUIPopupWindow::CRUIPopupWindow() {

}

CRUIPopupWindow::CRUIPopupWindow(CRUIWidget * contentWidget) {
    setContentWidget(contentWidget);
}
