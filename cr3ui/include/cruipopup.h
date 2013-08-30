#ifndef CRUIPOPUP_H
#define CRUIPOPUP_H

#include "cruilayout.h"

class CRUIPopupWindow : public CRUIContainerWidget {
    lUInt64 startTimestamp;
    int appearanceDelay;
    lUInt32 fadeColor;
public:
    virtual CRUIWidget * getContentWidget() { return getChildCount() > 0 ? getChild(0) : NULL; }
    virtual CRUIWidget * setContentWidget(CRUIWidget * widget);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
    virtual bool isAnimating();

    CRUIPopupWindow(int appearanceDelay = 0, lUInt32 fadeColor = 0xFF000000);
    CRUIPopupWindow(CRUIWidget * contentWidget, int appearanceDelay = 0, lUInt32 fadeColor = 0xFF000000);
};

#endif // CRUIPOPUP_H
