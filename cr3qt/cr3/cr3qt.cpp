
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
    int key = translateKeyCode(event->key());
    if (!key) {
        CRLog::warn("Skipping unknown key %d", event->key());
        return;
    }
    bool autorepeat = event->isAutoRepeat();
    int count = event->count();
    lUInt32 modifiers = translateModifiers(event->modifiers());
    CRUIKeyEvent * ev = new CRUIKeyEvent(type, key, autorepeat, count, modifiers);
    _eventManager->dispatchKeyEvent(ev);
    delete ev;
}

QOpenGLFunctions * _qtgl = NULL;



#include "cruiconfig.h"

void InitCREngine(lString16 exePath) {
    CRLog::setStderrLogger();
    CRLog::setLogLevel(CRLog::LL_TRACE);
    // fill config parameters

    // Logger
#ifdef _WIN32
    crconfig.logFile = UnicodeToUtf8(exePath + L"cr3.log");
#endif
    // Concurrency
    concurrencyProvider = new QtConcurrencyProvider();

#ifdef _WIN32
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\arial.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\ariali.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\arialbd.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\arialbi.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\tahoma.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\tahomabd.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\comic.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\comicbd.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\calibri.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\calibrib.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\calibrii.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\calibrili.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\cour.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\courbd.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\couri.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\courbi.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\times.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\timesi.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\timesbd.ttf");
    crconfig.fontFiles.add("C:\\Windows\\Fonts\\timesbi.ttf");
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
    deviceInfo.topDirs.addItem(DIR_TYPE_INTERNAL_STORAGE, lString8("c:\\"));
    deviceInfo.topDirs.addItem(DIR_TYPE_SD_CARD, lString8("c:\\"));
    //deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, lString8("c:\\"));
    //deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, lString8("d:\\"));
    deviceInfo.topDirs.addItem(DIR_TYPE_FAVORITE, lString8("c:\\Shared\\Books"));
    //deviceInfo.topDirs.addItem(DIR_TYPE_CURRENT_BOOK_DIR, lString8("c:\\Shared\\Books"));
    deviceInfo.topDirs.addItem(DIR_TYPE_DOWNLOADS, lString8("c:\\Shared\\Books\\Downloads"));
#else
    {
        struct passwd *pw = getpwuid(getuid());
        const char *homedir = pw->pw_dir;
        deviceInfo.topDirs.addItem(DIR_TYPE_FS_ROOT, lString8("/"));
        deviceInfo.topDirs.addItem(DIR_TYPE_FAVORITE, lString8(homedir));
        deviceInfo.topDirs.addItem(DIR_TYPE_DOWNLOADS, lString8(homedir) + "/Downloads");
        crconfig.uiFontFace = "DejaVu Sans";
        crconfig.monoFontFace = "DejaVu Sans Mono";
        crconfig.fallbackFontFace = "Liberation Sans";
    }
#endif

    // init
    crconfig.initEngine();
}

