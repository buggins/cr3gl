#include "cruiaction.h"

#undef CRACTION
#define CRACTION(id,cmd,param,name,icon) const char * name  = # name; const CRUIAction * ACTION_ ## id = new CRUIAction(CMD_ ## id,cmd,param,name,icon);
#include "cruiactiondef.h"

lString16 CRUIAction::getName() const {
    return name.empty() ? (name_res.empty() ? lString16() : _16(name_res.c_str())) : name;
}

CRUIAction::CRUIAction(const CRUIAction & v)
    : id(v.id), cmd(v.cmd), param(v.param), name(v.name), name_res(v.name_res), icon_res(v.icon_res), sparam(v.sparam) {
}

CRUIAction & CRUIAction::operator = (const CRUIAction & v) {
    id = v.id;
    cmd = v.cmd;
    param = v.param;
    name = v.name;
    name_res = v.name_res;
    icon_res = v.icon_res;
    sparam = v.sparam;
    return *this;
}

CRUIAction * CRUIAction::clone() const {
    return new CRUIAction(*this);
}

void CRUIActionList::addAll(const CRUIActionList & list) {
    for (int i = 0; i < list.length(); i++) {
        add(list[i]);
    }
}