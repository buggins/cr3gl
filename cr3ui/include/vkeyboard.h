#ifndef VKEYBOARD_H
#define VKEYBOARD_H

#include "cruilayout.h"

enum {
    VK_SWITCH_NONE,
    VK_SWITCH_SHIFT = 1,
    VK_SWITCH_PUNCT = 2,
    VK_SWITCH_LAYOUT = 4
};

class VKLayoutSet;
class VKLayout;
class VKLayoutKey;
class CRUIVirtualKeyboard : public CRUIVerticalLayout {
    VKLayoutSet * _layoutSet;
    VKLayout * _currentLayout;
    int _mode;
    void createWidgets();
public:
    int getMode() { return _mode; }
    CRUIVirtualKeyboard();
    virtual ~CRUIVirtualKeyboard();

    void onVKeyDown(VKLayoutKey * key);
    void onVKeyUp(VKLayoutKey * key);

    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
};

#endif // VKEYBOARD_H
