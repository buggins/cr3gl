#ifndef _GLRENDERERTEMPLATE_H_
#define _GLRENDERERTEMPLATE_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FUi.h>
#include <FUiIme.h>
#include <FGraphics.h>
#include <gl.h>
#include <FGrpIGlRenderer.h>
#include <FGrpGlPlayer.h>
#include "crui.h"
#include "cr3tizen.h"
#include "cruimain.h"
#include "gldrawbuf.h"


class LVDocView;
class CoolReaderApp;
class CoolReaderFrame;
class CR3Renderer
	: public Tizen::Graphics::Opengl::IGlRenderer
	, public Tizen::Ui::ITextEventListener
	, public CRUIScreenUpdateManagerCallback
	, public CRUIPlatform
{
	CoolReaderApp * _app;
	CoolReaderFrame * _frame;
	LVDocView * _docview;
	CRUIMainWidget * _widget;
	CRUIEventManager * _eventManager;
	CRUIEventAdapter * _eventAdapter;
	GLDrawBuf * _backbuffer;
	Tizen::Ui::Controls::Keypad * _keypad;
	bool _keypadShown;
	//bool _updateRequested;
public:

	CR3Renderer(CoolReaderApp * app, CoolReaderFrame * frame);
	~CR3Renderer(void);

	CRUIEventAdapter * getEventAdapter() { return _eventAdapter; }

	virtual bool InitializeGl(void);
	virtual bool TerminateGl(void);

	virtual bool Draw(void);

	virtual bool Pause(void);
	virtual bool Resume(void);

	virtual int GetTargetControlWidth(void);
	virtual int GetTargetControlHeight(void);
	virtual void SetTargetControlWidth(int width);
	virtual void SetTargetControlHeight(int height);

    void setPlayer(Tizen::Graphics::Opengl::GlPlayer * player) {
    	__player = player;
    }

    /// set animation fps (0 to disable) and/or update screen instantly
    virtual void setScreenUpdateMode(bool updateNow, int animationFps);

    // CRUIPlatform implementation
    /// CRUIScreenUpdateManagerCallback implementation - exit application
	virtual void exitApp();
	/// minimize app or show Home Screen
	virtual void minimizeApp();

    // copy text to clipboard
    virtual void copyToClipboard(lString16 text);

    /// return true if platform supports native virtual keyboard
    virtual bool supportsVirtualKeyboard();
    /// return true if platform native virtual keyboard is shown
    virtual bool isVirtualKeyboardShown();
    /// show platform native virtual keyboard
    virtual void showVirtualKeyboard(int mode, lString16 text, bool multiline);
    /// hide platform native virtual keyboard
    virtual void hideVirtualKeyboard();

    // ITextEventListener
    virtual void OnTextValueChanged(const Tizen::Ui::Control& source);
    virtual void OnTextValueChangeCanceled(const Tizen::Ui::Control& source);

private:
	int __controlWidth;
	int __controlHeight;
	int __angle;
	Tizen::Graphics::Opengl::GlPlayer * __player;
	bool __playerStarted;
};

#endif /* _GLRENDERERTEMPLATE_H_ */
