#include "cr3mainwindow.h"
#include <QApplication>

#include <lvstring.h>
#include <lvfntman.h>
#include <crui.h>
#include <gldrawbuf.h>
#include <glfont.h>
#include <fileinfo.h>

using namespace CRUI;

void InitCREngine(lString16 exePath) {
    lString16 logfile = exePath + L"cr3.log";
    CRLog::setFileLogger(LCSTR(logfile), true);
    CRLog::setLogLevel(CRLog::LL_TRACE);

    InitFontManager(lString8());
    LVInitGLFontManager(fontMan);
    fontMan->RegisterFont(lString8("C:\\Windows\\Fonts\\arial.ttf"));
    fontMan->RegisterFont(lString8("C:\\Windows\\Fonts\\ariali.ttf"));
    fontMan->RegisterFont(lString8("C:\\Windows\\Fonts\\arialbd.ttf"));
    fontMan->RegisterFont(lString8("C:\\Windows\\Fonts\\arialbi.ttf"));
    //fontMan->SetFallbackFontFace(lString8("Tizen Sans Fallback"));
    //dirs.add(UnicodeToUtf8(resourceDir));
    resourceDir = exePath + L"res\\";
    lString8 resDir8 = UnicodeToUtf8(resourceDir);
    lString8Collection dirs;
    dirs.add(resDir8 + "screen-density-xhigh");
    LVCreateResourceResolver(dirs);
    LVGLCreateImageCache();
    CRIniFileTranslator * fallbackTranslator = CRIniFileTranslator::create((resDir8 + "/i18n/en.ini").c_str());
    CRIniFileTranslator * mainTranslator = CRIniFileTranslator::create((resDir8 + "/i18n/ru.ini").c_str());
    CRI18NTranslator::setTranslator(mainTranslator);
    CRI18NTranslator::setDefTranslator(fallbackTranslator);

    lString8 dbFile = UnicodeToUtf8(exePath) + "cr3db.sqlite13";
    bookDB = new CRBookDB();
    if (bookDB->open(dbFile.c_str()))
        CRLog::error("Error while opening DB file");
    if (!bookDB->updateSchema())
        CRLog::error("Error while updating DB schema");
    if (!bookDB->fillCaches())
        CRLog::error("Error while filling caches");

    dirCache = new CRDirCache();
    lString8 dir("C:\\Shared\\Books");
    CRDirCacheItem * cachedir = dirCache->getOrAdd(dir);
    cachedir->refresh();

    currentTheme = new CRUITheme(lString8("BLACK"));
    currentTheme->setTextColor(0x000000);
    currentTheme->setFontForSize(CRUI::FONT_SIZE_XSMALL, fontMan->GetFont(PT_TO_PX(6), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_SMALL, fontMan->GetFont(PT_TO_PX(8), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_MEDIUM, fontMan->GetFont(PT_TO_PX(12), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_LARGE, fontMan->GetFont(PT_TO_PX(16), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
    currentTheme->setFontForSize(CRUI::FONT_SIZE_XLARGE, fontMan->GetFont(PT_TO_PX(22), 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));

    currentTheme->setListDelimiterVertical(resourceResolver->getIcon("divider_light_v3.png"));
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
    homeStyle->setBackground(resourceResolver->getIcon("tx_wood_v3.jpg", true));

    CRUIStyle * fileListStyle = currentTheme->addSubstyle("FILE_LIST");
    fileListStyle->setBackground(resourceResolver->getIcon("tx_wood_v3.jpg", true));
    fileListStyle->setListDelimiterVertical(resourceResolver->getIcon("divider_light_v3.png"));
}

int main(int argc, char *argv[])
{
    lString16 exePath = LVExtractPath(Utf8ToUnicode(argv[0]));
    LVAppendPathDelimiter(exePath);
    InitCREngine(exePath);
    QApplication a(argc, argv);
    OpenGLWindow w;
    w.show();
    
    return a.exec();
}
