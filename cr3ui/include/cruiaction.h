#ifndef CRUIACTION_H
#define CRUIACTION_H

#include "lvptrvec.h"
#include "lvstring.h"
#include "cri18n.h"
#include "lvdocview.h" // import DCMD_ defs

struct CRUIAction {
    int id; // action ID
    int cmd; // command
    int param; // optional command param
    lString16 name;    // name string
    lString8 name_res; // name string resource id
    lString8 icon_res; // icon resource id
    lString8 sparam;   // string parameter for some custom actions
    CRUIAction(int _id) : id(_id), cmd(0), param(0) { }
    CRUIAction(const CRUIAction & v);
    CRUIAction & operator = (const CRUIAction & v);
    CRUIAction * clone() const;
    lString16 getName();
};

class CRUIActionList {
    LVPtrVector<CRUIAction> _list;
public:
    void add(const CRUIAction * item) {
        _list.add(item->clone());
    }
    void insert(int pos, const CRUIAction * item) {
        _list.insert(pos, item->clone());
    }
    void remove(int pos) {
        delete _list.remove(pos);
    }
    void addAll(const CRUIActionList & list);
    int length() const { return _list; }
    void clear() { _list.clear(); }
    const CRUIAction * operator[] (int index) const { return _list[index]; }
    const CRUIAction * get(int index) const { return _list[index]; }
    CRUIActionList() {}
    CRUIActionList(const CRUIActionList & list) { addAll(list); }
    CRUIActionList & operator += (const CRUIActionList & list) { addAll(list); }
};

#undef CRACTION_IMPL
#define CRACTION_ENUM
#include "cruiactiondef.h"
#undef CRACTION_ENUM
#include "cruiactiondef.h"


#endif // CRUIACTION_H
