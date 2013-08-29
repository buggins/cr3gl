
#include "cr3qt.h"


#include <gldrawbuf.h>
#include <crui.h>
#include <lvstring.h>
#include <lvfntman.h>
#include <gldrawbuf.h>
#include <glfont.h>
#include <fileinfo.h>
#include <crconcurrent.h>
#include <crcoverpages.h>

#include <QtCore/QCoreApplication>
#include <QtGui/QMouseEvent>


CRUIEventAdapter::CRUIEventAdapter(CRUIEventManager * eventManager) : _eventManager(eventManager), _activePointer(NULL)
{

}

lUInt64 GetCurrentTimeMillis() {
#if defined(LINUX) || defined(ANDROID) || defined(_LINUX)
    timeval ts;
    gettimeofday(&ts, NULL);
    return ts.tv_sec * (lUInt64)1000 + ts.tv_usec / 1000;
#else
 #ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return (ft.dwLowDateTime | ((lInt64)ft.dwHighDateTime << 32)) / 1000;
 #else
 #error * You should define GetCurrentTimeMillis() *
 #endif
#endif
}

using namespace CRUI;

static lUInt32 pointerId = 1;
void CRUIEventAdapter::dispatchTouchEvent(QMouseEvent * event)
{
    int x = event->x();
    int y = event->y();
    int type = event->type();
    // CRLog::trace("dispatchTouchEvent() %d  %d,%d", type, x, y);
    if (type == QEvent::MouseButtonPress)
        pointerId++;
//    if (type == QEvent::MouseMove && !event->buttons())
//        return; // ignore not pressed moves
    unsigned long pointId = pointerId; //touchInfo.GetPointId();
    int action = 0;
    switch (type) {
    case QEvent::MouseButtonPress: //The touch pressed event type
        action = ACTION_DOWN; break;
    case QEvent::MouseButtonRelease: //The touch released event type
        action = ACTION_UP; break;
    case QEvent::MouseMove: //The touch moved event type
        action = ACTION_MOVE; break;
    }
    if (action) {
        lUInt64 ts = GetCurrentTimeMillis();
        //CRLog::trace("mouse event ts %lld", ts);
        bool isFirst = (action == ACTION_DOWN);
        bool isLast = (action == ACTION_CANCEL || action == ACTION_UP);
        if (isFirst) {
            if (_activePointer) {
                // cancel active pointer
                CRUIMotionEventItem * item = new CRUIMotionEventItem(_activePointer, _activePointer->getPointerId(), ACTION_CANCEL, x, y, ts);
                CRUIMotionEvent * event = new CRUIMotionEvent();
                event->addEvent(item);
                _eventManager->dispatchTouchEvent(event);
                delete item;
                delete event;
                delete _activePointer;
                _activePointer = NULL;
            }
        }
        CRUIMotionEventItem * lastItem = _activePointer;
        if (!lastItem && !isFirst) {
            //CRLog::warn("Ignoring unexpected touch event %d with id%lld", action, pointId);
            return;
        }
        CRUIMotionEventItem * item = new CRUIMotionEventItem(lastItem, pointId, action, x, y, ts);
        if (lastItem) {
            if (_activePointer) {
                delete _activePointer;
                _activePointer = NULL;
            }
            if (!isLast) {
                _activePointer = item;
            }
        } else {
            if (!isLast) {
                _activePointer = item;
            }
        }
        CRUIMotionEvent * event = new CRUIMotionEvent();
        event->addEvent(item);
        _eventManager->dispatchTouchEvent(event);
        delete event;
        if (isLast)
            delete item;
    }
}



lString16 resourceDir;
void setupResourcesForScreenSize() {
    lString8 resDir8 = UnicodeToUtf8(resourceDir);
    lString8Collection dirs;
    if (deviceInfo.shortSide <= 320) {
        dirs.add(resDir8 + "screen-density-normal");
        dirs.add(resDir8 + "screen-density-high");
        dirs.add(resDir8 + "screen-density-xhigh");
    } else if (deviceInfo.shortSide <= 480) {
        dirs.add(resDir8 + "screen-density-high");
        dirs.add(resDir8 + "screen-density-xhigh");
        dirs.add(resDir8 + "screen-density-normal");
    } else {
        dirs.add(resDir8 + "screen-density-xhigh");
        dirs.add(resDir8 + "screen-density-high");
        dirs.add(resDir8 + "screen-density-normal");
    }
    resourceResolver->setDirList(dirs);
}

QOpenGLFunctions * _qtgl = NULL;



void UninitCREngine() {
    if (coverCache) {
        delete coverCache;
        coverCache = NULL;
    }
    if (bookDB) {
        bookDB->close();
        delete bookDB;
        bookDB = NULL;
    }
}

void InitCREngine(lString16 exePath) {
    // Logger
    lString16 logfile = exePath + L"cr3.log";
    CRLog::setFileLogger(LCSTR(logfile), true);
    CRLog::setLogLevel(CRLog::LL_TRACE);
    // Concurrency
    concurrencyProvider = new QtConcurrencyProvider();

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

    // coverpage file cache
    lString16 coverCacheDir = exePath + L"coverpages";
    coverCache = new CRCoverFileCache(coverCacheDir);
    coverCache->open();
    coverImageCache = new CRCoverImageCache();
    coverPageManager = new CRCoverPageManager();

    // document cache
    lString16 docCacheDir = exePath + L"cache";
    ldomDocCache::init(docCacheDir, 32*1024*1024);

    // I18N
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
//    lString8 dir("c:\\Shared\\Books");
//    CRDirCacheItem * cachedir = dirCache->getOrAdd(dir);
//    cachedir->refresh();

    deviceInfo.topDirs.addItem(DIR_TYPE_INTERNAL_STORAGE, lString8("c:\\"));
    deviceInfo.topDirs.addItem(DIR_TYPE_SD_CARD, lString8("c:\\"));
    deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, lString8("c:\\"));
    deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, lString8("d:\\"));
    deviceInfo.topDirs.addItem(DIR_TYPE_FAVORITE, lString8("c:\\Shared\\Books"));
    deviceInfo.topDirs.addItem(DIR_TYPE_CURRENT_BOOK_DIR, lString8("c:\\Shared\\Books"));
    deviceInfo.topDirs.addItem(DIR_TYPE_DOWNLOADS, lString8("c:\\Shared\\Books\\Downloads"));

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
    homeStyle->setBackground(resourceResolver->getIcon("tx_wood_v3.jpg", true));

    CRUIStyle * fileListStyle = currentTheme->addSubstyle("FILE_LIST");
    fileListStyle->setBackground(resourceResolver->getIcon("tx_wood_v3.jpg", true));
    fileListStyle->setListDelimiterVertical(resourceResolver->getIcon("divider_light_v3.png"));

    CRUIStyle * homeListCaption = currentTheme->addSubstyle("HOME_LIST_CAPTION");
    //homeListCaption->setTextColor(0x40000000);
    homeListCaption->setTextColor(0x00402000);
    homeListCaption->setFontSize(CRUI::FONT_SIZE_SMALL);
}

