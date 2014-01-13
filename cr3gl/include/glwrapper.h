#ifndef GLWRAPPER_H
#define GLWRAPPER_H

#if QT_GL
//#include <QtGui/qopengl.h>
//#include <QtGui/qopenglext.h>
//#include <QtGui/qopenglfunctions.h>
#include <QtGui/QOpenGLFunctions>
extern QOpenGLFunctions * _qtgl;
//#include <QtGui/QOpenGLFunctions_ES2>
//#include <QtGui/qopengles2ext.h>
//#include <QtGui/qopenglext.h>
//#include <QtGui/qopengl.h>
//#include <QtOpenGL/QtOpenGL>
//#include <QtOpenGLExtensions/QOpenGLExtensions>
#define glActiveTexture _qtgl->glActiveTexture
#define glGenFramebuffersOES _qtgl->glGenFramebuffers
#define glBindFramebufferOES _qtgl->glBindFramebuffer
#define glFramebufferTexture2DOES _qtgl->glFramebufferTexture2D
#define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER
#define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0
#define glCheckFramebufferStatusOES _qtgl->glCheckFramebufferStatus
#define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE
#define glOrthof _qtgl->glOrthofOES
#define glDeleteFramebuffersOES _qtgl->glDeleteFramebuffers
#define glActiveTexture _qtgl->glActiveTexture

#else
//#ifdef _WIN32
//#include <GL/glew.h>
//#include <GL/wglew.h>
//#define glGenFramebuffersOES glGenFramebuffers
//#define glBindFramebufferOES glBindFramebuffer
//#define glFramebufferTexture2DOES glFramebufferTexture2D
//#define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER
//#define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0
//#define glCheckFramebufferStatusOES glCheckFramebufferStatus
//#define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE
//#define glOrthof glOrthofOES
//#define glDeleteFramebuffersOES glDeleteFramebuffers
//#else
#if defined(ANDROID)
#include <GLES/gl.h>
#include <GLES/glext.h>
//#include <EGL/egl.h>
//#define glGenFramebuffersOES glGenFramebuffers
//#define glBindFramebufferOES glBindFramebuffer
//#define glFramebufferTexture2DOES glFramebufferTexture2D
//#define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER
//#define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0
//#define glCheckFramebufferStatusOES glCheckFramebufferStatus
//#define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE

#else
#include <gl.h>
#include <glext.h>
#endif
//#endif
#endif

class CRGLSupport
#if QT_GL
    : protected QOpenGLFunctions
#endif
{
#ifdef QT_OPENGL_ES_2
    static QGLShaderProgram *program_texture;
    static QGLShaderProgram *program_solid;
#endif
public:
    CRGLSupport();

    void drawSolidFillRect(GLfloat * matrix, GLfloat vertices[], GLfloat colors[]);
    void drawColorAndTextureRect(GLfloat * matrix, GLfloat vertices[], GLfloat txcoords[], GLfloat colors[], GLint textureId);

    static void init();
    static void uninit();
};

#endif // GLWRAPPER_H
