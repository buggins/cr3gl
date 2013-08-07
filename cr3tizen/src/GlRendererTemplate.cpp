#include "GlRendererTemplate.h"
#include "gldrawbuf.h"
#include "glfont.h"
#include <crengine.h>

const GLfloat ONEP = GLfloat(+1.0f);
const GLfloat ONEN = GLfloat(-1.0f);
const GLfloat ZERO = GLfloat( 0.0f);

void SetPerspective(GLfloat fovDegree, GLfloat aspect, GLfloat zNear,  GLfloat zFar)
{
	// tan(double(degree) * 3.1415962 / 180.0 / 2.0);
	static const float HALF_TAN_TABLE[91] =
	{
		0.00000f, 0.00873f, 0.01746f, 0.02619f, 0.03492f, 0.04366f, 0.05241f, 0.06116f, 0.06993f,
		0.07870f, 0.08749f, 0.09629f, 0.10510f, 0.11394f, 0.12278f, 0.13165f, 0.14054f, 0.14945f,
		0.15838f, 0.16734f, 0.17633f, 0.18534f, 0.19438f, 0.20345f, 0.21256f, 0.22169f, 0.23087f,
		0.24008f, 0.24933f, 0.25862f, 0.26795f, 0.27732f, 0.28675f, 0.29621f, 0.30573f, 0.31530f,
		0.32492f, 0.33460f, 0.34433f, 0.35412f, 0.36397f, 0.37389f, 0.38386f, 0.39391f, 0.40403f,
		0.41421f, 0.42448f, 0.43481f, 0.44523f, 0.45573f, 0.46631f, 0.47698f, 0.48773f, 0.49858f,
		0.50953f, 0.52057f, 0.53171f, 0.54296f, 0.55431f, 0.56577f, 0.57735f, 0.58905f, 0.60086f,
		0.61280f, 0.62487f, 0.63707f, 0.64941f, 0.66189f, 0.67451f, 0.68728f, 0.70021f, 0.71329f,
		0.72654f, 0.73996f, 0.75356f, 0.76733f, 0.78129f, 0.79544f, 0.80979f, 0.82434f, 0.83910f,
		0.85408f, 0.86929f, 0.88473f, 0.90041f, 0.91633f, 0.93252f, 0.94897f, 0.96569f, 0.98270f,
		1.00000f
	};

	int degree = int(fovDegree + 0.5f);

	degree = (degree >=  0) ? degree :  0;
	degree = (degree <= 90) ? degree : 90;

	GLfloat fxdYMax  = GLfloat(zNear * HALF_TAN_TABLE[degree]);
	GLfloat fxdYMin  = -fxdYMax;

	GLfloat fxdXMax  = GLfloat(GLfloat(fxdYMax) * aspect);
	GLfloat fxdXMin  = -fxdXMax;

	glFrustumf(fxdXMin, fxdXMax, fxdYMin, fxdYMax, GLfloat(zNear), GLfloat(zFar));
}

GlRendererTemplate::GlRendererTemplate(void)
	: __controlWidth(0)
	, __controlHeight(0)
	, __angle(0)
{
	_docview = new LVDocView(32);
	_docview->Resize(300, 400);
	_docview->createDefaultDocument(lString16(L"Test document"), lString16(L"Just testing if GL rendering is working ok"));
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

	GLDrawBuf pagebuf(300, 400, 32, true);
	pagebuf.beforeDrawing();
	_docview->Draw(pagebuf, false);
	pagebuf.afterDrawing();


	LVFontRef font = fontMan->GetFont(24, 400, false, css_ff_sans_serif, lString8("Tizen Sans"), 0);
	GLDrawBuf backbuf(300, 400, 32, true);
	backbuf.beforeDrawing();
	backbuf.SetBackgroundColor(0x000000);
	backbuf.Clear(0x4040C0);
	backbuf.FillRect(10, 10, 200, 200, 0x0055aa55);
	backbuf.FillRect(100, 120, 250, 300, 0x80aa55aa);
	backbuf.FillRect(0, 0, 100, 100, 0x0055ff55);
	backbuf.FillRect(50, 50, 270, 200, 0x0080FF00);
	backbuf.FillRect(0, 0, 500, 10, 0x00ffffff);
	backbuf.SetTextColor(0xFFFF00);
	font->DrawTextString(&backbuf, 5, 5,
            L"Hello Tizen - testing text", 26,
            '?');
	backbuf.SetTextColor(0x0080FF);
	font->DrawTextString(&backbuf, 45, 45,
            L"Hello Tizen - testing text", 26,
            '?');
	font = fontMan->GetFont(32, 400, true, css_ff_sans_serif, lString8("Tizen Sans"), 0);
	backbuf.SetTextColor(0xFF0000);
	font->DrawTextString(&backbuf, 45, 145,
            L"Red text", 8,
            '?');
	backbuf.SetTextColor(0x00FF00);
	font->DrawTextString(&backbuf, 45, 185,
            L"Green text", 10,
            '?');
	backbuf.SetTextColor(0x0000FF);
	font->DrawTextString(&backbuf, 45, 225,
            L"Blue text", 9,
            '?');
	backbuf.afterDrawing();

//	glClearColor(0.5f, 0.5f, 0.7f, 1);
//	glClear(GL_COLOR_BUFFER_BIT);

	GLDrawBuf buf(GetTargetControlWidth(), GetTargetControlHeight(), 16, false);
	buf.beforeDrawing();


	buf.FillRect(100, 50, 300, 500, 0x0055aa55);
	buf.FillRect(200, 100, 400, 700, 0x80aa55aa);
	buf.FillRect(0, 0, 500, 10, 0x000000FF);
	pagebuf.DrawTo(&buf, 10, 10, 0, NULL);
	backbuf.DrawTo(&buf, 150, 70, 0, NULL);
	buf.DrawRescaled(&backbuf, 30, 250, 50, 50, 0);
	buf.FillRect(0, buf.GetHeight() - 1, buf.GetWidth(), buf.GetHeight(), 0x0000FFFF);
	buf.FillRect(buf.GetWidth() - 1, 0, buf.GetWidth(), buf.GetHeight(), 0x0000FF00);


	buf.afterDrawing();
	glFlush();
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
