#include "cruiaction.h"

#define CRACTION_IMPL
#include "cruiactiondef.h"
#undef CRACTION_IMPL


lString16 CRUIAction::getName() {
    return name.empty() ? (name_res.empty() ? lString16() : _16(name_res)) : name;
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
}

CRUIAction * CRUIAction::clone() const {
    return new CRUIAction(*this);
}

void CRUIActionList::addAll(const CRUIActionList & list) {
    for (int i = 0; i < list.length(); i++) {
        add(list[i]);
    }
}
