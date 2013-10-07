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
#include <crui.h>
#include "cr3tizen.h"
#include "cruimain.h"
#include "gldrawbuf.h"


class LVDocView;
class CoolReaderApp;
class CR3Renderer :
	public Tizen::Graphics::Opengl::IGlRenderer,
	public CRUIScreenUpdateManagerCallback,
	public CRUIPlatform
{
		CoolReaderApp * _app;
		LVDocView * _docview;
	    CRUIMainWidget * _widget;
		CRUIEventManager * _eventManager;
		CRUIEventAdapter * _eventAdapter;
		GLDrawBuf * _backbuffer;
		bool _updateRequested;
public:

		CR3Renderer(CoolReaderApp * app);
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

    /// set animation fps (0 to disable) and/or update screen instantly
    virtual void setScreenUpdateMode(bool updateNow, int animationFps);
    /// CRUIScreenUpdateManagerCallback implementation - exit application
	virtual void exitApp();

    void setPlayer(Tizen::Graphics::Opengl::GlPlayer * player) {
    	__player = player;
    }
private:
	int __controlWidth;
	int __controlHeight;
	int __angle;
	Tizen::Graphics::Opengl::GlPlayer * __player;
	bool __playerStarted;
};

#endif /* _GLRENDERERTEMPLATE_H_ */
