#ifndef CRUIMAIN_H
#define CRUIMAIN_H

#include "crui.h"
#include "cruifolderwidget.h"
#include "cruihomewidget.h"
#include "cruireadwidget.h"
#include "opdsbrowser.h"
#include "cruiopdsprops.h"
#include "cruiopdsbook.h"
#include "cruipopup.h"
#include "cruiwindow.h"
#include "cruisettingswidget.h"
#include "vkeyboard.h"

enum VIEW_MODE {
    MODE_HOME,
    MODE_FOLDER,
    MODE_SETTINGS,
    MODE_READ,
    MODE_TOC,
    MODE_BOOKMARKS,
    MODE_OPDS,
    MODE_OPDS_PROPS,
    MODE_OPDS_BOOK
};

class CRUIScreenUpdateManagerCallback {
public:
    /// set animation fps (0 to disable) and/or update screen instantly
    virtual void setScreenUpdateMode(bool updateNow, int animationFps) = 0;
    virtual ~CRUIScreenUpdateManagerCallback() {}
};

class CRUITextToSpeechVoice {
    lString8 _id;
    lString8 _name;
    lString8 _lang;
public:
    CRUITextToSpeechVoice(lString8 id, lString8 name, lString8 lang)
        : _id(id), _name(name), _lang(lang)
    {
    }
    lString8 getId() { return _id; }
    lString8 getName() { return _name; }
    lString8 getLang() { return _lang; }
};

class CRUITextToSpeech {
public:
    virtual CRUITextToSpeechCallback * getTextToSpeechCallback() = 0;
    virtual void setTextToSpeechCallback(CRUITextToSpeechCallback * callback) = 0;
    virtual void getAvailableVoices(LVPtrVector<CRUITextToSpeechVoice, false> & list) = 0;
    virtual CRUITextToSpeechVoice * getCurrentVoice() = 0;
    virtual CRUITextToSpeechVoice * getDefaultVoice() = 0;
    virtual bool setCurrentVoice(lString8 id) = 0;
    virtual bool canChangeCurrentVoice() = 0;
    virtual bool tell(lString16 text) = 0;
    virtual ~CRUITextToSpeech() {}
};

class CRUIPlatform {
public:
	/// completely exit app
	virtual void exitApp() = 0;
	/// minimize app or show Home Screen
	virtual void minimizeApp() = 0;
    // copy text to clipboard
    virtual void copyToClipboard(lString16 text) = 0;
    /// override to open URL in external browser; returns false if failed or feature not supported by platform
    virtual bool openLinkInExternalBrowser(lString8 url) { CR_UNUSED(url); return false; }
    /// override to open file in external application; returns false if failed or feature not supported by platform
    virtual bool openFileInExternalApp(lString8 filename, lString8 mimeType) { CR_UNUSED2(filename, mimeType); return false; }

    /// return true if device has hardware keyboard connected
    virtual bool hasHardwareKeyboard() { return false; }
    /// return true if platform supports native virtual keyboard
    virtual bool supportsVirtualKeyboard() { return false; }
    /// return true if platform native virtual keyboard is shown
    virtual bool isVirtualKeyboardShown() { return false; }
    /// show platform native virtual keyboard
    virtual void showVirtualKeyboard(int mode, lString16 text, bool multiline) { CR_UNUSED3(mode, text, multiline); }
    /// hide platform native virtual keyboard
    virtual void hideVirtualKeyboard() {}

    virtual bool supportsFullscreen() { return false; }
    virtual bool isFullscreen() { return false; }
    virtual void setFullscreen(bool fullscreen) { CR_UNUSED(fullscreen); }

    virtual void setFileToOpenOnStart(lString8 filename) {
        CRLog::debug("setFileToOpenOnStart(%s)", filename.c_str());
    }

    enum {
    	EINK_UPDATE_MODE_CLEAR = 0,
    	EINK_UPDATE_MODE_ONESHOT = 1,
    	EINK_UPDATE_MODE_ACTIVE = 2
    };

    virtual void setScreenUpdateMode(int mode) { CR_UNUSED(mode); }
    virtual void setScreenUpdateInterval(int interval) { CR_UNUSED(interval); }

    /// returns 0 if not supported, task ID if download task is started
    virtual int openUrl(lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs) {
        CR_UNUSED5(url, method, login, password, saveAs);
        return 0;
    }
    /// cancel specified download task
    virtual void cancelDownload(int downloadTaskId) { CR_UNUSED(downloadTaskId); }

    virtual CRUITextToSpeech * getTextToSpeech() { return NULL; }

	virtual ~CRUIPlatform() {}
};

class CRUIMainWidget;
class NavHistoryItem {
protected:
    CRUIMainWidget * main;
    CRUIWindowWidget * widget;
public:
    //virtual void setDirectory(CRDirCacheItem * item) { CR_UNUSED(item); }
    virtual const lString8 & getPathName() { return lString8::empty_str; }
    virtual VIEW_MODE getMode() = 0;
    virtual CRUIWindowWidget * getWidget() { return widget; }
    NavHistoryItem(CRUIMainWidget * _main, CRUIWindowWidget * widget) : main(_main), widget(widget) {}
    virtual ~NavHistoryItem() {}
};


class HomeItem : public NavHistoryItem {
public:
    virtual VIEW_MODE getMode() { return MODE_HOME; }
    //HomeItem(CRUIMainWidget * _main) : NavHistoryItem(_main, new CRUIHomeWidget(_main)) {}
    HomeItem(CRUIMainWidget * _main, CRUIHomeWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class ReadItem : public NavHistoryItem {
public:
    virtual VIEW_MODE getMode() { return MODE_READ; }
    ReadItem(CRUIMainWidget * _main, CRUIReadWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class SettingsItem : public NavHistoryItem {
public:
    virtual VIEW_MODE getMode() { return MODE_SETTINGS; }
    SettingsItem(CRUIMainWidget * _main, CRUISettingsWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class TOCItem : public NavHistoryItem {
public:
    virtual VIEW_MODE getMode() { return MODE_TOC; }
    TOCItem(CRUIMainWidget * _main, CRUITOCWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class BookmarksItem : public NavHistoryItem {
public:
    virtual VIEW_MODE getMode() { return MODE_BOOKMARKS; }
    BookmarksItem(CRUIMainWidget * _main, CRUIBookmarksWidget * _widget) : NavHistoryItem(_main, _widget) {}
};

class FolderItem : public NavHistoryItem {
    lString8 pathname;
public:
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
    static lString8 makePath(LVClonePtr<BookDBCatalog> & dir, const lString8 & url) {
        return lString8(OPDS_CATALOG_TAG) + lString8::itoa(dir->id) + (url.empty() ? lString8() : lString8(":") + url);
    }
    virtual void setDirectory(LVClonePtr<BookDBCatalog> & catalog, CRDirCacheItem * item) { ((CRUIOpdsBrowserWidget*)widget)->setDirectory(catalog, item); }
    virtual VIEW_MODE getMode() { return MODE_OPDS; }
    OPDSItem(CRUIMainWidget * _main, LVClonePtr<BookDBCatalog> & catalog, lString8 _pathname, lString8 url, lString16 title) : NavHistoryItem(_main, new CRUIOpdsBrowserWidget(_main)), pathname(_pathname) {
        CROpdsCatalogsItem * dir = (CROpdsCatalogsItem*)dirCache->getOrAdd(pathname);
        dir->setCatalog(catalog);
        dir->setURL(url);
        dir->setTitle(title);
        ((CRUIOpdsBrowserWidget*)widget)->setDirectory(catalog, dir);

    }
    virtual const lString8 & getPathName() { return pathname; }
    virtual ~OPDSItem() {
        if (widget)
            delete widget;
    }
};

class OPDSPropsItem : public NavHistoryItem {
    lString8 path;
    LVClonePtr<BookDBCatalog> catalog;
public:
    static lString8 makePath(LVClonePtr<BookDBCatalog> & _catalog) {
        return lString8("OPDS_BOOK:") + lString8::itoa(!_catalog ? 0 : _catalog->id);
    }
    virtual VIEW_MODE getMode() { return MODE_OPDS_PROPS; }
    OPDSPropsItem(CRUIMainWidget * _main, LVClonePtr<BookDBCatalog> & _catalog) : NavHistoryItem(_main, new CRUIOpdsPropsWidget(_main, _catalog)) {
        path = makePath(_catalog);
        catalog = _catalog;
    }
    virtual const lString8 & getPathName() { return path; }
    virtual ~OPDSPropsItem() {
        if (widget)
            delete widget;
    }
};

class OPDSBookItem : public NavHistoryItem {
    lString8 path;
    LVClonePtr<CROpdsCatalogsItem>  book;
public:
    static lString8 makePath(LVClonePtr<CROpdsCatalogsItem> & _book) {
        return lString8("OPDS_BOOK:") + _book->getPathName() + ":" + _book->getId();
    }

    virtual VIEW_MODE getMode() { return MODE_OPDS_BOOK; }
    OPDSBookItem(CRUIMainWidget * _main, LVClonePtr<CROpdsCatalogsItem> & _book) : NavHistoryItem(_main, new CRUIOpdsBookWidget(_main, _book)) {
        path = makePath(_book);
        book = _book;
    }
    virtual const lString8 & getPathName() { return path; }
    virtual ~OPDSBookItem() {
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
    CRUIVirtualKeyboard * _keyboard;
    LVDrawBuf * _popupBackground;
    //VIEW_MODE _mode;
    CRUIScreenUpdateManagerCallback * _screenUpdater;
    CRUIPlatform * _platform;
    lString8 _pendingFolder;
    lString8 _pendingBookOpenFolder;
    lString8 _pendingBookOpenFile;
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

    LVHashTable<lUInt32, CRUIWindowWidget *> _downloadMap;

    lString16 _messageText;
    lString8 _filenameToOpen;

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

    void setBatteryLevel(int level);
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

    virtual void setFileToOpenOnStart(lString8 filename);


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

    /// handle timer event; return true to allow recurring timer event occur more times, false to stop
    virtual bool onTimerEvent(lUInt32 timerId);
    void showMessage(lString16 text, int duration);

    void openBookFromFile(lString8 filename);
    void openBook(const CRFileItem * file);
    void showFolder(lString8 folder, bool appendHistory);
    void showOpds(LVClonePtr<BookDBCatalog> & dir, lString8 url, lString16 title);
    void showOpdsProps(LVClonePtr<BookDBCatalog> & dir);
    void showOpdsBook(LVClonePtr<CROpdsCatalogsItem> & book);
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

    void showVirtualKeyboard(int mode, lString16 text, bool multiline);
    void hideVirtualKeyboard();
    bool isVirtualKeyboardShown();

    /// returns 0 if not supported, task ID if download task is started
    virtual int openUrl(CRUIWindowWidget * callback, lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs);
    /// cancel specified download task
    virtual void cancelDownload(int downloadTaskId);
    /// pass download result to window
    virtual void onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream);
    /// download progress
    virtual void onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded);


    void updateFolderBookmarks();

    CRUIMainWidget();
    virtual ~CRUIMainWidget();
};


#endif // CRUIMAIN_H
