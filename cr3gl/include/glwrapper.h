#ifndef GLWRAPPER_H
#define GLWRAPPER_H

#if QT_GL

#ifdef QT_NO_OPENGL
#error "no opengl"
#endif


//#include <QtGui/qopengl.h>
//#include <QtGui/qopenglext.h>
//#include <QtGui/qopenglfunctions.h>
//#include <QtGui/QOpenGLFunctions>
//#include <QtOpenGL/QtOpenGLDepends>
//#include <QOpenGLFunctions>
//#include <QtOpenGL/QtOpenGL>
//#include <QGLFunctions>
#include <QtGui/QOpenGLFunctions>
#include <QtOpenGLExtensions/QOpenGLExtensions>

//#include <GL/gl.h>

//extern QOpenGLFunctions * _qtgl;
//#include <QtGui/QOpenGLFunctions_ES2>
//#include <QtGui/qopengles2ext.h>
//#include <QtGui/qopenglext.h>
//#include <QtGui/qopengl.h>
//#include <QtOpenGL/QtOpenGL>
//#include <QtOpenGLExtensions/QOpenGLExtensions>

#define glActiveTexture glActiveTexture
#define glGenFramebuffersOES glGenFramebuffers
#define glBindFramebufferOES glBindFramebuffer
#define glFramebufferTexture2DOES glFramebufferTexture2D
#define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER
#define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0
#define glCheckFramebufferStatusOES glCheckFramebufferStatus
#define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE
#define glOrthof glOrthofOES
#define glDeleteFramebuffersOES glDeleteFramebuffers
#define glActiveTexture glActiveTexture

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


#endif // GLWRAPPER_H
