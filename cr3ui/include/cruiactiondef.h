
#define CRACTION_UI(id,icon) CRACTION(id,0,0,ACTION_ ## id, icon)
#define CRACTION_DCMD(id,dcmd,param,icon) CRACTION(id,dcmd,param,ACTION_ ## id, icon)

CRACTION_UI(NO_ACTION, "no_action")
CRACTION_UI(EXIT, "close_window")
CRACTION_UI(MENU, "menu_more")
CRACTION_UI(SETTINGS, "fantasy")
CRACTION_UI(BACK, "left_circular")
CRACTION_UI(LINK_BACK, "undo")
CRACTION_UI(LINK_FORWARD, "redo")
CRACTION_UI(SELECTION_COPY, "copy")
CRACTION_UI(SELECTION_ADD_BOOKMARK, "bookmark")
CRACTION_UI(BOOKMARKS, "bookmark")
CRACTION_UI(BOOKMARK_GOTO, "arrow")
CRACTION_UI(BOOKMARK_REMOVE, "cancel")
CRACTION_UI(FIND_TEXT, "google_web_search")
CRACTION_UI(HELP, "help")
CRACTION_UI(NIGHT_MODE, "moon")
CRACTION_UI(DAY_MODE, "sun")
CRACTION_UI(TOGGLE_NIGHT_MODE, "moon")
CRACTION_UI(READER_HOME, "home")
CRACTION_UI(SHOW_FOLDER, "folder_icon")
CRACTION_UI(CURRENT_BOOK, "book")
CRACTION_UI(GOTO_PERCENT, "arrow")
CRACTION_UI(TOC, "document")
CRACTION_UI(FOLDER_BOOKMARK_ADD, "plus")
CRACTION_UI(FOLDER_BOOKMARK_REMOVE, "minus")
CRACTION_UI(FOLDER_BOOKMARK_USE_FOR_DOWNLOADS, "download")
CRACTION_UI(OPDS_CATALOG_ADD, "plus")
CRACTION_UI(OPDS_CATALOG_REMOVE, "minus")
CRACTION_UI(OPDS_CATALOG_OPEN, "redo")
CRACTION_UI(OPDS_CATALOG_SEARCH, "google_web_search")
CRACTION_UI(OPDS_CATALOG_CANCEL_CHANGES, "cancel")
CRACTION_DCMD(PAGE_DOWN, DCMD_PAGEDOWN, 1, "down_circular")
CRACTION_DCMD(PAGE_UP, DCMD_PAGEUP, 1, "up_circular")
