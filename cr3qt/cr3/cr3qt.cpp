
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
#include <QtCore/QDir>
#include <QtGui/QMouseEvent>

#ifdef _WIN32
    #include <ShlObj.h>
    #pragma comment(lib, "shell32.lib")
#endif
#ifdef _LINUX
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif


CRUIEventAdapter::CRUIEventAdapter(CRUIEventManager * eventManager) : _eventManager(eventManager), _activePointer(NULL)
{

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
        if (isLast || (item && item->isCancelled())) {
            if (_activePointer == item)
                _activePointer = NULL;
            delete item;
        }
    }
}

static int translateKeyCode(int key) {
    switch(key) {
    case Qt::Key_F1: return CR_KEY_F1;
    case Qt::Key_F2: return CR_KEY_F2;
    case Qt::Key_F3: return CR_KEY_F3;
    case Qt::Key_F4: return CR_KEY_F4;
    case Qt::Key_F5: return CR_KEY_F5;
    case Qt::Key_F6: return CR_KEY_F6;
    case Qt::Key_F7: return CR_KEY_F7;
    case Qt::Key_F8: return CR_KEY_F8;
    case Qt::Key_F9: return CR_KEY_F9;
    case Qt::Key_F10: return CR_KEY_F10;
    case Qt::Key_F11: return CR_KEY_F11;
    case Qt::Key_F12: return CR_KEY_F12;
    case Qt::Key_0: return CR_KEY_0;
    case Qt::Key_1: return CR_KEY_1;
    case Qt::Key_2: return CR_KEY_2;
    case Qt::Key_3: return CR_KEY_3;
    case Qt::Key_4: return CR_KEY_4;
    case Qt::Key_5: return CR_KEY_5;
    case Qt::Key_6: return CR_KEY_6;
    case Qt::Key_7: return CR_KEY_7;
    case Qt::Key_8: return CR_KEY_8;
    case Qt::Key_9: return CR_KEY_9;
    case Qt::Key_Space: return CR_KEY_SPACE;
    case Qt::Key_Shift: return CR_KEY_SHIFT;
    case Qt::Key_Alt: return CR_KEY_SHIFT;
    case Qt::Key_Meta: return CR_KEY_SHIFT;

    case Qt::Key_Return: return CR_KEY_RETURN;
    case Qt::Key_Home: return CR_KEY_HOME;
    case Qt::Key_End: return CR_KEY_END;
    case Qt::Key_PageUp: return CR_KEY_PGUP;
    case Qt::Key_PageDown: return CR_KEY_PGDOWN;
    case Qt::Key_Left: return CR_KEY_LEFT;
    case Qt::Key_Up: return CR_KEY_UP;
    case Qt::Key_Right: return CR_KEY_RIGHT;
    case Qt::Key_Down: return CR_KEY_DOWN;

    case Qt::Key_Escape: return CR_KEY_ESC;
    case Qt::Key_Delete: return CR_KEY_DELETE;
    case Qt::Key_Backspace: return CR_KEY_BACKSPACE;
    case Qt::Key_Back: return CR_KEY_BACK;
    case Qt::Key_Tab: return CR_KEY_TAB;
    case Qt::Key_Backtab: return CR_KEY_TAB;
    default:
        return 0;
    }
}

static lUInt32 translateModifiers(lUInt32 flags) {
    lUInt32 res = 0;
    if (flags & Qt::ShiftModifier) res |= CR_KEY_MODIFIER_SHIFT;
    if (flags & Qt::ControlModifier) res |= CR_KEY_MODIFIER_CONTROL;
    if (flags & Qt::AltModifier) res |= CR_KEY_MODIFIER_ALT;
    if (flags & Qt::MetaModifier) res |= CR_KEY_MODIFIER_META;
    if (flags & Qt::KeypadModifier) res |= CR_KEY_MODIFIER_KEYPAD;
    return res;
}

// key event listener
void CRUIEventAdapter::dispatchKeyEvent(QKeyEvent * event) {
    KEY_EVENT_TYPE type = event->type() == QEvent::KeyPress ? KEY_ACTION_PRESS : KEY_ACTION_RELEASE;
    QString s = event->text();
    int key = translateKeyCode(event->key());
    if (!key && !s.length()) {
        CRLog::warn("Skipping unknown key %d w/o text", event->key());
        return;
    }
    if (key == 0) {
        key = 0x1000 + event->key();
    }
    bool autorepeat = event->isAutoRepeat();
    int count = event->count();
    lUInt32 modifiers = translateModifiers(event->modifiers());
    CRLog::trace("QKeyEvent type=%d srckey=%08x key=%04x srcmodifiers=%08x modifiers=%04x", type, event->key(), key, event->modifiers(), modifiers);
    CRUIKeyEvent * ev = new CRUIKeyEvent(type, key, autorepeat, count, modifiers);
    if (s.length()) {
        lString16 txt = Utf8ToUnicode(s.toUtf8().constData());
        ev->setText(txt);
    }
    _eventManager->dispatchKeyEvent(ev);
    delete ev;
}

//QOpenGLFunctions * _qtgl = NULL;



#include "cruiconfig.h"

void InitCREngine(lString16 exePath) {
    // Logger
#ifdef _WIN32
    crconfig.logFile = UnicodeToUtf8(exePath + L"cr3.log");
    CRLog::setFileLogger(crconfig.logFile.c_str(), true);
#else
    CRLog::setStderrLogger();
#endif
    CRLog::setLogLevel(CRLog::LL_TRACE);
    // fill config parameters

    // Concurrency
    concurrencyProvider = new QtConcurrencyProvider();

#ifdef _WIN32
    lString8 downloadsPath;
    lString8 fontsPath;

    WCHAR szPath[MAX_PATH];

    if(SUCCEEDED(SHGetFolderPathW(NULL,
                                 CSIDL_FONTS|CSIDL_FLAG_NO_ALIAS|CSIDL_FLAG_DONT_UNEXPAND,
                                 NULL,
                                 0,
                                 szPath)))
    {
        fontsPath = UnicodeToUtf8(szPath);
    }
    if(SUCCEEDED(SHGetFolderPathW(NULL,
                                  CSIDL_PROFILE|CSIDL_FLAG_NO_ALIAS|CSIDL_FLAG_DONT_UNEXPAND|CSIDL_FLAG_CREATE,
                                 NULL,
                                 0,
                                 szPath)))
    {
        downloadsPath = UnicodeToUtf8(szPath) + "\\Downloads";
    }

//    PWSTR downpathptr = NULL;
//    if (S_OK == SHGetKnownFolderPath(FOLDERID_Downloads, KF_FLAG_NO_ALIAS|KF_FLAG_DONT_UNEXPAND, NULL, &downpathptr)) {
//        downloadsPath = UnicodeToUtf8(downpathptr);
//    }
//    lString8 fontsPath;
//    PWSTR fontspathptr = NULL;
//    if (S_OK == SHGetKnownFolderPath(FOLDERID_Fonts, KF_FLAG_NO_ALIAS|KF_FLAG_DONT_UNEXPAND, NULL, &fontspathptr)) {
//        fontsPath = UnicodeToUtf8(fontspathptr);
//    }
    CRLog::info("Downloads path: %s", downloadsPath.c_str());
    CRLog::info("Fonts path: %s", fontsPath.c_str());
    if (fontsPath.empty())
        fontsPath = "C:\\Windows\\Fonts";

    crconfig.fontFiles.add(fontsPath + "\\arial.ttf");
    crconfig.fontFiles.add(fontsPath + "\\ariali.ttf");
    crconfig.fontFiles.add(fontsPath + "\\arialbd.ttf");
    crconfig.fontFiles.add(fontsPath + "\\arialbi.ttf");
    crconfig.fontFiles.add(fontsPath + "\\tahoma.ttf");
    crconfig.fontFiles.add(fontsPath + "\\tahomabd.ttf");
    crconfig.fontFiles.add(fontsPath + "\\comic.ttf");
    crconfig.fontFiles.add(fontsPath + "\\comicbd.ttf");
    crconfig.fontFiles.add(fontsPath + "\\calibri.ttf");
    crconfig.fontFiles.add(fontsPath + "\\calibrib.ttf");
    crconfig.fontFiles.add(fontsPath + "\\calibrii.ttf");
    crconfig.fontFiles.add(fontsPath + "\\calibrili.ttf");
    crconfig.fontFiles.add(fontsPath + "\\cour.ttf");
    crconfig.fontFiles.add(fontsPath + "\\courbd.ttf");
    crconfig.fontFiles.add(fontsPath + "\\couri.ttf");
    crconfig.fontFiles.add(fontsPath + "\\courbi.ttf");
    crconfig.fontFiles.add(fontsPath + "\\times.ttf");
    crconfig.fontFiles.add(fontsPath + "\\timesi.ttf");
    crconfig.fontFiles.add(fontsPath + "\\timesbd.ttf");
    crconfig.fontFiles.add(fontsPath + "\\timesbi.ttf");
    crconfig.uiFontFace = "Arial";
    crconfig.monoFontFace = "Courier New";
    crconfig.fallbackFontFace = "Arial Unicode MS";
#endif

    QString home = QDir::homePath();
    QByteArray homeutf8 = home.toUtf8();
    lString8 home8(homeutf8.constData());
    LVAppendPathDelimiter(home8);
    lString8 cr3dir = home8 + ".cr3";
    LVCreateDirectory(Utf8ToUnicode(cr3dir));
    LVAppendPathDelimiter(cr3dir);
    //crconfig.setupUserDir(UnicodeToUtf8(exePath));
    crconfig.setupUserDir(cr3dir);

    crconfig.setupResources(UnicodeToUtf8(exePath + L"res"));
    crconfig.coverDirMaxFiles = 2000;
    crconfig.coverDirMaxItems = 10000;
    crconfig.coverDirMaxSize = 1024*1024 * 32;
    crconfig.coverRenderCacheMaxBytes = 1024*1024 * 16;
    crconfig.coverRenderCacheMaxItems = 200;

    //QString homePath = QDir::homePath();

#ifdef _WIN32
    for (char letter = 'A'; letter <= 'Z'; letter++) {
        lString16 s;
        s << letter;
        s << L":\\";
        int t = GetDriveTypeW(s.c_str());
        if (t != DRIVE_UNKNOWN && t != DRIVE_NO_ROOT_DIR) {
            deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, UnicodeToUtf8(s));
        }
    }
    //deviceInfo.topDirs.addItem(DIR_TYPE_SD_CARD, lString8("c:\\"));
    //deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, lString8("c:\\"));
    //deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, lString8("d:\\"));
    //deviceInfo.topDirs.addItem(DIR_TYPE_FAVORITE, lString8("c:\\Shared\\Books"));
    //deviceInfo.topDirs.addItem(DIR_TYPE_CURRENT_BOOK_DIR, lString8("c:\\Shared\\Books"));
    crconfig.defaultDownloadsDir = downloadsPath;
    //deviceInfo.topDirs.addItem(DIR_TYPE_DOWNLOADS, lString8("c:\\Shared\\Books\\Downloads"));
#else
    {
        struct passwd *pw = getpwuid(getuid());
        const char *homedir = pw->pw_dir;
        deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, lString8("/"));
        deviceInfo.topDirs.addItem(DIR_TYPE_FAVORITE, lString8(homedir));
        crconfig.defaultDownloadsDir = lString8(homedir) + "/Downloads";
        //deviceInfo.topDirs.addItem(DIR_TYPE_DOWNLOADS, lString8(homedir) + "/Downloads");
        crconfig.uiFontFace = "DejaVu Sans";
        crconfig.monoFontFace = "DejaVu Sans Mono";
        crconfig.fallbackFontFace = "Liberation Sans";
    }
#endif

    crconfig.touchMode = false;
    // init
    crconfig.initEngine(false);
}

