#include "GlRendererTemplate.h"
#include "gldrawbuf.h"
#include "glfont.h"
#include <crengine.h>
#include "crui.h"
#include "fileinfo.h"
#include "cruimain.h"
#include "cruiwidget.h"

using namespace CRUI;

const GLfloat ONEP = GLfloat(+1.0f);
const GLfloat ONEN = GLfloat(-1.0f);
const GLfloat ZERO = GLfloat( 0.0f);

GlRendererTemplate::GlRendererTemplate(void)
	: _backbuffer(NULL)
	, _updateRequested(false)
	, __controlWidth(0)
	, __controlHeight(0)
	, __angle(0)
	, __player(NULL)
	, __playerStarted(true)
{
	_eventManager = new CRUIEventManager();
	_eventAdapter = new CRUIEventAdapter(_eventManager);
	_widget = new CRUIMainWidget();
	_eventManager->setRootWidget(_widget);
	_widget->setScreenUpdater(this);
}

GlRendererTemplate::~GlRendererTemplate(void)
{

}

bool
GlRendererTemplate::InitializeGl(void)
{
	// TODO:
	// Initialize GL status. 

	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, GetTargetControlWidth(), GetTargetControlHeight());
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, GetTargetControlWidth(), GetTargetControlHeight(), 0, -1.0f, 1.0f);
	glClearColor(1, 1, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	return true;
}

bool
GlRendererTemplate::TerminateGl(void)
{
	// TODO:
	// Terminate and reset GL status. 
	return true;
}

bool
GlRendererTemplate::Draw(void)
{
	CRLog::debug("GlRendererTemplate::Draw is called");

	_updateRequested = false;

	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, GetTargetControlWidth(), GetTargetControlHeight());
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, GetTargetControlWidth(), GetTargetControlHeight(), 0, -1.0f, 1.0f);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!_backbuffer) {
		_backbuffer = new GLDrawBuf(GetTargetControlWidth(), GetTargetControlHeight(), 32, true);
	} else if (_backbuffer->GetWidth() != GetTargetControlWidth() || _backbuffer->GetHeight() != GetTargetControlHeight()) {
		delete _backbuffer;
		_backbuffer = new GLDrawBuf(GetTargetControlWidth(), GetTargetControlHeight(), 32, true);
	}

	_backbuffer->beforeDrawing();

	bool needLayout, needDraw, animating;
	CRUICheckUpdateOptions(_widget, needLayout, needDraw, animating);
	_widget->invalidate();
	if (needLayout) {
		//CRLog::trace("need layout");
		_widget->measure(__controlWidth, __controlHeight);
		_widget->layout(0, 0, _widget->getMeasuredWidth(), _widget->getMeasuredHeight());
	}
	if (needDraw) {
		//CRLog::trace("need draw");
		_widget->draw(_backbuffer);
	}
	_backbuffer->afterDrawing();

	GLDrawBuf buf(GetTargetControlWidth(), GetTargetControlHeight(), 32, false);
	buf.beforeDrawing();
	_backbuffer->DrawTo(&buf, 0, 0, 0, NULL);
	buf.afterDrawing();

	glFlush();


	//CRLog::debug("GlRendererTemplate::Draw exiting");
	return true;
}

bool
GlRendererTemplate::Pause(void)
{
	// TODO:
	// Do something necessary when Plyaer is paused. 

	return true;
}

bool
GlRendererTemplate::Resume(void)
{
	// TODO:
	// Do something necessary when Plyaer is resumed. 

	return true;
}

int
GlRendererTemplate::GetTargetControlWidth(void)
{
	// TODO:
	// Return target control width

	return __controlWidth;
}

int
GlRendererTemplate::GetTargetControlHeight(void)
{
	// TODO:
	// Return target control height

	return __controlHeight;
}

void
GlRendererTemplate::SetTargetControlWidth(int width)
{
	// TODO:
	// Assign target control width

	__controlWidth = width;
}

void
GlRendererTemplate::SetTargetControlHeight(int height)
{
	// TODO:
	// Assign target control height

	__controlHeight = height;
}

/// set animation fps (0 to disable) and/or update screen instantly
void GlRendererTemplate::setScreenUpdateMode(bool updateNow, int animationFps) {
	//CRLog::trace("setScreenUpdateMode(%s, %d fps)", (updateNow ? "update now" : "no update"), animationFps);
	if (!animationFps) {
		if (updateNow) {
			//CRLog::trace("Updating player");
			if (!_updateRequested) {
				_updateRequested = true;
				__player->Redraw();
			}
		}
		if (__playerStarted) {
			//CRLog::trace("Pausing player");
			__player->Pause();
			__playerStarted = false;
		}
	} else {
		if (!__playerStarted) {
			//CRLog::trace("Resuming player");
			__player->SetFps(animationFps);
			__player->Resume();
			__playerStarted = true;
		}
	}
}
