#include "cruimain.h"

using namespace CRUI;

#include "gldrawbuf.h"
#include "crcoverpages.h"
#include "stringresource.h"
#include "cruiconfig.h"
#include "cruisettings.h"
#include "cruisettingswidget.h"

#define WINDOW_ANIMATION_DELAY 250
//#define WINDOW_ANIMATION_DELAY 750
#define SLOW_OPERATION_POPUP_DELAY 100
#define SLOW_OPERATION_POPUP_DIMMING_DURATION 1000
#define SLOW_OPERATION_DIM_COLOR 0xE0000000

void applyThemeChange(CRUIWidget * widget) {
    if (!widget)
        return;
    widget->onThemeChanged();
    for (int i = 0; i < widget->getChildCount(); i++)
        applyThemeChange(widget->getChild(i));
}

void CRUIMainWidget::onSystemLanguageChanged() {
	if (_currentSettings->getStringDef(PROP_APP_INTERFACE_LANGUAGE, PROP_APP_INTERFACE_LANGUAGE_VALUE_SYSTEM) == PROP_APP_INTERFACE_LANGUAGE_VALUE_SYSTEM) {
		crconfig.setInterfaceLanguage(lString8(PROP_APP_INTERFACE_LANGUAGE_VALUE_SYSTEM));
		requestLayout();
		update(true);
	}
}

void CRUIMainWidget::onThemeChanged()
{
    applyThemeChange(_home);
    applyThemeChange(_read);

    for (int i = 0; i < _history.length(); i++) {
        CRUIWidget * widget = _history[i]->getWidget();
        if (widget != _home && widget != _read)
            applyThemeChange(widget);
    }
    requestLayout();
}

/// returns true if widget is child of this
bool CRUIMainWidget::isChild(CRUIWidget * widget) {
    if (_keyboard && _keyboard->isChild(widget))
        return true;
    return widget == this || _history.currentWidget()->isChild(widget);
}

void CRUIMainWidget::showHome() {
    startAnimation(0, WINDOW_ANIMATION_DELAY);
}

void CRUIMainWidget::back(bool fast) {
    if (_history.hasBack()) {
        int newpos = _history.pos() - 1;
        if (fast && _history.currentMode() == MODE_SETTINGS) {
            while (_history[newpos]->getMode() == MODE_SETTINGS && newpos > 0)
                newpos--;
        }
        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
    } else {
    	_platform->minimizeApp();
    }
}

class CoverpagesReadyCallback : public CRRunnable {
    CRUIMainWidget * _main;
    int _newpos;
public:
    CoverpagesReadyCallback(CRUIMainWidget * main, int newpos) : _main(main), _newpos(newpos) {}
    virtual void run() {
        _main->onAllCoverpagesReady(_newpos);
    }
};

void CRUIMainWidget::onAllCoverpagesReady(int newpos) {
    if (_history.next() && _history.next()->getMode() == MODE_FOLDER) {
        // animating move to folder view
        hideSlowOperationPopup();
        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
    }
}

static bool first_recent_dir_scan = true;
void CRUIMainWidget::onDirectoryScanFinished(CRDirContentItem * item) {
    CRLog::trace("CRUIMainWidget::onDirectoryScanFinished");
    if (item->getDirType() == DIR_TYPE_RECENT) {
        // set recent book
        if (item->itemCount() == 0 && first_recent_dir_scan) {
            CRLog::trace("Recent books list is empty: creating manual book item");
            first_recent_dir_scan = false;
            CRFileItem * book = createManualBook();
            LVPtrVector<BookDBBook> books;
            books.add(book->getBook()->clone());
            bookDB->saveBooks(books);
            BookDBBookmark * _lastPosition = new BookDBBookmark();
            _lastPosition->bookId = books[0]->id;
            _lastPosition->type = bmkt_lastpos;
            _lastPosition->percent = 0;
            _lastPosition->timestamp = GetCurrentTimeMillis();
            _lastPosition->titleText = "Cool Reader Manual";
            dirCache->saveLastPosition(books[0], _lastPosition);
            delete _lastPosition;
            delete book;
        }
        if (item->itemCount() > 0) {
            _home->requestLayout();
            //_home->setLastBook(item->getItem(0));
        }
        update(true);
        return;
    }
    if (_pendingFolder == item->getPathName()) {
        item->sort(CRUI::BY_TITLE);
        int newpos = _history.findPosByMode(MODE_FOLDER, _pendingFolder);
        if (newpos < 0) {
            _history.setNext(new FolderItem(this, _pendingFolder));
            newpos = _history.pos() + 1;
        }
        CRLog::info("Directory %s is ready", item->getPathName().c_str());
        _pendingFolder.clear();
        CRUIFolderWidget * folder = static_cast<CRUIFolderWidget *>(_history[newpos]->getWidget());
        folder->measure(_pos.width(), _pos.height());
        folder->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
        if (folder && !folder->requestAllVisibleCoverpages()) {
            // initiate wait until all coverpages ready
            coverPageManager->setAllTasksFinishedCallback(new CoverpagesReadyCallback(this, newpos));
        } else {
            hideSlowOperationPopup();
            startAnimation(newpos, WINDOW_ANIMATION_DELAY);
        }
    } else {
        update(true);
    }
}

void CRUIMainWidget::onDocumentLoadFinished(lString8 pathname, bool success) {
    //
    CRLog::info("Document loaded: %s %s", pathname.c_str(), (success ? "successfully" : "with error"));

    BookDBBook * book = success ? bookDB->loadBook(pathname) : NULL;

    if (book) {
        CRFileItem * entry = dirCache->scanFile(pathname);
        //_home->setLastBook(entry);
        delete entry;
    } else {
        //_home->setLastBook(NULL);
    }
}

void CRUIMainWidget::onDocumentRenderFinished(lString8 pathname) {
    CRLog::info("Document rendered: %s", pathname.c_str());
    hideSlowOperationPopup();
    if (_history.current()->getMode() != MODE_READ) {
        int newpos = _history.findPosByMode(MODE_READ);
        if (newpos < 0) {
            _history.setNext(new ReadItem(this, _read));
            newpos = _history.pos() + 1;
        }
        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
    } else {
    	CRLog::trace("updating screen after render is finished");
        update(true);
    }
}

CRUISettingsList * CRUIMainWidget::findSettings(lString8 path) {
    if (!path.startsWith("@settings/"))
        return NULL;
    path = path.substr(10); // strlen("@settings/")
    if (path.startsWith("browser")) {
        // TODO: support subsettings
        return &_browserSettings;
    } else if (path.startsWith("reader")) {
        // TODO: support subsettings
        return &_readerSettings;
    } else {
        return NULL;
    }
}

void CRUIMainWidget::showSettings(lString8 path) {
    CRUISettingsList * setting = findSettings(path);
    showSettings(setting);
}

void CRUIMainWidget::showSettings(CRUISettingsItem * setting) {
    if (setting) {
        if (_history[_history.pos()]->getMode() != MODE_SETTINGS) {
            _newSettings = LVClonePropsContainer(_currentSettings);
        }
        CRUISettingsWidget * widget = new CRUISettingsWidget(this, setting);
        _history.setNext(new SettingsItem(this, widget));
        int newpos = _history.pos() + 1;
        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
    }
}

void CRUIMainWidget::showTOC(CRUITOCWidget * toc) {
    _history.setNext(new TOCItem(this, toc));
    int newpos = _history.pos() + 1;
    startAnimation(newpos, WINDOW_ANIMATION_DELAY);
}

void CRUIMainWidget::showBookmarks(CRUIBookmarksWidget * bm) {
    _history.setNext(new BookmarksItem(this, bm));
    int newpos = _history.pos() + 1;
    startAnimation(newpos, WINDOW_ANIMATION_DELAY);
}

#define MESSAGE_TIMER_ID 32145
void CRUIMainWidget::showMessage(lString16 text, int duration) {
    _messageText = text;
    _eventManager->setTimer(MESSAGE_TIMER_ID, this, duration, false);
    update(true);
}

bool CRUIMainWidget::onTimerEvent(lUInt32 timerId) {

    if (timerId == MESSAGE_TIMER_ID) {
        _messageText.clear();
        update(true);
        return false;
    }
    return false;
}

void CRUIMainWidget::showSlowOperationPopup()
{
	if (_popup) {
		CRLog::error("showSlowOperationPopup() called twice");
		return;
	}
	CRLog::trace("CRUIMainWidget::showSlowOperationPopup()");
#if 0
    CRUITextWidget * pleaseWait = new CRUITextWidget(lString16("Please wait"));
    pleaseWait->setBackground(0xFFFFFF);
    pleaseWait->setPadding(PT_TO_PX(7));
    pleaseWait->setAlign(ALIGN_CENTER);
    pleaseWait->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
#else
    CRUISpinnerWidget * pleaseWait = new CRUISpinnerWidget("spinner_white_48", 360 + 180);
    //CRUISpinnerWidget * pleaseWait = new CRUISpinnerWidget("ic_menu_back");
    pleaseWait->setPadding(PT_TO_PX(7));
    pleaseWait->setAlign(ALIGN_CENTER);
    pleaseWait->setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
#endif
    GLDrawBuf * buf = new GLDrawBuf(_pos.width(), _pos.height(), 32, true);
    buf->beforeDrawing();
    _history.currentWidget()->draw(buf);
    buf->afterDrawing();
    _popupBackground = buf;
    _popup = new CRUIPopupWindow(pleaseWait, SLOW_OPERATION_POPUP_DELAY, SLOW_OPERATION_POPUP_DIMMING_DURATION, SLOW_OPERATION_DIM_COLOR);
    _popup->measure(_pos.width(), _pos.height());
    _popup->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    update(true);
}

void CRUIMainWidget::hideSlowOperationPopup()
{
	if (!_popup) {
		CRLog::error("hideSlowOperationPopup() called twice");
		return;
	}
    if (_popup) {
        delete _popup;
        _popup = NULL;
    }
    if (_popupBackground) {
        delete _popupBackground;
        _popupBackground = NULL;
    }
}

void CRUIMainWidget::showOpdsProps(LVClonePtr<BookDBCatalog> &  dir) {
    lString8 folder = OPDSPropsItem::makePath(dir);
    int newpos = _history.findPosByMode(MODE_OPDS_PROPS, folder);
    if (newpos < 0) {
        // create page now, to lock corresponding folder
        _history.setNext(new OPDSPropsItem(this, dir));
        newpos = _history.pos() + 1;
    }
    startAnimation(newpos, WINDOW_ANIMATION_DELAY);
}

void CRUIMainWidget::showOpdsBook(LVClonePtr<CROpdsCatalogsItem> & book) {
    lString8 folder = OPDSBookItem::makePath(book);
    int newpos = _history.findPosByMode(MODE_OPDS_BOOK, folder);
    if (newpos < 0) {
        // create page now, to lock corresponding folder
        _history.setNext(new OPDSBookItem(this, book));
        newpos = _history.pos() + 1;
    }
    startAnimation(newpos, WINDOW_ANIMATION_DELAY);
}

void CRUIMainWidget::showOpds(LVClonePtr<BookDBCatalog> & dir, lString8 url, lString16 title) {
    CRLog::info("showOpds(catalog=%s url=%s)", dir->url.c_str(), url.c_str());
   //if ((_currentFolder != folder && _pendingFolder != folder) || _mode != MODE_FOLDER) {
    //_pendingFolder = folder;
    dir->lastUsage = GetCurrentTimeMillis();
    bookDB->updateOpdsCatalogLastUsage(dir->id);

    //lString8 folder(dir->url.c_str());
    if (url.empty())
        url = (dir->url.c_str());
    lString8 folder = OPDSItem::makePath(dir, url);
    int newpos = _history.findPosByMode(MODE_OPDS, folder);
    if (newpos < 0) {
        // create page now, to lock corresponding folder
        _history.setNext(new OPDSItem(this, dir, folder, url, title));
        newpos = _history.pos() + 1;
    }
    startAnimation(newpos, WINDOW_ANIMATION_DELAY);
//    showSlowOperationPopup();
//    CRLog::info("Starting background directory scan for %s", folder.c_str());
//    dirCache->scan(folder);

//    int newpos = _history.findPosByMode(MODE_FOLDER, folder);
//    if (newpos < 0) {
//        showSlowOperationPopup();
//        _history.setNext(new FolderItem(this, folder));
//        //_popup->setBackground(0xC0000000); // dimming
//        CRLog::info("Starting background directory scan for %s", folder.c_str());
//        dirCache->scan(folder);
//    } else {
//        // found existing
//        // do nothing
//        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
//    }
}

void CRUIMainWidget::showFolder(lString8 folder, bool appendHistory) {
    CR_UNUSED(appendHistory);
   //if ((_currentFolder != folder && _pendingFolder != folder) || _mode != MODE_FOLDER) {
    _pendingFolder = folder;
    int newpos = _history.findPosByMode(MODE_FOLDER, folder);
    if (newpos < 0) {
        // create page now, to lock corresponding folder
        _history.setNext(new FolderItem(this, folder));
    }
    showSlowOperationPopup();
    CRLog::info("Starting background directory scan for %s", folder.c_str());
    dirCache->scan(folder);

//    int newpos = _history.findPosByMode(MODE_FOLDER, folder);
//    if (newpos < 0) {
//        showSlowOperationPopup();
//        _history.setNext(new FolderItem(this, folder));
//        //_popup->setBackground(0xC0000000); // dimming
//        CRLog::info("Starting background directory scan for %s", folder.c_str());
//        dirCache->scan(folder);
//    } else {
//        // found existing
//        // do nothing
//        startAnimation(newpos, WINDOW_ANIMATION_DELAY);
//    }
}

void CRUIMainWidget::openBook(const CRFileItem * file) {
    if (!file) {
        CRDirContentItem * dir = dirCache->find(lString8(RECENT_DIR_TAG));
        file = dir && dir->itemCount() ? static_cast<CRFileItem*>(dir->getItem(0)) : NULL;
    } else {
        if (!file->getBook()) {
            file = dirCache->scanFile(file->getPathName());
        }
    }
    if (!file)
        return;
    CRLog::debug("Opening book %s", file->getPathName().c_str());
    if (_animation.active) {
        CRLog::debug("Animation is active. Stopping.");
        stopAnimation();
    }
    _read->measure(_measuredWidth, _measuredHeight);
    _read->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    _read->openBook(file);
}

void CRUIMainWidget::runStartupTasksIfNeeded() {
    if (_initialized)
        return;
    _initialized = true;
    dirCache->setDefaultCallback(this);
    dirCache->scan(lString8(RECENT_DIR_TAG));
}

void CRUIMainWidget::createBrowserSettings() {
    CRUISettingsOptionList * themes = new CRUISettingsOptionList(STR_SETTINGS_THEME, NULL, PROP_APP_THEME);
    themes->addOption(new CRUIOptionItem(PROP_APP_THEME_VALUE_LIGHT, STR_SETTINGS_THEME_VALUE_LIGHT));
    themes->addOption(new CRUIOptionItem(PROP_APP_THEME_VALUE_DARK, STR_SETTINGS_THEME_VALUE_DARK));
    themes->addOption(new CRUIOptionItem(PROP_APP_THEME_VALUE_WHITE, STR_SETTINGS_THEME_VALUE_WHITE));
    themes->addOption(new CRUIOptionItem(PROP_APP_THEME_VALUE_BLACK, STR_SETTINGS_THEME_VALUE_BLACK));
    CRUISettingsOptionList * uilangs = new CRUISettingsOptionList(STR_SETTINGS_INTERFACE_LANGUAGE, NULL, PROP_APP_INTERFACE_LANGUAGE);
    for (int i = 0; i < crconfig.interfaceLanguages.length(); i++) {
        CRUIInterfaceLanguage * lang = crconfig.interfaceLanguages[i];
        uilangs->addOption(new CRUIOptionItem(lang->id.c_str(), lang->nameRes.c_str()));
    }
    //themes->setDefaultValue(PROP_APP_THEME_VALUE_LIGHT);
    _browserSettings.addChild(themes);
    _browserSettings.addChild(uilangs);
}

void CRUIMainWidget::createReaderSettings() {
    CRUISettingsList * fontsAndColors = new CRUISettingsList(STR_SETTINGS_FONTS_AND_COLORS, NULL, SETTINGS_PATH_READER_FONTSANDCOLORS);
    CRUISettingsOptionList * fontFaces = new CRUIFontFaceSetting(STR_SETTINGS_FONT_FACE, NULL, PROP_FONT_FACE);
    lString16Collection faceList;
    fontMan->getFaceList(faceList);
    for (int i = 0; i < faceList.length(); i++) {
        fontFaces->addOption(new CRUIOptionItem(UnicodeToUtf8(faceList[i]), faceList[i]));
    }
    fontsAndColors->addChild(fontFaces);
    fontsAndColors->addChild(new CRUIFontSizeSetting(STR_SETTINGS_FONT_SIZE, NULL, PROP_FONT_SIZE));
    CRUIFontRenderingSetting * fontRendering = new CRUIFontRenderingSetting(STR_SETTINGS_FONT_RENDERING, NULL, SETTINGS_PATH_READER_FONTRENDERING);
    fontsAndColors->addChild(fontRendering);

    //fontsAndColors->addChild(new CRUISettingsCheckbox(STR_SETTINGS_FONT_ANTIALIASING, NULL, PROP_FONT_ANTIALIASING, STR_SETTINGS_FONT_ANTIALIASING_VALUE_ON, STR_SETTINGS_FONT_ANTIALIASING_VALUE_OFF));
    fontsAndColors->addChild(new CRUIColorSetting(STR_SETTINGS_FONT_COLOR, NULL, PROP_FONT_COLOR));
    fontsAndColors->addChild(new CRUIColorSetting(STR_SETTINGS_BACKGROUND_COLOR, NULL, PROP_BACKGROUND_COLOR));
    CRUIBackgroundTextureSetting * textures = new CRUIBackgroundTextureSetting(STR_SETTINGS_BACKGROUND_TEXTURE, NULL, PROP_BACKGROUND_IMAGE);
    for (int i = 0; i < resourceResolver->backgroundCount(); i++) {
        textures->addOption(new CRUITextureOptionItem(resourceResolver->getBackground(i)));
    }
    fontsAndColors->addChild(textures);

    _readerSettings.addChild(fontsAndColors);

    CRUISettingsList * interfaceSettings = new CRUISettingsList(STR_SETTINGS_INTERFACE, STR_SETTINGS_INTERFACE_DESCRIPTION, SETTINGS_PATH_READER_INTERFACE);
    CRUISettingsOptionList * uilangs = new CRUISettingsOptionList(STR_SETTINGS_INTERFACE_LANGUAGE, NULL, PROP_APP_INTERFACE_LANGUAGE);
    for (int i = 0; i < crconfig.interfaceLanguages.length(); i++) {
        CRUIInterfaceLanguage * lang = crconfig.interfaceLanguages[i];
        uilangs->addOption(new CRUIOptionItem(lang->id.c_str(), lang->nameRes.c_str()));
    }
    interfaceSettings->addChild(uilangs);

    CRUISettingsOptionList * themes = new CRUISettingsOptionList(STR_SETTINGS_THEME, NULL, PROP_APP_THEME);
    themes->addOption(new CRUIOptionItem(PROP_APP_THEME_VALUE_LIGHT, STR_SETTINGS_THEME_VALUE_LIGHT));
    themes->addOption(new CRUIOptionItem(PROP_APP_THEME_VALUE_DARK, STR_SETTINGS_THEME_VALUE_DARK));
    themes->addOption(new CRUIOptionItem(PROP_APP_THEME_VALUE_WHITE, STR_SETTINGS_THEME_VALUE_WHITE));
    themes->addOption(new CRUIOptionItem(PROP_APP_THEME_VALUE_BLACK, STR_SETTINGS_THEME_VALUE_BLACK));
    //themes->setDefaultValue(PROP_APP_THEME_VALUE_LIGHT);
    interfaceSettings->addChild(themes);
    _readerSettings.addChild(interfaceSettings);

    CRUISettingsList * pageLayout = new CRUISettingsList(STR_SETTINGS_PAGE_LAYOUT, STR_SETTINGS_PAGE_LAYOUT_DESCRIPTION, SETTINGS_PATH_READER_PAGELAYOUT);
    pageLayout->addChild(new CRUIInterlineSpaceSetting(STR_SETTINGS_INTERLINE_SPACE, NULL, PROP_INTERLINE_SPACE));
    pageLayout->addChild(new CRUIPageMarginsSetting(STR_SETTINGS_PAGE_MARGINS, NULL, PROP_PAGE_MARGINS));
    CRUISettingsOptionList * viewmode = new CRUISettingsOptionList(STR_SETTINGS_VIEW_MODE, STR_SETTINGS_VIEW_MODE_DESCRIPTION, PROP_PAGE_VIEW_MODE);
    viewmode->addOption(new CRUIOptionItem(PROP_PAGE_VIEW_MODE_VALUE_SCROLL, STR_SETTINGS_VIEW_MODE_VALUE_SCROLL));
    viewmode->addOption(new CRUIOptionItem(PROP_PAGE_VIEW_MODE_VALUE_1PAGE, STR_SETTINGS_VIEW_MODE_VALUE_1PAGE));
    viewmode->addOption(new CRUIOptionItem(PROP_PAGE_VIEW_MODE_VALUE_2PAGES, STR_SETTINGS_VIEW_MODE_VALUE_2PAGES));
    pageLayout->addChild(viewmode);
    CRUISettingsOptionList * animationmode = new CRUISettingsOptionList(STR_SETTINGS_VIEW_PAGE_ANIMATION, STR_SETTINGS_VIEW_PAGE_ANIMATION_DESCRIPTION, PROP_PAGE_VIEW_ANIMATION);
    animationmode->addOption(new CRUIOptionItem(PROP_PAGE_VIEW_ANIMATION_VALUE_NONE, STR_SETTINGS_VIEW_PAGE_ANIMATION_VALUE_NONE));
    animationmode->addOption(new CRUIOptionItem(PROP_PAGE_VIEW_ANIMATION_VALUE_SLIDE1, STR_SETTINGS_VIEW_PAGE_ANIMATION_VALUE_SLIDE1));
    animationmode->addOption(new CRUIOptionItem(PROP_PAGE_VIEW_ANIMATION_VALUE_SLIDE2, STR_SETTINGS_VIEW_PAGE_ANIMATION_VALUE_SLIDE2));
    animationmode->addOption(new CRUIOptionItem(PROP_PAGE_VIEW_ANIMATION_VALUE_FADE, STR_SETTINGS_VIEW_PAGE_ANIMATION_VALUE_FADE));
    animationmode->addOption(new CRUIOptionItem(PROP_PAGE_VIEW_ANIMATION_VALUE_3D, STR_SETTINGS_VIEW_PAGE_ANIMATION_VALUE_3D));
    pageLayout->addChild(animationmode);
    _readerSettings.addChild(pageLayout);

    CRUISettingsList * formattingOptions = new CRUISettingsList(STR_SETTINGS_TEXT_FORMATTING, STR_SETTINGS_TEXT_FORMATTING_DESCRIPTION, SETTINGS_PATH_READER_TEXTFORMATTING);
    formattingOptions->addChild(new CRUISettingsCheckbox(STR_SETTINGS_FLOATING_PUNCTUATION, NULL, PROP_FLOATING_PUNCTUATION, STR_SETTINGS_FLOATING_PUNCTUATION_VALUE_ON, STR_SETTINGS_FLOATING_PUNCTUATION_VALUE_OFF));
    formattingOptions->addChild(new CRUISettingsCheckbox(STR_SETTINGS_FONT_KERNING, NULL, PROP_FONT_KERNING_ENABLED, STR_SETTINGS_FONT_KERNING_VALUE_ON, STR_SETTINGS_FONT_KERNING_VALUE_OFF));
    CRUISettingsOptionList * hyph = new CRUISettingsOptionList(STR_SETTINGS_HYPHENATION_DICTIONARY, NULL, PROP_HYPHENATION_DICT);
    for (int i = 0; i < crconfig.hyphenationDictionaries.length(); i++) {
        CRUIHyphenationDictionary * dict = crconfig.hyphenationDictionaries[i];
        hyph->addOption(new CRUIOptionItem(dict->id.c_str(), dict->nameRes.c_str()));
    }
    formattingOptions->addChild(hyph);
    _readerSettings.addChild(formattingOptions);

}

void CRUIMainWidget::showVirtualKeyboard(int mode, lString16 text, bool multiline) {
    if (_keyboard)
        return;
    if (_platform->supportsVirtualKeyboard()) {
        if (_platform->isVirtualKeyboardShown())
            return;
        _platform->showVirtualKeyboard(mode, text, multiline);
    }else {
        _keyboard = new CRUIVirtualKeyboard();
        requestLayout();
    }
}

void CRUIMainWidget::hideVirtualKeyboard() {
    if (_platform->supportsVirtualKeyboard()) {
        if (_platform->isVirtualKeyboardShown())
            _platform->hideVirtualKeyboard();
    } else {
        if (!_keyboard)
            return;
        delete _keyboard;
        _keyboard = NULL;
        requestLayout();
    }
}

bool CRUIMainWidget::isVirtualKeyboardShown() {
    if (_keyboard != NULL)
        return true;
    if (_platform->supportsVirtualKeyboard()) {
        return _platform->isVirtualKeyboardShown();
    }
    return false;
}

void CRUIMainWidget::updateFolderBookmarks() {
    if (_home)
        _home->updateFolderBookmarks();
}


CRUIMainWidget::CRUIMainWidget()
: _eventManager(NULL), _home(NULL), _read(NULL)
, _popup(NULL), _keyboard(NULL), _popupBackground(NULL),    _screenUpdater(NULL)
, _platform(NULL), _lastAnimationTs(0), _initialized(false)
, _browserSettings(STR_SETTINGS_BROWSER, STR_SETTINGS_BROWSER_DESC, SETTINGS_PATH_BROWSER)
, _readerSettings(STR_SETTINGS_READER, STR_SETTINGS_READER_DESC, SETTINGS_PATH_READER)
, _downloadMap(100)
{
    setId("MAIN");
    _currentSettings = LVCreatePropsContainer(); // currently active settings
    _newSettings = LVCreatePropsContainer(); // to be edited by Settings editors
    CRLog::info("Loading settings from %s", crconfig.iniFile.c_str());
    LVStreamRef stream = LVOpenFileStream(crconfig.iniFile.c_str(), LVOM_READ);
    if (!stream.isNull())
        _currentSettings->loadFromStream(stream.get());
    int oldPropCount = _currentSettings->getCount();
    _currentSettings->setStringDef(PROP_APP_INTERFACE_LANGUAGE, PROP_APP_INTERFACE_LANGUAGE_VALUE_SYSTEM);
    _currentSettings->setStringDef(PROP_HYPHENATION_DICT, "en");
    _currentSettings->setStringDef(PROP_APP_THEME, PROP_APP_THEME_VALUE_LIGHT);
    _currentSettings->setStringDef(PROP_APP_THEME_DAY, PROP_APP_THEME_VALUE_LIGHT);
    _currentSettings->setStringDef(PROP_APP_THEME_NIGHT, PROP_APP_THEME_VALUE_DARK);
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_DOUBLE "1", "LINK_BACK");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_DOUBLE "2", "TOGGLE_NIGHT_MODE");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_DOUBLE "3", "TOC");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_DOUBLE "5", "SETTINGS");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_DOUBLE "7", "GOTO_PERCENT");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "1", "PAGE_UP");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "2", "PAGE_UP");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "3", "PAGE_DOWN");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "4", "PAGE_UP");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "5", "MENU");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "6", "PAGE_DOWN");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "7", "PAGE_DOWN");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "8", "PAGE_DOWN");
    _currentSettings->setStringDef(PROP_APP_TAP_ZONE_ACTION_NORMAL "9", "PAGE_DOWN");

    _currentSettings->setStringDef(PROP_PAGE_VIEW_MODE, PROP_PAGE_VIEW_MODE_VALUE_2PAGES);
    _currentSettings->setStringDef(PROP_PAGE_VIEW_ANIMATION, PROP_PAGE_VIEW_ANIMATION_VALUE_3D);

    _currentSettings->setStringDef(PROP_FONT_FACE, crconfig.uiFontFace.c_str());
    _currentSettings->setIntDef(PROP_FONT_SIZE, crconfig.defFontSize);
    _currentSettings->setColorDef(PROP_FONT_COLOR, 0x000000);
    _currentSettings->setColorDef(PROP_FONT_COLOR_DAY, 0x000000);
    _currentSettings->setColorDef(PROP_FONT_COLOR_NIGHT, 0xE6D577);
    _currentSettings->setColorDef(PROP_BACKGROUND_COLOR, 0xFFFFFF);
    _currentSettings->setColorDef(PROP_BACKGROUND_COLOR_DAY, 0xFFFFFF);
    _currentSettings->setColorDef(PROP_BACKGROUND_COLOR_NIGHT, 0x000000);
    _currentSettings->setStringDef(PROP_BACKGROUND_IMAGE_ENABLED, "1");
    _currentSettings->setStringDef(PROP_BACKGROUND_IMAGE_ENABLED_DAY, "1");
    _currentSettings->setStringDef(PROP_BACKGROUND_IMAGE_ENABLED_NIGHT, "1");
    _currentSettings->setColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS, COLOR_TRANSFORM_BRIGHTNESS_NONE);
    _currentSettings->setColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS_DAY, COLOR_TRANSFORM_BRIGHTNESS_NONE);
    _currentSettings->setColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS_NIGHT, 0x454543);
    _currentSettings->setColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST, COLOR_TRANSFORM_CONTRAST_NONE);
    _currentSettings->setColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST_DAY, COLOR_TRANSFORM_CONTRAST_NONE);
    _currentSettings->setColorDef(PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST_NIGHT, COLOR_TRANSFORM_CONTRAST_NONE);

    _currentSettings->setStringDef(PROP_BACKGROUND_IMAGE, "@sand1");
    _currentSettings->setStringDef(PROP_BACKGROUND_IMAGE_DAY, "@sand1");
    _currentSettings->setStringDef(PROP_BACKGROUND_IMAGE_NIGHT, "@sand1_dark");
    _currentSettings->setStringDef(PROP_FONT_ANTIALIASING, "1");
    _currentSettings->setStringDef(PROP_FONT_HINTING, "0");
    _currentSettings->setStringDef(PROP_FONT_KERNING_ENABLED, "1");
    _currentSettings->setStringDef(PROP_FLOATING_PUNCTUATION, "1");
    _currentSettings->setStringDef(PROP_FONT_WEIGHT_EMBOLDEN, "0");
    _currentSettings->setStringDef(PROP_FONT_GAMMA_INDEX, "15");
    _currentSettings->setStringDef(PROP_FONT_GAMMA_INDEX_DAY, "15");
    _currentSettings->setStringDef(PROP_FONT_GAMMA_INDEX_NIGHT, "15");
    _currentSettings->setIntDef(PROP_INTERLINE_SPACE, 120);
    _currentSettings->setIntDef(PROP_PAGE_MARGINS, 500);
    _currentSettings->setStringDef(PROP_NIGHT_MODE, "0");

    _currentSettings->setIntDef(PROP_HIGHLIGHT_COMMENT_BOOKMARKS, (int)highlight_mode_solid);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_SELECTION_COLOR, 0xD0D0D0);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_BOOKMARK_COLOR_COMMENT, 0xA08000);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_BOOKMARK_COLOR_CORRECTION, 0xA00000);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_SELECTION_COLOR_DAY, 0xD0D0D0);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_BOOKMARK_COLOR_COMMENT_DAY, 0xA08000);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_BOOKMARK_COLOR_CORRECTION_DAY, 0xA00000);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_SELECTION_COLOR_NIGHT, 0x606050);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_BOOKMARK_COLOR_COMMENT_NIGHT, 0x808020);
    _currentSettings->setColorDef(PROP_HIGHLIGHT_BOOKMARK_COLOR_CORRECTION_NIGHT, 0x800000);

    if (_currentSettings->getCount() != oldPropCount) {
        saveSettings();
    }
    createBrowserSettings();
    createReaderSettings();

    _home = new CRUIHomeWidget(this);
    _read = new CRUIReadWidget(this);
    _history.add(new HomeItem(this, _home));

    applySettings(_currentSettings, _currentSettings, _currentSettings);
}

static volatile int LAST_UPDATE_REQUEST_ID = 1;
static volatile bool LAST_UPDATE_REQUEST_UPDATE_NOW = false;
static volatile bool LAST_UPDATE_REQUEST_CANCELLED_ALL = false;

class CRUIUpdateEvent : public CRRunnable {
    CRUIScreenUpdateManagerCallback * _screenUpdater;
    int _updateRequestId;
    bool _updateNow;
    int _animationFps;
public:
    CRUIUpdateEvent(CRUIScreenUpdateManagerCallback * screenUpdater, bool updateNow, int animationFps)
        : _screenUpdater(screenUpdater), _updateNow(updateNow), _animationFps(animationFps) {
        _updateRequestId = ++LAST_UPDATE_REQUEST_ID;
        LAST_UPDATE_REQUEST_UPDATE_NOW |= updateNow;
    }

    virtual void run() {
        if (_updateRequestId == LAST_UPDATE_REQUEST_ID && !LAST_UPDATE_REQUEST_CANCELLED_ALL) {
            _screenUpdater->setScreenUpdateMode(LAST_UPDATE_REQUEST_UPDATE_NOW | _updateNow, _animationFps);
            LAST_UPDATE_REQUEST_UPDATE_NOW = false;
        }
    }
};

/// forward screen update request to external code
void CRUIMainWidget::setScreenUpdateMode(bool updateNow, int animationFps) {
    if (_screenUpdater) {
        concurrencyProvider->executeGui(new CRUIUpdateEvent(_screenUpdater, updateNow, animationFps));
        //_screenUpdater->setScreenUpdateMode(updateNow, animationFps);
    }
}

CRUIMainWidget::~CRUIMainWidget() {
    LAST_UPDATE_REQUEST_CANCELLED_ALL = true;
    if (_home)
        delete _home;
    if (_read)
        delete _read;
}

void CRUIMainWidget::clearImageCaches() {
	CRLog::info("CRUIMainWidget::clearImageCaches()");
	_read->clearImageCaches();
	if (_popup)
		hideSlowOperationPopup();
}

int CRUIMainWidget::getChildCount() {
    int cnt = 0;
    if (_history.currentWidget())
        cnt++;
    if (_popup)
        cnt++;
    if (_keyboard)
        cnt++;
    return cnt; //_currentWidget->getChildCount();
}

CRUIWidget * CRUIMainWidget::getChild(int index) {
    if (index == 0)
        return _history.currentWidget();
    if (_popup && index == 1)
        return _popup;
    if (_keyboard && index == 1)
        return _keyboard;
    if (_popup && _keyboard)
        return _keyboard;
    return NULL;
    //return _currentWidget->getChild(index);
}

/// measure dimensions
void CRUIMainWidget::measure(int baseWidth, int baseHeight) {
    if (_popupBackground)
        return;
    _history.currentWidget()->measure(baseWidth, baseHeight);
    _measuredWidth = _history.currentWidget()->getMeasuredWidth();
    _measuredHeight = _history.currentWidget()->getMeasuredHeight();
    if (_popup)
        _popup->measure(baseWidth, baseHeight);
    if (_keyboard)
        _keyboard->measure(baseWidth, baseHeight);
}

/// updates widget position based on specified rectangle
void CRUIMainWidget::layout(int left, int top, int right, int bottom) {
    //CRLog::trace("CRUIMainWidget::layout");
    if (_popupBackground)
        return;
    _history.currentWidget()->layout(left, top, right, bottom);
    _pos.left = left;
    _pos.top = top;
    _pos.right = right;
    _pos.bottom = bottom;
    if (_popup)
        _popup->layout(left, top, right, bottom);
    if (_keyboard)
        _keyboard->layout(left, bottom - _keyboard->getMeasuredHeight(), right, bottom);
    _layoutRequested = false;
}

/// draw now if force == true, layout/draw if necessary when force == false
void CRUIMainWidget::update(bool force) {
    if (force)
        invalidate();
    bool needLayout, needDraw, animating;
    CRUICheckUpdateOptions(this, needLayout, needDraw, animating);
    if (force || animating)
        needDraw = true;
    if (needDraw || animating)
        setScreenUpdateMode(needDraw, animating ? 30 : 0);

}

// apply changed settings
void CRUIMainWidget::applySettings() {
    CRPropRef diff = LVCreatePropsContainer();
    for (int i = 0; i <_newSettings->getCount(); i++) {
        lString16 oldValue = _currentSettings->getStringDef(_newSettings->getName(i));
        lString16 newValue = _newSettings->getValue(i);
        if (oldValue != newValue)
            diff->setString(_newSettings->getName(i), _newSettings->getValue(i));
    }
    applySettings(diff, _currentSettings, _newSettings);
}

// apply changed settings
void CRUIMainWidget::applySettings(CRPropRef changed, CRPropRef oldSettings, CRPropRef newSettings) {
    if (changed.isNull() || changed->getCount() == 0)
        return; // no changes
    bool themeChanged = false;
    _read->applySettings(changed, oldSettings, newSettings);
    for (int i = 0; i < _history.length(); i++) {
        if (_history[i]->getMode() != MODE_READ)
            _history[i]->getWidget()->applySettings(changed, oldSettings, newSettings);
    }
    for (int i = 0; i <changed->getCount(); i++) {
        lString8 key(changed->getName(i));
        //lString16 oldValue = oldSettings->getStringDef(key.c_str());
        lString16 newValue = changed->getValue(i);
        if (key == PROP_APP_INTERFACE_LANGUAGE) {
            lString8 lang = UnicodeToUtf8(newValue);
            crconfig.setInterfaceLanguage(lang);
            requestLayout();
        }
        if (key == PROP_APP_THEME) {
            crconfig.setTheme(UnicodeToUtf8(newValue));
            crconfig.setupResourcesForScreenSize();
            themeChanged = true;
        }
    }
    _currentSettings->set(LVClonePropsContainer(newSettings));
    saveSettings();
    invalidate();
    if (themeChanged)
        onThemeChanged();
}

void CRUIMainWidget::saveSettings() {
    LVStreamRef stream = LVOpenFileStream(crconfig.iniFile.c_str(), LVOM_WRITE);
    if (!stream.isNull())
        _currentSettings->saveToStream(stream.get());
}

void CRUIMainWidget::beforeNavigation(NavHistoryItem * from, NavHistoryItem * to) {
    from->getWidget()->beforeNavigationFrom();
    to->getWidget()->beforeNavigationTo();
}

void CRUIMainWidget::afterNavigation(NavHistoryItem * from, NavHistoryItem * to) {
    if (from->getMode() == MODE_SETTINGS && to->getMode() != MODE_SETTINGS) {
    	CRLog::info("Closed settings - applying changes");
        applySettings();
    }
    from->getWidget()->afterNavigationFrom();
    to->getWidget()->afterNavigationTo();
}

void CRUIMainWidget::startAnimation(int newpos, int duration, const CRUIMotionEvent * event) {
    if (_animation.active) {
        stopAnimation();
    }
    int oldpos = _history.pos();
    if (newpos == oldpos)
        return;

    beforeNavigation(_history[oldpos], _history[newpos]);
    CRUIWidget * newWidget = _history[newpos]->getWidget();
    CRUIWidget * oldWidget = _history[oldpos]->getWidget();
    if (!newWidget || !oldWidget)
        return;
    bool manual = event != NULL;
    int direction;
    if (oldpos < newpos)
        direction = -1;
    else
        direction = +1;
    CRLog::trace("starting animation %d -> %d", oldpos, newpos);

    if (event) {
        // manual
    	CRLog::trace("Intercepting touch event for navigation dragging");
    	_eventManager->interceptTouchEvent(event, this);
        _animation.startPoint.x = event->getAvgStartX();
        _animation.startPoint.y = event->getAvgStartY();
    }

    CRReinitTimer();
    _animation.active = true;
    _animation.oldpos = oldpos;
    _animation.newpos = newpos;
    _animation.duration = duration * 10;
    _animation.direction = direction;
    _animation.progress = 0;
    _animation.manual = manual;

    oldWidget->measure(_pos.width(), _pos.height());
    oldWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    if (_history[oldpos]->getMode() != MODE_READ) {
        _animation.oldimage = new GLDrawBuf(_pos.width(), _pos.height(), 32, true);
        _animation.oldimage->beforeDrawing();
        oldWidget->draw(_animation.oldimage);
        _animation.oldimage->afterDrawing();
    }
    newWidget->measure(_pos.width(), _pos.height());
    newWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    if (_history[newpos]->getMode() != MODE_READ) {
        _animation.newimage = new GLDrawBuf(_pos.width(), _pos.height(), 32, true);
        _animation.newimage->beforeDrawing();
        newWidget->draw(_animation.newimage);
        _animation.newimage->afterDrawing();
    }


    if (duration == 0) {
        stopAnimation();
    } else {
        requestLayout();
        update(true);
    }
}

void CRUIMainWidget::stopAnimation() {
    if (!_animation.active)
        return;
    CRLog::trace("stopping animation %d, %d", _animation.oldpos, _animation.newpos);
    _animation.active = false;
    CRUIWidget * newWidget = _history[_animation.newpos]->getWidget();
    CRUIWidget * oldWidget = _history[_animation.oldpos]->getWidget();
    oldWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    newWidget->layout(_pos.left, _pos.top, _pos.right, _pos.bottom);
    if (_animation.oldimage) {
        delete _animation.oldimage;
        _animation.oldimage = NULL;
    }
    if (_animation.newimage) {
        delete _animation.newimage;
        _animation.newimage = NULL;
    }
    afterNavigation(_history[_animation.oldpos], _history[_animation.newpos]);
    _history.setPos(_animation.newpos);
    requestLayout();
    update(true);
}

void CRUIMainWidget::animate(lUInt64 millisPassed) {
    CRUIWidget::animate(millisPassed);
    if (_animation.active) {
        if (!_animation.manual) {
            _animation.progress += (int)millisPassed * 10;
        }
        int p = _animation.progress;
        //CRLog::trace("animating ts = %lld,  passed = %d   %d of %d", (lUInt64)GetCurrentTimeMillis(), (int)millisPassed, (int)p, (int)_animation.duration);
        if (p > _animation.duration) {
            stopAnimation();
        } else {
            int x = _pos.width() * p / _animation.duration;
            //CRLog::trace("animation position %d", x);
            if (_animation.direction < 0) {
                _animation.oldimagex = _pos.left - x;
                _animation.newimagex = _pos.right - x;
            } else {
                _animation.oldimagex = _pos.left + x;
                _animation.newimagex = _pos.left - _pos.width() + x;
            }
        }
    }
}

class MainWidgetUpdateCallback : public CRRunnable {
    CRUIMainWidget * _main;
public:
    MainWidgetUpdateCallback(CRUIMainWidget * main) : _main(main) {}
    virtual void run() {
    	//CRLog::trace("Updating from callback");
        _main->requestLayout();
        _main->update(true);
    }
};

CRRunnable * CRUIMainWidget::createUpdateCallback() {
	//CRLog::trace("Creating update callback");
    return new MainWidgetUpdateCallback(this);
}

bool CRUIMainWidget::isAnimating() {
    return _animation.active;
}


/// draws widget with its children to specified surface
void CRUIMainWidget::draw(LVDrawBuf * buf) {
    if (!_initialized)
        runStartupTasksIfNeeded();

    bool needLayout, needDraw, animating;
    CRUICheckUpdateOptions(this, needLayout, needDraw, animating);
    if (animating) {
        lUInt64 ts = GetCurrentTimeMillis();
        if (!_lastAnimationTs)
            _lastAnimationTs = ts;
        lUInt64 millisDiff = ts - _lastAnimationTs;
        /// call animate
        animate(millisDiff);
        _lastAnimationTs = ts;
    } else {
        _lastAnimationTs = 0;
    }

    if (_popupBackground) {
        //CRLog::trace("Drawing static background");
        buf->DrawRescaled(_popupBackground, 0, 0, _pos.width(), _pos.height(), 0);
        //_popupBackground->DrawTo(buf, 0, 0, 0, NULL);
    } else {
        if (_animation.active) {
            if (_animation.oldimage)
                _animation.oldimage->DrawTo(buf, _animation.oldimagex, _pos.top, 0, NULL);
            else {
                CRUIWidget * oldWidget = _history[_animation.oldpos]->getWidget();
                oldWidget->layout(_pos.left + _animation.oldimagex, _pos.top, _pos.right + _animation.oldimagex, _pos.bottom);
                oldWidget->draw(buf);
            }
            if (_animation.newimage)
                _animation.newimage->DrawTo(buf, _animation.newimagex, _pos.top, 0, NULL);
            else {
                CRUIWidget * newWidget = _history[_animation.newpos]->getWidget();
                newWidget->layout(_pos.left + _animation.newimagex, _pos.top, _pos.right + _animation.newimagex, _pos.bottom);
                newWidget->draw(buf);
            }
        } else {
            _history.currentWidget()->draw(buf);
        }
    }
    if (_popup) {
        //CRLog::trace("Drawing popup");
        _popup->draw(buf);
    }
    if (_keyboard) {
        //CRLog::trace("Drawing popup");
        _keyboard->draw(buf);
    }
    if (!_messageText.empty()) {
        CRUITextWidget text(_messageText);
        text.setFontSize(FONT_SIZE_SMALL);
        text.setMaxLines(2);
        text.setPadding(PT_TO_PX(7));
        text.setLayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        text.measure(_pos.width(), _pos.height());
        text.setBackground(0x60FFFFFF);
        text.setTextColor(0x00FF0000);
        int dx = text.getMeasuredWidth();
        int dy = text.getMeasuredHeight();
        if (dx > _pos.width())
            dx = _pos.width();
        if (dy > _pos.height())
            dy = _pos.height();
        int ddx = (_pos.width() - dx) / 2;
        int ddy = (_pos.height() - dy);
        lvRect rc;
        rc.left = _pos.left + ddx;
        rc.right = _pos.right - ddx;
        rc.top = _pos.top + ddy * 4 / 5;
        rc.bottom = _pos.bottom - ddy / 5;
        text.layout(rc.left, rc.top, rc.right, rc.bottom);
        text.draw(buf);
    }
    _drawRequested = false;
    setScreenUpdateMode(false, animating ? 30 : 0);
}

/// motion event handler, returns true if it handled event
bool CRUIMainWidget::onTouchEvent(const CRUIMotionEvent * event) {
    if (_keyboard && _keyboard->onTouchEvent(event))
        return true;
    return _history.currentWidget()->onTouchEvent(event);
}

CRFileItem * CRUIMainWidget::createManualBook() {
    lString8 fn = crconfig.manualsDir + "manual_template_en.fb2";
    CRFileItem * f = new CRFileItem(fn, false);
    LVPtrVector<BookDBBook> books;
    BookDBBook * book = NULL;
    book = bookDB->loadBook(fn);
    if (!book)
        book = new BookDBBook();
    book->pathname = DBString(fn.c_str());
    book->filename = DBString(LVExtractFilename(fn).c_str());
    book->title = DBString("Cool Reader Manual");
    BookDBFolder * folder = new BookDBFolder();
    folder->name = DBString(LVExtractPath(fn).c_str());
    book->folder = folder;
    books.add(book);
    if (book->id == 0)
        bookDB->saveBooks(books);
    f->setBook(book->clone());
    return f;
}

void copyDayNightSetting(CRPropRef & props, const char * from, const char * to, const char * prop) {
    lString16 value;
    lString8 key = lString8(prop);
    lString8 fromKey = key + from;
    lString8 toKey = key + to;
    if (props->getString(fromKey.c_str(), value))
        props->setString(toKey.c_str(), value);
}

void copyDayNightSettings(CRPropRef & props, const char * from, const char * to) {
    copyDayNightSetting(props, from, to, PROP_FONT_COLOR);
    copyDayNightSetting(props, from, to, PROP_APP_THEME);
    copyDayNightSetting(props, from, to, PROP_BACKGROUND_IMAGE_ENABLED);
    copyDayNightSetting(props, from, to, PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS);
    copyDayNightSetting(props, from, to, PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST);
    copyDayNightSetting(props, from, to, PROP_BACKGROUND_COLOR);
    copyDayNightSetting(props, from, to, PROP_BACKGROUND_IMAGE);
    copyDayNightSetting(props, from, to, PROP_FONT_GAMMA_INDEX);
    copyDayNightSetting(props, from, to, PROP_HIGHLIGHT_SELECTION_COLOR);
    copyDayNightSetting(props, from, to, PROP_HIGHLIGHT_BOOKMARK_COLOR_COMMENT);
    copyDayNightSetting(props, from, to, PROP_HIGHLIGHT_BOOKMARK_COLOR_CORRECTION);
}

/// handle menu or other action
bool CRUIMainWidget::onAction(const CRUIAction * action) {
    if (!action)
        return NULL;
    switch (action->id) {
    case CMD_EXIT:
        if (getPlatform() != NULL)
            getPlatform()->exitApp();
        return true;
    case CMD_READER_HOME:
        showHome();
        return true;
    case CMD_CURRENT_BOOK:
        openBook(NULL);
        return true;
    case CMD_SHOW_FOLDER:
        showFolder(action->sparam, false);
        return true;
    case CMD_NIGHT_MODE:
    case CMD_DAY_MODE:
    case CMD_TOGGLE_NIGHT_MODE:
        {
        	CRLog::trace("toggling Night Mode");
            CRPropRef props = initNewSettings();
            if (props->getBoolDef(PROP_NIGHT_MODE, false)) {
                // to day mode
                copyDayNightSettings(props, "", ".night");
                copyDayNightSettings(props, ".day", "");
                props->setBool(PROP_NIGHT_MODE, false);
            } else {
                // to night mode
                copyDayNightSettings(props, "", ".day");
                copyDayNightSettings(props, ".night", "");
                props->setBool(PROP_NIGHT_MODE, true);
            }
            applySettings();
            CRLog::trace("Night mode is now %s", props->getBoolDef(PROP_NIGHT_MODE, false) ? "on" : "off");
            return true;
        }
    case CMD_HELP:
        {
            CRFileItem * book = createManualBook();
            openBook(book);
            delete book;
        }
        return true;
    }
    return false;
}

/// handle menu or other action - find standard action by id
bool CRUIMainWidget::onAction(int actionId) {
    return onAction(CRUIActionByCode(actionId));
}

/// motion event handler - before children, returns true if it handled event
bool CRUIMainWidget::onTouchEventPreProcess(const CRUIMotionEvent * event) {
    lvPoint pt(event->getX(), event->getY());
    bool insideVirtualKeyboard = false;
    if (_keyboard && _keyboard->getPos().isPointInside(pt))
        insideVirtualKeyboard = true;
    if (insideVirtualKeyboard)
        return false;
    // by returning of true, just ignore all events while animation is on
    if (_animation.active && _animation.manual) {
        switch(event->getAction()) {
        case ACTION_MOVE:
            {
                int dx = _animation.startPoint.x - event->getAvgX();
                int p = dx * _animation.duration / _pos.width();
                if (_animation.direction > 0)
                    p = -p;
                _animation.progress = p > 0 ? p : 0;
            	CRLog::trace("Tracking navigation touch event: dx=%d pointers=%d, progress=%d", dx, event->count(), p);
                invalidate();
                //CRLog::trace("manual animation progress %d", p);
            }
            break;
        case ACTION_CANCEL:
        case ACTION_DOWN:
        case ACTION_UP:
            _animation.manual = false;
            event->cancelAllPointers();
            break;
        default:
            break;
        }

        return true;
    } else {
		//CRLog::trace("onTouchEventPreProcess action=%d pointers=%d", event->getAction(), event->count());
    	if (event->count() == 2 && event->getAction() == ACTION_MOVE && event->getWidget() != this) {
    		if (event->getWidget() && isChild(event->getWidget()) && !event->getWidget()->allowInterceptTouchEvent(event))
    			return false;
    		bool startDrag = false;
    		int dx1 = event->getDeltaX(0);
    		int dx2 = event->getDeltaX(1);
    		int adx1 = event->getDistanceX(0);
    		int adx2 = event->getDistanceX(1);
    		int ady1 = event->getDistanceY(0);
    		int ady2 = event->getDistanceY(1);
    		//CRLog::trace("checking if need to intercept navigation dx1=%d, dx2=%d, adx1=%d, adx2=%d, ady1=%d, ady2=%d", dx1, dx2, adx1, adx2, ady1, ady2);
    		if ((adx1 > DRAG_THRESHOLD_X || adx2 > DRAG_THRESHOLD_X) && (adx1 + adx2 > ady1 + ady2)) {
    			if (dx1 < 0 && dx2 < 0 && adx1 > DRAG_THRESHOLD_X / 2 && adx2 > DRAG_THRESHOLD_X / 2) {
    				startDrag = true;
    			} else if (dx1 > 0 && dx2 > 0 && adx1 > DRAG_THRESHOLD_X / 2 && adx2 > DRAG_THRESHOLD_X / 2) {
    				startDrag = true;
    			}
    		}
    		if (startDrag) {
    			CRLog::trace("Intercepting double finger horizontal drag");
    			return startDragging(event, false);
    		}
    	}
    }
    if (_animation.active)
        return true;
    if (_history.currentWidget() && event->getAction() == ACTION_DOWN)
        return _history.currentWidget()->onTouchEventPreProcess(event);
    return false;
}

/// return true if drag operation is intercepted
bool CRUIMainWidget::startDragging(const CRUIMotionEvent * event, bool vertical) {
    if (vertical)
        return false;
    int dx = event->getAvgDeltaX();
    if (dx > 0 && !_history.hasBack())
        return false;
    if (dx < 0 && !_history.hasForward())
        return false;
    if (dx < 0) {
        // FORWARD dragging
    	CRLog::trace("Initiating manual FORWARD dragging");
        startAnimation(_history.pos() + 1, WINDOW_ANIMATION_DELAY, event);
        return true;
    } else {
        // BACK dragging
    	CRLog::trace("Initiating manual BACK dragging");
        startAnimation(_history.pos() - 1, WINDOW_ANIMATION_DELAY, event);
        return true;
    }
}

/// returns 0 if not supported, task ID if download task is started
int CRUIMainWidget::openUrl(CRUIWindowWidget * callback, lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs) {
    int id = _platform->openUrl(url, method, login, password, saveAs);
    if (!id)
        return 0;
    _downloadMap.set(id, callback);
    return id;
}

/// cancel specified download task
void CRUIMainWidget::cancelDownload(int downloadTaskId) {
    _platform->cancelDownload(downloadTaskId);
    _downloadMap.remove(downloadTaskId);
}

/// pass download result to window
void CRUIMainWidget::onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream) {
    CRUIWindowWidget * callback = _downloadMap.get(downloadTaskId);
    if (!isChild(callback))
        return;
    callback->onDownloadResult(downloadTaskId, url, result, resultMessage, mimeType, size, stream);
}

/// download progress
void CRUIMainWidget::onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded) {
    CRUIWindowWidget * callback = _downloadMap.get(downloadTaskId);
    if (!isChild(callback))
        return;
    callback->onDownloadProgress(downloadTaskId, url, result, resultMessage, mimeType, size, sizeDownloaded);
}
