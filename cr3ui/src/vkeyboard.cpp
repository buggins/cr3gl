#include "vkeyboard.h"
#include "cruicontrols.h"
#include "crui.h"

using namespace CRUI;

class VKLayoutKey {
public:
    lString16 text;
    int keycode;
    int mode; // special mode switching key code
    int weight; // to specify "wide" keys
    VKLayoutKey() : keycode(0), mode(VK_SWITCH_NONE), weight(0) {}
    VKLayoutKey(const wchar_t * txt, int code) : text(txt), keycode(code), mode(VK_SWITCH_NONE), weight(0) {}
    VKLayoutKey(const char * txt, int code, int w = 0) : text(txt), keycode(code), mode(VK_SWITCH_NONE), weight(w) {}
    VKLayoutKey(int keyCode, int w) : keycode(keyCode), mode(VK_SWITCH_NONE), weight(w) {}
};

class VKLayoutRow {
public:
    LVPtrVector<VKLayoutKey> items;
    VKLayoutRow * add(const wchar_t * text, int keycode = 0) {
        items.add(new VKLayoutKey(text, keycode));
        return this;
    }
    VKLayoutRow * add(const char * text, int keycode = 0, int w = 0) {
        items.add(new VKLayoutKey(text, keycode, w));
        return this;
    }
    VKLayoutRow * add(int keyCode, int weight = 0) {
        items.add(new VKLayoutKey(keyCode, weight));
        return this;
    }
    VKLayoutRow * addSpecial(int mode, int weight = 0) {
        VKLayoutKey * key = new VKLayoutKey();
        key->mode = mode;
        key->weight = weight;
        items.add(key);
        return this;
    }
};

class VKLayout {
public:
    LVPtrVector<VKLayoutRow> rows;
    int mode;
    VKLayout(int m = VK_SWITCH_NONE) : mode(m) {}
    VKLayoutRow * addRow() { VKLayoutRow * row = new VKLayoutRow(); rows.add(row); return row; }
};

class VKLayoutSet {
public:
    LVPtrVector<VKLayout> layouts;
    VKLayout * addLayout(int mode) { VKLayout * res = new VKLayout(mode); layouts.add(res); return res; }
    VKLayout * findByMode(int mode) {
        for (int i = 0; i < layouts.length(); i++)
            if (layouts[i]->mode == mode)
                return layouts[i];
        return NULL;
    }
};

static VKLayoutSet * createEnglishLayout() {
    VKLayoutSet * res = new VKLayoutSet();
    VKLayout * l = res->addLayout(VK_SWITCH_NONE);
    l->addRow()->add("q", CR_KEY_Q)->add("w", CR_KEY_W)->add("e", CR_KEY_E)->add("r", CR_KEY_R)->add("t", CR_KEY_T)->add("y", CR_KEY_Y)->add("u", CR_KEY_U)->add("i", CR_KEY_I)->add("o", CR_KEY_O)->add("p", CR_KEY_P);
    l->addRow()->add("a", CR_KEY_A)->add("s", CR_KEY_S)->add("d", CR_KEY_D)->add("f", CR_KEY_F)->add("g", CR_KEY_G)->add("h", CR_KEY_H)->add("j", CR_KEY_J)->add("k", CR_KEY_K)->add("l", CR_KEY_L);
    l->addRow()->addSpecial(VK_SWITCH_SHIFT)->add("z", CR_KEY_Z)->add("x", CR_KEY_X)->add("c", CR_KEY_C)->add("v", CR_KEY_V)->add("b", CR_KEY_B)->add("n", CR_KEY_N)->add("m", CR_KEY_M)->add(CR_KEY_BACKSPACE);
    l->addRow()->addSpecial(VK_SWITCH_PUNCT)->addSpecial(VK_SWITCH_LAYOUT)->add(" ", CR_KEY_SPACE, 3)->add(".")->add(CR_KEY_RETURN, 1);
    l = res->addLayout(VK_SWITCH_SHIFT);
    l->addRow()->add("Q", CR_KEY_Q)->add("W", CR_KEY_W)->add("E", CR_KEY_E)->add("R", CR_KEY_R)->add("T", CR_KEY_T)->add("Y", CR_KEY_Y)->add("U", CR_KEY_U)->add("I", CR_KEY_I)->add("O", CR_KEY_O)->add("P", CR_KEY_P);
    l->addRow()->add("A", CR_KEY_A)->add("S", CR_KEY_S)->add("D", CR_KEY_D)->add("F", CR_KEY_F)->add("G", CR_KEY_G)->add("H", CR_KEY_H)->add("J", CR_KEY_J)->add("K", CR_KEY_K)->add("L", CR_KEY_L);
    l->addRow()->addSpecial(VK_SWITCH_SHIFT)->add("Z", CR_KEY_Z)->add("X", CR_KEY_X)->add("C", CR_KEY_C)->add("V", CR_KEY_V)->add("B", CR_KEY_B)->add("N", CR_KEY_N)->add("M", CR_KEY_M)->add(CR_KEY_BACKSPACE);
    l->addRow()->addSpecial(VK_SWITCH_PUNCT)->addSpecial(VK_SWITCH_LAYOUT)->add(" ", CR_KEY_SPACE, 3)->add(".")->add(CR_KEY_RETURN, 1);
    l = res->addLayout(VK_SWITCH_PUNCT);
    l->addRow()->add("1", CR_KEY_1)->add("2", CR_KEY_2)->add("3", CR_KEY_3)->add("4", CR_KEY_4)->add("5", CR_KEY_5)->add("6", CR_KEY_6)->add("7", CR_KEY_7)->add("8", CR_KEY_8)->add("9", CR_KEY_9)->add("0", CR_KEY_0);
    l->addRow()->add("-", CR_KEY_A)->add("/", CR_KEY_S)->add(":", CR_KEY_D)->add(";", CR_KEY_F)->add("(", CR_KEY_G)->add(")", CR_KEY_H)->add("$", CR_KEY_J)->add("&", CR_KEY_K)->add("@", CR_KEY_L)->add("\"");
    l->addRow()->addSpecial(VK_SWITCH_SHIFT)->add(".", CR_KEY_Z)->add(",", CR_KEY_X)->add("?", CR_KEY_C)->add("!", CR_KEY_V)->add("'", CR_KEY_B)->add(CR_KEY_BACKSPACE);
    l->addRow()->addSpecial(VK_SWITCH_PUNCT)->addSpecial(VK_SWITCH_LAYOUT)->add(" ", CR_KEY_SPACE, 3)->add(".")->add(CR_KEY_RETURN, 1);
    l = res->addLayout(VK_SWITCH_PUNCT|VK_SWITCH_SHIFT);
    l->addRow()->add("[", CR_KEY_1)->add("]", CR_KEY_2)->add("{", CR_KEY_3)->add("}", CR_KEY_4)->add("#", CR_KEY_5)->add("%", CR_KEY_6)->add("^", CR_KEY_7)->add("*", CR_KEY_8)->add("+", CR_KEY_9)->add("=", CR_KEY_0);
    l->addRow()->add("_", CR_KEY_A)->add("\\", CR_KEY_S)->add("|", CR_KEY_D)->add("~", CR_KEY_F)->add("<", CR_KEY_G)->add(">", CR_KEY_H)->add("$", CR_KEY_J)->add("$", CR_KEY_K)->add("$", CR_KEY_L)->add(".");
    l->addRow()->addSpecial(VK_SWITCH_SHIFT)->add(".", CR_KEY_Z)->add(",", CR_KEY_X)->add("?", CR_KEY_C)->add("!", CR_KEY_V)->add("'", CR_KEY_B)->add(CR_KEY_BACKSPACE);
    l->addRow()->addSpecial(VK_SWITCH_PUNCT)->addSpecial(VK_SWITCH_LAYOUT)->add(" ", CR_KEY_SPACE, 3)->add(".")->add(CR_KEY_RETURN, 1);
    return res;
}

CRUIVirtualKeyboard::CRUIVirtualKeyboard() : _layoutSet(NULL), _currentLayout(NULL), _mode(VK_SWITCH_NONE) {
    //_currentLayout = &_normalLayout;
    _layoutSet = createEnglishLayout();
    _currentLayout = _layoutSet->layouts[0];
    setAlign(ALIGN_BOTTOM|ALIGN_HCENTER);
    createWidgets();
    setStyle("VIRTUAL_KEYBOARD");
}

CRUIVirtualKeyboard::~CRUIVirtualKeyboard() {
    delete _layoutSet;
    _layoutSet = NULL;
    _currentLayout = NULL;
}

/// draws widget with its children to specified surface
void CRUIVirtualKeyboard::draw(LVDrawBuf * buf) {
    CRUIVerticalLayout::draw(buf);
}

class VKButton : public CRUIButton {
    CRUIVirtualKeyboard * _keyboard;
    VKLayoutKey * _key;
public:
    VKButton(CRUIVirtualKeyboard * keyboard, VKLayoutKey * key) : CRUIButton(lString16(" ")), _keyboard(keyboard), _key(key) {
        if (key->mode) {
            if (key->mode == VK_SWITCH_PUNCT) {
                if (_keyboard->getMode() & VK_SWITCH_PUNCT)
                    setText(lString16("ABC"));
                else
                    setText(lString16("123"));
            } else if (key->mode == VK_SWITCH_SHIFT) {
                if (_keyboard->getMode() & VK_SWITCH_PUNCT)
                    setText(lString16("#$"));
                else
                    setText(lString16("Aa"));
            } else if (key->mode == VK_SWITCH_LAYOUT)
                setText(lString16(L"\x25cb"));
            setStyle("VIRTUAL_KEYBOARD_KEY_SPECIAL");
        } else if (!key->text.empty()) {
            setText(key->text);
            setStyle("VIRTUAL_KEYBOARD_KEY_NORMAL");
        } else {
            if (key->keycode == CR_KEY_BACKSPACE)
                setText(lString16("<<"));
            else if (key->keycode == CR_KEY_RETURN)
                setText(lString16(L"\x25ba"));
            else
                setText(lString16("?"));
            setStyle("VIRTUAL_KEYBOARD_KEY_SPECIAL");
        }
        if (key->weight || true) {
            setLayoutParams(FILL_PARENT, WRAP_CONTENT);
            setLayoutWeight(key->weight + 1);
        } else
            setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
    }
    /// click handler, returns true if it handled event
    virtual bool onClickEvent() {
        _keyboard->onVKeyUp(_key);
        return true;
    }
    /// click handler called on key press / touch down, returns true if it handled event
    virtual bool onClickDownEvent() {
        _keyboard->onVKeyDown(_key);
        return true;
    }

};

void CRUIVirtualKeyboard::onVKeyDown(VKLayoutKey * key) {
    CR_UNUSED(key);
}

void CRUIVirtualKeyboard::onVKeyUp(VKLayoutKey * key) {
    if (key->mode) {
        VKLayout * newlayout = NULL;
        int newmode = -1;
        if (key->mode == VK_SWITCH_SHIFT) {
            newmode = _mode ^ VK_SWITCH_SHIFT;
        } else if (key->mode == VK_SWITCH_PUNCT) {
            newmode = _mode ^ VK_SWITCH_PUNCT;
            newmode &= ~VK_SWITCH_SHIFT;
        }
        newlayout = _layoutSet->findByMode(newmode);
        if (newlayout) {
            _mode = newmode;
            _currentLayout = newlayout;
            createWidgets();
            requestLayout();
        }
    } else {
        CRUIKeyEvent * eventpress = new CRUIKeyEvent(KEY_ACTION_PRESS, key->keycode, false, 1, CR_KEY_MODIFIER_NONE);
        eventpress->setText(key->text);
        CRUIKeyEvent * eventrelease = new CRUIKeyEvent(KEY_ACTION_RELEASE, key->keycode, false, 1, CR_KEY_MODIFIER_NONE);
        eventrelease->setText(key->text);
        CRUIEventManager::dispatchKey(eventpress);
        CRUIEventManager::dispatchKey(eventrelease);
    }
}

/// measure dimensions
void CRUIVirtualKeyboard::measure(int baseWidth, int baseHeight) {
    int rowh = deviceInfo.shortSide / 2 / _children.length() - PT_TO_PX(4);
    int fs = rowh / 2 - 2;
    for (int i = 0; i < _children.length(); i++) {
        CRUIWidget * child = _children[i];
        child->setMinHeight(rowh);
        child->setMaxHeight(rowh);
        for (int j = 0; j < child->getChildCount(); j++)
            child->getChild(j)->setFontSize(fs);
    }
    CRUIVerticalLayout::measure(baseWidth, baseHeight);
}

void CRUIVirtualKeyboard::createWidgets() {
    _children.clear();
    //CRUIVerticalLayout * vlayout = new CRUIVerticalLayout();
    //vlayout->
    setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    for (int i = 0; i < _currentLayout->rows.length(); i++) {
        CRUIHorizontalLayout * hlayout = new CRUIHorizontalLayout();
        hlayout->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
        VKLayoutRow * row = _currentLayout->rows[i];
        for (int j = 0; j < row->items.length(); j++) {
            VKLayoutKey * key = row->items[j];
            VKButton * btn = new VKButton(this, key);
            hlayout->addChild(btn);
        }
        //vlayout->
        addChild(hlayout);
    }
    //addChild(vlayout);
}
