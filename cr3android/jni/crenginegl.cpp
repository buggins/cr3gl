#include <jni.h>
#include "cr3java.h"
#include "org_coolreader_newui_CRView.h"
#include "cruimain.h"
#include "crconcurrent.h"
#include "cruiconfig.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "crcoverpages.h"
#include "gldrawbuf.h"
#include "glfont.h"
#if defined(__arm__)
#include "coffeecatch/coffeecatch.h"
#include "coffeecatch/coffeejni.h"
#else
#define COFFEE_TRY_JNI(ENV, CODE) CODE;
#endif

static jfieldID gNativeObjectID = 0;

class CRUIEventAdapter {
    CRUIEventManager * _eventManager;
	LVPtrVector<CRUIMotionEventItem> _activePointers;
	int findPointer(lUInt64 id) {
		for (int i=0; i<_activePointers.length(); i++)
			if (_activePointers[i]->getPointerId() == id)
				return i;
		return -1;
	}
public:
    CRUIEventAdapter(CRUIEventManager * eventManager) : _eventManager(eventManager) {

    }

    #define MAX_DOWN_DURATION 60*1000

    // android action codes
    enum {
    	AACTION_DOWN = 0,
    	AACTION_UP = 1,
    	AACTION_MOVE = 2,
    	AACTION_CANCEL = 3,
    	AACTION_OUTSIDE = 4,
    	AACTION_POINTER_DOWN = 5,
    	AACTION_POINTER_UP = 6,
    	AACTION_SCROLL = 8,

    };
    static int translateTouchAction(int action) {
    	switch(action) {
    	case AACTION_POINTER_DOWN:
    	case AACTION_DOWN:
    		return CRUI::ACTION_DOWN;
    	case AACTION_POINTER_UP:
    	case AACTION_UP:
    		return CRUI::ACTION_UP;
    	case AACTION_MOVE:
    		return CRUI::ACTION_MOVE;
    	case AACTION_CANCEL:
    		return CRUI::ACTION_CANCEL;
    	case AACTION_OUTSIDE:
    		return CRUI::ACTION_FOCUS_OUT;
    	case AACTION_SCROLL:
    	default:
    		return -1;
    	}
    }

    // touch event listener
    bool dispatchTouchEvent(CRTouchEventWrapper * event, int x0, int y0) {
    	x0 = y0 = 0; // coords already relative to the view
    	int pointerCount = event->getPointerCount();
    	int action = translateTouchAction(event->getActionMasked());
    	if (action < 0) {
    		CRLog::trace("ignoring unknown touch event %d", event->getAction());
    		return false; // ignore unknown actions
    	}
    	int actionIndex = event->getActionIndex();
    	int actionPointerId = event->getPointerId(actionIndex);
    	lUInt64 ts = event->getEventTime();
		CRUIMotionEventItem * actionItem = NULL;
//		if (crconfig.einkMode)
//			CRLog::trace("x0=%d y0=%d pointerCount = %d action = %d actionIndex = %d actionPointerId = %d x=%d y=%d", x0, y0, pointerCount, action, actionIndex, actionPointerId, event->getX(0) - x0, event->getY(0) - y0);
    	for (int i = 0; i < pointerCount; i++) {
    		int x = event->getX(i) - x0;
    		int y = event->getY(i) - y0;
    		int pointerId = event->getPointerId(i);
    		int pointerAction = CRUI::ACTION_MOVE;
    		if (actionIndex == i) {
    			actionPointerId = pointerId;
    			pointerAction = action;
    		}
    		int p = findPointer(pointerId);
    		CRUIMotionEventItem * item = NULL;
    		if (p >= 0) {
    			CRUIMotionEventItem * olditem = _activePointers[p];
    			//CRLog::trace("found old pointer %d,%d", olditem->getX(), olditem->getStartY());
    			item = new CRUIMotionEventItem(olditem, pointerId, pointerAction, x, y, ts);
    			//delete olditem;
    			_activePointers.set(p, item);
    		} else {
    			item = new CRUIMotionEventItem(NULL, pointerId, pointerAction, x, y, ts);
    			_activePointers.add(item);
    		}
    		if (actionIndex == i) {
    			//CRLog::trace("action item found with index = %d", i);
    			actionItem = item;
    		}
    	}
		CRUIMotionEvent * crevent = new CRUIMotionEvent();
		// add current action item
		if (actionItem)
			crevent->addEvent(actionItem);
		// add other items
		int activeActionItemIndex = -1;
		for (int i = _activePointers.length() - 1; i >= 0; i--) {
			CRUIMotionEventItem * item = _activePointers[i];
			if (item->getDownDuration() > MAX_DOWN_DURATION) {
				// remove obsolete item
				// TODO: send CANCEL events?
				delete _activePointers.remove(i);
				continue;
			}
			if (item != actionItem)
				crevent->addEvent(item);
			else
				activeActionItemIndex = i;
		}
		//CRLog::trace("Posting touch event ptr=%d action=%d (%d,%d)", (int)crevent->getPointerId(), crevent->getAction(), crevent->getX(), crevent->getY());
		bool res = _eventManager->dispatchTouchEvent(crevent);
		delete crevent;
		if (activeActionItemIndex >= 0) {
			for (int i = _activePointers.length() - 1; i >= 0; i--) {
				if (_activePointers[i]->getAction() == CRUI::ACTION_UP || _activePointers[i]->getAction() == CRUI::ACTION_CANCEL || _activePointers[i]->isCancelled())
					delete _activePointers.remove(i);
			}
		}
//		if (activeActionItemIndex >= 0 && actionItem && (actionItem->getAction() == CRUI::ACTION_UP || actionItem->getAction() == CRUI::ACTION_CANCEL)) {
//			delete _activePointers.remove(activeActionItemIndex);
//		}
		return res;
    }

    enum {
    	AKEY_BACK = 4,
    	AKEY_MENU = 82,
    	AKEY_VOLUMEUP = 24,
    	AKEY_VOLUMEDOWN = 25,
    	AKEY_DEL = 67,
    	AKEY_DPAD_UP = 19,
    	AKEY_DPAD_DOWN = 20,
    	AKEY_DPAD_LEFT = 21,
    	AKEY_DPAD_RIGHT = 22,
    	AKEY_DPAD_CENTER = 23,
    	AKEY_ENTER = 66,
    	AKEY_ESCAPE = 111,
    	AKEY_0 = 7,
    	AKEY_1 = 8,
    	AKEY_2 = 9,
    	AKEY_3 = 10,
    	AKEY_4 = 11,
    	AKEY_5 = 12,
    	AKEY_6 = 13,
    	AKEY_7 = 14,
    	AKEY_8 = 15,
    	AKEY_9 = 16
    };
    static int translateKeyCode(int key) {
    	switch(key) {
    	case AKEY_BACK: return CR_KEY_BACK;
    	case AKEY_MENU: return CR_KEY_MENU;
    	case AKEY_DEL: return CR_KEY_BACKSPACE;
    	case AKEY_ENTER: return CR_KEY_RETURN;
    	case AKEY_DPAD_CENTER: return CR_KEY_RETURN;
    	case AKEY_DPAD_UP: return CR_KEY_UP;
    	case AKEY_DPAD_DOWN: return CR_KEY_DOWN;
    	case AKEY_DPAD_LEFT: return CR_KEY_LEFT;
    	case AKEY_DPAD_RIGHT: return CR_KEY_RIGHT;
    	case AKEY_VOLUMEUP: return CR_KEY_VOLUME_UP;
    	case AKEY_VOLUMEDOWN: return CR_KEY_VOLUME_DOWN;
    	case AKEY_ESCAPE: return CR_KEY_ESC;
    	case AKEY_0: return CR_KEY_0;
    	case AKEY_1: return CR_KEY_1;
    	case AKEY_2: return CR_KEY_2;
    	case AKEY_3: return CR_KEY_3;
    	case AKEY_4: return CR_KEY_4;
    	case AKEY_5: return CR_KEY_5;
    	case AKEY_6: return CR_KEY_6;
    	case AKEY_7: return CR_KEY_7;
    	case AKEY_8: return CR_KEY_8;
    	case AKEY_9: return CR_KEY_9;
    	default:
    		return -1;
    	}
    }

    enum {
    	AKEY_META_SHIFT_ON = 1,
    	AKEY_META_ALT_ON = 2,
    	AKEY_META_SYM_ON = 4,
    	AKEY_META_CTRL_ON = 0x00001000,
    };
    static int translateModifiers(int m) {
    	int res = 0;
    	if (m & AKEY_META_SHIFT_ON)
    		res |= CR_KEY_MODIFIER_SHIFT;
    	if (m & AKEY_META_CTRL_ON)
    		res |= CR_KEY_MODIFIER_CONTROL;
    	if (m & AKEY_META_ALT_ON)
    		res |= CR_KEY_MODIFIER_ALT;
    	return res;
    }

    enum {
    	AKEYACTION_DOWN = 0,
    	AKEYACTION_UP = 1,
    	AKEYACTION_MULTIPLE = 2,
    };
    static KEY_EVENT_TYPE translateKeyAction(int action) {
    	switch(action) {
    	case AKEYACTION_DOWN: return KEY_ACTION_PRESS;
    	case AKEYACTION_UP: return KEY_ACTION_RELEASE;
    	default:
    		return KEY_ACTION_UNKNOWN;
    	}
    }

    // key event listener
    bool dispatchKeyEvent(CRKeyEventWrapper * event) {
    	int naction = event->getAction();
    	KEY_EVENT_TYPE action = translateKeyAction(naction);
    	int key = translateKeyCode(event->getKeyCode());
    	int mod = translateModifiers(event->getModifiers());
    	lString16 text = event->getCharacters();
    	CRLog::trace("dispatchKeyEvent action=%d key=%d mod=%d text=%s", action, key, mod, LCSTR(text));
    	if (naction == AKEYACTION_MULTIPLE && !text.empty()) {
    		CRLog::trace("key action MULTIPLE: %s", LCSTR(text));
    		CRUIKeyEvent * crevent = new CRUIKeyEvent(KEY_ACTION_PRESS, 0, false, 0, mod);
    		crevent->setText(text);
    		bool res = _eventManager->dispatchKeyEvent(crevent);
    		crevent = new CRUIKeyEvent(KEY_ACTION_RELEASE, 0, false, 0, mod);
    		crevent->setText(text);
    		res = _eventManager->dispatchKeyEvent(crevent) || res;
    		return res;
    	}
    	if (key < 0 || action == KEY_ACTION_UNKNOWN)
    		return false;
		CRUIKeyEvent * crevent = new CRUIKeyEvent(action, key, false, 0, mod);
		crevent->setText(text);
		bool res = _eventManager->dispatchKeyEvent(crevent);
		//delete crevent;
		return res;
    }
};

class CRUITextToSpeechAdapter : public CRUITextToSpeech {
    CRJNIEnv _env;
    CRObjectAccessor _obj;
    // VoiceInfo[] getAvailableVoices();
    CRMethodAccessor _getAvailableVoicesMethod;
    // VoiceInfo getCurrentVoice();
    CRMethodAccessor _getCurrentVoiceMethod;
    // VoiceInfo getDefaultVoice();
    CRMethodAccessor _getDefaultVoiceMethod;
    // boolean setCurrentVoice(String id);
    CRMethodAccessor _setCurrentVoiceMethod;
    // boolean setRate(int rate);
    CRMethodAccessor _setRateMethod;
    // int getRate();
    CRMethodAccessor _getRateMethod;
    // boolean canChangeCurrentVoice();
    CRMethodAccessor _canChangeCurrentVoiceMethod;
    // boolean tell(String text);
    CRMethodAccessor _tellMethod;
    // boolean isSpeaking();
    CRMethodAccessor _isSpeakingMethod;
    // void stop();
    CRMethodAccessor _stopMethod;
    // boolean isInitialized();
    CRMethodAccessor _isInitializedMethod;
    // boolean isError();
    CRMethodAccessor _isErrorMethod;
    CRUITextToSpeechCallback * _callback;
    int _rate;
    LVPtrVector<CRUITextToSpeechVoice, false> _voices;
    bool _speaking;
public:
	CRUITextToSpeechAdapter(jobject obj)
	: _obj(obj)
	// VoiceInfo[] getAvailableVoices();
	, _getAvailableVoicesMethod(_obj, "getAvailableVoices", "()[Lorg/coolreader/newui/CRTTS$VoiceInfo;")
	// VoiceInfo getCurrentVoice();
	, _getCurrentVoiceMethod(_obj, "getCurrentVoice", "()Lorg/coolreader/newui/CRTTS$VoiceInfo;")
	// VoiceInfo getDefaultVoice();
	, _getDefaultVoiceMethod(_obj, "getDefaultVoice", "()Lorg/coolreader/newui/CRTTS$VoiceInfo;")
	// boolean setCurrentVoice(String id);
	, _setCurrentVoiceMethod(_obj, "setCurrentVoice", "(Ljava/lang/String;)Z")
	// boolean setRate(int rate);
	, _setRateMethod(_obj, "setRate", "(I)Z")
	// int getRate();
	, _getRateMethod(_obj, "getRate", "()I")
	// boolean canChangeCurrentVoice();
	, _canChangeCurrentVoiceMethod(_obj, "canChangeCurrentVoice", "()Z")
	// boolean tell(String text);
	, _tellMethod(_obj, "tell", "(Ljava/lang/String;)Z")
	// boolean isSpeaking();
	, _isSpeakingMethod(_obj, "isSpeaking", "()Z")
	// void stop();
	, _stopMethod(_obj, "stop", "()V")
	// boolean isInitialized();
	, _isInitializedMethod(_obj, "isInitialized", "()Z")
	// boolean isError();
	, _isErrorMethod(_obj, "isError", "()Z")
	, _callback(NULL)
	, _rate(50)
	, _speaking(false)
	{
		jobjectArray arr = _getAvailableVoicesMethod.callObjArray();
		if (arr) {
			int len = _env->GetArrayLength(arr);
			for ( int i=0; i<len; i++ ) {
				jobject obj = (jobject)_env->GetObjectArrayElement(arr, i);
				if (obj) {
					_voices.add(voiceFromJava(obj));
				}
			}
        	_env->DeleteLocalRef(arr);
		}
	}

	virtual CRUITextToSpeechCallback * getTextToSpeechCallback() {
    	return _callback;
    }

    virtual void setTextToSpeechCallback(CRUITextToSpeechCallback * callback) {
    	_callback = callback;
    }

    virtual void getAvailableVoices(LVPtrVector<CRUITextToSpeechVoice, false> & list) {
    	list.clear();
    	for(int i = 0; i < _voices.length(); i++)
    		list.add(_voices[i]);
    }

    CRUITextToSpeechVoice * voiceFromJava(jobject obj) {
    	CRUITextToSpeechVoice * res = NULL;
    	if (obj) {
    	    CRObjectAccessor acc(obj);
    		CRStringField id(acc, "id");
    		CRStringField name(acc, "name");
    		CRStringField lang(acc, "lang");
    		res = new CRUITextToSpeechVoice(id.get8(), name.get8(), lang.get8());
        	_env->DeleteLocalRef(obj);
    	}
    	return res;
    }

    CRUITextToSpeechVoice * toLocalVoice(CRUITextToSpeechVoice * v) {
    	if (!v)
    		return NULL;
    	for (int i = 0; i < _voices.length(); i++) {
    		if (_voices[i]->getId() == v->getId()) {
    			delete v;
    			return _voices[i];
    		}
    	}
    	return v;
    }

    virtual CRUITextToSpeechVoice * getCurrentVoice() {
    	CRUITextToSpeechVoice * res = voiceFromJava(_getCurrentVoiceMethod.callObj());
    	return toLocalVoice(res);
    }

    virtual CRUITextToSpeechVoice * getDefaultVoice() {
    	CRUITextToSpeechVoice * res = voiceFromJava(_getDefaultVoiceMethod.callObj());
    	return toLocalVoice(res);
    }

	void onSentenceFinished() {
		_speaking = false;
		if (_callback)
			_callback->onSentenceFinished();
	}

    virtual bool setCurrentVoice(lString8 id) {
    	jstring s = _env.toJavaString(Utf8ToUnicode(id));
    	bool res = _setCurrentVoiceMethod.callBool(s);
    	_env->DeleteLocalRef(s);
    	return res;
    }
    virtual bool setRate(int rate) {
    	if (rate < 0)
    		rate = 0;
    	if (rate > 100)
    		rate = 100;
    	_rate = rate;
    	return _setRateMethod.callBool(rate);
    }
    virtual int getRate() {
    	return _rate;
    }
    virtual bool canChangeCurrentVoice() {
    	return _canChangeCurrentVoiceMethod.callBool();
    }
    virtual bool tell(lString16 text) {
    	jstring s = _env.toJavaString(text);
    	bool res = _tellMethod.callBool(s);
    	_env->DeleteLocalRef(s);
    	if (res)
    		_speaking = true;
    	return res;
    }
    virtual bool isSpeaking() {
    	return _speaking; //_isSpeakingMethod.callBool();
    }
    virtual void stop() {
    	_stopMethod.callVoid();
    	_speaking = false;
    }
    virtual ~CRUITextToSpeechAdapter() {

    }
};


class DocViewNative : public LVAssetContainerFactory, public CRUIScreenUpdateManagerCallback, public CRUIPlatform {
    CRJNIEnv _env;
    CRObjectAccessor _obj;
    CRMethodAccessor _openResourceStreamMethod;
    CRMethodAccessor _listResourceDirMethod;
    CRMethodAccessor _getLeftMethod;
    CRMethodAccessor _getTopMethod;
    CRMethodAccessor _updateScreenMethod;
    CRMethodAccessor _setVolumeKeysEnabledMethod;
    CRMethodAccessor _setScreenOrientationMethod;
    CRMethodAccessor _setScreenBacklightTimeoutMethod;
    CRMethodAccessor _setScreenBacklightBrightnessMethod;
    CRMethodAccessor _copyToClipboardMethod;
    CRMethodAccessor _openLinkInExternalBrowserMethod;
    CRMethodAccessor _openFileInExternalAppMethod;
    CRMethodAccessor _showVirtualKeyboardMethod;
    CRMethodAccessor _hideVirtualKeyboardMethod;
    CRMethodAccessor _isFullscreenMethod;
    CRMethodAccessor _setFullscreenMethod;
    CRMethodAccessor _openUrlMethod;
    CRMethodAccessor _cancelDownloadMethod;
    CRMethodAccessor _setScreenUpdateModeMethod;
    CRMethodAccessor _setScreenUpdateIntervalMethod;
    CRMethodAccessor _getTTSMethod;

    CRUIMainWidget * _widget;
    CRUIEventManager _eventManager;
    CRUIEventAdapter _eventAdapter;
    CRUITextToSpeechAdapter * _tts;
	bool _surfaceCreated;
	bool _virtualKeyboardShown;
	int _dx;
	int _dy;
	lString8 _fileToOpen;
	bool m_coverpageManagerPaused;
public:
    DocViewNative(jobject obj);

    bool create() {
		CRLog::trace("Creating widget");
    	_widget = new CRUIMainWidget(this, this);
		CRLog::trace("_eventManager.setRootWidget for main widget");
    	_eventManager.setRootWidget(_widget);
		CRLog::trace("setFullscreen");
    	setFullscreen(_widget->getSettings()->getBoolDef(PROP_APP_FULLSCREEN, false));
    	if (!_fileToOpen.empty()) {
    		CRLog::trace("DocViewNative::create - setting book to open on start: %s", _fileToOpen.c_str());
    		_widget->setFileToOpenOnStart(_fileToOpen);
    		_fileToOpen.clear();
        	//_widget->openBookFromFile(_fileToOpen);
    	}
		CRLog::trace("Created widget");
    	return true;
    }

    // LVAssetContainerFactory implementation
	virtual LVContainerRef openAssetContainer(lString16 path);

	virtual LVStreamRef openAssetStream(lString16 path);

	virtual void uninit() {
		crconfig.uninitEngine();
	}

	virtual void onSurfaceChanged(int dx, int dy) {
		_dx = dx;
		_dy = dy;
		_surfaceCreated = false;
		deviceInfo.setScreenDimensions(_dx, _dy, 0);
		crconfig.setupResourcesForScreenSize();
	}

	virtual void clearImageCaches() {
		if (_widget)
			_widget->clearImageCaches();
		crconfig.clearGraphicsCaches();
	}

	virtual void onSurfaceDestroyed() {
		clearImageCaches();
		_surfaceCreated = false;
	}

	virtual void onPause() {
		if (!m_coverpageManagerPaused) {
		    CRPauseCoverpageManager();
		    m_coverpageManagerPaused = true;
		}
		clearImageCaches();
	}

	virtual void onResume() {
//		if (m_coverpageManagerPaused) {
//		    CRResumeCoverpageManager();
//		    m_coverpageManagerPaused = false;
//		}
	}

	virtual void onDraw() {

		if (!_surfaceCreated) {
			CRLog::info("onDraw() - surface just created");
			if (!_widget) {
				CRLog::info("onDraw() - creating widget");
				create();
			} else {
				CRLog::info("onDraw() - invalidating GL caches");
				clearImageCaches();
			}
			_surfaceCreated = true;
			_widget->measure(_dx, _dy);
			_widget->layout(0, 0, _dx, _dy);
		}


		lvRect pos = _widget->getPos();

		bool needLayout, needDraw, animating;
		CRUICheckUpdateOptions(_widget, needLayout, needDraw, animating);
		_widget->invalidate();
		if (needLayout) {
			//CRLog::trace("need layout");
			_widget->measure(pos.width(), pos.height());
			_widget->layout(0, 0, pos.width(), pos.height());
		}

		//CRLog::debug("Drawing CR GL %dx%d", pos.width(), pos.height());
		GLDrawBuf buf(pos.width(), pos.height(), 32, false);
		buf.beforeDrawing();
		_widget->draw(&buf);
//		static int updateTrigger = 0;
//		updateTrigger++;
//		buf.FillRect(10, 10, 20, 20, (updateTrigger & 1) ? 0x80800000 : 0x8000FFFF);
		buf.afterDrawing();

		if (m_coverpageManagerPaused) {
		    CRResumeCoverpageManager();
		    m_coverpageManagerPaused = false;
		}
	}

	bool handleTouchEvent(CRTouchEventWrapper * event) {
		if (!_surfaceCreated)
			return false;
		int x0 = _getLeftMethod.callInt();
		int y0 = _getTopMethod.callInt();
		return _eventAdapter.dispatchTouchEvent(event, x0, y0);
	}

	bool handleKeyEvent(CRKeyEventWrapper * event) {
		if (!_surfaceCreated)
			return false;
		return _eventAdapter.dispatchKeyEvent(event);
	}

    /// set animation fps (0 to disable) and/or update screen instantly
    virtual void setScreenUpdateMode(bool updateNow, int animationFps) {
    	_updateScreenMethod.callVoid(updateNow ? JNI_TRUE : JNI_FALSE, animationFps > 0 ? JNI_TRUE : JNI_FALSE);
    }

    virtual bool supportsVolumeKeys() {
    	return true;
    }
    virtual void setVolumeKeysEnabled(bool flg) {
    	_setVolumeKeysEnabledMethod.callVoid(flg ? JNI_TRUE : JNI_FALSE);
    }

    virtual bool supportsScreenOrientation() { return true; }
    virtual void setScreenOrientation(int n) {
    	//CRLog::trace("setScreenOrientation %d", n);
    	_setScreenOrientationMethod.callVoidInt(n);
    }

    virtual bool supportsScreenBacklightTimeout() { return true; }
    virtual void setScreenBacklightTimeout(int n) {
    	_setScreenBacklightTimeoutMethod.callVoidInt(n);
    }

    virtual bool supportsScreenBacklightBrightness() { return true; }
    virtual void setScreenBacklightBrightness(int n) {
    	_setScreenBacklightBrightnessMethod.callVoidInt(n);
    }


    // CRUIPlatform methods
    virtual void exitApp() {
        CRMethodAccessor method(_obj, "exitApp", "()V");
        method.callVoid();
    }

	/// minimize app or show Home Screen
	virtual void minimizeApp() {
        CRMethodAccessor method(_obj, "minimizeApp", "()V");
        method.callVoid();
	}

    // copy text to clipboard
    virtual void copyToClipboard(lString16 text) {
    	jstring s = _env.toJavaString(text);
    	_copyToClipboardMethod.callVoid(s);
    	_env->DeleteLocalRef(s);
    }

    /// override to open URL in external browser; returns false if failed or feature not supported by platform
    virtual bool openLinkInExternalBrowser(lString8 url) {
    	jstring s = _env.toJavaString(Utf8ToUnicode(url));
    	bool res = _openLinkInExternalBrowserMethod.callBool(s);
    	_env->DeleteLocalRef(s);
    	return res;
    }

    /// override to open file in external application; returns false if failed or feature not supported by platform
    virtual bool openFileInExternalApp(lString8 filename, lString8 mimeType) {
    	jstring s = _env.toJavaString(Utf8ToUnicode(filename));
    	jstring s2 = _env.toJavaString(Utf8ToUnicode(mimeType));
    	bool res = _openFileInExternalAppMethod.callBool(s, s2);
    	_env->DeleteLocalRef(s);
    	_env->DeleteLocalRef(s2);
    	return res;
    }

    /// return true if platform supports native virtual keyboard
    virtual bool supportsVirtualKeyboard() {
    	return true;
    }
    /// return true if platform native virtual keyboard is shown
    virtual bool isVirtualKeyboardShown() {
    	return _virtualKeyboardShown;
    }
    /// show platform native virtual keyboard
    virtual void showVirtualKeyboard(int mode, lString16 text, bool multiline) {
    	CR_UNUSED3(mode, text, multiline);
    	if (_virtualKeyboardShown)
    		return;
    	_virtualKeyboardShown = true;
    	_showVirtualKeyboardMethod.callVoid();

    }

    /// hide platform native virtual keyboard
    virtual void hideVirtualKeyboard() {
    	if (!_virtualKeyboardShown)
    		return;
    	_virtualKeyboardShown = false;
    	_hideVirtualKeyboardMethod.callVoid();
    }

    virtual bool supportsFullscreen() {
    	return true;
    }
    virtual bool isFullscreen() {
    	return _isFullscreenMethod.callBool();
    }
    virtual void setFullscreen(bool fullscreen) {
    	return _setFullscreenMethod.callVoidBool(fullscreen);
    }

    virtual void setScreenUpdateMode(int mode) {
    	_setScreenUpdateModeMethod.callVoid(mode);
    }

    virtual void setScreenUpdateInterval(int interval) {
    	_setScreenUpdateIntervalMethod.callVoid(interval);
    }

    void setBatteryLevel(int level) {
    	// TODO
    }

    void loadDocument(lString16 path) {
    	if (_widget) {
        	_widget->openBookFromFile(UnicodeToUtf8(path));
    	} else {
    		CRLog::error("DocViewNative::loadDocument() called w/o create");
    		_fileToOpen = UnicodeToUtf8(path);
    	}
    }

    /// returns 0 if not supported, task ID if download task is started
    virtual int openUrl(lString8 url, lString8 method, lString8 login, lString8 password, lString8 saveAs) {
    	jstring _url = _env.toJavaString(Utf8ToUnicode(url));
    	jstring _method = _env.toJavaString(Utf8ToUnicode(method));
    	jstring _login = _env.toJavaString(Utf8ToUnicode(login));
    	jstring _password = _env.toJavaString(Utf8ToUnicode(password));
    	jstring _saveAs = _env.toJavaString(Utf8ToUnicode(saveAs));
    	int res = _openUrlMethod.callInt(_url, _method, _login, _password, _saveAs);
    	_env->DeleteLocalRef(_url);
    	_env->DeleteLocalRef(_method);
    	_env->DeleteLocalRef(_login);
    	_env->DeleteLocalRef(_password);
    	_env->DeleteLocalRef(_saveAs);
        return res;
    }

    /// cancel specified download task
    virtual void cancelDownload(int downloadTaskId) {
    	_cancelDownloadMethod.callVoidInt(downloadTaskId);
    }

    /// pass download result to window
    virtual void onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream) {
    	_eventManager.dispatchDownloadResult(downloadTaskId, url, result, resultMessage, mimeType, size, stream);
    }

    /// download progress
    virtual void onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded) {
    	_eventManager.dispatchDownloadProgress(downloadTaskId, url, result, resultMessage, mimeType, size, sizeDownloaded);
    }

    virtual CRUITextToSpeech * getTextToSpeech() {
    	return _tts;
    }

	void onSentenceFinished() {
		if (_tts) {
			_tts->onSentenceFinished();
		}
	}

	void onTtsInitialized() {
		CRLog::info("onTtsInitialized");
		_tts = new CRUITextToSpeechAdapter(_getTTSMethod.callObj());
		_widget->ttsInitialized();
	}

	~DocViewNative() {
    	delete _widget;
    	if (_tts)
    		delete _tts;
    	LVSetAssetContainerFactory(NULL);
    }
};

class CRAssetDir : public LVContainer {
	DocViewNative * _native;
	lString16 _path;
	LVPtrVector<LVContainerItemInfo> _items;
public:
	class FileItem : public LVContainerItemInfo {
		lString16 _name;
	public:
		FileItem(lString16 name) : _name(name) { }
		virtual lvsize_t        GetSize() const { return 0; }
	    virtual const lChar16 * GetName() const {
	    	return _name.c_str();
	    }
	    virtual lUInt32         GetFlags() const {
	    	return 0;
	    }
	    virtual bool            IsContainer() const {
	    	return false;
	    }
	};
	class DirItem : public LVContainerItemInfo {
		lString16 _name;
	public:
		DirItem(lString16 name) : _name(name) { }
		virtual lvsize_t        GetSize() const { return 0; }
	    virtual const lChar16 * GetName() const {
	    	return _name.c_str();
	    }
	    virtual lUInt32         GetFlags() const {
	    	return 0;
	    }
	    virtual bool            IsContainer() const {
	    	return true;
	    }
	};
	CRAssetDir(DocViewNative * native, lString16 path, lString16Collection & items) : _native(native), _path(path) {
		lString16 lastDir;
		for (int i = 0; i < items.length(); i++) {
			lString16 itemPath = items[i];
			int p = itemPath.pos("/");
			if (p >= 0) {
				itemPath = itemPath.substr(0, p);
				if (itemPath == lastDir)
					continue;
				DirItem * item = new DirItem(itemPath);
				lastDir = itemPath;
				_items.add(item);
			} else {
				FileItem * item = new FileItem(itemPath);
				_items.add(item);
			}
		}
	}
    virtual LVContainer * GetParentContainer() { return NULL; }
    //virtual const LVContainerItemInfo * GetObjectInfo(const wchar_t * pname);
    virtual const LVContainerItemInfo * GetObjectInfo(int index) {
    	if (index >= 0 && index < _items.length())
    		return _items[index];
    	return NULL;
    }
    virtual int GetObjectCount() const {
    	return _items.length();
    }
    virtual LVStreamRef OpenStream( const lChar16 * fname, lvopen_mode_t mode ) {
    	if (mode != LVOM_READ)
    		return LVStreamRef();
    	lString16 path(fname);
    	lString16 base = _path;
    	if (base.length())
    		base += "/";
    	return _native->openAssetStream(base + path);
    }
    /// returns object size (file size or directory entry count)
    virtual lverror_t GetSize( lvsize_t * pSize ) {
    	if (pSize)
    		*pSize = _items.length();
    	return LVERR_OK;
    }
};

class CRInputStream : public LVStream {
	lvoffset_t pos;
	lvoffset_t size;
	bool eof;
public:

	/// Seek (change file pos)
    /**
        \param offset is file offset (bytes) relateve to origin
        \param origin is offset base
        \param pNewPos points to place to store new file position
        \return lverror_t status: LVERR_OK if success
    */
    virtual lverror_t Seek( lvoffset_t offset, lvseek_origin_t origin, lvpos_t * pNewPos ) {
    	lvoffset_t newpos = pos;
    	switch (origin) {
    	case LVSEEK_CUR:
    		newpos = pos + offset;
    		break;
    	case LVSEEK_SET:
    		newpos = offset;
    		break;
    	case LVSEEK_END:
    		newpos = size + offset;
    		break;
    	}
    	if (newpos < pos) {
        	//CRLog::trace("resetMethod(%d, %d)", (int)newpos, (int)pos);
    		_resetMethod.callVoid();
    		pos = 0;
    		eof = (size == 0);
    	}
    	if (newpos > pos) {
    		lvoffset_t toSkip = newpos - pos;
        	while (toSkip > 0) {
        		jlong skipped = _skipMethod.callLong(toSkip);
            	//CRLog::trace("skipMethod(%d, %d)", (int)toSkip, (int)skipped);
        		if (skipped <= 0)
        			break;
        		pos += skipped;
        		toSkip -= skipped;
    		}
    	}
    	if (pNewPos)
    		*pNewPos = pos;
    	//CRLog::trace("Seek(%d, %d, %d)", (int)offset, (int)origin, (int)pos);
		eof = (pos >= size);
    	return LVERR_OK;
    }

    /// Get file position
    /**
        \return lvpos_t file position
    */
    virtual lvpos_t GetPos()
    {
        return pos;
    }

    virtual lverror_t SetSize( lvsize_t size ) {
    	return LVERR_NOTIMPL;
    }

    /// Read
    /**
        \param buf is buffer to place bytes read from stream
        \param count is number of bytes to read from stream
        \param nBytesRead is place to store real number of bytes read from stream
        \return lverror_t status: LVERR_OK if success
    */
    virtual lverror_t Read( void * buf, lvsize_t count, lvsize_t * nBytesRead ) {
    	//CRLog::trace("Read(%d)", (int)count);
    	lUInt8 * pbuf = (lUInt8*)buf;
    	lvsize_t bytesRead = 0;
    	if (count <= 0) {
    		if (nBytesRead)
    			*nBytesRead = 0;
    		return LVERR_OK;
    	}
    	int bufSize = count > 16384 ? 16384 : count;
    	jbyteArray array = _obj->NewByteArray(bufSize);
    	while (count > 0 && pos < size) {
    		int read = _readMethod.callInt(array);
    		if (read <= 0) {
    			CRLog::warn("no bytes read");
    			break;
    		}
    		//CRLog::trace("readMethod(%d) - %d bytes read", bufSize, read);
    		jboolean isCopy;
    		jbyte* data = _obj->GetByteArrayElements(array, &isCopy);
    		memcpy(pbuf, data, read);
    	    _obj->ReleaseByteArrayElements(array, data, 0);
    	    //CRLog::trace("bytes: %02x %02x %02x %02x", (lUInt8)data[0], (lUInt8)data[1], (lUInt8)data[2], (lUInt8)data[3]);
    	    //CRLog::trace("lastbytes: %02x %02x %02x %02x", (lUInt8)pbuf[read-4], (lUInt8)pbuf[read-3], (lUInt8)pbuf[read-2], (lUInt8)pbuf[read-1]);
    		bytesRead += read;
    		count -= read;
    		pbuf += read;
    		pos += read;
    	}
    	_obj->DeleteLocalRef(array);
    	eof = (pos >= size);
		if (nBytesRead)
			*nBytesRead = bytesRead;
    	return LVERR_OK;
    }

    virtual lverror_t Write( const void * buf, lvsize_t count, lvsize_t * nBytesWritten ) {
    	return LVERR_NOTIMPL;
    }

    /// Check whether end of file is reached
    /**
        \return true if end of file reached
    */
    virtual bool Eof() {
    	return eof;
    }

    /// Destructor
    virtual ~CRInputStream() {
    	_closeMethod.callVoid();
    }
    /// Constructor
    CRInputStream(jobject obj)
    : _obj(obj)
    , _closeMethod(_obj, "close", "()V")
    , _readMethod(_obj, "read", "([B)I")
    , _resetMethod(_obj, "reset", "()V")
    , _skipMethod(_obj, "skip", "(J)J")
    {
    	pos = 0;
    	size = 0;
    	eof = false;
    	// calculating size
    	for (;;) {
    		jlong skipped = _skipMethod.callLong(65536);
    		if (skipped <= 0)
    			break;
    		size += skipped;
    	}
    	_resetMethod.callVoid();
//    	CRLog::trace("stream size is %d", (int)size);
//    	size = 0;
//    	// calculating size
//    	for (;;) {
//    		jlong skipped = _skipMethod.callLong(65536);
//    		if (skipped <= 0)
//    			break;
//    		size += skipped;
//    	}
//    	CRLog::trace("stream size 2 is %d", (int)size);
//    	_resetMethod.callVoid();
		eof = (size == 0);
    }
private:
    CRObjectAccessor _obj;
    CRMethodAccessor _closeMethod;
    CRMethodAccessor _readMethod;
    CRMethodAccessor _resetMethod;
    CRMethodAccessor _skipMethod;
};

LVStreamRef DocViewNative::openAssetStream(lString16 path) {
	jstring str = _env.toJavaString(path);
	jobject obj = _openResourceStreamMethod.callObj((jobject)str);
	_env->DeleteLocalRef(str);
	if (!obj)
		return LVStreamRef();
	LVStreamRef res = LVStreamRef(new CRInputStream(obj));
	if (!res.isNull())
		res = LVCreateMemoryStream(res);
	return res;
}

LVContainerRef DocViewNative::openAssetContainer(lString16 path) {
	jstring str = _obj.toJavaString(path);
	jobjectArray array = _listResourceDirMethod.callObjArray(str);
	if (!array)
		return LVContainerRef();
	lString16Collection items;
	_obj.fromJavaStringArray(array, items);
	LVContainerRef res(new CRAssetDir(this, path, items));
	_obj->DeleteLocalRef(str);
	_obj->DeleteLocalRef(array);
	return res;
}

DocViewNative::DocViewNative(jobject obj)
	: _obj(obj)
	, _openResourceStreamMethod(_obj, "openResourceStream", "(Ljava/lang/String;)Ljava/io/InputStream;")
	, _listResourceDirMethod(_obj, "listResourceDir", "(Ljava/lang/String;)[Ljava/lang/String;")
	, _getLeftMethod(_obj, "getLeft", "()I")
	, _getTopMethod(_obj, "getTop", "()I")
	, _updateScreenMethod(_obj, "updateScreen", "(ZZ)V")
	, _setVolumeKeysEnabledMethod(_obj, "setVolumeKeysEnabled", "(Z)V")
	, _setScreenOrientationMethod(_obj, "setScreenOrientation", "(I)V")
	, _setScreenBacklightTimeoutMethod(_obj, "setScreenBacklightTimeout", "(I)V")
	, _setScreenBacklightBrightnessMethod(_obj, "setScreenBacklightBrightness", "(I)V")
	, _copyToClipboardMethod(_obj, "copyToClipboard", "(Ljava/lang/String;)V")
	, _openLinkInExternalBrowserMethod(_obj, "openLinkInExternalBrowser", "(Ljava/lang/String;)Z")
	, _openFileInExternalAppMethod(_obj, "openFileInExternalApp", "(Ljava/lang/String;Ljava/lang/String;)Z")
	, _showVirtualKeyboardMethod(_obj, "showVirtualKeyboard", "()V")
	, _hideVirtualKeyboardMethod(_obj, "hideVirtualKeyboard", "()V")
	, _isFullscreenMethod(_obj, "isFullscreen", "()Z")
	, _setFullscreenMethod(_obj, "setFullscreen", "(Z)V")
	, _openUrlMethod(_obj, "openUrl", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I")
	, _cancelDownloadMethod(_obj, "cancelDownload", "(I)V")
	, _setScreenUpdateModeMethod(_obj, "setScreenUpdateMode", "(I)V")
	, _setScreenUpdateIntervalMethod(_obj, "setScreenUpdateInterval", "(I)V")
	, _getTTSMethod(_obj, "getTTS", "()Lorg/coolreader/newui/CRTTS;")
	, _widget(NULL)
	, _eventAdapter(&_eventManager)
	, _surfaceCreated(false)
	, _virtualKeyboardShown(false)
	, _dx(480)
	, _dy(320)
{
	LVSetAssetContainerFactory(this);
	m_coverpageManagerPaused = true;
	_tts = NULL;
}

static DocViewNative * getNative(JNIEnv * env, jobject _this)
{
	DocViewNative * res = (DocViewNative *)env->GetLongField(_this, gNativeObjectID);
	if (res == NULL)
		CRLog::warn("Native DocView is NULL");
	return res;
}

class JNICDRLogger : public CRLog
{
public:
    JNICDRLogger()
    {
    	curr_level = CRLog::LL_DEBUG;
    }
protected:

	virtual void log( const char * lvl, const char * msg, va_list args)
	{
	    #define MAX_LOG_MSG_SIZE 1024
		static char buffer[MAX_LOG_MSG_SIZE+1];
		vsnprintf(buffer, MAX_LOG_MSG_SIZE, msg, args);
		int level = ANDROID_LOG_DEBUG;
		//LOGD("CRLog::log is called with LEVEL %s, pattern %s", lvl, msg);
		if ( !strcmp(lvl, "FATAL") )
			level = ANDROID_LOG_FATAL;
		else if ( !strcmp(lvl, "ERROR") )
			level = ANDROID_LOG_ERROR;
		else if ( !strcmp(lvl, "WARN") )
			level = ANDROID_LOG_WARN;
		else if ( !strcmp(lvl, "INFO") )
			level = ANDROID_LOG_INFO;
		else if ( !strcmp(lvl, "DEBUG") )
			level = ANDROID_LOG_DEBUG;
		else if ( !strcmp(lvl, "TRACE") )
			level = ANDROID_LOG_VERBOSE;
		__android_log_write(level, LOG_TAG, buffer);
	}
};

//typedef void (lv_FatalErrorHandler_t)(int errorCode, const char * errorText );

void cr3androidFatalErrorHandler(int errorCode, const char * errorText )
{
	static char str[1001];
	snprintf(str, 1000, "CoolReader Fatal Error #%d: %s", errorCode, errorText);
	//LOGE(str);
	LOGE("CoolReader Fatal Error #%d: %s", errorCode, errorText);
	//LOGASSERTFAILED(errorText, str);
	LOGASSERTFAILED("%s  %s", errorText, str);
}

class AndroidConcurrencyProvider : public CRConcurrencyProvider {
    CRObjectAccessor crviewObject;
    CRMethodAccessor crviewRunInGLThread;
    CRMethodAccessor crviewRunInGLThreadDelayed;
    CRMethodAccessor crviewCreateCRThread;
    CRMethodAccessor crviewSleepMs;
    CRClassAccessor lockClass;
    CRClassAccessor monitorClass;
    CRClassAccessor threadClass;
public:

    class AndroidMutex : public CRMutex {
        CRObjectAccessor mutex;
        CRMethodAccessor lock;
        CRMethodAccessor unlock;
    public:
        AndroidMutex(jobject object)
        		: mutex(object),
        		  lock(mutex, "lock", "()V"),
        		  unlock(mutex, "unlock", "()V") {}
        virtual void acquire() { lock.callVoid(); }
        virtual void release() { unlock.callVoid(); }
    };

    class AndroidMonitor : public CRMonitor {
        CRObjectAccessor monitor;
        CRMethodAccessor lock;
        CRMethodAccessor unlock;
        CRMethodAccessor waitMethod;
        CRMethodAccessor notifyMethod;
        CRMethodAccessor notifyAllMethod;
    public:
        AndroidMonitor(jobject object) :
        	monitor(object),
        	lock(monitor, "lock", "()V"),
        	unlock(monitor, "unlock", "()V"),
        	waitMethod(monitor, "await", "()V"),
        	notifyMethod(monitor, "signal", "()V"),
        	notifyAllMethod(monitor, "signalAll", "()V")
        	{}
        virtual void acquire() { lock.callVoid(); }
        virtual void release() { unlock.callVoid(); }
        virtual void wait() { waitMethod.callVoid(); }
        virtual void notify() { notifyMethod.callVoid(); }
        virtual void notifyAll() { notifyAllMethod.callVoid(); }
        virtual ~AndroidMonitor() {
        	//CRLog::trace("~AndroidMonitor");
        }
    };

    class AndroidThread : public CRThread {
        CRObjectAccessor thread;
        CRMethodAccessor startMethod;
        CRMethodAccessor joinMethod;
    public:
        AndroidThread(jobject object) :
        	thread(object),
        	startMethod(thread, "start", "()V"),
        	joinMethod(thread, "join", "()V")
        	{}
        virtual ~AndroidThread() {
        	CRLog::trace("~AndroidThread");
        }
        virtual void start() {
        	crSetSignalHandler();
        	startMethod.callVoid();
        }
        virtual void join() {
        	joinMethod.callVoid();
        }
    };

public:
    virtual CRMutex * createMutex() {
        return new AndroidMutex(lockClass.newObject());
    }

    virtual CRMonitor * createMonitor() {
        return new AndroidMonitor(monitorClass.newObject());
    }

    class CRRunnableWrapper : public CRRunnable {
    	CRRunnable * _task;
    public:
    	CRRunnableWrapper(CRRunnable * task) : _task(task) {

    	}
    	virtual void run() {
    		_task->run();
    	}
    };

    virtual CRThread * createThread(CRRunnable * threadTask) {
    	CRRunnableWrapper * wrappedTask = new CRRunnableWrapper(threadTask);
        return new AndroidThread(crviewCreateCRThread.callObj((jlong)wrappedTask));
    }

    /// task will be deleted after execution
    virtual void executeGui(CRRunnable * task) {
    	// run in GL thread
    	crviewRunInGLThread.callVoid((jlong)task);
    }

    /// execute task delayed; already scheduled but not executed task will be deleted; pass NULL task to cancel active tasks
    virtual void executeGui(CRRunnable * task, int delayMillis) {
    	if (!task)
    		return; // it was called to cancel timer; ignore under Android
    	crviewRunInGLThreadDelayed.callVoidLongLong((jlong)task, (jlong)delayMillis);
    }

    AndroidConcurrencyProvider(jobject _crviewObject) :
    		crviewObject(_crviewObject),
    		crviewRunInGLThread(crviewObject, "runInGLThread", "(J)V"),
    		crviewRunInGLThreadDelayed(crviewObject, "runInGLThreadDelayed", "(JJ)V"),
    		crviewCreateCRThread(crviewObject, "createCRThread", "(J)Lorg/coolreader/newui/CRThread;"),
    		crviewSleepMs(crviewObject, "sleepMs", "(J)V"),
    		lockClass("org/coolreader/newui/CRLock"),
    		monitorClass("org/coolreader/newui/CRCondition"),
    		threadClass("org/coolreader/newui/CRThread")

    {
    }
    /// sleep current thread
    virtual void sleepMs(int durationMs) {
    	crviewSleepMs.callVoid((jlong)durationMs);
    }

    virtual ~AndroidConcurrencyProvider() {}
};


/*
 * Class:     org_coolreader_newui_CRView
 * Method:    initInternal
 * Signature: (Lorg/coolreader/newui/CRConfig;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_coolreader_newui_CRView_initInternal
  (JNIEnv * _env, jobject _this, jobject _config) {
    CRJNIEnv env(_env);
    jclass rvClass = env->FindClass("org/coolreader/newui/CRView");
    gNativeObjectID = env->GetFieldID(rvClass, "mNativeObject", "J");

	LOGI("initInternal called");
	// set fatal error handler
	crSetFatalErrorHandler( &cr3androidFatalErrorHandler );
	LOGD("Redirecting CDRLog to Android");
	CRLog::setLogger( new JNICDRLogger() );
	CRLog::setLogLevel( CRLog::LL_TRACE );

	CRLog::trace("Setting crash handler");
	crSetSignalHandler();

	SaveJVMPointer(_env);

    DocViewNative * obj = new DocViewNative(_this);
    env->SetLongField(_this, gNativeObjectID, (jlong)obj);

	concurrencyProvider = new AndroidConcurrencyProvider(_this);

    // init config
    CRObjectAccessor cfg(_env, _config);

    // set screen size
    int screenX = CRIntField(cfg,"screenX").get();
    int screenY = CRIntField(cfg,"screenY").get();
    int screenDPI = CRIntField(cfg,"screenDPI").get();
    deviceInfo.setScreenDimensions(screenX, screenY, screenDPI);

    crconfig.apiLevel = CRIntField(cfg,"apiLevel").get();

    crconfig.coverCacheDir = CRStringField(cfg,"coverCacheDir").get8();
    crconfig.cssDir = CRStringField(cfg,"cssDir").get8();
    crconfig.dbFile = CRStringField(cfg,"dbFile").get8();
    crconfig.iniFile = CRStringField(cfg,"iniFile").get8();
    crconfig.hyphDir = CRStringField(cfg,"hyphDir").get8();
    crconfig.resourceDir = CRStringField(cfg,"resourceDir").get8();
    crconfig.themesDir = CRStringField(cfg,"themesDir").get8();
    crconfig.manualsDir = CRStringField(cfg,"manualsDir").get8();
    crconfig.manualFile = CRStringField(cfg,"manualFile").get8();
    crconfig.uiFontFace = CRStringField(cfg,"uiFontFace").get8();
    crconfig.fallbackFontFace = CRStringField(cfg,"fallbackFontFace").get8();
    crconfig.docCacheDir = CRStringField(cfg,"docCacheDir").get8();
    crconfig.defaultDownloadsDir = CRStringField(cfg,"defaultDownloadsDir").get8();
    crconfig.i18nDir = CRStringField(cfg,"i18nDir").get8();
    crconfig.systemLanguage = CRStringField(cfg,"systemLanguage").get8();
    crconfig.updateScreenModeInCurrentThread = true;

    lString8 internalStorageDir = CRStringField(cfg,"internalStorageDir").get8();
    lString8 sdcardDir = CRStringField(cfg,"sdcardDir").get8();
    deviceInfo.topDirs.clear();
    if (!internalStorageDir.empty())
    	deviceInfo.topDirs.addItem(DIR_TYPE_INTERNAL_STORAGE, internalStorageDir, 0);
    if (!sdcardDir.empty())
    	deviceInfo.topDirs.addItem(DIR_TYPE_SD_CARD, sdcardDir, 1);

    crconfig.docCacheMaxBytes = CRIntField(cfg,"docCacheMaxBytes").get();
    crconfig.coverDirMaxItems = CRIntField(cfg,"coverDirMaxItems").get();
    crconfig.coverDirMaxFiles = CRIntField(cfg,"screenDPI").get();
    crconfig.coverDirMaxSize = CRIntField(cfg,"coverDirMaxSize").get();
    crconfig.coverRenderCacheMaxItems = CRIntField(cfg,"coverRenderCacheMaxItems").get();
    crconfig.coverRenderCacheMaxBytes = CRIntField(cfg,"coverRenderCacheMaxBytes").get();
    crconfig.einkMode = CRBoolField(cfg,"einkMode").get();
    crconfig.einkModeSettingsSupported = CRBoolField(cfg,"einkModeSettingsSupported").get();
    lString16Collection fonts;
    CRStringArrayField fontFilesField(cfg, "fontFiles");
    for (int i = 0; i < fontFilesField.length(); i++) {
    	crconfig.fontFiles.add(fontFilesField.get8(i));
    }
    //env.fromJavaStringArray()

    CRLog::info("Calling initEngine");
    crconfig.initEngine(false);
    crconfig.setupResources(crconfig.resourceDir);
    CRLog::info("Done initEngine");

    CRPauseCoverpageManager();
    //obj->create(); // will be called on first draw

    return JNI_TRUE;
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    isLink
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_coolreader_newui_CRView_isLink
  (JNIEnv * env, jclass obj, jstring pathname)
{
	if ( !pathname )
		return NULL;
	int res = JNI_FALSE;
	jboolean iscopy;
	const char * s = env->GetStringUTFChars(pathname, &iscopy);
	struct stat st;
	lString8 path;
	if ( !lstat( s, &st) ) {
		if ( S_ISLNK(st.st_mode) ) {
			char buf[1024];
			int len = readlink(s, buf, sizeof(buf) - 1);
			if (len != -1) {
				buf[len] = 0;
				path = lString8(buf);
			}
		}
	}
	env->ReleaseStringUTFChars(pathname, s);
	return !path.empty() ? (jstring)env->NewGlobalRef(env->NewStringUTF(path.c_str())) : NULL;
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    callCRRunnableInternal
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_callCRRunnableInternal
	(JNIEnv * env, jclass obj, jlong ptr)
{
	//CRLog::trace("Calling CRRunnable %08x", (unsigned)ptr);
	CRRunnable * runnable = (CRRunnable*)ptr;
	COFFEE_TRY_JNI(env, runnable->run());
	delete runnable;
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    uninitInternal
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_coolreader_newui_CRView_uninitInternal
  (JNIEnv * _env, jobject _this)
{
	DocViewNative * native = getNative(_env, _this);
	COFFEE_TRY_JNI(_env, native->uninit());
	return JNI_TRUE;
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    drawInternal
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_drawInternal
  (JNIEnv * _env, jobject _this)
{
	DocViewNative * native = getNative(_env, _this);
	//CRLog::trace("Java_org_coolreader_newui_CRView_drawInternal");
	COFFEE_TRY_JNI(_env, native->onDraw());
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    onSentenceFinishedInternal
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_onSentenceFinishedInternal
  (JNIEnv * _env, jobject _this)
{
	DocViewNative * native = getNative(_env, _this);
	CRLog::trace("Java_org_coolreader_newui_CRView_onSentenceFinishedInternal");
	COFFEE_TRY_JNI(_env, native->onSentenceFinished());
	//native->onPause();
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    onTtsInitializedInternal
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_onTtsInitializedInternal
  (JNIEnv * _env, jobject _this)
{
	DocViewNative * native = getNative(_env, _this);
	CRLog::trace("Java_org_coolreader_newui_CRView_onTtsInitializedInternal");
	COFFEE_TRY_JNI(_env, native->onTtsInitialized());
	//native->onPause();
}


/*
 * Class:     org_coolreader_newui_CRView
 * Method:    onPauseInternal
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_onPauseInternal
  (JNIEnv * _env, jobject _this)
{
	DocViewNative * native = getNative(_env, _this);
	CRLog::trace("Java_org_coolreader_newui_CRView_onPauseInternal");
	COFFEE_TRY_JNI(_env, native->onPause());
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    onResumeInternal
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_onResumeInternal
  (JNIEnv * _env, jobject _this)
{
	DocViewNative * native = getNative(_env, _this);
	CRLog::trace("Java_org_coolreader_newui_CRView_onResumeInternal");
	COFFEE_TRY_JNI(_env, native->onResume());
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    surfaceChangedInternal
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_surfaceChangedInternal
  (JNIEnv * _env, jobject _this, jint dx, jint dy)
{
	DocViewNative * native = getNative(_env, _this);
	COFFEE_TRY_JNI(_env, native->onSurfaceChanged(dx, dy));
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    surfaceDestroyedInternal
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_surfaceDestroyedInternal
  (JNIEnv * _env, jobject _this)
{
	DocViewNative * native = getNative(_env, _this);
	COFFEE_TRY_JNI(_env, native->onSurfaceDestroyed());
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    handleKeyEventInternal
 * Signature: (Landroid/view/KeyEvent;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_coolreader_newui_CRView_handleKeyEventInternal
  (JNIEnv * _env, jobject _this, jobject _event)
{
	DocViewNative * native = getNative(_env, _this);
	CRKeyEventWrapper event(_env, _event);
	jboolean res = JNI_FALSE;
	COFFEE_TRY_JNI(_env, res = native->handleKeyEvent(&event) ? JNI_TRUE : JNI_FALSE);
	return res;
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    handleTouchEventInternal
 * Signature: (Landroid/view/MotionEvent;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_coolreader_newui_CRView_handleTouchEventInternal
  (JNIEnv * _env, jobject _this, jobject _event)
{
	DocViewNative * native = getNative(_env, _this);
	CRTouchEventWrapper event(_env, _event);
	jboolean res = JNI_FALSE;
	COFFEE_TRY_JNI(_env, res = native->handleTouchEvent(&event) ? JNI_TRUE : JNI_FALSE);
    return res;
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    loadBookInternal
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_loadBookInternal
  (JNIEnv * _env, jobject _this, jstring _path)
{
    CRJNIEnv env(_env);
	DocViewNative * native = getNative(_env, _this);
	lString16 path = env.fromJavaString(_path);
	COFFEE_TRY_JNI(_env, native->loadDocument(path));
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    setBatteryLevelInternal
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_setBatteryLevelInternal
  (JNIEnv * _env, jobject _this, jint level)
{
	DocViewNative * native = getNative(_env, _this);
	COFFEE_TRY_JNI(_env, native->setBatteryLevel(level));
}

// void onDownloadResult(int downloadTaskId, String url, int result, String resultMessage, String mimeType, int size, byte[] data, String file);

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    onDownloadResult
 * Signature: (ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;I[B)V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_onDownloadResult
  (JNIEnv * _env, jobject _this, jint downloadTaskId, jstring _url, jint result, jstring _resultMessage, jstring _mimeType, jint size, jbyteArray data)
{
    CRJNIEnv env(_env);
	DocViewNative * native = getNative(_env, _this);
	lString8 url = UnicodeToUtf8(env.fromJavaString(_url));
	lString8 resultMessage = UnicodeToUtf8(env.fromJavaString(_resultMessage));
	lString8 mimeType = UnicodeToUtf8(env.fromJavaString(_mimeType));
	LVStreamRef stream;
	if (data) {
		stream = env.jbyteArrayToStream(data);
	}
	COFFEE_TRY_JNI(_env, native->onDownloadResult(downloadTaskId, url, result, resultMessage, mimeType, size, stream));
}

// void onDownloadProgress(int downloadTaskId, String url, int result, String resultMessage, String mimeType, int size, int sizeDownloaded);
/*
 * Class:     org_coolreader_newui_CRView
 * Method:    onDownloadProgress
 * Signature: (ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_onDownloadProgress
  (JNIEnv * _env, jobject _this, jint downloadTaskId, jstring _url, jint result, jstring _resultMessage, jstring _mimeType, jint size, jint sizeDownloaded)
{
    CRJNIEnv env(_env);
	DocViewNative * native = getNative(_env, _this);
	lString8 url = UnicodeToUtf8(env.fromJavaString(_url));
	lString8 resultMessage = UnicodeToUtf8(env.fromJavaString(_resultMessage));
	lString8 mimeType = UnicodeToUtf8(env.fromJavaString(_mimeType));
	COFFEE_TRY_JNI(_env, native->onDownloadProgress(downloadTaskId, url, result, resultMessage, mimeType, size, sizeDownloaded));
}


//============================================================================================================
// register JNI methods

static JNINativeMethod sCRViewMethods[] =
{
    /* name, signature, funcPtr */
	{"initInternal", "(Lorg/coolreader/newui/CRConfig;)Z", (void*)Java_org_coolreader_newui_CRView_initInternal},
	{"uninitInternal", "()Z",                              (void*)Java_org_coolreader_newui_CRView_uninitInternal},
	{"isLink", "(Ljava/lang/String;)Ljava/lang/String;",   (void*)Java_org_coolreader_newui_CRView_isLink},
	{"callCRRunnableInternal", "(J)V",                     (void*)Java_org_coolreader_newui_CRView_callCRRunnableInternal},
	{"drawInternal", "()V",                                (void*)Java_org_coolreader_newui_CRView_drawInternal},
	{"onPauseInternal", "()V",                             (void*)Java_org_coolreader_newui_CRView_onPauseInternal},
	{"onResumeInternal", "()V",                            (void*)Java_org_coolreader_newui_CRView_onResumeInternal},
	{"surfaceChangedInternal", "(II)V",                    (void*)Java_org_coolreader_newui_CRView_surfaceChangedInternal},
	{"surfaceDestroyedInternal", "()V",                    (void*)Java_org_coolreader_newui_CRView_surfaceDestroyedInternal},
	{"handleKeyEventInternal", "(Landroid/view/KeyEvent;)Z", (void*)Java_org_coolreader_newui_CRView_handleKeyEventInternal},
	{"handleTouchEventInternal", "(Landroid/view/MotionEvent;)Z", (void*)Java_org_coolreader_newui_CRView_handleTouchEventInternal},
	{"loadBookInternal", "(Ljava/lang/String;)V", (void*)Java_org_coolreader_newui_CRView_loadBookInternal},
	{"setBatteryLevelInternal", "(I)V", (void*)Java_org_coolreader_newui_CRView_setBatteryLevelInternal},
	{"onDownloadResult", "(ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;I[B)V", (void*)Java_org_coolreader_newui_CRView_onDownloadResult},
	{"onDownloadProgress", "(ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;II)V", (void*)Java_org_coolreader_newui_CRView_onDownloadProgress},
	{"onSentenceFinishedInternal", "()V", (void*)Java_org_coolreader_newui_CRView_onSentenceFinishedInternal},
	{"onTtsInitializedInternal", "()V", (void*)Java_org_coolreader_newui_CRView_onTtsInitializedInternal}
};

/*
 * Register native JNI-callable methods.
 *
 * "className" looks like "java/lang/String".
 */
static int jniRegisterNativeMethods(JNIEnv* env, const char* className,
    const JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    LOGV("Registering %s natives\n", className);
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'\n", className);
        return -1;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'\n", className);
        return -1;
    }
    return 0;
}


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
   JNIEnv* env = NULL;
   jint res = -1;
 
#ifdef JNI_VERSION_1_6
    if (res==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOGI("JNI_OnLoad: JNI_VERSION_1_6\n");
   	    res = JNI_VERSION_1_6;
    }
#endif
#ifdef JNI_VERSION_1_4
    if (res==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_4) == JNI_OK) {
        LOGI("JNI_OnLoad: JNI_VERSION_1_4\n");
   	    res = JNI_VERSION_1_4;
    }
#endif
#ifdef JNI_VERSION_1_2
    if (res==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_2) == JNI_OK) {
        LOGI("JNI_OnLoad: JNI_VERSION_1_2\n");
   	    res = JNI_VERSION_1_2;
    }
#endif
	if ( res==-1 )
		return res;
 
    jniRegisterNativeMethods(env, "org/coolreader/newui/CRView", sCRViewMethods, sizeof(sCRViewMethods)/sizeof(JNINativeMethod));
    LOGI("JNI_OnLoad: native methods are registered!\n");
    return res;
}
