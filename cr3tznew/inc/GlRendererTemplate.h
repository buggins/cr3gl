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
#include <crui.h>
#include "cr3tizen.h"
#include "cruimain.h"

class LVDocView;
class GlRendererTemplate :
	public Tizen::Graphics::Opengl::IGlRenderer,
	public CRUIScreenUpdateManagerCallback
{
		LVDocView * _docview;
	    CRUIMainWidget * _widget;
		CRUIEventManager * _eventManager;
		CRUIEventAdapter * _eventAdapter;
public:

	GlRendererTemplate(void);
	~GlRendererTemplate(void);

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

private:
	int __controlWidth;
	int __controlHeight;
	int __angle;
};

#endif /* _GLRENDERERTEMPLATE_H_ */
