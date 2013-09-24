
#define CRACTION_UI(id,icon) CRACTION(id,0,0,ACTION_ ## id, icon)
#define CRACTION_DCMD(id,dcmd,param,icon) CRACTION(id,dcmd,param,ACTION_ ## id, icon)

CRACTION_UI(EXIT, "home")
CRACTION_UI(MENU, "menu")
CRACTION_UI(SETTINGS, "fantasy")
CRACTION_UI(BACK, "left_circular")
CRACTION_DCMD(PAGE_DOWN, DCMD_PAGEDOWN, 1, "down_circular")
CRACTION_DCMD(PAGE_UP, DCMD_PAGEUP, 1, "up_circular")
