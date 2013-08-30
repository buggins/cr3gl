#ifndef CRUIPOPUP_H
#define CRUIPOPUP_H

#include "cruilayout.h"

class CRUIPopupWindow : public CRUIContainerWidget {
public:
    virtual CRUIWidget * getContentWidget() { return getChildCount() > 0 ? getChild(0) : NULL; }
    virtual CRUIWidget * setContentWidget(CRUIWidget * widget);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);

    CRUIPopupWindow();
    CRUIPopupWindow(CRUIWidget * contentWidget);
};

#endif // CRUIPOPUP_H
