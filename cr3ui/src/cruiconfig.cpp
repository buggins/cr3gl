#include "cruiconfig.h"
#include "lvstream.h"
#include "glfont.h"
#include "gldrawbuf.h"
#include "fileinfo.h"
#include "lvtinydom.h"
#include "cruitheme.h"
#include "crui.h"
#include "crcoverpages.h"
#include "stringresource.h"
#include "cruisettings.h"
#include "hyphman.h"

using namespace CRUI;

CRUIConfig crconfig;

CRUIConfig::CRUIConfig() {
    docCacheMaxBytes = 32*1024*1024;
    coverDirMaxItems = 1000;
    coverDirMaxFiles = 200;
    coverDirMaxSize = 16*1024*1024;
    coverRenderCacheMaxItems = 1000;
    coverRenderCacheMaxBytes = 16 * 1024 * 1024;
    uiFontFace = "Arial";
}

void CRUIConfig::setupUserDir(lString8 baseDir) {
    LVAppendPathDelimiter(baseDir);
    CRLog::info("Setting up user directory: %s", baseDir.c_str());
    // coverpage file cache
    crconfig.coverCacheDir = baseDir + "coverpages";
    // document cache
    crconfig.docCacheDir = baseDir + "cache";

    crconfig.dbFile = baseDir + "cr3db.sqlite";
    crconfig.iniFile = baseDir + "cr3.ini";
}

/// sets resourceDir, i18ndir, hyphdir
void CRUIConfig::setupResources(lString8 baseDir) {
    LVAppendPathDelimiter(baseDir);
    CRLog::info("Setting up resources directory: %s", baseDir.c_str());
    resourceDir = baseDir;
    i18nDir = baseDir + "i18n";
    hyphDir = baseDir + "hyph";
    cssDir = baseDir + "css";
    themesDir = baseDir + "themes";
    manualsDir = baseDir + "manuals";
    LVAppendPathDelimiter(i18nDir);
    LVAppendPathDelimiter(hyphDir);
    LVAppendPathDelimiter(cssDir);
    LVAppendPathDelimiter(themesDir);
    LVAppendPathDelimiter(manualsDir);
    currentThemeDir = themesDir + "light";
    LVAppendPathDelimiter(currentThemeDir);
    interfaceLanguages.add(new CRUIInterfaceLanguage(PROP_APP_INTERFACE_LANGUAGE_VALUE_SYSTEM, STR_INTERFACE_LANGUAGE_VALUE_SYSTEM, lString8()));
    interfaceLanguages.add(new CRUIInterfaceLanguage("en", STR_INTERFACE_LANGUAGE_VALUE_EN, i18nDir + "en.ini"));
    interfaceLanguages.add(new CRUIInterfaceLanguage("ru", STR_INTERFACE_LANGUAGE_VALUE_RU, i18nDir + "ru.ini"));
    hyphenationDictionaries.add(new CRUIHyphenationDictionary(PROP_HYPHENATION_DICT_VALUE_NONE, PROP_HYPHENATION_DICT_VALUE_NONE, STR_HYPHENATION_DICTIONARY_VALUE_NONE, lString8()));
    hyphenationDictionaries.add(new CRUIHyphenationDictionary(PROP_HYPHENATION_DICT_VALUE_ALGORITHM, PROP_HYPHENATION_DICT_VALUE_ALGORITHM, STR_HYPHENATION_DICTIONARY_VALUE_ALGORITHM, lString8()));
    hyphenationDictionaries.add(new CRUIHyphenationDictionary("en", "en", STR_HYPHENATION_DICTIONARY_VALUE_EN, hyphDir + "en.pattern"));
    hyphenationDictionaries.add(new CRUIHyphenationDictionary("ru", "ru", STR_HYPHENATION_DICTIONARY_VALUE_RU, hyphDir + "ru.pattern"));
    CRLog::info("resourceDir=%s", resourceDir.c_str());
    CRLog::info("i18nDir=%s", i18nDir.c_str());
    CRLog::info("hyphDir=%s", hyphDir.c_str());
    CRLog::info("cssDir=%s", cssDir.c_str());
    CRLog::info("themesDir=%s", themesDir.c_str());
    CRLog::info("manualsDir=%s", manualsDir.c_str());
}

static int nearestIconSize(int sz) {
    if (sz < 48)
        return 32;
    else if (sz < 64)
        return 48;
    else if (sz < 128)
        return 64;
    else if (sz < 256)
        return 128;
    else
        return 256;
}

void CRUIConfig::setupResourcesForScreenSize() {
	CRLog::trace("setupResourcesForScreenSize(%d,%d)", deviceInfo.shortSide, deviceInfo.longSide);

    // calculate fonts size
    int sz = deviceInfo.shortSide;
    int sz1 = sz / 38;
    //int sz2 = sz / 32;
    int sz3 = sz / 28;
    int sz4 = sz / 24;
    int sz5 = sz / 19;

    minFontSize = sz1;
    maxFontSize = sz / 8;
    if (maxFontSize > 180)
        maxFontSize = 180;
    if (minFontSize < 9)
        minFontSize = 9;
    defFontSize = sz3;

    // calculate folder icons size
    int folderIconSize;
    int menuIconSize;
    bool vertical = deviceInfo.width < deviceInfo.height * 85 / 100;
    if (vertical) {
        int nowReadingH = deviceInfo.height * 20 / 100;
        int recentH = deviceInfo.height * 25 / 100;
        int otherH = (deviceInfo.height - nowReadingH - recentH) / 3;
        folderIconSize = otherH - sz5*2 - sz4 - PT_TO_PX(4);
        menuIconSize = MIN_ITEM_PX * 2 / 3 - PT_TO_PX(4);
    } else {
        int nowReadingH = deviceInfo.height * 30 / 100;
        int recentH = deviceInfo.height * 40 / 100;
        int otherH = (deviceInfo.height - nowReadingH - recentH);
        folderIconSize = otherH - sz5*2 - sz4 - PT_TO_PX(4);
        menuIconSize = MIN_ITEM_PX * 2 / 3 - PT_TO_PX(4);
    }
    folderIconSize = nearestIconSize(folderIconSize);
    menuIconSize = nearestIconSize(menuIconSize);

    lString8Collection dirs;
    lString8Collection themedirs;

    char s[32];
    sprintf(s, "icons/%dx%d", menuIconSize, menuIconSize);
    dirs.add(resourceDir + s);
    sprintf(s, "folder_icons/%dx%d", folderIconSize, folderIconSize);
    dirs.add(resourceDir + s);

    if (deviceInfo.shortSide <= 420) {
    	CRLog::info("screen density normal");
        dirs.add(resourceDir + "screen-density-normal");
        dirs.add(resourceDir + "screen-density-high");
        dirs.add(resourceDir + "screen-density-xhigh");
        themedirs.add(currentThemeDir + "screen-density-normal");
        themedirs.add(currentThemeDir + "screen-density-high");
        themedirs.add(currentThemeDir + "screen-density-xhigh");
    } else if (deviceInfo.shortSide <= 480) {
    	CRLog::info("screen density high");
        dirs.add(resourceDir + "screen-density-high");
        dirs.add(resourceDir + "screen-density-xhigh");
        dirs.add(resourceDir + "screen-density-normal");
        themedirs.add(currentThemeDir + "screen-density-high");
        themedirs.add(currentThemeDir + "screen-density-xhigh");
        themedirs.add(currentThemeDir + "screen-density-normal");
    } else if (deviceInfo.shortSide <= 800) {
    	CRLog::info("screen density xhigh");
        dirs.add(resourceDir + "screen-density-xhigh");
        dirs.add(resourceDir + "screen-density-xxhigh");
        dirs.add(resourceDir + "screen-density-high");
        dirs.add(resourceDir + "screen-density-normal");
        themedirs.add(currentThemeDir + "screen-density-xhigh");
        themedirs.add(currentThemeDir + "screen-density-xxhigh");
        themedirs.add(currentThemeDir + "screen-density-high");
        themedirs.add(currentThemeDir + "screen-density-normal");
    } else {
    	CRLog::info("screen density xxhigh");
        dirs.add(resourceDir + "screen-density-xxhigh");
        dirs.add(resourceDir + "screen-density-xhigh");
        dirs.add(resourceDir + "screen-density-high");
        dirs.add(resourceDir + "screen-density-normal");
        themedirs.add(currentThemeDir + "screen-density-xxhigh");
        themedirs.add(currentThemeDir + "screen-density-xhigh");
        themedirs.add(currentThemeDir + "screen-density-high");
        themedirs.add(currentThemeDir + "screen-density-normal");
    }
    resourceResolver->setDirList(dirs);
    resourceResolver->setThemeDirList(themedirs);
    loadTheme();
}


/// loads theme from themesDirectory + "/" + theme + "/cr3theme.xml"; pass empty string to reload current theme
void CRUIConfig::setTheme(lString8 theme) {
    if (!theme.empty()) {
        if (theme.startsWith("@"))
            theme = theme.substr(1);
        currentThemeDir = themesDir + theme;
        LVAppendPathDelimiter(currentThemeDir);
    }
}

void CRUIConfig::loadTheme() {
    if (currentTheme)
        delete currentTheme;
    currentTheme = new CRUITheme(lString8("BLACK"));
    if (!currentTheme->loadFromFile(currentThemeDir + "cr3theme.xml")) {
        // cannot load! create default theme
        currentTheme->setTextColor(0x000000);
#if 0

        //currentTheme->setListDelimiterVertical(resourceResolver->getIcon("divider_light_v3.png"));
        //currentTheme->setListDelimiterVertical("list_delimiter_h.png");
        CRUIStyle * buttonStyle = currentTheme->addSubstyle("BUTTON");
        //keyboard_key_feedback_background.9
        buttonStyle->setBackground("btn_default_normal.9")->setFontSize(FONT_SIZE_LARGE);
        //buttonStyle->setBackground("keyboard_key_feedback_background.9")->setFontSize(FONT_SIZE_LARGE)->setPadding(10);
        //buttonStyle->setBackground("btn_default_normal.9")->setFontSize(FONT_SIZE_LARGE)->setPadding(10);
        buttonStyle->addSubstyle(STATE_PRESSED, STATE_PRESSED)->setBackground("btn_default_pressed.9");
        buttonStyle->addSubstyle(STATE_FOCUSED, STATE_FOCUSED)->setBackground("btn_default_selected.9");
        buttonStyle->addSubstyle(STATE_DISABLED, STATE_DISABLED)->setTextColor(0x80000000);

        buttonStyle = currentTheme->addSubstyle("BUTTON_NOBACKGROUND");
        buttonStyle->addSubstyle(STATE_PRESSED, STATE_PRESSED)->setBackground(0xC0C0C080);
        buttonStyle->addSubstyle(STATE_FOCUSED, STATE_FOCUSED)->setBackground(0xE0C0C080);
        buttonStyle->addSubstyle(STATE_DISABLED, STATE_DISABLED)->setTextColor(0x80000000);

        CRUIStyle * listItemStyle = currentTheme->addSubstyle("LIST_ITEM");
        listItemStyle->setMargin(0)->setPadding(7);
        listItemStyle->addSubstyle(STATE_FOCUSED, STATE_FOCUSED)->setBackground(0x40C0C080);
        listItemStyle->addSubstyle(STATE_DISABLED, STATE_DISABLED)->setTextColor(0x80000000);

        CRUIStyle * homeStyle = currentTheme->addSubstyle("HOME_WIDGET");
        homeStyle->setBackground("tx_wood_v3.jpg", true);
        homeStyle->setBackground2("list_shadow_vertical.9");

        CRUIStyle * fileListStyle = currentTheme->addSubstyle("FILE_LIST");
        fileListStyle->setBackground("tx_wood_v3.jpg", true);
        fileListStyle->setBackground2("list_shadow_vertical.9");
        fileListStyle->setListDelimiterVertical("divider_light_v3.png");

        CRUIStyle * homeListCaption = currentTheme->addSubstyle("HOME_LIST_CAPTION");
        homeListCaption->setTextColor(0x00402000);
        homeListCaption->setFontSize(CRUI::FONT_SIZE_SMALL);

        CRUIStyle * toolbar = currentTheme->addSubstyle("TOOL_BAR");
       toolbar->setBackground("tx_wood_v3.jpg", true);
        toolbar->setBackground2("toolbar_shadow.9");
        toolbar->setFontSize(CRUI::FONT_SIZE_SMALL);

        CRUIStyle * popupframe = currentTheme->addSubstyle("POPUP_FRAME");
        popupframe->setBackground("tx_wood_v3.jpg", true);
        popupframe->setBackground2("toolbar_shadow.9");
        popupframe->setPadding(PT_TO_PX(2));

        CRUIStyle * popupframehandle = currentTheme->addSubstyle("POPUP_FRAME_HANDLE");
        popupframehandle->setPadding(PT_TO_PX(4));
        popupframehandle->setMargin(PT_TO_PX(0));
        popupframehandle->setBackground(0xE0808080);
        popupframehandle->setBackground("home_frame.9");

        CRUIStyle * menulist = currentTheme->addSubstyle("MENU_LIST");
        menulist->setListDelimiterVertical("divider_light_v3");
        CRUIStyle * menuitem = currentTheme->addSubstyle("MENU_ITEM");
        menuitem->setPadding(PT_TO_PX(0));
        CRUIStyle * menuitemicon = currentTheme->addSubstyle("MENU_ITEM_ICON");
        menuitemicon->setMargin(PT_TO_PX(1));
        menuitemicon->setAlign(ALIGN_CENTER);
        CRUIStyle * menuitemtext = currentTheme->addSubstyle("MENU_ITEM_TEXT");
        menuitemtext->setMargin(PT_TO_PX(2));
        menuitemtext->setFontSize(CRUI::FONT_SIZE_MEDIUM);
        menuitemtext->setAlign(ALIGN_VCENTER | ALIGN_LEFT);

        CRUIStyle * settingsWindow = currentTheme->addSubstyle("SETTINGS_WIDGET");
        settingsWindow->setBackground("tx_wood_v3.jpg", true);
        settingsWindow->setBackground2("list_shadow_vertical.9");
        CRUIStyle * settingsList = currentTheme->addSubstyle("SETTINGS_ITEM_LIST");
        settingsList->setListDelimiterVertical(CRUIImageRef(new CRUISolidFillImage(0x60A08060, 2)));
        //settingsList->setListDelimiterVertical("divider_light_v3");
        CRUIStyle * settingsItem = currentTheme->addSubstyle("SETTINGS_ITEM");
        settingsItem->setPadding(lvRect(PT_TO_PX(3), PT_TO_PX(0), PT_TO_PX(3), PT_TO_PX(0)));
        settingsItem->setMinHeight(MIN_ITEM_PX);
        CRUIStyle * settingsItemTitle = currentTheme->addSubstyle("SETTINGS_ITEM_TITLE");
        settingsItemTitle->setPadding(lvRect(PT_TO_PX(3), PT_TO_PX(0), PT_TO_PX(3), PT_TO_PX(0)));
        //settingsItemTitle->setPadding(PT_TO_PX(2));
        settingsItemTitle->setFontSize(FONT_SIZE_LARGE);
        settingsItemTitle->setAlign(ALIGN_VCENTER | ALIGN_LEFT);
        CRUIStyle * settingsItemLayout = currentTheme->addSubstyle("SETTINGS_ITEM_TEXT_LAYOUT");
        settingsItemLayout->setPadding(PT_TO_PX(2));
        settingsItemLayout->setAlign(ALIGN_VCENTER | ALIGN_LEFT);
        CRUIStyle * settingsItemIcon = currentTheme->addSubstyle("SETTINGS_ITEM_ICON");
        settingsItemIcon->setAlign(ALIGN_CENTER);
        settingsItemIcon->setMargin(PT_TO_PX(3));
        CRUIStyle * settingsItemDescription = currentTheme->addSubstyle("SETTINGS_ITEM_DESCRIPTION");
        settingsItemDescription->setFontSize(FONT_SIZE_SMALL);
        //settingsItemDescription->setPadding(PT_TO_PX(2));
        settingsItemDescription->setAlign(ALIGN_BOTTOM | ALIGN_LEFT);
#endif
    }

    resourceResolver->setIconColorTransform(currentTheme->getColor(COLOR_ID_ICON_COLOR_TRANSFORM_BRIGHTNESS), currentTheme->getColor(COLOR_ID_ICON_COLOR_TRANSFORM_CONTRAST));

    lvRect hardcoverClientRect(53, 29, 319, 378); // area where to place cover image
    lvRect hardcoverNeutralRect(60, 12, 300, 20); // area with neutral color
    coverPageManager->setCoverPageTemplate(resourceResolver->getImageSource("hardcover"), hardcoverClientRect, hardcoverNeutralRect);
}

/// on GL context close, delete all GL objects
void CRUIConfig::clearGraphicsCaches() {
	CRLog::trace("CRUIConfig::clearGraphicsCaches()");
	//coverPageManager->cancelAll();
	//coverPageManager->clearImageCache();
	resourceResolver->clearImageCache();
	LVGLClearImageCache();
	fontMan->clearGlyphCache();
}

void CRUIConfig::initEngine(bool setLogger) {

    if (systemLanguage.empty())
        systemLanguage = "en";

    LVAppendPathDelimiter(hyphDir);
    LVAppendPathDelimiter(cssDir);
    LVAppendPathDelimiter(resourceDir);
    LVAppendPathDelimiter(coverCacheDir);
    LVAppendPathDelimiter(docCacheDir);
    LVAppendPathDelimiter(i18nDir);

    // Logger
    if (setLogger) {
		if (!logFile.empty())
			CRLog::setFileLogger(logFile.c_str(), true);
		else
			CRLog::setStderrLogger();
    }
    CRLog::setLogLevel(CRLog::LL_TRACE);

    // Concurrency
    CRSetupEngineConcurrency();

    InitFontManager(lString8());
    LVInitGLFontManager(fontMan);
    for (int i = 0; i<fontFiles.length(); i++) {
        fontMan->RegisterFont(fontFiles[i]);
    }
    lString8 fallbackFont = fallbackFontFace;
    bool configuredFallbackFontFound = false;
    lString8 foundFallbackFont;
    lString16Collection faceList;
    fontMan->getFaceList(faceList);
    for (int i = 0; i < faceList.length(); i++) {
    	lString8 face = UnicodeToUtf8(faceList[i]);
    	if (face == fallbackFont)
    		configuredFallbackFontFound = true;
    	if (face == "Droid Sans Fallback" || face == "Arial Unicode") // TODO: more faces
    		foundFallbackFont = face;
    }
    if (!configuredFallbackFontFound)
    	fallbackFont = foundFallbackFont;
    if (!fallbackFont.empty()) {
    	fontMan->SetFallbackFontFace(fallbackFont);
    }


    //fontMan->SetFallbackFontFace(lString8("Tizen Sans Fallback"));
    //dirs.add(UnicodeToUtf8(resourceDir));
    lString8Collection dirs;
    dirs.add(resourceDir + "screen-density-xhigh");
    LVCreateResourceResolver(dirs);
    LVGLCreateImageCache();

    // coverpage file cache
    CRSetupCoverpageManager(Utf8ToUnicode(coverCacheDir), coverDirMaxItems, coverDirMaxFiles, coverDirMaxSize, coverRenderCacheMaxItems, coverRenderCacheMaxBytes);

    // document cache
    ldomDocCache::init(Utf8ToUnicode(docCacheDir), docCacheMaxBytes);

    // I18N
    CRIniFileTranslator * fallbackTranslator = CRIniFileTranslator::create((i18nDir + "en.ini").c_str());
    CRIniFileTranslator * mainTranslator = CRIniFileTranslator::create((i18nDir + "en.ini").c_str());
    CRI18NTranslator::setTranslator(mainTranslator);
    CRI18NTranslator::setDefTranslator(fallbackTranslator);

    HyphMan::initDictionaries(lString16(), true);

    bookDB = new CRBookDB();
    if (bookDB->open(dbFile.c_str()))
        CRLog::error("Error while opening DB file");
    if (!bookDB->updateSchema())
        CRLog::error("Error while updating DB schema");
    if (!bookDB->fillCaches())
        CRLog::error("Error while filling caches");

    CRSetupDirectoryCacheManager();

    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@paper1"), lString8(STR_RESOURCE_BACKGROUND_NAME_PAPER2), lString8("tx_paper.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@paper1_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_PAPER2_DARK), lString8("tx_paper_dark.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@oldbook1"), lString8(STR_RESOURCE_BACKGROUND_NAME_OLDBOOK1), lString8("tx_old_book.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@oldbook1_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_OLDBOOK1_DARK), lString8("tx_old_book_dark.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@oldbook2"), lString8(STR_RESOURCE_BACKGROUND_NAME_OLDBOOK2), lString8("tx_old_paper.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@oldbook2_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_OLDBOOK2_DARK), lString8("tx_old_paper_dark.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@fabric1"), lString8(STR_RESOURCE_BACKGROUND_NAME_FABRIC1), lString8("tx_fabric.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@fabric1_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_FABRIC1_DARK), lString8("tx_fabric_dark.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@fabric2"), lString8(STR_RESOURCE_BACKGROUND_NAME_FABRIC2), lString8("tx_fabric_indigo_fibre.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@fabric2_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_FABRIC2_DARK), lString8("tx_fabric_indigo_fibre_dark.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@sand1"), lString8(STR_RESOURCE_BACKGROUND_NAME_SAND1), lString8("tx_sand.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@sand1_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_SAND1_DARK), lString8("tx_sand_dark.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@sand2"), lString8(STR_RESOURCE_BACKGROUND_NAME_SAND2), lString8("tx_gray_sand.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@sand2_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_SAND2_DARK), lString8("tx_gray_sand_dark.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@wall1"), lString8(STR_RESOURCE_BACKGROUND_NAME_WALL1), lString8("tx_green_wall.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@wall1_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_WALL1_DARK), lString8("tx_green_wall_dark.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@stones1"), lString8(STR_RESOURCE_BACKGROUND_NAME_STONES1), lString8("tx_stones.jpg")));
    resourceResolver->addBackground(new CRUIBackgroundImageResource(lString8("@stones1_dark"), lString8(STR_RESOURCE_BACKGROUND_NAME_STONES1_DARK), lString8("tx_stones_dark.jpg")));

    setTheme(lString8("light"));
    setupResourcesForScreenSize();
}

bool CRUIConfig::setHyphenationDictionary(lString8 id, lString8 fallbackId) {
    if (id == PROP_HYPHENATION_DICT_VALUE_NONE) {
        HyphMan::activateDictionary(lString16(HYPH_DICT_ID_NONE));
        return true;
    }
    if (id == PROP_HYPHENATION_DICT_VALUE_ALGORITHM) {
        HyphMan::activateDictionary(lString16(HYPH_DICT_ID_ALGORITHM));
        return true;
    }
    CRUIHyphenationDictionary * dict = findHyphenationDictionary(id);
    if (!dict && id.length() > 2) {
        id = id.substr(0, 2);
        dict = findHyphenationDictionary(id);
    }
    if (!dict)
        dict = findHyphenationDictionary(fallbackId);
    if (!dict && fallbackId.length() > 2) {
        fallbackId = fallbackId.substr(0, 2);
        dict = findHyphenationDictionary(fallbackId);
    }
    if (!dict) {
        return false;
    }
    LVStreamRef stream = LVOpenFileStream(dict->fileName.c_str(), LVOM_READ);
    if (stream.isNull())
        return false;
    return HyphMan::activateDictionaryFromStream(stream);
}

CRUIHyphenationDictionary * CRUIConfig::findHyphenationDictionary(lString8 id) {
    for (int i = 0; i < hyphenationDictionaries.length(); i++) {
        if (hyphenationDictionaries[i]->id == id)
            return hyphenationDictionaries[i];
    }
    return NULL;
}

CRUIInterfaceLanguage * CRUIConfig::findInterfaceLanguage(lString8 id) {
    if (id == PROP_APP_INTERFACE_LANGUAGE_VALUE_SYSTEM)
        id = systemLanguage;
    for (int i = 0; i < interfaceLanguages.length(); i++) {
        if (interfaceLanguages[i]->id == id)
            return interfaceLanguages[i];
    }
    return NULL;
}

void CRUIConfig::setInterfaceLanguage(lString8 id) {
    CRUIInterfaceLanguage * lang = findInterfaceLanguage(id);
    if (!lang)
        lang = findInterfaceLanguage(lString8("en"));
    if (lang) {
        CRIniFileTranslator * mainTranslator = CRIniFileTranslator::create(lang->fileName.c_str());
        CRI18NTranslator::setTranslator(mainTranslator);
    }
}

void CRUIConfig::uninitEngine() {
    CRStopCoverpageManager();
    CRStopDirectoryCacheManager();
    if (bookDB) {
        bookDB->close();
        delete bookDB;
        bookDB = NULL;
    }
}

lString16 CRUIInterfaceLanguage::getName() {
    if (nameRes.empty())
        return name;
    return _16(nameRes.c_str());
}

lString16 CRUIHyphenationDictionary::getName() {
    if (nameRes.empty())
        return name;
    return _16(nameRes.c_str());
}
