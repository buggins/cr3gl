#include "cruiconfig.h"
#include "lvstream.h"
#include "glfont.h"
#include "gldrawbuf.h"
#include "fileinfo.h"
#include "lvtinydom.h"
#include "cruitheme.h"
#include "crui.h"
#include "crcoverpages.h"

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
    crconfig.resourceDir = baseDir;
    crconfig.i18nDir = baseDir + "i18n";
    crconfig.hyphDir = baseDir + "hyph";
}

void CRUIConfig::setupResourcesForScreenSize() {
    lString8Collection dirs;
    if (deviceInfo.shortSide <= 320) {
        dirs.add(resourceDir + "screen-density-normal");
        dirs.add(resourceDir + "screen-density-high");
        dirs.add(resourceDir + "screen-density-xhigh");
    } else if (deviceInfo.shortSide <= 480) {
        dirs.add(resourceDir + "screen-density-high");
        dirs.add(resourceDir + "screen-density-xhigh");
        dirs.add(resourceDir + "screen-density-normal");
    } else {
        dirs.add(resourceDir + "screen-density-xhigh");
        dirs.add(resourceDir + "screen-density-high");
        dirs.add(resourceDir + "screen-density-normal");
    }
    resourceResolver->setDirList(dirs);
    int sz = deviceInfo.shortSide;
    int sz1 = sz / 35;
    int sz2 = sz / 26;
    int sz3 = sz / 22;
    int sz4 = sz / 17;
    int sz5 = sz / 14;
    currentTheme->setFontForSize(CRUI::FONT_SIZE_XSMALL, fontMan->GetFont(sz1, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_SMALL, fontMan->GetFont(sz2, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_MEDIUM, fontMan->GetFont(sz3, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_LARGE, fontMan->GetFont(sz4, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_XLARGE, fontMan->GetFont(sz5, 400, false, css_ff_sans_serif, lString8("Arial"), 0));
}


void createDefaultTheme() {
    currentTheme = new CRUITheme(lString8("BLACK"));
    currentTheme->setTextColor(0x000000);
    currentTheme->setFontForSize(CRUI::FONT_SIZE_XSMALL, fontMan->GetFont(PT_TO_PX(6), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_SMALL, fontMan->GetFont(PT_TO_PX(8), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_MEDIUM, fontMan->GetFont(PT_TO_PX(12), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_LARGE, fontMan->GetFont(PT_TO_PX(16), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_XLARGE, fontMan->GetFont(PT_TO_PX(22), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));

    //currentTheme->setListDelimiterVertical(resourceResolver->getIcon("divider_light_v3.png"));
    currentTheme->setListDelimiterVertical(resourceResolver->getIcon("list_delimiter_h.png"));
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
    fileListStyle->setListDelimiterVertical(resourceResolver->getIcon("divider_light_v3.png"));

    CRUIStyle * homeListCaption = currentTheme->addSubstyle("HOME_LIST_CAPTION");
    //homeListCaption->setTextColor(0x40000000);
    homeListCaption->setTextColor(0x00402000);
    homeListCaption->setFontSize(CRUI::FONT_SIZE_SMALL);

    CRUIStyle * toolbar = currentTheme->addSubstyle("TOOL_BAR");
    //homeListCaption->setTextColor(0x40000000);
    toolbar->setBackground("tx_wood_v3.jpg", true);
    toolbar->setBackground2("toolbar_shadow.9");
    toolbar->setFontSize(CRUI::FONT_SIZE_SMALL);
}

void CRUIConfig::initEngine() {

    LVAppendPathDelimiter(hyphDir);
    LVAppendPathDelimiter(resourceDir);
    LVAppendPathDelimiter(coverCacheDir);
    LVAppendPathDelimiter(docCacheDir);
    LVAppendPathDelimiter(i18nDir);

    // Logger
    if (!logFile.empty())
        CRLog::setFileLogger(logFile.c_str(), true);
    CRLog::setLogLevel(CRLog::LL_TRACE);

    // Concurrency
    CRSetupEngineConcurrency();

    InitFontManager(lString8());
    LVInitGLFontManager(fontMan);
    for (int i = 0; i<fontFiles.length(); i++) {
        fontMan->RegisterFont(fontFiles[i]);
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
    CRIniFileTranslator * mainTranslator = CRIniFileTranslator::create((i18nDir + "ru.ini").c_str());
    CRI18NTranslator::setTranslator(mainTranslator);
    CRI18NTranslator::setDefTranslator(fallbackTranslator);

    bookDB = new CRBookDB();
    if (bookDB->open(dbFile.c_str()))
        CRLog::error("Error while opening DB file");
    if (!bookDB->updateSchema())
        CRLog::error("Error while updating DB schema");
    if (!bookDB->fillCaches())
        CRLog::error("Error while filling caches");

    CRSetupDirectoryCacheManager();

    createDefaultTheme();
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
