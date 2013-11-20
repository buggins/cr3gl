#ifndef CRUIMAIN_H
#define CRUIMAIN_H

#include "crui.h"
#include "cruifolderwidget.h"
#include "cruihomewidget.h"
#include "cruireadwidget.h"
#include "opdsbrowser.h"
#include "cruipopup.h"
#include "cruiwindow.h"
#include "cruisettingswidget.h"

enum VIEW_MODE {
    MODE_HOME,
    MODE_FOLDER,
    MODE_SETTINGS,
    MODE_READ,
    MODE_TOC,
    MODE_BOOKMARKS,
    MODE_OPDS
};

class CRUIScreenUpdateManagerCallback {
public:
    /// set animation fps (0 to disable) and/or update screen instantly
    virtual void setScreenUpdateMode(bool updateNow, int animationFps) = 0;
    virtual ~CRUIScreenUpdateManagerCallback() {}
};

class CRUIPlatform {
public:
	/// completely exit app
	virtual void exitApp() = 0;
	/// minimize app or show Home Screen
	virtual void minimizeApp() = 0;
    // copy text to clipboard
    virtual void copyToClipboard(lString16 text) = 0;
	virtual ~CRUIPlatform() {}
};

class CRUIMainWidget;
class NavHistoryItem {
protected:
    CRUIMainWidget * main;
    CRUIWindowWidget * widget;
public:
    virtual CRUIWindowWidget * recreate() = 0;
    virtual void setDirectory(CRDirCacheItem * item) { CR_UNUSED(item); }
    virtual const lString8 & getPathName() { return lString8::empty_str; }
    virtual VIEW_MODE getMode() = 0;
    virtual CRUIWindowWidget * getWidget() { return widget; }
    NavHistoryItem(CRUIMainWidget * _main, CRUIWindowWidget * widget) : main(_main), widget(widget) {}
    virtual ~NavHistoryItem() {}
};


class HomeItem : public NavHistoryItem {
public:
    virtual CRUIWindowWidget * recreate() {
        if (widget)
            delete widget;
        widget = new CRUIHomeWidget(main);
        return widget;
    }
    virtual VIEW_MODE getMode() { return MODE_HOME; }
    //HomeItem(CRUIMainWidget * _main) : NavHistoryItem(_main, new CRUIHomeWidget(_main)) {}
    HomeItem(CRUIMainWidget * _main, CRUIHomeWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class ReadItem : public NavHistoryItem {
public:
    // recreate on config change
    virtual CRUIWindowWidget * recreate() {
        lvRect pos = ((CRUIWidget*)main)->getPos();
        widget->measure(pos.width(), pos.height());
        widget->layout(pos.left, pos.top, pos.right, pos.bottom);
        return widget;
    }
    virtual VIEW_MODE getMode() { return MODE_READ; }
    ReadItem(CRUIMainWidget * _main, CRUIReadWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class SettingsItem : public NavHistoryItem {
public:
    // recreate on config change
    virtual CRUIWindowWidget * recreate() {
        lvRect pos = ((CRUIWidget*)main)->getPos();
        widget->measure(pos.width(), pos.height());
        widget->layout(pos.left, pos.top, pos.right, pos.bottom);
        return widget;
    }
    virtual VIEW_MODE getMode() { return MODE_SETTINGS; }
    SettingsItem(CRUIMainWidget * _main, CRUISettingsWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class TOCItem : public NavHistoryItem {
public:
    // recreate on config change
    virtual CRUIWindowWidget * recreate() {
        lvRect pos = ((CRUIWidget*)main)->getPos();
        widget->measure(pos.width(), pos.height());
        widget->layout(pos.left, pos.top, pos.right, pos.bottom);
        return widget;
    }
    virtual VIEW_MODE getMode() { return MODE_TOC; }
    TOCItem(CRUIMainWidget * _main, CRUITOCWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class BookmarksItem : public NavHistoryItem {
public:
    // recreate on config change
    virtual CRUIWindowWidget * recreate() {
        lvRect pos = ((CRUIWidget*)main)->getPos();
        widget->measure(pos.width(), pos.height());
        widget->layout(pos.left, pos.top, pos.right, pos.bottom);
        return widget;
    }
    virtual VIEW_MODE getMode() { return MODE_BOOKMARKS; }
    BookmarksItem(CRUIMainWidget * _main, CRUIBookmarksWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class FolderItem : public NavHistoryItem {
    lString8 pathname;
public:
    virtual CRUIWindowWidget * recreate() {
        if (widget)
            delete widget;
        widget = new CRUIFolderWidget(main);
        ((CRUIFolderWidget*)widget)->setDirectory(dirCache->getOrAdd(pathname));
        return widget;
    }
    virtual void setDirectory(CRDirCacheItem * item) { ((CRUIFolderWidget*)widget)->setDirectory(item); }
    virtual VIEW_MODE getMode() { return MODE_FOLDER; }
    FolderItem(CRUIMainWidget * _main, lString8 _pathname) : NavHistoryItem(_main, new CRUIFolderWidget(_main)), pathname(_pathname) {
        ((CRUIFolderWidget*)widget)->setDirectory(dirCache->getOrAdd(pathname));
    }
    virtual const lString8 & getPathName() { return pathname; }
    virtual ~FolderItem() {
        if (widget)
            delete widget;
    }
};

class OPDSItem : public NavHistoryItem {
    lString8 pathname;
public:
    virtual CRUIWindowWidget * recreate() {
        if (widget)
            delete widget;
        widget = new CRUIFolderWidget(main);
        ((CRUIFolderWidget*)widget)->setDirectory(dirCache->getOrAdd(pathname));
        return widget;
    }
    virtual void setDirectory(CRDirCacheItem * item) { ((CRUIOpdsBrowserWidget*)widget)->setDirectory(item); }
    virtual VIEW_MODE getMode() { return MODE_OPDS; }
    OPDSItem(CRUIMainWidget * _main, lString8 _pathname) : NavHistoryItem(_main, new CRUIOpdsBrowserWidget(_main)), pathname(_pathname) {
        ((CRUIOpdsBrowserWidget*)widget)->setDirectory(dirCache->getOrAdd(pathname));
    }
    virtual const lString8 & getPathName() { return pathname; }
    virtual ~OPDSItem() {
        if (widget)
            delete widget;
    }
};

class NavHistory {
    LVPtrVector<NavHistoryItem> _list;
    int _pos;
public:
    NavHistory() : _pos(0) {}
    bool hasBack() const { return _pos > 0; }
    bool hasForward() const { return _pos < _list.length() - 1; }

    /// returns current widget
    CRUIWidget * currentWidget() { return _pos >= 0 && _pos < _list.length() ? _list[_pos]->getWidget() : NULL; }
    /// returns current mode
    VIEW_MODE currentMode()  { return _pos >= 0 && _pos < _list.length() ? _list[_pos]->getMode() : MODE_HOME; }
    /// returns current window
    NavHistoryItem * current() { return _pos >= 0 && _pos < _list.length() ? _list[_pos] : NULL; }
    /// returns previous window, NULL if none
    NavHistoryItem * prev() { return _pos > 0 && _pos < _list.length() ? _list[_pos - 1] : NULL; }
    /// returns next window, NULL if none
    NavHistoryItem * next() { return _pos >= 0 && _pos < _list.length() - 1 ? _list[_pos + 1] : NULL; }
    void truncateForward() {
        while (_pos < _list.length() - 1) {
            NavHistoryItem * removed = _list.remove(_pos + 1);
            delete removed;
        }
    }

    /// sets next item, clears forward history - if any
    void setNext(NavHistoryItem * item) {
        truncateForward();
        _list.add(item);
    }
    /// sets next item and move to it, clears forward history - if any
    void add(NavHistoryItem * item) {
        setNext(item);
        if (_pos < _list.length() - 1)
            _pos++;
    }
    /// returns current position
    int pos() const { return _pos; }
    /// returns count of items
    int length() const { return _list.length(); }
    /// returns item by index
    NavHistoryItem * operator[] (int index) { return index >= 0 && index < _list.length() ? _list[index] : NULL; }
    /// clears the whole history
    void clear() { _list.clear(); }
    /// sets history position
    void setPos(int p) { if (p >= 0 && p < _list.length()) _pos = p; }
    /// searches for history position by specified mode
    int findPosByMode(VIEW_MODE mode) {
        int p = -1;
        for (int i = 0; i < _list.length(); i++) {
            if (_list[i]->getMode() == mode) {
                p = i;
                break;
            }
        }
        return p;
    }
    /// sets history position by finding existing item of specified mode
    bool setPosByMode(VIEW_MODE mode, bool truncateFwd) {
        int p = findPosByMode(mode);
        if (p < 0)
            return false;
        _pos = p;
        if (truncateFwd)
            truncateForward();
        return true;
    }
    /// searches for history position by specified mode and path
    int findPosByMode(VIEW_MODE mode, lString8 pathname) {
        int p = -1;
        for (int i = 0; i < _list.length(); i++) {
            if (_list[i]->getMode() == mode && _list[i]->getPathName() == pathname) {
                p = i;
                break;
            }
        }
        return p;
    }
    /// sets history position by finding existing item of specified mode and path
    bool setPosByMode(VIEW_MODE mode, lString8 pathname, bool truncateFwd) {
        int p = findPosByMode(mode, pathname);
        if (p < 0)
            return false;
        _pos = p;
        if (truncateFwd)
            truncateForward();
        return true;
    }
};

class CRUIEventManager;
class CRUIMainWidget : public CRUIWidget, public CRDirScanCallback, public CRUIScreenUpdateManagerCallback,
        public CRDocumentLoadCallback, public CRDocumentRenderCallback
{
	friend class CRUIEventManager;
	CRUIEventManager * _eventManager;
    CRUIHomeWidget * _home;
    //CRUIFolderWidget * _folder;
    CRUIReadWidget * _read;
    CRUIPopupWindow * _popup;
    LVDrawBuf * _popupBackground;
    //VIEW_MODE _mode;
    CRUIScreenUpdateManagerCallback * _screenUpdater;
    CRUIPlatform * _platform;
    lString8 _pendingFolder;
    lUInt64 _lastAnimationTs;

    struct AnimationControl {
        bool active;
        bool manual;
        int direction;
        int duration;
        int progress;
        int oldpos;
        int newpos;
        int oldimagex;
        int newimagex;
        LVDrawBuf * oldimage;
        LVDrawBuf * newimage;
        lvPoint startPoint;
        lUInt64 startTs;
        AnimationControl() : active(false), oldimage(NULL), newimage(NULL) {}
    };
    AnimationControl _animation;

    NavHistory _history;

    CRThreadExecutor _backgroundThread;

    bool _initialized;

    CRUISettingsList _browserSettings;
    CRUISettingsList _readerSettings;

    CRPropRef _currentSettings; // curretnly active settings
    CRPropRef _newSettings; // to be edited by Settings editors

    void createBrowserSettings();
    void createReaderSettings();

    void beforeNavigation(NavHistoryItem * from, NavHistoryItem * to);
    void afterNavigation(NavHistoryItem * from, NavHistoryItem * to);
    void startAnimation(int newpos, int duration, const CRUIMotionEvent * event = NULL);
    void stopAnimation();
    /// if not initialized, run background tasks
    void runStartupTasksIfNeeded();

    void saveSettings();

    void setEventManager(CRUIEventManager * eventManager) { _eventManager = eventManager; }
public:
    CRPropRef getSettings() { return _currentSettings; } // curretnly active settings
    CRPropRef getNewSettings() { return _newSettings; } // to be edited by Settings editors
    CRPropRef initNewSettings() { _newSettings = LVClonePropsContainer(_currentSettings); return _newSettings; } // to be edited by Settings editors
    // apply changed settings
    void applySettings();
    // apply changed settings
    void applySettings(CRPropRef changed, CRPropRef oldSettings, CRPropRef newSettings);
    CRUISettingsList * findSettings(lString8 path);

    CRRunnable * createUpdateCallback();
    void executeBackground(CRRunnable * task) { _backgroundThread.execute(task); }
    VIEW_MODE getMode() { return _history.currentMode(); }
    CRUIWidget * currentWidget() { return _history.currentWidget(); }
    NavHistory & history() { return _history; }
    virtual void animate(lUInt64 millisPassed);
    virtual bool isAnimating();

    /// draw now if force == true, layout/draw if necessary when force == false
    virtual void update(bool force);

    /// forward screen update request to external code
    virtual void setScreenUpdateMode(bool updateNow, int animationFps);

    virtual void setScreenUpdater(CRUIScreenUpdateManagerCallback * screenUpdater) { _screenUpdater = screenUpdater; }

    virtual void setPlatform(CRUIPlatform * platform) {
    	_platform = platform;
    }
    CRUIPlatform * getPlatform() {
    	return _platform;
    }


    virtual int getChildCount();
    virtual CRUIWidget * getChild(int index);
    /// measure dimensions
    virtual void measure(int baseWidth, int baseHeight);
    /// updates widget position based on specified rectangle
    virtual void layout(int left, int top, int right, int bottom);
    /// draws widget with its children to specified surface
    virtual void draw(LVDrawBuf * buf);
    /// motion event handler, returns true if it handled event
    virtual bool onTouchEvent(const CRUIMotionEvent * event);
    /// motion event handler - before children, returns true if it handled event
    virtual bool onTouchEventPreProcess(const CRUIMotionEvent * event);

    /// handle menu or other action
    virtual bool onAction(const CRUIAction * action);
    /// handle menu or other action - find standard action by id
    virtual bool onAction(int actionId);

    /// return true if drag operation is intercepted
    virtual bool startDragging(const CRUIMotionEvent * event, bool vertical);

    /// returns true if widget is child of this
    virtual bool isChild(CRUIWidget * widget);

    void showSlowOperationPopup();
    void hideSlowOperationPopup();

    void openBook(const CRFileItem * file);
    void showFolder(lString8 folder, bool appendHistory);
    void showOpds(BookDBCatalog * dir);
    void showHome();
    void showSettings(lString8 path);
    void showSettings(CRUISettingsItem * setting);
    void showTOC(CRUITOCWidget * toc);
    void showBookmarks(CRUIBookmarksWidget * toc);
    void back(bool fast = false);

    virtual void onAllCoverpagesReady(int newpos);
    virtual void onDirectoryScanFinished(CRDirContentItem * item);
    virtual void onDocumentLoadFinished(lString8 pathname, bool success);
    virtual void onDocumentRenderFinished(lString8 pathname);

    virtual void onThemeChanged();

    virtual void onSystemLanguageChanged();

    virtual void clearImageCaches();

    CRFileItem * createManualBook();

    CRUIMainWidget();
    virtual ~CRUIMainWidget();
};

#endif // CRUIMAIN_H
