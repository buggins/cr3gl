/*
 * cr3tizen.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#include "tizenx.h"
#include "cr3tizen.h"
#include "lvstring.h"
#include "glfont.h"
#include "gldrawbuf.h"
#include "crui.h"
#include "cr3db.h"
#include "fileinfo.h"
#include <lvhashtable.h>
#include "cruiconfig.h"
#include "crconcurrent.h"
#include <sys/time.h>
#include <FBaseColArrayList.h>
#include "CoolReaderFrame.h"

using namespace CRUI;

class CRTizenLogger : public CRLog
{
protected:
    virtual void log( const char * level, const char * msg, va_list args )
    {
    	char buf[4096];
    	vsnprintf(buf, 4095, msg, args);
        if (!strcmp("ERROR", level))
        	AppLogExceptionInternal("", 0, "%s", buf);
        else if (!strcmp("INFO", level))
        	AppLogInternal("", 0, "%s", buf);
        else
        	AppLogDebugInternal("", 0, "%s", buf);
    }
public:
    CRTizenLogger()
    {
        static const char * log_level_names[] = {
        "FATAL",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "TRACE",
        };
        info( "Started logging. Level=%s", log_level_names[getLogLevel()] );
    }

    virtual ~CRTizenLogger() {
    }
};

void LVSetTizenLogger() {
	CRLog::setLogger(new CRTizenLogger());
}

class TizenConcurrencyProvider : public CRConcurrencyProvider {
    CRExecutor * guiExecutor;
public:

    class TizenMutex : public CRMutex {
    	Tizen::Base::Runtime::Mutex mutex;
    public:
        TizenMutex() {
        	result r = mutex.Create(Tizen::Base::Runtime::NonRecursiveMutexTag());
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Mutex::create failed");
        	}
        }
        virtual void acquire() {
        	result r = mutex.Acquire();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Mutex::acquire failed");
        	}
        }
        virtual void release() {
        	result r = mutex.Release();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Mutex::release failed");
        	}
        }
    };

    class TizenMonitor : public CRMonitor {
    	Tizen::Base::Runtime::Monitor monitor;
    public:
        TizenMonitor() {
        	result r;
        	r = monitor.Construct();
        	//CRLog::trace("TizenMonitor construct result %d", (int)r);
        }
        virtual void acquire() {
        	result r = monitor.Enter();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Monitor::acquire failed");
        	}
        }
        virtual void release() {
        	//CRLog::trace("TizenMonitor release");
        	result r = monitor.Exit();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Monitor::release failed");
        	}
        }
        virtual void wait() {
        	//CRLog::trace("TizenMonitor before wait");
        	result r = monitor.Wait();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Monitor::wait failed");
        	}
        }
        virtual void notify() {
        	//CRLog::trace("TizenMonitor notify");
        	result r = monitor.Notify();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Monitor::notify failed");
        	}
        }
        virtual void notifyAll() {
        	//CRLog::trace("TizenMonitor notifyAll");
        	result r = monitor.NotifyAll();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Monitor::notifyAll failed");
        	}
        }
    };

    class TizenThread : public CRThread, public Tizen::Base::Runtime::Thread {
    	CRRunnable * runnable;
    public:
        TizenThread(CRRunnable * _runnable) : runnable(_runnable) {
        	result r = Construct();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Thread::create failed");
        	}
        }
        virtual ~TizenThread() {
            result r = Join();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Thread::join failed");
        	}
        }
        virtual void start() {
            result r = Start();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Thread::start failed");
        	}
        }
        virtual void join() {
            result r = Join();
        	if (r != E_SUCCESS) {
        		CRLog::fatal("Thread::join failed");
        	}
        }
        Object * Run(void) {
    		CRLog::debug("Tizen - thread started");
        	runnable->run();
    		CRLog::debug("Tizen - thread finished");
        	return NULL;
        }
    };

public:
    virtual CRMutex * createMutex() {
		//CRLog::debug("Tizen - create mutex");
        return new TizenMutex();
    }

    virtual CRMonitor * createMonitor() {
		//CRLog::debug("Tizen - create monitor");
        return new TizenMonitor();
    }

    virtual CRThread * createThread(CRRunnable * threadTask) {
		//CRLog::debug("Tizen - create thread");
        return new TizenThread(threadTask);
    }
    virtual void executeGui(CRRunnable * task) {
		//CRLog::debug("Tizen - execute GUI");
        guiExecutor->execute(task);
    }

    TizenConcurrencyProvider(CRExecutor * _guiExecutor) {
        guiExecutor = _guiExecutor;
    }
    /// sleep current thread
    virtual void sleepMs(int durationMs) {
    	Tizen::Base::Runtime::Thread::Sleep(durationMs);
    }

    virtual ~TizenConcurrencyProvider() {}
};


//class MyEvent : public Event
//{
//	CRRunnable * _runnable;
//protected:
//    virtual void FireImpl(IEventListener& listener, const IEventArg& arg);
//public:
//    MyEvent(CRRunnable * runnable) : _runnable(runnable) {}
//};
//
//void
//MyEvent::FireImpl(IEventListener& listener, const IEventArg& arg)
//{
//	_runnable->run();
//	delete _runnable;
//}

using namespace Tizen::Base::Collection;

class TizenGuiExecutor : public CRExecutor {
public:
	TizenGuiExecutor() {
	}

	virtual void execute(CRRunnable * runnable) {
		//CRLog::debug("TizenGuiExecutor execute task");
		   // ArrayList parameters put on the String object
		   ArrayList* pList = new ArrayList();
		   pList->Construct();
		   //String* pData = new String(L"Inter thread communication");
		   pList->Add(new CRRunnableContainer(runnable));

		   // Send messages using the SendUserEvent() function
		   Tizen::Ui::Controls::Frame* pFrame = Tizen::App::UiApp::GetInstance()->GetAppFrame()->GetFrame();
		   //CoolReaderForm * form = dynamic_cast<CoolReaderForm *>(pFrame->GetCurrentForm());
		   if (pFrame)
			   pFrame->SendUserEvent(UI_UPDATE_REQUEST, pList);
		   //CRLog::debug("TizenGuiExecutor execute task - done");
	}
};

void LVInitCoolReaderTizen(const wchar_t * resourceDir, const wchar_t * dbDir) {
	LVSetTizenLogger();
	CRLog::info("Starting CoolReader");
	CRLog::setLogLevel(CRLog::LL_TRACE);

	Tizen::Graphics::Dimension phys = Tizen::Graphics::CoordinateSystem::GetPhysicalResolution();
	Tizen::Graphics::Dimension logical = Tizen::Graphics::CoordinateSystem::GetLogicalResolution();
	// support
	int dpi = 316;
	if (phys.width <= 480)
		dpi = 207;
	CRLog::info("Logical resolution: %dx%d  physical resolution %dx%d using dpi=%d", phys.width, phys.height, logical.width, logical.height, dpi);
	deviceInfo.setScreenDimensions(phys.width, phys.height, dpi);

	concurrencyProvider = new TizenConcurrencyProvider(new TizenGuiExecutor());

//	CRLog::trace("testing concurrency provider");
//	CRThreadExecutor * executor = new CRThreadExecutor();
//	class SampleRunnable : public CRRunnable {
//	public:
//		virtual void run() {
//			CRLog::trace("inside runnable");
//		}
//	};
//	executor->execute(new SampleRunnable());
//	executor->execute(new SampleRunnable());
//	executor->execute(new SampleRunnable());
//	CRMonitor * monitor = concurrencyProvider->createMonitor();
//	monitor->acquire();
//	monitor->wait();
	Tizen::Base::String language;
	Tizen::System::SettingInfo::GetValue("http://tizen.org/setting/locale.language", language);
	crconfig.systemLanguage = UnicodeToUtf8(language.GetPointer());
	CRLog::info("System language: %s", crconfig.systemLanguage.c_str());
	if (crconfig.systemLanguage.length() > 2)
		crconfig.systemLanguage = crconfig.systemLanguage.substr(0, 2);

	crconfig.fontFiles.add("/usr/share/fonts/TizenSansRegular.ttf");
	crconfig.fontFiles.add("/usr/share/fonts/TizenSansMeduim.ttf");
	crconfig.fontFiles.add("/usr/share/fallback_fonts/TizenSansFallback.ttf");
	//fontMan->SetFallbackFontFace(lString8("Tizen Sans Fallback"));

    crconfig.setupUserDir(UnicodeToUtf8(dbDir));
    crconfig.setupResources(UnicodeToUtf8(resourceDir));

    deviceInfo.topDirs.addItem(DIR_TYPE_INTERNAL_STORAGE, lString8("/mnt/ums"));
    deviceInfo.topDirs.addItem(DIR_TYPE_DOWNLOADS, lString8("/mnt/ums/Downloads"));
    deviceInfo.topDirs.addItem(DIR_TYPE_DEFAULT_BOOKS_DIR, lString8("/mnt/ums/Books"));

    crconfig.initEngine(false); // don't set logger
	CRLog::info("Engine initialization done");
}



CRUIEventAdapter::CRUIEventAdapter(CRUIEventManager * eventManager) : _eventManager(eventManager), _keyModifiers(0), _keyModifiersRight(0)
{

}

using namespace Tizen::Ui;

int CRUIEventAdapter::findPointer(lUInt64 id) {
	for (int i=0; i<_activePointers.length(); i++)
		if (_activePointers[i]->getPointerId() == id)
			return i;
	return -1;
}

//lUInt64 GetCurrentTimeMillis() {
//#if defined(LINUX) || defined(ANDROID) || defined(_LINUX)
//	timeval ts;
//	gettimeofday(&ts, NULL);
//	return ts.tv_sec * (lUInt64)1000 + ts.tv_usec / 1000;
//#else
//	#error * You should define GetCurrentTimeMillis() *
//#endif
//}

void CRUIEventAdapter::dispatchTouchEvent(const Tizen::Ui::TouchEventInfo &touchInfo)
{
	int x = touchInfo.GetCurrentPosition().x;
	int y = touchInfo.GetCurrentPosition().y;
	unsigned long pointId = touchInfo.GetPointId();
//	int startX = touchInfo.GetStartPosition().x;
//	int starty = touchInfo.GetStartPosition().y;
	int status = touchInfo.GetTouchStatus();
	int action = 0;
	//CRLog::trace("CRUIEventAdapter::dispatchTouchEvent(pointer=%d  pos=(%d,%d) status=%d ", pointId, x, y, status);

	switch (status) {
	case TOUCH_PRESSED: //The touch pressed event type
		action = ACTION_DOWN; break;
	case TOUCH_LONG_PRESSED: //The touch long pressed event type
		//action = ACTION_DOWN; ignore
		break;
	case TOUCH_RELEASED: //The touch released event type
		action = ACTION_UP; break;
	case TOUCH_MOVED: //The touch moved event type
		action = ACTION_MOVE; break;
	case TOUCH_DOUBLE_PRESSED: //The touch double pressed event type
		//action = ACTION_DOWN; // ignore
		break;
	case TOUCH_FOCUS_IN: //The touch focus-in event type
		action = ACTION_FOCUS_IN; break;
	case TOUCH_FOCUS_OUT: //The touch focus-out event type
		action = ACTION_FOCUS_OUT; break;
	case TOUCH_CANCELED: //The touch canceled event type
		action = ACTION_CANCEL; break;
	}
	if (action) {
		int index = findPointer(pointId);
		CRUIMotionEventItem * lastItem = index >= 0 ? _activePointers[index] : NULL;
		bool isLast = (action == ACTION_CANCEL || action == ACTION_UP);
		bool isFirst = (action == ACTION_DOWN);
		if (!lastItem && !isFirst) {
			CRLog::warn("Ignoring unexpected touch event %d with id%lld", action, pointId);
			return;
		}
		lUInt64 ts = GetCurrentTimeMillis();
		CRUIMotionEventItem * item = new CRUIMotionEventItem(lastItem, pointId, action, x, y, ts);
		if (index >= 0) {
			if (!isLast)
				_activePointers.set(index, item);
			else
				_activePointers.remove(index);
		} else {
			if (!isLast)
				_activePointers.add(item);
		}
		CRUIMotionEvent * event = new CRUIMotionEvent();
		event->addEvent(item);
		for (int i=0; i<_activePointers.length(); i++) {
			if (_activePointers[i] != item)
				event->addEvent(_activePointers[i]);
		}
		_eventManager->dispatchTouchEvent(event);
		delete event;
		for (int i = _activePointers.length() - 1; i >= 0; i--) {
			if (_activePointers[i]->getAction() == CRUI::ACTION_UP || _activePointers[i]->getAction() == CRUI::ACTION_CANCEL || _activePointers[i]->isCancelled())
				delete _activePointers.remove(i);
		}
	}
}

static bool checkModifier(int & modifiers, int flag, KEY_EVENT_TYPE action, Tizen::Ui::KeyCode keyCode, Tizen::Ui::KeyCode expectedKeyCode) {
	if (keyCode == expectedKeyCode) {
		if (action == KEY_ACTION_PRESS)
			modifiers |= flag;
		else if (action == KEY_ACTION_RELEASE)
			modifiers = modifiers & (~flag);
		return true;
	}
	return false;
}

static int translateKey(Tizen::Ui::KeyCode keyCode) {
	switch (keyCode) {
	case KEY_LEFT_SHIFT:
	case KEY_RIGHT_SHIFT:
		return CR_KEY_SHIFT;
	case KEY_LEFT_ALT:
	case KEY_RIGHT_ALT:
		return CR_KEY_ALT;
	case KEY_LEFT_CTRL:
	case KEY_RIGHT_CTRL:
		return CR_KEY_CONTROL;
	case KEY_ESC:
		return CR_KEY_ESC;
	case KEY_MENU:
		return CR_KEY_MENU;
	case KEY_BACK:
		return CR_KEY_BACK;
	case KEY_SPACE:
		return CR_KEY_SPACE;
	case KEY_ENTER:
		return CR_KEY_RETURN;
	case KEY_DELETE:
		return CR_KEY_DELETE;
	case KEY_BACKSPACE:
		return CR_KEY_BACKSPACE;
	case KEY_SIDE_UP:
		return CR_KEY_VOLUME_UP;
	case KEY_SIDE_DOWN:
		return CR_KEY_VOLUME_DOWN;
	case KEY_0:
	case KEY_1:
	case KEY_2:
	case KEY_3:
	case KEY_4:
	case KEY_5:
	case KEY_6:
	case KEY_7:
	case KEY_8:
	case KEY_9:
		return CR_KEY_0 + (keyCode - KEY_0);
	case KEY_A:
	case KEY_B:
	case KEY_C:
	case KEY_D:
	case KEY_E:
	case KEY_F:
	case KEY_G:
	case KEY_H:
	case KEY_I:
	case KEY_J:
	case KEY_K:
	case KEY_L:
	case KEY_M:
	case KEY_N:
	case KEY_O:
	case KEY_P:
	case KEY_Q:
	case KEY_R:
	case KEY_S:
	case KEY_T:
	case KEY_U:
	case KEY_V:
	case KEY_W:
	case KEY_X:
	case KEY_Y:
	case KEY_Z:
		return CR_KEY_A + (keyCode - KEY_A);
	default:
		return 0;
	}
}

bool CRUIEventAdapter::dispatchKeyEvent(KEY_EVENT_TYPE action, Tizen::Ui::KeyCode keyCode) {
	int oldModifiers = _keyModifiers | _keyModifiersRight;
	bool isModifier = false;
	isModifier = checkModifier(_keyModifiers, CR_KEY_MODIFIER_SHIFT, action, keyCode, KEY_LEFT_SHIFT) || isModifier;
	isModifier = checkModifier(_keyModifiers, CR_KEY_MODIFIER_ALT, action, keyCode, KEY_LEFT_ALT) || isModifier;
	isModifier = checkModifier(_keyModifiers, CR_KEY_MODIFIER_CONTROL, action, keyCode, KEY_LEFT_CTRL) || isModifier;
	isModifier = checkModifier(_keyModifiersRight, CR_KEY_MODIFIER_SHIFT, action, keyCode, KEY_RIGHT_SHIFT) || isModifier;
	isModifier = checkModifier(_keyModifiersRight, CR_KEY_MODIFIER_ALT, action, keyCode, KEY_RIGHT_ALT) || isModifier;
	isModifier = checkModifier(_keyModifiersRight, CR_KEY_MODIFIER_CONTROL, action, keyCode, KEY_RIGHT_CTRL) || isModifier;
	int newModifiers = _keyModifiers | _keyModifiersRight;
	if (isModifier && oldModifiers == newModifiers)
		return true; // ignore
	int crKey = translateKey(keyCode);
	if (crKey) {
		CRUIKeyEvent * event = new CRUIKeyEvent(action, crKey, false, 0, newModifiers);
		return _eventManager->dispatchKeyEvent(event);
	}
	CRLog::warn("Unknown Tizen keycode: %d (0x%04x)", keyCode, keyCode);
	return false;
}

void  CRUIEventAdapter::OnTouchCanceled (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchFocusIn (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchFocusOut (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchMoved (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchPressed (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnTouchReleased (const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	dispatchTouchEvent(touchInfo);
}

void  CRUIEventAdapter::OnKeyLongPressed (const Tizen::Ui::Control &source, Tizen::Ui::KeyCode keyCode)
{
	// ignore

}

void  CRUIEventAdapter::OnKeyPressed (const Tizen::Ui::Control &source, Tizen::Ui::KeyCode keyCode)
{
	dispatchKeyEvent(KEY_ACTION_PRESS, keyCode);
}

void  CRUIEventAdapter::OnKeyReleased (const Tizen::Ui::Control &source, Tizen::Ui::KeyCode keyCode)
{
	dispatchKeyEvent(KEY_ACTION_RELEASE, keyCode);
}
