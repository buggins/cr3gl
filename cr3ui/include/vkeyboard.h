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
class CRUIVirtualKeyboard : public CRUIFrameLayout {
    VKLayoutSet * _layoutSet;
    VKLayout * _currentLayout;
    int _mode;
    void createWidgets();
public:
    CRUIVirtualKeyboard();
    virtual ~CRUIVirtualKeyboard();
};

#endif // VKEYBOARD_H
