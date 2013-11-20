#include "vkeyboard.h"

struct VKLayoutKey {
    lString16 text;
    int keycode;
    int mode; // special mode switching key code
    int weight; // to specify "wide" keys
    VKLayoutKey() : keycode(0), mode(VK_SWITCH_NONE), weight(0) {}
    VKLayoutKey(const wchar_t * txt, int code) : text(txt), keycode(code), mode(VK_SWITCH_NONE), weight(0) {}
    VKLayoutKey(const char * txt, int code, int w = 0) : text(txt), keycode(code), mode(VK_SWITCH_NONE), weight(w) {}
    VKLayoutKey(int keyCode, int w) : keycode(keyCode), mode(VK_SWITCH_NONE), weight(w) {}
};

struct VKLayoutRow {
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

struct VKLayout {
    LVPtrVector<VKLayoutRow> rows;
    int mode;
    VKLayout(int m = VK_SWITCH_NONE) : mode(m) {}
    VKLayoutRow * addRow() { VKLayoutRow * row = new VKLayoutRow(); rows.add(row); return row; }
};

struct VKLayoutSet {
    LVPtrVector<VKLayout> layouts;
    VKLayout * addLayout(int mode) { VKLayout * res = new VKLayout(mode); layouts.add(res); return res; }
    VKLayout * findByMode(int mode) {
        for (int i = 0; i < layouts.length(); i++)
            if (layouts[i]->mode == mode)
                return layouts[i];
        return NULL;
    }
};

VKLayoutSet * createEnglishLayout() {
    VKLayoutSet * res = new VKLayoutSet();
    VKLayout * l = res->addLayout(VK_SWITCH_NONE);
    l->addRow()->add("q", CR_KEY_Q)->add("w", CR_KEY_W)->add("e", CR_KEY_E)->add("r", CR_KEY_R)->add("t", CR_KEY_T)->add("y", CR_KEY_Y)->add("u", CR_KEY_U)->add("i", CR_KEY_I)->add("o", CR_KEY_O)->add("p", CR_KEY_P);
    l->addRow()->add("a", CR_KEY_A)->add("s", CR_KEY_S)->add("d", CR_KEY_D)->add("f", CR_KEY_F)->add("g", CR_KEY_G)->add("h", CR_KEY_H)->add("j", CR_KEY_J)->add("k", CR_KEY_K)->add("l", CR_KEY_L);
    l->addRow()->addSpecial(VK_SWITCH_SHIFT)->add("z", CR_KEY_Z)->add("x", CR_KEY_X)->add("c", CR_KEY_C)->add("v", CR_KEY_V)->add("b", CR_KEY_B)->add("n", CR_KEY_N)->add("m", CR_KEY_M)->add(CR_KEY_BACKSPACE);
    l->addRow()->addSpecial(VK_SWITCH_PUNCT)->addSpecial(VK_SWITCH_LAYOUT)->add(" ", CR_KEY_SPACE)->add(".")->add(CR_KEY_RETURN);
    l = res->addLayout(VK_SWITCH_SHIFT);
    l->addRow()->add("Q", CR_KEY_Q)->add("W", CR_KEY_W)->add("E", CR_KEY_E)->add("R", CR_KEY_R)->add("T", CR_KEY_T)->add("Y", CR_KEY_Y)->add("U", CR_KEY_U)->add("I", CR_KEY_I)->add("O", CR_KEY_O)->add("P", CR_KEY_P);
    l->addRow()->add("A", CR_KEY_A)->add("S", CR_KEY_S)->add("D", CR_KEY_D)->add("F", CR_KEY_F)->add("G", CR_KEY_G)->add("H", CR_KEY_H)->add("J", CR_KEY_J)->add("K", CR_KEY_K)->add("L", CR_KEY_L);
    l->addRow()->addSpecial(VK_SWITCH_SHIFT)->add("Z", CR_KEY_Z)->add("X", CR_KEY_X)->add("C", CR_KEY_C)->add("V", CR_KEY_V)->add("B", CR_KEY_B)->add("N", CR_KEY_N)->add("M", CR_KEY_M)->add(CR_KEY_BACKSPACE);
    l->addRow()->addSpecial(VK_SWITCH_PUNCT)->addSpecial(VK_SWITCH_LAYOUT)->add(" ", CR_KEY_SPACE)->add(".")->add(CR_KEY_RETURN);
    l = res->addLayout(VK_SWITCH_PUNCT);
    l->addRow()->add("1", CR_KEY_1)->add("2", CR_KEY_2)->add("3", CR_KEY_3)->add("4", CR_KEY_4)->add("5", CR_KEY_5)->add("6", CR_KEY_6)->add("7", CR_KEY_7)->add("8", CR_KEY_8)->add("9", CR_KEY_9)->add("0", CR_KEY_0);
    l->addRow()->add("-", CR_KEY_A)->add("/", CR_KEY_S)->add(":", CR_KEY_D)->add(";", CR_KEY_F)->add("(", CR_KEY_G)->add(")", CR_KEY_H)->add("$", CR_KEY_J)->add("&", CR_KEY_K)->add("@", CR_KEY_L)->add("\"");
    l->addRow()->addSpecial(VK_SWITCH_SHIFT)->add(".", CR_KEY_Z)->add(",", CR_KEY_X)->add("?", CR_KEY_C)->add("!", CR_KEY_V)->add("'", CR_KEY_B)->add(CR_KEY_BACKSPACE);
    l->addRow()->addSpecial(VK_SWITCH_PUNCT)->addSpecial(VK_SWITCH_LAYOUT)->add(" ", CR_KEY_SPACE)->add(".")->add(CR_KEY_RETURN);
    l = res->addLayout(VK_SWITCH_PUNCT|VK_SWITCH_SHIFT);
    l->addRow()->add("[", CR_KEY_1)->add("]", CR_KEY_2)->add("{", CR_KEY_3)->add("}", CR_KEY_4)->add("#", CR_KEY_5)->add("%", CR_KEY_6)->add("^", CR_KEY_7)->add("*", CR_KEY_8)->add("+", CR_KEY_9)->add("=", CR_KEY_0);
    l->addRow()->add("_", CR_KEY_A)->add("\\", CR_KEY_S)->add("|", CR_KEY_D)->add("~", CR_KEY_F)->add("<", CR_KEY_G)->add(">", CR_KEY_H)->add("$", CR_KEY_J)->add("$", CR_KEY_K)->add("$", CR_KEY_L)->add(".");
    l->addRow()->addSpecial(VK_SWITCH_SHIFT)->add(".", CR_KEY_Z)->add(",", CR_KEY_X)->add("?", CR_KEY_C)->add("!", CR_KEY_V)->add("'", CR_KEY_B)->add(CR_KEY_BACKSPACE);
    l->addRow()->addSpecial(VK_SWITCH_PUNCT)->addSpecial(VK_SWITCH_LAYOUT)->add(" ", CR_KEY_SPACE)->add(".")->add(CR_KEY_RETURN);
    return res;
}

CRUIVirtualKeyboard::CRUIVirtualKeyboard() : _layoutSet(NULL), _currentLayout(NULL), _mode(VK_SWITCH_NONE) {
    //_currentLayout = &_normalLayout;
    _layoutSet = createEnglishLayout();
    _currentLayout = _layoutSet->layouts[0];
    createWidgets();
}

CRUIVirtualKeyboard::~CRUIVirtualKeyboard() {
    delete _layoutSet;
}

void CRUIVirtualKeyboard::createWidgets() {

}
