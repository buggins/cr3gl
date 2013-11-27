

#include "crui.h"
#include "cruiopdsprops.h"
#include "cruilist.h"
#include "cruicontrols.h"
#include "crcoverpages.h"
#include "cruimain.h"
#include "cruicoverwidget.h"
#include "stringresource.h"

using namespace CRUI;


CRUIOpdsPropsWidget::CRUIOpdsPropsWidget(CRUIMainWidget * main, BookDBCatalog * catalog) : CRUIWindowWidget(main), _title(NULL)
//, _fileList(NULL),
    , _catalog(NULL)
{
    if (catalog)
        _catalog = catalog ->clone();
    else {
        _catalog = new BookDBCatalog();
        _catalog->name = "";
        _catalog->url = "http://";
    }
    _title = new CRUITitleBarWidget(lString16(""), this, this, false);
    _title->setTitle(STR_OPDS_CATALOG_PROPS_DIALOG_TITLE);
    _body->addChild(_title);
    //_fileList = new CRUIOpdsItemListWidget(this);
    //_body->addChild(_fileList);
    //_fileList->setOnItemClickListener(this);
    CRUITableLayout * layout = new CRUITableLayout(2);
    layout->setLayoutParams(FILL_PARENT, FILL_PARENT);

    // add edit boxes
    CRUITextWidget * label = new CRUITextWidget(STR_OPDS_CATALOG_NAME);
    _edTitle = new CRUIEditWidget();
    label->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
    _edTitle->setText(Utf8ToUnicode(_catalog->name.c_str()));
    _edTitle->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    layout->addChild(label);
    layout->addChild(_edTitle);
    label = new CRUITextWidget(STR_OPDS_CATALOG_URL);
    label->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
    _edUrl = new CRUIEditWidget();
    _edUrl->setText(Utf8ToUnicode(_catalog->url.c_str()));
    _edUrl->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    layout->addChild(label);
    layout->addChild(_edUrl);
    label = new CRUITextWidget(STR_OPDS_CATALOG_LOGIN);
    label->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
    _edLogin = new CRUIEditWidget();
    _edLogin->setText(Utf8ToUnicode(_catalog->login.c_str()));
    _edLogin->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    layout->addChild(label);
    layout->addChild(_edLogin);
    label = new CRUITextWidget(STR_OPDS_CATALOG_PASSWORD);
    label->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
    _edPassword = new CRUIEditWidget();
    _edPassword->setText(Utf8ToUnicode(_catalog->password.c_str()));
    _edPassword->setPasswordChar('*');
    _edPassword->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    layout->addChild(label);
    layout->addChild(_edPassword);
    layout->setPadding(PT_TO_PX(3));

    // add spacers
    CRUIWidget * spacer = new CRUIWidget();
    spacer->setLayoutParams(WRAP_CONTENT, FILL_PARENT);
    layout->addChild(spacer);
    spacer = new CRUIWidget();
    spacer->setLayoutParams(FILL_PARENT, FILL_PARENT);
    layout->addChild(spacer);

    layout->setStyle("SETTINGS_ITEM_LIST");

    _body->addChild(layout);
}

bool CRUIOpdsPropsWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    else if (widget->getId() == "MENU") {
        onAction(CMD_MENU);
    }
    return true;
}

bool CRUIOpdsPropsWidget::onLongClick(CRUIWidget * widget) {
//    if (widget->getId() == "BACK") {
//        CRUIActionList actions;
//        lString8 path = _dir->getPathName();
//        lString8 lastPath = path;
//        for (;;) {
//            LVRemovePathDelimiter(path);
//            path = LVExtractPath(path);
//            if (path == lastPath)
//                break;
//            LVRemovePathDelimiter(path);
//            CRUIAction action(CMD_SHOW_FOLDER);
//            action.icon_res = "folder_icon";
//            action.name = Utf8ToUnicode(path);
//            action.sparam = path;
//            actions.add(&action);
//            lastPath = path;
//            if (path=="/" || path.endsWith(":\\") || path.endsWith("\\\\") || path == "@/" || path == "@\\")
//                break;
//        }
//        actions.add(ACTION_CURRENT_BOOK);
//        actions.add(ACTION_READER_HOME);
//        lvRect margins;
//        //margins.right = MIN_ITEM_PX * 120 / 100;
//        showMenu(actions, ALIGN_TOP, margins, false);
//    } else
    if (widget->getId() == "MENU") {
        onAction(CMD_SETTINGS);
    }
    return true;
}

/// handle menu or other action
bool CRUIOpdsPropsWidget::onAction(const CRUIAction * action) {
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    case CMD_MENU:
    {
        CRUIActionList actions;
        actions.add(ACTION_BACK);
        if (_main->getSettings()->getBoolDef(PROP_NIGHT_MODE, false))
            actions.add(ACTION_DAY_MODE);
        else
            actions.add(ACTION_NIGHT_MODE);
        actions.add(ACTION_SETTINGS);
        actions.add(ACTION_READER_HOME);
        actions.add(ACTION_EXIT);
        lvRect margins;
        //margins.right = MIN_ITEM_PX * 120 / 100;
        showMenu(actions, ALIGN_TOP, margins, false);
        return true;
    }
    case CMD_SETTINGS:
        _main->showSettings(lString8("@settings/browser"));
        return true;
    }
    return false;
}

CRUIOpdsPropsWidget::~CRUIOpdsPropsWidget()
{
    if (_catalog)
        delete _catalog;
}

bool CRUIOpdsPropsWidget::onKeyEvent(const CRUIKeyEvent * event) {
    int key = event->key();
    if (event->getType() == KEY_ACTION_PRESS) {
        if (key == CR_KEY_ESC || key == CR_KEY_BACK || key == CR_KEY_MENU) {
            return true;
        }
    } else if (event->getType() == KEY_ACTION_RELEASE) {
        if (key == CR_KEY_ESC || key == CR_KEY_BACK) {
            _main->back();
            return true;
        } else if (key == CR_KEY_MENU) {
            return onAction(CRUIActionByCode(CMD_MENU));
            return true;
        }
    }
    return false;
}

void CRUIOpdsPropsWidget::beforeNavigationFrom() {
    // save if possible
    lString8 title = UnicodeToUtf8(_edTitle->getText());
    lString8 url = UnicodeToUtf8(_edUrl->getText());
    lString8 login = UnicodeToUtf8(_edLogin->getText());
    lString8 password = UnicodeToUtf8(_edPassword->getText());
    title.trim();
    url.trim();
    login.trim();
    password.trim();
    if (_catalog->name != title.c_str() || _catalog->url != url.c_str() || _catalog->login != login.c_str() || _catalog->password != password.c_str()) {
        if (!title.empty()) {
            if ((url.startsWith("http://") || url.startsWith("https://")) && url.length() >= 12) {
                _catalog->name = title.c_str();
                _catalog->url = url.c_str();
                _catalog->login = login.c_str();
                _catalog->password = password.c_str();
                bookDB->saveOpdsCatalog(_catalog);
            }
        }
    }
}

/// motion event handler, returns true if it handled event
bool CRUIOpdsPropsWidget::onTouchEvent(const CRUIMotionEvent * event) {
    int action = event->getAction();
    int delta = event->getX() - event->getStartX();
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
    switch (action) {
    case ACTION_DOWN:
        break;
    case ACTION_UP:
        break;
    case ACTION_FOCUS_IN:
        break;
    case ACTION_FOCUS_OUT:
        return false; // to continue tracking
        break;
    case ACTION_CANCEL:
        break;
    case ACTION_MOVE:
        if ((delta > DRAG_THRESHOLD_X) || (-delta > DRAG_THRESHOLD_X))
            getMain()->startDragging(event, false);
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
    }
    return true;
}

