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
    CRUIAction(int _id, int _cmd, int _param, const char * _name_res, const char * _icon_res) : id(_id), cmd(_cmd), param(_param), name_res(_name_res), icon_res(_icon_res) { }
    CRUIAction(const CRUIAction & v);
    CRUIAction & operator = (const CRUIAction & v);
    CRUIAction * clone() const;
    lString16 getName() const;
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
    int length() const { return _list.length(); }
    void clear() { _list.clear(); }
    const CRUIAction * operator[] (int index) const { return _list[index]; }
    const CRUIAction * get(int index) const { return _list[index]; }
    CRUIActionList() {}
    CRUIActionList(const CRUIActionList & list) { addAll(list); }
    CRUIActionList & operator += (const CRUIActionList & list) { addAll(list); }
};

#ifndef CRACTION_ENUM_INCLUDED
#define CRACTION_ENUM_INCLUDED
// define enum
#undef CRACTION
#define CRACTION(id,cmd,param,name,icon) CMD_ ## id,
enum {
    CMD_FIRST = 10000,

    #include "cruiactiondef.h"

    CMD_LAST
};
#endif

#ifndef CRACTION_EXTERN_INCLUDED
#define CRACTION_EXTERN_INCLUDED
// define external instances
#undef CRACTION
#define CRACTION(id,cmd,param,name,icon) extern const char * STR_ ## name; extern const CRUIAction * ACTION_ ## id;
#include "cruiactiondef.h"
#endif

const CRUIAction * CRUIActionByName(const char * id);
const CRUIAction * CRUIActionByCode(int code);


#endif // CRUIACTION_H
