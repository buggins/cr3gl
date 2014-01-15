#ifndef GLWRAPPER_H
#define GLWRAPPER_H

#include <lvtypes.h>

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

QT_FORWARD_DECLARE_CLASS(QGLShaderProgram);

class CRGLSupport
#if QT_GL
    : protected QOpenGLFunctions
#endif
{
#ifdef QT_OPENGL_ES_2
    static QGLShaderProgram *program_texture;
    static QGLShaderProgram *program_solid;
#endif
    void init();
    void uninit();
public:
    CRGLSupport();
    ~CRGLSupport();

    void drawSolidFillRect(GLfloat * matrix, GLfloat vertices[], GLfloat color);
    void drawSolidFillRect(GLfloat * matrix, GLfloat vertices[], GLfloat colors[]);
    void drawColorAndTextureRect(GLfloat * matrixPtr, GLfloat vertices[], GLfloat texcoords[], GLfloat color, GLint textureId);
    void drawColorAndTextureRect(GLfloat * matrix, GLfloat vertices[], GLfloat txcoords[], GLfloat colors[], GLint textureId);

    GLuint genTexture();
    void deleteTexture(GLuint textureId);
    /// set texture image in RGBA format, returns false if failed
    bool setTextureImage(GLuint textureId, int dx, int dy, unsigned char * pixels);
    /// sets texture image as ALPHA only, returns false if failed
    bool setTextureImageAlpha(GLuint textureId, int dx, int dy, unsigned char * pixels);
    /// returns texture ID for buffer, 0 if failed
    bool createFramebuffer(GLuint &textureId, GLuint &framebufferId, int dx, int dy);

    void deleteFramebuffer(GLuint &textureId, GLuint &framebufferId);

    bool bindFramebuffer(GLuint framebufferId);
    void setOrthoProjection(int dx, int dy);
    void flush();

    static CRGLSupport * instance();
};

void LVGLFillColor(lUInt32 color, float * buf, int count);

#define CRGL CRGLSupport::instance()

#endif // GLWRAPPER_H
