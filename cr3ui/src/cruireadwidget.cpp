/*
 * cruireadwidget.cpp
 *
 *  Created on: Aug 21, 2013
 *      Author: vlopatin
 */

// uncomment to simulate slow render
//#define SLOW_RENDER_SIMULATION

#include "stringresource.h"
#include "cruireadwidget.h"
#include "crui.h"
#include "cruimain.h"
#include "gldrawbuf.h"
#include "fileinfo.h"
#include "cruiconfig.h"
#include "lvstsheet.h"
#include "hyphman.h"

using namespace CRUI;

lUInt32 applyAlpha(lUInt32 cl1, lUInt32 cl2, int alpha) {
	if (alpha <=0)
		return cl1;
	else if (alpha >= 255)
		return cl2;
    lUInt32 opaque = 256 - alpha;
    lUInt32 n1 = (((cl2 & 0xFF00FF) * alpha + (cl1 & 0xFF00FF) * opaque) >> 8) & 0xFF00FF;
    lUInt32 n2 = (((cl2 >> 8) & 0xFF00FF) * alpha + ((cl1 >> 8) & 0xFF00FF) * opaque) & 0xFF00FF00;
    return n1 | n2;
}

static bool isDocViewProp(const lString8 & key) {
    return key == PROP_FONT_FACE || key == PROP_FONT_COLOR || key == PROP_FONT_WEIGHT_EMBOLDEN
            || key == PROP_FONT_SIZE || key == PROP_FONT_FACE
            || key == PROP_BACKGROUND_COLOR
            || key == PROP_INTERLINE_SPACE
            || key == PROP_BACKGROUND_IMAGE
            || key == PROP_BACKGROUND_IMAGE_ENABLED
            || key == PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS
            || key == PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST
            || key == PROP_FONT_GAMMA_INDEX
            || key == PROP_FONT_ANTIALIASING
            || key == PROP_FONT_WEIGHT_EMBOLDEN
            || key == PROP_PAGE_MARGINS
            || key == PROP_FONT_HINTING;
}

void drawVGradient(LVDrawBuf * buf, lvRect & rc, lUInt32 colorTop, lUInt32 colorBottom) {
    buf->GradientRect(rc.left, rc.top, rc.right, rc.bottom, colorTop, colorTop, colorBottom, colorBottom);
//	lvRect rc2 = rc;
//	for (int y = 0; y < rc.height(); y++) {
//		int alpha = (y * 255) / rc.height();
//		rc2.top = rc.top + y;
//		rc2.bottom = rc.top + y + 1;
//		buf->FillRect(rc2, applyAlpha(colorTop, colorBottom, alpha));
//	}
}

static CRUIDocView * createDocView() {
	CRUIDocView * _docview = new CRUIDocView();
    _docview->setViewMode(DVM_SCROLL, 1);
    LVArray<int> sizes;
    for (int i = deviceInfo.shortSide / 40; i < deviceInfo.shortSide / 10 && i < 200; i++)
    	sizes.add(i);
	_docview->setFontSizes(sizes, false);
	_docview->setFontSize(deviceInfo.shortSide / 20);
	lvRect margins(deviceInfo.shortSide / 20, deviceInfo.shortSide / 20, deviceInfo.shortSide / 20, deviceInfo.shortSide / 20);
	_docview->setPageMargins(margins);
	return _docview;
}

CRUIReadWidget::CRUIReadWidget(CRUIMainWidget * main)
    : CRUIWindowWidget(main)
    , _pinchSettingPreview(NULL)
	, _isDragging(false)
	, _dragStartOffset(0)
	, _locked(false)
	, _fileItem(NULL)
	, _lastPosition(NULL)
	, _startPositionIsUpdated(false)
	, _pinchOp(PINCH_OP_NONE)
{
    setId("READ");
    _docview = createDocView();
}

CRUIReadWidget::~CRUIReadWidget() {
    if (_fileItem)
        delete _fileItem;
    if (_lastPosition)
        delete _lastPosition;
}

/// measure dimensions
void CRUIReadWidget::measure(int baseWidth, int baseHeight) {
    _measuredWidth = baseWidth;
    _measuredHeight = baseHeight;
}

/// updates widget position based on specified rectangle
void CRUIReadWidget::layout(int left, int top, int right, int bottom) {
    CRUIWindowWidget::layout(left, top, right, bottom);
    if (!_locked) {
        if (_docview->GetWidth() != right - left || _docview->GetHeight() != bottom - top) {
            _docview->Resize(right-left, bottom-top);
        }
    }
}

void CRUIReadWidget::prepareScroll(int direction) {
    if (renderIfNecessary()) {
    	CRLog::trace("CRUIReadWidget::prepareScroll(%d)", direction);
        _scrollCache.prepare(_docview, _docview->GetPos(), _measuredWidth, _measuredHeight, direction, true);
    }
}

/// draws widget with its children to specified surface
void CRUIReadWidget::draw(LVDrawBuf * buf) {
    _popupControl.updateLayout(_pos);
    if (_pinchOp && _pinchSettingPreview) {
    	_pinchSettingPreview->SetPos(0, false);
        buf->SetTextColor(_pinchSettingPreview->getTextColor());
        buf->SetBackgroundColor(_pinchSettingPreview->getBackgroundColor());
        _pinchSettingPreview->Draw(*buf, false);
    	_drawRequested = false;
    	return;
    }
    if (renderIfNecessary()) {
        //CRLog::trace("Document is ready, drawing");
        _scrollCache.prepare(_docview, _docview->GetPos(), _measuredWidth, _measuredHeight, 1, false);
        _scrollCache.draw(buf, _docview->GetPos(), _pos.left, _pos.top);
    } else {
        // document render in progress; draw just page background
        //CRLog::trace("Document is locked, just drawing background");
        _docview->drawPageBackground(*buf, 0, 0);
    }
    // scroll bottom and top gradients
    lvRect top = _pos;
    top.bottom = top.top + deviceInfo.shortSide / 60;
    lvRect top2 = _pos;
    top2.top = top.bottom;
    top2.bottom = top.top + deviceInfo.shortSide / 30;
    lvRect bottom = _pos;
    bottom.top = bottom.bottom - deviceInfo.shortSide / 60;
    lvRect bottom2 = _pos;
    bottom2.bottom = bottom.top;
    bottom2.top = bottom.bottom - deviceInfo.shortSide / 30;
    drawVGradient(buf, top, 0xA0000000, 0xE0000000);
    drawVGradient(buf, top2, 0xE0000000, 0xFF000000);
    drawVGradient(buf, bottom2, 0xFF000000, 0xE0000000);
    drawVGradient(buf, bottom, 0xE0000000, 0xA0000000);
    // popup support
    if (_popupControl.popupBackground)
        _popupControl.popupBackground->draw(buf);
    if (_popupControl.popup)
        _popupControl.popup->draw(buf);
}

class BookLoadedNotificationTask : public CRRunnable {
    lString8 pathname;
    CRDocumentLoadCallback * callback;
    CRDocumentLoadCallback * callback2;
    bool success;
public:
    BookLoadedNotificationTask(lString8 _pathname, bool _success, CRDocumentLoadCallback * _callback, CRDocumentLoadCallback * _callback2) {
        pathname = _pathname;
        pathname.modify();
        callback = _callback;
        callback2 = _callback2;
        success = _success;
    }
    virtual void run() {
        CRLog::trace("BookLoadedNotificationTask.run()");
        callback2->onDocumentLoadFinished(pathname, success);
        callback->onDocumentLoadFinished(pathname, success);
    }
};

class BookRenderedNotificationTask : public CRRunnable {
    lString8 pathname;
    CRDocumentRenderCallback * callback;
    CRDocumentRenderCallback * callback2;
public:
    BookRenderedNotificationTask(lString8 _pathname, CRDocumentRenderCallback * _callback, CRDocumentRenderCallback * _callback2) {
        pathname = _pathname;
        pathname.modify();
        callback = _callback;
        callback2 = _callback2;
    }
    virtual void run() {
        CRLog::trace("BookRenderedNotificationTask.run()");
        callback2->onDocumentRenderFinished(pathname);
        callback->onDocumentRenderFinished(pathname);
    }
};

class OpenBookTask : public CRRunnable {
    lString8 _pathname;
    CRUIMainWidget * _main;
    CRUIReadWidget * _read;
public:
    OpenBookTask(lString16 pathname, CRUIMainWidget * main, CRUIReadWidget * read) : _main(main), _read(read) {
        _pathname = UnicodeToUtf8(pathname);
    }
    virtual void run() {
        CRLog::info("Loading book in background thread");
        bool success = _read->getDocView()->LoadDocument(Utf8ToUnicode(_pathname).c_str()) != 0;
        CRLog::info("Loading is finished %s", success ? "successfully" : "with error");
#ifdef SLOW_RENDER_SIMULATION
        concurrencyProvider->sleepMs(3000);
#endif
        if (!success) {
            _read->getDocView()->createDefaultDocument(lString16("Cannot open document"), lString16("Error occured while trying to open document"));
        }
        concurrencyProvider->executeGui(new BookLoadedNotificationTask(_pathname, success, _main, _read));
        CRLog::info("Rendering book in background thread");
        _read->getDocView()->Render();
        _read->restorePosition();
        _read->getDocView()->updateCache();
#ifdef SLOW_RENDER_SIMULATION
        concurrencyProvider->sleepMs(3000);
#endif
        CRLog::info("Render is finished");
        concurrencyProvider->executeGui(new BookRenderedNotificationTask(_pathname, _main, _read));
    }
};

class RenderBookTask : public CRRunnable {
    lString8 _pathname;
    CRUIMainWidget * _main;
    CRUIReadWidget * _read;
public:
    RenderBookTask(lString16 pathname, CRUIMainWidget * main, CRUIReadWidget * read) : _main(main), _read(read) {
        _pathname = UnicodeToUtf8(pathname);
    }
    virtual void run() {
        CRLog::info("Rendering in background thread");
        _read->getDocView()->Render();
        _read->getDocView()->updateCache();
#ifdef SLOW_RENDER_SIMULATION
        concurrencyProvider->sleepMs(3000);
#endif
        CRLog::info("Render in background thread is finished");
        concurrencyProvider->executeGui(new BookRenderedNotificationTask(_pathname, _main, _read));
    }
};

void CRUIReadWidget::closeBook() {
    updatePosition();
    _scrollCache.clear();
    if (_fileItem)
        delete _fileItem;
    if (_lastPosition)
        delete _lastPosition;
    _fileItem = NULL;
    _lastPosition = NULL;
    _docview->close();
}

bool CRUIReadWidget::restorePosition() {
    if (!_fileItem || !_fileItem->getBook())
        return false;
    BookDBBookmark * bmk = dirCache->loadLastPosition(_fileItem->getBook());
    if (bmk) {
        // found position
        ldomXPointer bm = _docview->getDocument()->createXPointer(lString16(bmk->startPos.c_str()));
        _docview->goToBookmark(bm);
        if (!_lastPosition)
            _lastPosition = bmk;
        else
            delete bmk;
        return true;
    }
    return false;
}

void CRUIReadWidget::beforeNavigationFrom() {
    updatePosition();
}

void CRUIReadWidget::updatePosition() {
    CRLog::trace("CRUIReadWidget::updatePosition()");
    if (!_fileItem || !_fileItem->getBook())
        return;
    ldomXPointer ptr = _docview->getBookmark();
    if ( ptr.isNull() )
        return;
    CRBookmark bm(ptr);
    lString16 comment;
    lString16 titleText;
    lString16 posText;
    bm.setType( bmkt_lastpos );
    if ( _docview->getBookmarkPosText( ptr, titleText, posText ) ) {
         bm.setTitleText( titleText );
         bm.setPosText( posText );
    }
    bm.setStartPos( ptr.toString() );
    int pos = ptr.toPoint().y;
    int fh = _docview->getDocument()->getFullHeight();
    int percent = fh > 0 ? (int)(pos * (lInt64)10000 / fh) : 0;
    if ( percent<0 )
        percent = 0;
    if ( percent>10000 )
        percent = 10000;
    bm.setPercent( percent );
    bm.setCommentText( comment );
    if (!_lastPosition)
        _lastPosition = new BookDBBookmark();
    _lastPosition->bookId = _fileItem->getBook()->id;
    _lastPosition->type = bm.getType();
    _lastPosition->percent = bm.getPercent();
    _lastPosition->shortcut = bm.getShortcut();
    _lastPosition->timestamp = GetCurrentTimeMillis();
    _lastPosition->startPos = UnicodeToUtf8(bm.getStartPos()).c_str();
    _lastPosition->endPos = UnicodeToUtf8(bm.getEndPos()).c_str();
    _lastPosition->titleText = UnicodeToUtf8(bm.getTitleText()).c_str();
    _lastPosition->posText = UnicodeToUtf8(bm.getPosText()).c_str();
    _lastPosition->commentText = UnicodeToUtf8(bm.getCommentText()).c_str();
    _lastPosition->startPos = UnicodeToUtf8(bm.getStartPos()).c_str();
    dirCache->saveLastPosition(_fileItem->getBook(), _lastPosition);
}

lString8 lastBookLang;
lString8 lastSettingsLang;
bool setHyph(lString8 bookLang, lString8 settingsLang) {
    if (bookLang == lastBookLang && settingsLang == lastSettingsLang) // don't set duplicate
        return false;
    lastBookLang = bookLang;
    lastSettingsLang = settingsLang;
    return crconfig.setHyphenationDictionary(bookLang, settingsLang);
}

bool CRUIReadWidget::openBook(const CRFileItem * file) {
    if (_locked)
        return false;
    if (!file)
        return false;
    closeBook();
    _locked = true;
    _scrollCache.clear();
    _main->showSlowOperationPopup();
    _fileItem = static_cast<CRFileItem*>(file->clone());
    _lastPosition = bookDB->loadLastPosition(file->getBook());
    lString8 bookLang(_fileItem->getBook() ? _fileItem->getBook()->language.c_str() : "");
    lString8 systemLang = crconfig.systemLanguage;
    setHyph(bookLang, systemLang);
    _main->executeBackground(new OpenBookTask(Utf8ToUnicode(getPathName()), _main, this));
    return true;
}

void CRUIReadWidget::onDocumentLoadFinished(lString8 pathname, bool success) {
    CR_UNUSED(pathname);
    if (!success) {
        if (_fileItem)
            delete _fileItem;
        if (_lastPosition)
            delete _lastPosition;
        _fileItem = NULL;
        _lastPosition = NULL;
    }
    // force update reading position - to refresh timestamp
    _startPositionIsUpdated = false;
}

void CRUIReadWidget::onDocumentRenderFinished(lString8 pathname) {
    CR_UNUSED(pathname);
    CRLog::trace("Render is finished - unlocking document");
    _locked = false;
    invalidate();
    _scrollCache.clear();
    if (!_startPositionIsUpdated) {
        // call update position to refresh last access timestamp
        _startPositionIsUpdated = true;
        updatePosition();
    }
    //_main->update();
}

/// returns true if document is ready, false if background rendering is in progress
bool CRUIReadWidget::renderIfNecessary() {
    if (_locked) {
        CRLog::trace("Document is locked");
        return false;
    }
    if (_docview->GetWidth() != _pos.width() || _docview->GetHeight() != _pos.height()) {
        CRLog::trace("Changing docview size to %dx%d", _pos.width(), _pos.height());
        _docview->Resize(_pos.width(), _pos.height());
    }
    if (_docview->IsRendered())
        return true;
    CRLog::info("Render is required! Starting render task");
    _locked = true;
    _scrollCache.clear();
    _main->showSlowOperationPopup();
    _main->executeBackground(new RenderBookTask(Utf8ToUnicode(getPathName()), _main, this));
    return false;
}

#define SCROLL_SPEED_CALC_INTERVAL 2000
#define SCROLL_MIN_SPEED 3
#define SCROLL_FRICTION 13

/// overriden to treat popup as first child
int CRUIReadWidget::getChildCount() {
    int cnt = 0;
    if (_popupControl.popup)
        cnt++;
    if (_popupControl.popupBackground)
        cnt++;
    return cnt;
}

/// overriden to treat popup as first child
CRUIWidget * CRUIReadWidget::getChild(int index) {
    CR_UNUSED(index);
    if (index == 0) {
        if (_popupControl.popupBackground)
            return _popupControl.popupBackground;
        return _popupControl.popup;
    }
    return _popupControl.popup;
}

void CRUIReadWidget::animate(lUInt64 millisPassed) {
    if (_locked) {
        if (_scroll.isActive())
            _scroll.stop();
        return;
    }
    bool scrollWasActive = _scroll.isActive();
    CRUIWidget::animate(millisPassed);
    bool changed = _scroll.animate(millisPassed);
    if (changed) {
        int oldpos = _docview->GetPos();
        //CRLog::trace("scroll animation: new position %d", _scroll.pos());
        _docview->SetPos(_scroll.pos(), false);
        if (oldpos == _docview->GetPos()) {
            //CRLog::trace("scroll animation - stopping at %d since set position not changed position", _scroll.pos());
            // stopping: bounds
            _scroll.stop();
        }
    }
    if (scrollWasActive && !_scroll.isActive())
        updatePosition();
}

bool CRUIReadWidget::isAnimating() {
    return _scroll.isActive();
}

void CRUIReadWidget::animateScrollTo(int newpos, int speed) {
    if (_locked)
        return;
    CRLog::trace("animateScrollTo( %d -> %d )", _docview->GetPos(), newpos);
    _scroll.start(_docview->GetPos(), newpos, speed, SCROLL_FRICTION);
    invalidate();
    _main->update(true);
}

bool CRUIReadWidget::doCommand(int cmd, int param) {
    if (_locked)
        return false;
    int pos = _docview->GetPos();
    int newpos = pos;
    int speed = 0;
    switch (cmd) {
    case DCMD_PAGEUP:
        newpos = pos - _pos.height() * 9 / 10;
        speed = _pos.height() * 2;
        break;
    case DCMD_PAGEDOWN:
        newpos = pos + _pos.height() * 9 / 10;
        speed = _pos.height() * 2;
        break;
    case DCMD_LINEUP:
        newpos = pos - _docview->getFontSize();
        speed = _pos.height() / 2;
        break;
    case DCMD_LINEDOWN:
        newpos = pos + _docview->getFontSize();
        speed = _pos.height() / 2;
        break;
    default:
        return _docview->doCommand((LVDocCmd)cmd, param);
    }
    if (pos != newpos) {
        animateScrollTo(newpos, speed);
    }
    return true;
}

void CRUIReadWidget::clearImageCaches() {
	_scrollCache.clear();
}

bool CRUIReadWidget::onKeyEvent(const CRUIKeyEvent * event) {
    if (_locked)
        return false;
    if (event->getType() == KEY_ACTION_PRESS) {
        if (_scroll.isActive())
            _scroll.stop();
        int key = event->key();
        //CRLog::trace("keyDown(0x%04x) oldpos=%d", key,  _docview->GetPos());
        switch(key) {
        case CR_KEY_PGDOWN:
        case CR_KEY_SPACE:
            doCommand(DCMD_PAGEDOWN);
            break;
        case CR_KEY_PGUP:
            doCommand(DCMD_PAGEUP);
            break;
        case CR_KEY_HOME:
            _docview->doCommand(DCMD_BEGIN);
            break;
        case CR_KEY_END:
            _docview->doCommand(DCMD_END);
            break;
        case CR_KEY_UP:
            doCommand(DCMD_LINEUP, 1);
            break;
        case CR_KEY_DOWN:
            doCommand(DCMD_LINEDOWN, 1);
            break;
        case CR_KEY_ESC:
        case CR_KEY_BACK:
            _main->back();
            return true;
        default:
            break;
        }
    }
    //CRLog::trace("new pos=%d", _docview->GetPos());
    invalidate();
    return true;
}

int CRUIReadWidget::pointToTapZone(int x, int y) {
    int x0 = x / ((_pos.width() + 2) / 3);
    int y0 = y / ((_pos.height() + 2) / 3);
    if (x0 > 2) x0 = 2;
    if (x0 < 0) x0 = 0;
    if (y0 > 2) y0 = 2;
    if (y0 < 0) y0 = 0;
    return y0 * 3 + x0 + 1;
}

bool CRUIReadWidget::onTapZone(int zone, bool additionalAction) {
    lString8 settingName;
    if (additionalAction)
        settingName = PROP_APP_TAP_ZONE_ACTION_DOUBLE;
    else
        settingName = PROP_APP_TAP_ZONE_ACTION_NORMAL;
    settingName += lString8::itoa(zone);
    lString8 action = UnicodeToUtf8(_main->getSettings()->getStringDef(settingName.c_str()));
    if (!action.empty()) {
        const CRUIAction * a = CRUIActionByName(action.c_str());
        if (a != NULL) {
            return onAction(a);
        }
    }
    return false;
}

void CRUIReadWidget::startPinchOp(int op, int dx, int dy) {
	if (_pinchOp)
		return;
	_pinchOp = op;
	_pinchOpStartDx = dx;
	_pinchOpStartDy = dy;
    _pinchOpCurrentDx = dx;
    _pinchOpCurrentDy = dy;
    _pinchSettingPreview = createDocView();
    CRPropRef changed = _main->getSettings();
    CRPropRef docviewprops = LVCreatePropsContainer();
    //bool backgroundChanged = false;
    for (int i = 0; i < changed->getCount(); i++) {
        lString8 key(changed->getName(i));
        lString8 value(UnicodeToUtf8(changed->getValue(i)));
        if (isDocViewProp(key)) {
            docviewprops->setString(key.c_str(), value.c_str());
            if (key == PROP_FONT_COLOR) {
            	_pinchSettingPreview->setTextColor(changed->getColorDef(PROP_FONT_COLOR, 0));
            }
        }
    }
    _pinchSettingPreview->propsApply(docviewprops);
    lString16 title;
    switch(_pinchOp) {
    case PINCH_OP_HORIZONTAL:
    	title = _16(STR_PINCH_CHANGING_PAGE_MARGINS);
    	break;
    case PINCH_OP_VERTICAL:
    	title = _16(STR_PINCH_CHANGING_INTERLINE_SPACING);
    	break;
    case PINCH_OP_DIAGONAL:
    	title = _16(STR_PINCH_CHANGING_FONT_SIZE);
    	_pinchOpSettingValue = _docview->getFontSize();
    	break;
    }
    lString16 sampleText = _16(STR_SETTINGS_FONT_SAMPLE_TEXT);
    sampleText = sampleText + " " + sampleText;
    _pinchSettingPreview->createDefaultDocument(title, sampleText + "\n"
    		+ sampleText + "\n" + sampleText + "\n" + sampleText
    		+ "\n" + sampleText + "\n" + sampleText + "\n" + sampleText + "\n" + sampleText);
    _pinchSettingPreview->Resize(_pos.width(), _pos.height());
    CRLog::trace("startPinchOp %d   %d %d", _pinchOp, dx, dy);
    invalidate();
}

void CRUIReadWidget::updatePinchOp(int dx, int dy) {
	if (!_pinchOp)
		return;
    _pinchOpCurrentDx = dx;
    _pinchOpCurrentDy = dy;
    int delta = 0;
    int startSettingValue = 0;
    int newSettingValue = 0;
    switch(_pinchOp) {
    case PINCH_OP_HORIZONTAL:
		{
			startSettingValue = _main->getSettings()->getIntDef(PROP_PAGE_MARGINS, 100);
			delta = (dx) - (_pinchOpStartDx);
			int maxdiff = 2000 - 100;
			newSettingValue = startSettingValue + maxdiff * (-delta) * 120 / 100 / deviceInfo.shortSide;
			if (newSettingValue < 100)
				newSettingValue = 100;
			if (newSettingValue > 2000)
				newSettingValue = 2000;
			CRPropRef props = LVCreatePropsContainer();
			props->setInt(PROP_PAGE_MARGINS, newSettingValue);
			_pinchOpSettingValue = newSettingValue;
			_pinchSettingPreview->propsApply(props);
			invalidate();
		}
    	break;
    case PINCH_OP_VERTICAL:
		{
			startSettingValue = _main->getSettings()->getIntDef(PROP_INTERLINE_SPACE, 100);
			delta = (dy) - (_pinchOpStartDy);
			int maxdiff = 200 - 80;
			newSettingValue = startSettingValue + maxdiff * delta * 120 / 100 / deviceInfo.shortSide;
			if (newSettingValue < 80)
				newSettingValue = 80;
			if (newSettingValue > 200)
				newSettingValue = 200;
			CRPropRef props = LVCreatePropsContainer();
			props->setInt(PROP_INTERLINE_SPACE, newSettingValue);
			_pinchOpSettingValue = newSettingValue;
			_pinchSettingPreview->propsApply(props);
			invalidate();
		}
    	break;
    case PINCH_OP_DIAGONAL:
		{
			delta = (dx + dy) - (_pinchOpStartDx + _pinchOpStartDy);
			int maxdiff = crconfig.maxFontSize - crconfig.minFontSize;
			startSettingValue = _docview->getFontSize();

			if (delta > 0) {
				newSettingValue = startSettingValue + maxdiff * delta * 120 / 100 / deviceInfo.shortSide;
				CRLog::trace("Zoom in %d -> %d", startSettingValue, newSettingValue);
			} else {
				newSettingValue = startSettingValue + maxdiff * delta * 120 / 100  / deviceInfo.shortSide;
				//newSettingValue = startSettingValue - startSettingValue * ((-delta) * 2 / deviceInfo.shortSide);
				CRLog::trace("Zoom out %d -> %d", startSettingValue, newSettingValue);
			}
			if (newSettingValue < crconfig.minFontSize)
				newSettingValue = crconfig.minFontSize;
			if (newSettingValue > crconfig.maxFontSize)
				newSettingValue = crconfig.maxFontSize;
			if (_pinchSettingPreview->getFontSize() != newSettingValue) {
				_pinchOpSettingValue = newSettingValue;
				_pinchSettingPreview->setFontSize(newSettingValue);
				invalidate();
			}
			break;
		}
    }
    CRLog::trace("updatePinchOp %d   %d %d", _pinchOp, dx, dy);
}

void CRUIReadWidget::endPinchOp(int dx, int dy, bool cancel) {
	if (!_pinchOp)
		return;
    CRLog::trace("endPinchOp %d   %d %d", _pinchOp, dx, dy);
	if (_pinchSettingPreview) {
		delete _pinchSettingPreview;
		_pinchSettingPreview = NULL;
	}
	if (!cancel) {
		switch(_pinchOp) {
		case PINCH_OP_HORIZONTAL:
			_main->initNewSettings()->setInt(PROP_PAGE_MARGINS, _pinchOpSettingValue);
			_main->applySettings();
			break;
		case PINCH_OP_VERTICAL:
			_main->initNewSettings()->setInt(PROP_INTERLINE_SPACE, _pinchOpSettingValue);
			_main->applySettings();
			break;
		case PINCH_OP_DIAGONAL:
			_main->initNewSettings()->setInt(PROP_FONT_SIZE, _pinchOpSettingValue);
			_main->applySettings();
			break;
		}
	}
	_pinchOp = PINCH_OP_NONE;
    invalidate();
}

/// returns true to allow parent intercept this widget which is currently handled by this widget
bool CRUIReadWidget::allowInterceptTouchEvent(const CRUIMotionEvent * event) {
	if (_isDragging || _pinchOp)
		return false;
	return true;
}

/// motion event handler, returns true if it handled event
bool CRUIReadWidget::onTouchEvent(const CRUIMotionEvent * event) {
    if (_locked)
        return false;
    int action = event->getAction();
    if (action != ACTION_MOVE && event->count() > 1)
    	CRLog::trace("CRUIReadWidget::onTouchEvent multitouch %d pointers action = %d", event->count(), action);
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d)", action, event->getX(), event->getY());
    int dx = event->getX() - event->getStartX();
    int dy = event->getY() - event->getStartY();
    int pinchDx = event->getPinchDx();
    int pinchDy = event->getPinchDy();
    int delta = dy; //isVertical() ? dy : dx;
    int delta2 = dx; //isVertical() ? dy : dx;
    //CRLog::trace("CRUIListWidget::onTouchEvent %d (%d,%d) dx=%d, dy=%d, delta=%d, itemIndex=%d [%d -> %d]", action, event->getX(), event->getY(), dx, dy, delta, index, _dragStartOffset, _scrollOffset);
    switch (action) {
    case ACTION_DOWN:
        _isDragging = false;
        _dragStart.x = event->getX();
        _dragStart.y = event->getY();
        _dragStartOffset = _docview->GetPos();
        if (_scroll.isActive())
            _scroll.stop();
        invalidate();
        //CRLog::trace("list DOWN");
        break;
    case ACTION_UP:
        {
            invalidate();
            if (_pinchOp) {
            	endPinchOp(pinchDx, pinchDy, false);
            	event->cancelAllPointers();
            } else if (_isDragging) {
                lvPoint speed = event->getSpeed(SCROLL_SPEED_CALC_INTERVAL);
                if (speed.y < -SCROLL_MIN_SPEED || speed.y > SCROLL_MIN_SPEED) {
                    _scroll.start(_docview->GetPos(), -speed.y, SCROLL_FRICTION);
                    CRLog::trace("Starting scroll with speed %d", _scroll.speed());
                }
            	event->cancelAllPointers();
            } else {
            	int x = event->getX();
            	int y = event->getY();
                bool longTap = (event->getDownDuration() > 500);
                bool twoFinigersTap = false;
                if (event->count() == 2) {
                	int dx1 = myAbs(event->getX(0) - event->getStartX(0));
                	int dy1 = myAbs(event->getY(0) - event->getStartY(0));
                	int dx2 = myAbs(event->getX(1) - event->getStartX(1));
                	int dy2 = myAbs(event->getY(1) - event->getStartY(1));
                	if (dx1 < DRAG_THRESHOLD && dy1 < DRAG_THRESHOLD && dx2 < DRAG_THRESHOLD && dy2 < DRAG_THRESHOLD) {
                		twoFinigersTap = true;
                		x = (x + event->getX(1)) / 2;
                		y = (y + event->getY(1)) / 2;
                	}
                }
                int zone = pointToTapZone(event->getX(), event->getY());
                event->cancelAllPointers();
                //bool twoFingersTap = (event->count() == 2) && event->get
                onTapZone(zone, twoFinigersTap);
            }
            _dragStartOffset = 0; //NO_DRAG;
            _isDragging = false;
//            setScrollOffset(_scrollOffset);
//            if (itemIndex != -1) {
//                //CRLog::trace("UP ts=%lld downTs=%lld downDuration=%lld", event->getEventTimestamp(), event->getDownEventTimestamp(), event->getDownDuration());
//                bool isLong = event->getDownDuration() > LONG_TOUCH_THRESHOLD; // 0.5 seconds threshold
//                if (isLong && onItemLongClickEvent(itemIndex))
//                    return true;
//                onItemClickEvent(itemIndex);
//            }
        }
        // fire onclick
        //CRLog::trace("list UP");
        break;
    case ACTION_FOCUS_IN:
//        if (isDragging)
//            setScrollOffset(_dragStartOffset - delta);
//        else
//            _selectedItem = index;
        //invalidate();
        //CRLog::trace("list FOCUS IN");
        break;
    case ACTION_FOCUS_OUT:
//        if (isDragging)
//            setScrollOffset(_dragStartOffset - delta);
//        else
//            _selectedItem = -1;
        //invalidate();
        return false; // to continue tracking
        //CRLog::trace("list FOCUS OUT");
        break;
    case ACTION_CANCEL:
        if (_pinchOp) {
        	endPinchOp(pinchDx, pinchDy, true);
        }
        _isDragging = false;
        //setScrollOffset(_scrollOffset);
        //CRLog::trace("list CANCEL");
        break;
    case ACTION_MOVE:
    	if (_pinchOp) {
    		updatePinchOp(pinchDx, pinchDy);
    	} else if (!_isDragging && event->count() == 2) {
			int ddx0 = myAbs(event->getStartX(0) - event->getStartX(1));
			int ddy0 = myAbs(event->getStartY(0) - event->getStartY(1));
			int ddx1 = myAbs(event->getX(0) - event->getX(1));
			int ddy1 = myAbs(event->getY(0) - event->getY(1));
			int op0, op1;
			if (ddx0 > ddy0 * 3)
				op0 = PINCH_OP_HORIZONTAL;
			else if (ddy0 > ddx0 * 3)
				op0 = PINCH_OP_VERTICAL;
			else
				op0 = PINCH_OP_DIAGONAL;
			if (ddx1 > ddy1 * 3)
				op1 = PINCH_OP_HORIZONTAL;
			else if (ddy1 > ddx1 * 3)
				op1 = PINCH_OP_VERTICAL;
			else
				op1 = PINCH_OP_DIAGONAL;
			int ddd = myAbs(pinchDx) + myAbs(pinchDy);
			if (op0 == op1 && ddd > DRAG_THRESHOLD_X * 2 / 3) {
				startPinchOp(op0, pinchDx, pinchDy);
			}
    	} else if (!_isDragging && ((delta > DRAG_THRESHOLD) || (-delta > DRAG_THRESHOLD))) {
            _isDragging = true;
            _docview->SetPos(_dragStartOffset - delta, false);
            prepareScroll(-delta);
            invalidate();
            _main->update(true);
        } else if (_isDragging) {
            _docview->SetPos(_dragStartOffset - delta, false);
            invalidate();
            _main->update(true);
        } else if (!_isDragging) {
        	if (event->count() == 2) {
        		int ddx0 = myAbs(event->getStartX(0) - event->getStartX(1));
        		int ddy0 = myAbs(event->getStartY(0) - event->getStartY(1));
        		int ddx1 = myAbs(event->getX(0) - event->getX(1));
        		int ddy1 = myAbs(event->getY(0) - event->getY(1));
        		int op0, op1;
        		if (ddx0 > ddy0 / 2)
        			op0 = PINCH_OP_HORIZONTAL;
        		else if (ddy0 > ddx0 / 2)
        			op0 = PINCH_OP_VERTICAL;
        		else
        			op0 = PINCH_OP_DIAGONAL;
        		if (ddx1 > ddy1 / 2)
        			op1 = PINCH_OP_HORIZONTAL;
        		else if (ddy1 > ddx1 / 2)
        			op1 = PINCH_OP_VERTICAL;
        		else
        			op0 = PINCH_OP_DIAGONAL;
        		int ddd = myAbs(pinchDx) + myAbs(pinchDy);
        		if (op0 == op1 && ddd > DRAG_THRESHOLD_X * 2 / 3) {
        			startPinchOp(op0, pinchDx, pinchDy);
        		}
        	} else {
        		if ((delta2 > DRAG_THRESHOLD_X) || (-delta2 > DRAG_THRESHOLD_X)) {
        			_main->startDragging(event, false);
        		}
            }
        }
        // ignore
        //CRLog::trace("list MOVE");
        break;
    default:
        return CRUIWidget::onTouchEvent(event);
    }
    return true;
}

void CRUIReadWidget::goToPosition(lString16 path) {
    ldomXPointer pt = _docview->getDocument()->createXPointer(path);
    _docview->goToBookmark(pt);
    _scrollCache.clear();
}

// formats percent value 0..10000  as  XXX.XX%
static lString16 formatPercent(int percent) {
    char s[100];
    sprintf(s, "%d.%02d%%", percent / 100, percent % 100);
    return Utf8ToUnicode(s);
}

static void updateScrollPosMessage(CRUIWidget * title, int pos) {
    if (title) {
        lString16 str = _16(STR_ACTION_GOTO_PERCENT);
        str += ": ";
        str += formatPercent(pos);
        title->setText(str);
    }
}

bool CRUIReadWidget::onScrollPosChange(CRUISliderWidget * widget, int pos, bool manual) {
    if (!manual)
        return false;
    CRUIWidget * title = widget->getParent()->childById("POPUP_TITLE");
    updateScrollPosMessage(title, pos);
    int maxpos = _docview->GetFullHeight() - _docview->GetHeight();
    if (maxpos < 0)
        maxpos = 0;
    int p = (int)(pos * (lInt64)maxpos / 10000);
    _docview->SetPos(p, false);
    _scrollCache.prepare(_docview, p, _pos.width(), _pos.height(), 1, false);
    invalidate();
    return true;
}

bool CRUIReadWidget::hasTOC() {
    LVTocItem * toc = _docview->getToc();
    return toc && toc->getChildCount();
}

void CRUIReadWidget::showTOC() {
    if (!hasTOC())
        return;
    CRUITOCWidget * widget = new CRUITOCWidget(_main, this);
    _main->showTOC(widget);
}

void CRUIReadWidget::showGoToPercentPopup() {
    CRUIVerticalLayout * popup = new CRUIVerticalLayout();
    popup->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    popup->setPadding(MIN_ITEM_PX / 3);

    int percent = _docview->getPosPercent();

    CRUITextWidget * title = new CRUITextWidget(_16(STR_ACTION_GOTO_PERCENT));
    title->setId("POPUP_TITLE");
    title->setFontSize(FONT_SIZE_LARGE);
    updateScrollPosMessage(title, percent);
    popup->addChild(title);

    CRUISliderWidget * slider = new CRUISliderWidget(0, 10000, percent);
    slider->setId("POSITION_PERCENT");
    slider->setScrollPosCallback(this);
    popup->addChild(slider);

    lvRect margins;
    margins.left = 0;
    margins.right = 0;
    margins.bottom = MIN_ITEM_PX / 3;
    preparePopup(popup, ALIGN_BOTTOM, margins);
}

/// override to handle menu or other action
bool CRUIReadWidget::onAction(const CRUIAction * action) {
    if (!action)
        return false;
    if (action->cmd) {
        doCommand(action->cmd, action->param);
        return true;
    }
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    case CMD_GOTO_PERCENT:
        showGoToPercentPopup();
        return true;
    case CMD_TOC:
        showTOC();
        return true;
    case CMD_MENU:
    {
        CRUIActionList actions;
        actions.add(ACTION_EXIT);
        actions.add(ACTION_SETTINGS);
        actions.add(ACTION_GOTO_PERCENT);
        if (hasTOC())
            actions.add(ACTION_TOC);
        actions.add(ACTION_BACK);
        lvRect margins;
        showMenu(actions, ALIGN_TOP, margins, false);
        return true;
    }
    case CMD_SETTINGS:
        _main->showSettings(lString8("@settings/reader"));
        return true;
    }
    return false;
}

/// applies properties, returns list of not recognized properties
CRPropRef CRUIDocView::propsApply(CRPropRef props) {
    //CRPropRef oldSettings = propsGetCurrent();
    CRPropRef newSettings = propsGetCurrent() | props;
    CRPropRef forDocview = LVCreatePropsContainer();
    bool backgroundChanged = false;
    bool needClearCache = false;
    for (int i = 0; i < props->getCount(); i++) {
        lString8 key(props->getName(i));
        //lString8 value(UnicodeToUtf8(props->getValue(i)));
        if (key == PROP_PAGE_MARGINS) {
        	int marginPercent = props->getIntDef(key.c_str(), 5000);
        	int hmargin = deviceInfo.shortSide * marginPercent / 10000;
        	lvRect margins(hmargin, hmargin / 2, hmargin, hmargin / 2);
        	setPageMargins(margins);
        	requestRender();
        } else if (key == PROP_FONT_ANTIALIASING) {
            int antialiasingMode = props->getIntDef(PROP_FONT_ANTIALIASING, 2);
            if (antialiasingMode == 1) {
                antialiasingMode = 2;
            }
            if (fontMan->GetAntialiasMode() != antialiasingMode) {
                fontMan->SetAntialiasMode(antialiasingMode);
            }
            requestRender();
        } else if (key == PROP_FONT_HINTING) {
            bool bytecode = props->getBoolDef(PROP_FONT_HINTING, 1);
            int hintingMode = bytecode ? HINTING_MODE_BYTECODE_INTERPRETOR : HINTING_MODE_AUTOHINT;
            if ((int)fontMan->GetHintingMode() != hintingMode && hintingMode >= 0 && hintingMode <= 2) {
                //CRLog::debug("Setting hinting mode to %d", mode);
                fontMan->SetHintingMode((hinting_mode_t)hintingMode);
            }
            requestRender();
        } else if (key == PROP_FONT_GAMMA_INDEX) {
            int gammaIndex = props->getIntDef(PROP_FONT_GAMMA_INDEX, 15);
            int oldGammaIndex = fontMan->GetGammaIndex();
            if (oldGammaIndex != gammaIndex) {
                fontMan->SetGammaIndex(gammaIndex);
            }
        } else {
            forDocview->setString(key.c_str(), props->getValue(i));
        }
        if (key == PROP_BACKGROUND_COLOR
                || key == PROP_BACKGROUND_IMAGE
                || key == PROP_BACKGROUND_IMAGE_ENABLED
                || key == PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS
                || key == PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST) {
            backgroundChanged = true;
            needClearCache = true;
        }
    }
    if (backgroundChanged) {
        setBackground(resourceResolver->getBackgroundImage(newSettings));
    }
    return LVDocView::propsApply(forDocview);
}

// apply changed settings
void CRUIReadWidget::applySettings(CRPropRef changed, CRPropRef oldSettings, CRPropRef newSettings) {
    CR_UNUSED(oldSettings);
    CRPropRef docviewprops = LVCreatePropsContainer();
    //bool backgroundChanged = false;
    bool needClearCache = false;
    for (int i = 0; i < changed->getCount(); i++) {
        lString8 key(changed->getName(i));
        lString8 value(UnicodeToUtf8(changed->getValue(i)));
        if (isDocViewProp(key)) {
            docviewprops->setString(key.c_str(), value.c_str());
            if (key == PROP_FONT_COLOR) {
                _docview->setTextColor(changed->getColorDef(PROP_FONT_COLOR, 0));
                needClearCache = true;
            }
        }
        if (key == PROP_BACKGROUND_COLOR
                || key == PROP_BACKGROUND_IMAGE
                || key == PROP_BACKGROUND_IMAGE_ENABLED
                || key == PROP_BACKGROUND_IMAGE_CORRECTION_BRIGHTNESS
                || key == PROP_BACKGROUND_IMAGE_CORRECTION_CONTRAST) {
            //backgroundChanged = true;
            needClearCache = true;
        }
        if (key == PROP_HYPHENATION_DICT) {
            setHyph(lastBookLang, value);
            _docview->requestRender();
            needClearCache = true;
            invalidate();
        }
    }
//    if (backgroundChanged) {
//        _docview->setBackground(resourceResolver->getBackgroundImage(newSettings));
//    }
    if (needClearCache) {
        _scrollCache.clear();
    }
    if (docviewprops->getCount())
        _docview->propsApply(docviewprops);
}

/// on starting file loading
void CRUIReadWidget::OnLoadFileStart( lString16 filename ) {
    CR_UNUSED(filename);
}

/// format detection finished
void CRUIReadWidget::OnLoadFileFormatDetected(doc_format_t fileFormat) {
    lString8 cssFile = crconfig.cssDir + LVDocFormatCssFileName(fileFormat);

    lString8 css;
    if (!LVLoadStylesheetFile(Utf8ToUnicode(cssFile), css)) {
        // todo: fallback
    }
    _docview->setStyleSheet(css);
}

/// file loading is finished successfully - drawCoveTo() may be called there
void CRUIReadWidget::OnLoadFileEnd() {

}

/// first page is loaded from file an can be formatted for preview
void CRUIReadWidget::OnLoadFileFirstPagesReady() {

}

/// file progress indicator, called with values 0..100
void CRUIReadWidget::OnLoadFileProgress( int percent) {
    CR_UNUSED(percent);
}

/// document formatting started
void CRUIReadWidget::OnFormatStart() {

}

/// document formatting finished
void CRUIReadWidget::OnFormatEnd() {
	invalidate();
}

/// format progress, called with values 0..100
void CRUIReadWidget::OnFormatProgress(int percent) {
    CR_UNUSED(percent);
}

/// format progress, called with values 0..100
void CRUIReadWidget::OnExportProgress(int percent) {
    CR_UNUSED(percent);
}

/// file load finiished with error
void CRUIReadWidget::OnLoadFileError(lString16 message) {
    CR_UNUSED(message);
}

/// Override to handle external links
void CRUIReadWidget::OnExternalLink(lString16 url, ldomNode * node) {
    CR_UNUSED2(url, node);
}

/// Called when page images should be invalidated (clearImageCache() called in LVDocView)
void CRUIReadWidget::OnImageCacheClear() {
    class ClearCache : public CRRunnable {
        CRUIReadWidget * _widget;
    public:
        ClearCache(CRUIReadWidget * widget) : _widget(widget) {}
        virtual void run() {
            _widget->_scrollCache.clear();
        }
    };
    concurrencyProvider->executeGui(new ClearCache(this));
}

/// return true if reload will be processed by external code, false to let internal code process it
bool CRUIReadWidget::OnRequestReload() {
    return false;
}




//================================================================
// Scroll Mode page image cache

CRUIReadWidget::ScrollModePageCache::ScrollModePageCache() : minpos(0), maxpos(0), dx(0), dy(0), tdx(0), tdy(0) {

}

#define MIN_TEX_SIZE 64
#define MAX_TEX_SIZE 4096
static int nearestPOT(int n) {
	for (int i = MIN_TEX_SIZE; i <= MAX_TEX_SIZE; i++) {
		if (n <= i)
			return i;
	}
	return MIN_TEX_SIZE;
}

LVDrawBuf * CRUIReadWidget::ScrollModePageCache::createBuf() {
    return new GLDrawBuf(dx, tdy, 32, true);
}

void CRUIReadWidget::ScrollModePageCache::setSize(int _dx, int _dy) {
    if (dx != _dx || dy != _dy) {
        clear();
        dx = _dx;
        dy = _dy;
        tdx = nearestPOT(dx);
        tdy = nearestPOT(dy);
    }
}

/// ensure images are prepared
void CRUIReadWidget::ScrollModePageCache::prepare(LVDocView * _docview, int _pos, int _dx, int _dy, int direction, bool force) {
    setSize(_dx, _dy);
    if (_pos >= minpos && _pos + dy <= maxpos && !force)
        return; // already prepared
    int y0 = direction > 0 ? (_pos - dy / 4) : (_pos - dy * 5 / 4);
    int y1 = direction > 0 ? (_pos + dy + dy * 5 / 4) : (_pos + dy + dy / 4);
    if (y0 < 0)
    	y0 = 0;
    int pos0 = y0 / tdy * tdy;
    int pos1 = (y1 + tdy - 1) / tdy * tdy;
    int pageCount = (pos1 - pos0) / tdy + 1;
    for (int i = pages.length() - 1; i >= 0; i--) {
        ScrollModePage * p = pages[i];
        if (!p->intersects(y0, y1)) {
            pages.remove(i);
            delete p;
        }
    }
    for (int i = 0; i < pageCount; i++) {
        int pos = pos0 + i * tdy;
        bool found = false;
        for (int k = pages.length() - 1; k >= 0; k--) {
            if (pages[k]->pos == pos) {
                found = true;
                break;
            }
        }
        if (!found) {
            ScrollModePage * page = new ScrollModePage();
            page->dx = dx;
            page->dy = tdy;
            page->pos = pos;
            page->drawbuf = createBuf();
            LVDrawBuf * buf = page->drawbuf; //dynamic_cast<GLDrawBuf*>(page->drawbuf);
            buf->beforeDrawing();
            int oldpos = _docview->GetPos();
            _docview->SetPos(pos, false);
            buf->SetTextColor(_docview->getTextColor());
            buf->SetBackgroundColor(_docview->getBackgroundColor());
            _docview->Draw(*buf, false);
            _docview->SetPos(oldpos, false);
            buf->afterDrawing();
            pages.add(page);
            CRLog::trace("new page cache item %d..%d", page->pos, page->pos + page->dy);
        }
    }
    minpos = maxpos = -1;
    for (int k = 0; k < pages.length(); k++) {
        //CRLog::trace("page cache item [%d] %d..%d", k, pages[k]->pos, pages[k]->pos + pages[k]->dy);
        if (minpos == -1 || minpos > pages[k]->pos) {
            minpos = pages[k]->pos;
        }
        if (maxpos == -1 || maxpos < pages[k]->pos + pages[k]->dy) {
            maxpos = pages[k]->pos + pages[k]->dy;
        }
    }
}

void CRUIReadWidget::ScrollModePageCache::draw(LVDrawBuf * dst, int pos, int x, int y) {
    // workaround for no-rtti builds
	GLDrawBuf * glbuf = dst->asGLDrawBuf(); //dynamic_cast<GLDrawBuf*>(buf);
    if (glbuf) {
        //glbuf->beforeDrawing();
        for (int k = pages.length() - 1; k >= 0; k--) {
            if (pages[k]->intersects(pos, pos + dy)) {
                // draw fragment
                int y0 = pages[k]->pos - pos;
                pages[k]->drawbuf->DrawTo(glbuf, x, y + y0, 0, NULL);
            }
        }
        //glbuf->afterDrawing();
    }
}

void CRUIReadWidget::ScrollModePageCache::clear() {
    pages.clear();
    minpos = 0;
    maxpos = 0;
}


static void addTocItems(LVPtrVector<LVTocItem, false> & toc, LVTocItem * item) {
    if (item->getParent())
        toc.add(item);
    for (int i = 0; i < item->getChildCount(); i++)
        addTocItems(toc, item->getChild(i));
}

CRUITOCWidget::CRUITOCWidget(CRUIMainWidget * main, CRUIReadWidget * read) : CRUIWindowWidget(main), _readWidget(read) {
    _title = new CRUITitleBarWidget(lString16(), this, this, false);
    _title->setTitle(STR_READER_TOC);
    _body->addChild(_title);
    _list = new CRUIListWidget(true, this);
    _list->setOnItemClickListener(this);
    _list->setStyle("SETTINGS_ITEM_LIST");
    _body->addChild(_list);
    addTocItems(_toc, read->getDocView()->getToc());
    _itemWidget = new CRUIHorizontalLayout();
    _itemWidget->setMinHeight(MIN_ITEM_PX * 2 / 3);
    _itemWidget->setPadding(PT_TO_PX(2));
    _chapter = new CRUITextWidget();
    _page = new CRUITextWidget();
    _itemWidget->addChild(_chapter);
    _itemWidget->addChild(_page);
    _page->setAlign(ALIGN_RIGHT|ALIGN_VCENTER);
    _chapter->setAlign(ALIGN_LEFT|ALIGN_VCENTER);
    _chapter->setLayoutParams(FILL_PARENT, WRAP_CONTENT);
    _itemWidget->setStyle("LIST_ITEM");
}

int CRUITOCWidget::getItemCount(CRUIListWidget * list) {
    CR_UNUSED(list);
    return _toc.length();
}

CRUIWidget * CRUITOCWidget::getItemWidget(CRUIListWidget * list, int index) {
    CR_UNUSED(list);
    LVTocItem * item = _toc[index];
    _chapter->setText(item->getName());
    _page->setText(formatPercent(item->getPercent()));
    lvRect padding;
    padding.left = (item->getLevel() - 1) * MIN_ITEM_PX / 3;
    _chapter->setPadding(padding);
    return _itemWidget;
}

// list item click
bool CRUITOCWidget::onListItemClick(CRUIListWidget * widget, int itemIndex) {
    CR_UNUSED(widget);
    LVTocItem * item = _toc[itemIndex];
    _readWidget->goToPosition(item->getPath());
    onAction(CMD_BACK);
    return true;
}

bool CRUITOCWidget::onClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    return true;
}

bool CRUITOCWidget::onLongClick(CRUIWidget * widget) {
    if (widget->getId() == "BACK")
        onAction(CMD_BACK);
    return true;
}

/// handle menu or other action
bool CRUITOCWidget::onAction(const CRUIAction * action) {
    switch (action->id) {
    case CMD_BACK:
        _main->back();
        return true;
    default:
        break;
    }
    return false;
}

