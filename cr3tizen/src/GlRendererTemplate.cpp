#include "GlRendererTemplate.h"

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

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	SetPerspective(60.0f, 1.0f * GetTargetControlWidth() / GetTargetControlHeight(), 1.0f, 400.0f);

	glClearColor(0, 0, 0, 1);
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
	// TODO:
	// Draw a scene and perform what to be necessary for each scene.

	static const GLfloat vertices[] =
	{
		ZERO, ONEP, ZERO,
		ONEN, ONEN, ZERO,
		ONEP, ONEN, ZERO,
		ZERO, ONEP, ZERO,
		ONEP, ONEN, ZERO,
		ONEN, ONEN, ZERO
	};

	static const GLfloat vertexColor[] =
	{
		ONEP, ZERO, ZERO, ONEP,
		ZERO, ONEP, ZERO, ONEP,
		ZERO, ZERO, ONEP, ONEP,
		ONEP, ZERO, ZERO, ONEP,
		ZERO, ZERO, ONEP, ONEP,
		ZERO, ONEP, ZERO, ONEP
	};

	static const unsigned short indexBuffer[] =
	{
		0, 1, 2, 3, 4, 5
	};

	__angle = (__angle + 10) % (360 * 3);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, vertexColor);

	glMatrixMode(GL_MODELVIEW);
	{
		glLoadIdentity();
		glTranslatef(0, 0, GLfloat(-5.0f));
		glRotatef(GLfloat(__angle), 0, GLfloat(1.0f), 0);
	}

	glDrawElements(GL_TRIANGLES, 3*2, GL_UNSIGNED_SHORT, &indexBuffer[0]);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

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
