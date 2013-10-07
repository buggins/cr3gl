/**
 * Name        : CoolReader
 * Version     :
 * Vendor      :
 * Description :
 */


#include "CoolReader.h"
#include "CoolReaderFrame.h"
#include "CR3Renderer.h"

#include "glfont.h"
#include "cr3tizen.h"
#include "lvstring.h"
#include "cruiconfig.h"

using namespace Tizen::App;
using namespace Tizen::Base;
using namespace Tizen::System;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;

CoolReaderApp::CoolReaderApp(void)
{
	LVInitCoolReaderTizen(GetAppResourcePath().GetPointer(), GetAppDataPath().GetPointer());
}

CoolReaderApp::~CoolReaderApp(void)
{
}

UiApp*
CoolReaderApp::CreateInstance(void)
{
	// Create the instance through the constructor.
	return new CoolReaderApp();
}

bool
CoolReaderApp::OnAppInitializing(AppRegistry& appRegistry)
{
	// TODO:
	// Initialize Frame and App specific data.
	// The App's permanent data and context can be obtained from the appRegistry.
	//
	// If this method is successful, return true; otherwise, return false.
	// If this method returns false, the App will be terminated.

	// Uncomment the following statement to listen to the screen on/off events.
	//PowerManager::SetScreenEventListener(*this);

	// TODO:
	// Add your initialization code here
	return true;
}

bool
CoolReaderApp::OnAppInitialized(void)
{
	// TODO:
        // Add code to do after initialization here. 

	// Create a Frame
	CoolReaderFrame* pCoolReaderFrame = new CoolReaderFrame();
	pCoolReaderFrame->Construct();
	pCoolReaderFrame->SetName(L"CoolReader");
	AddFrame(*pCoolReaderFrame);

	//pCoolReaderFrame->AddKeyEventListener(*this);

    crconfig.setupResourcesForScreenSize();
    //crconfig.loadTheme(lString8());

	{
		__player = new Tizen::Graphics::Opengl::GlPlayer;
		//__player->Construct(Tizen::Graphics::Opengl::EGL_CONTEXT_CLIENT_VERSION_1_X, pCoolReaderFrame->GetCurrentForm());
		__player->Construct(Tizen::Graphics::Opengl::EGL_CONTEXT_CLIENT_VERSION_1_X, pCoolReaderFrame);

		__player->SetFps(5);
		__player->SetEglAttributePreset(Tizen::Graphics::Opengl::EGL_ATTRIBUTES_PRESET_RGB565);

		__player->Start();
	}

	__renderer = new CR3Renderer(this);
	__player->SetIGlRenderer(__renderer);
//	if (pCoolReaderFrame->GetCurrentForm())
//		pCoolReaderFrame->GetCurrentForm()->AddTouchEventListener(*__renderer->getEventAdapter());
	pCoolReaderFrame->setRenderer(__renderer);
	__renderer->setPlayer(__player);

	return true;
}

bool
CoolReaderApp::OnAppWillTerminate(void)
{
	// TODO:
        // Add code to do somethiing before application termination. 
	return true;
}


bool
CoolReaderApp::OnAppTerminating(AppRegistry& appRegistry, bool forcedTermination)
{
	// TODO:
	// Deallocate resources allocated by this App for termination.
	// The App's permanent data and context can be saved via appRegistry.

	__player->Stop();

	if(__renderer != null)
	{
		delete __renderer;
	}
	delete __player;

	return true;
}

void
CoolReaderApp::OnForeground(void)
{
	// TODO:
	// Start or resume drawing when the application is moved to the foreground.
}

void
CoolReaderApp::OnBackground(void)
{
	// TODO:
	// Stop drawing when the application is moved to the background.
}

void
CoolReaderApp::OnLowMemory(void)
{
	// TODO:
	// Free unused resources or close the application.
}

void
CoolReaderApp::OnBatteryLevelChanged(BatteryLevel batteryLevel)
{
	// TODO:
	// Handle any changes in battery level here.
	// Stop using multimedia features(camera, mp3 etc.) if the battery level is CRITICAL.
}

void
CoolReaderApp::OnScreenOn(void)
{
	// TODO:
	// Get the released resources or resume the operations that were paused or stopped in OnScreenOff().
}

void
CoolReaderApp::OnScreenOff(void)
{
	// TODO:
	// Unless there is a strong reason to do otherwise, release resources (such as 3D, media, and sensors) to allow the device
	// to enter the sleep mode to save the battery.
	// Invoking a lengthy asynchronous method within this listener method can be risky, because it is not guaranteed to invoke a
	// callback before the device enters the sleep mode.
	// Similarly, do not perform lengthy operations in this listener method. Any operation must be a quick one.
}

void
CoolReaderApp::OnKeyPressed(const Control& source, KeyCode keyCode)
{
	// TODO:
}

void
CoolReaderApp::OnKeyReleased(const Control& source, KeyCode keyCode)
{
}

void
CoolReaderApp::OnKeyLongPressed(const Control& source, KeyCode keyCode)
{
	// TODO:
}
