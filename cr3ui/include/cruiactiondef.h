#ifndef CRUIACTIONDEF_H
#define CRUIACTIONDEF_H

#ifdef CRACTION_ENUM
enum {
    CMD_FIRST = 10000,
#else
namespace ACTION {
#endif

#ifdef CRACTION_IMPL
  #define CRACTION(id,cmd,param,name,icon) const CRUIAction * id = new CRUIAction(id,cmd,param,name,icon);
#else
  #ifdef CRACTION_ENUM
    #define CRACTION(id,cmd,param,name,icon) CMD_ # id,
  #else
    #define CRACTION(id,cmd,param,name,icon) extern const CRUIAction * id;
  #endif
#endif
#define CRACTION_UI(id, icon) CRACTION(CMD_ ## id, 0, 0, "STR_ACTION_" # id, icon);




CRACTION_UI(EXIT, "home")
CRACTION_UI(BACK, "left_circular")
CRACTION_UI(PAGE_DOWN, "down_circular")
CRACTION_UI(PAGE_UP, "up_circular")


#ifdef CRACTION_ENUM
    CMD_LAST
};
#else
}
#endif


#endif // CRUIACTIONDEF_H
