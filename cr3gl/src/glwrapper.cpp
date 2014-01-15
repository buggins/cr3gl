#include "glwrapper.h"
#include <lvstring.h>


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
class CRGLSupportImpl : public CRGLSupport
#if QT_GL
    , protected QOpenGLFunctions
#endif
{
#ifdef QT_OPENGL_ES_2
    static QGLShaderProgram *program_texture;
    static QGLShaderProgram *program_solid;
#endif
    void init();
    void uninit();
public:
    CRGLSupportImpl();
    ~CRGLSupportImpl();

    void drawSolidFillRect(float * matrix, float vertices[], float color);
    void drawSolidFillRect(float * matrix, float vertices[], float colors[]);
    void drawColorAndTextureRect(float * matrixPtr, float vertices[], float texcoords[], float color, lUInt32 textureId);
    void drawColorAndTextureRect(float * matrix, float vertices[], float txcoords[], float colors[], lUInt32 textureId);

    lUInt32 genTexture();
    bool isTexture(lUInt32 textureId);
    void deleteTexture(lUInt32 textureId);
    /// set texture image in RGBA format, returns false if failed
    bool setTextureImage(lUInt32 textureId, int dx, int dy, unsigned char * pixels);
    /// sets texture image as ALPHA only, returns false if failed
    bool setTextureImageAlpha(lUInt32 textureId, int dx, int dy, unsigned char * pixels);
    /// returns texture ID for buffer, 0 if failed
    bool createFramebuffer(lUInt32 &textureId, lUInt32 &framebufferId, int dx, int dy);

    void deleteFramebuffer(lUInt32 &textureId, lUInt32 &framebufferId);

    bool bindFramebuffer(lUInt32 framebufferId);
    void setOrthoProjection(int dx, int dy);
    void setRotation(int x, int y, int rotationAngle);
    void flush();

    static CRGLSupport * instance();
};

static CRGLSupport * _crGLSupportInstance = NULL;
CRGLSupport * CRGLSupport::instance() {
    if (_crGLSupportInstance)
        return _crGLSupportInstance;
    _crGLSupportInstance = new CRGLSupportImpl();
    return _crGLSupportInstance;
}




CRGLSupportImpl::CRGLSupportImpl() {
    init();
}

CRGLSupportImpl::~CRGLSupportImpl() {
    uninit();
}

static bool _checkError(const char *srcfile, int line, const char * context) {
    int err = glGetError();
    if (err != GL_NO_ERROR) {
        CRLog::error("CRGLSupport : GL Error %04x at %s : %s : %d", err, context, srcfile, line);
        return true;
    }
    return false;
}
#define checkError(context) _checkError(__FILE__, __LINE__, context)



#ifdef QT_OPENGL_ES_2
#include <QtOpenGL/QGLShaderProgram>
QGLShaderProgram *CRGLSupportImpl::program_texture = NULL;
QGLShaderProgram *CRGLSupportImpl::program_solid = NULL;
#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_COLOR_ATTRIBUTE 1
#define PROGRAM_TEXCOORD_ATTRIBUTE 2
#endif

void CRGLSupportImpl::drawSolidFillRect(float * matrixPtr, float vertices[], float color) {
    float colors[6*4];
    LVGLFillColor(color, colors, 6);
    drawSolidFillRect(matrixPtr, vertices, colors);
}

void CRGLSupportImpl::drawSolidFillRect(float * matrixPtr, float vertices[], float colors[]) {
#ifdef QT_OPENGL_ES_2
    QMatrix4x4 matrix(matrixPtr);
    program_solid->setUniformValue("matrix", m);
#else
    CR_UNUSED(matrixPtr);
    glEnable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    checkError("glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)");
    glEnableClientState(GL_VERTEX_ARRAY);
    checkError("glEnableClientState(GL_VERTEX_ARRAY)");
    glEnableClientState(GL_COLOR_ARRAY);
    checkError("glEnableClientState(GL_COLOR_ARRAY)");
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    checkError("glVertexPointer(3, GL_FLOAT, 0, vertices)");
    glColorPointer(4, GL_FLOAT, 0, colors);
    checkError("glColorPointer(4, GL_FLOAT, 0, colors)");

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkError("glDrawArrays(GL_TRIANGLES, 0, 6)");

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
#endif
}

void CRGLSupportImpl::drawColorAndTextureRect(float * matrixPtr, float vertices[], float texcoords[], float color, lUInt32 textureId) {
    float colors[6*4];
    LVGLFillColor(color, colors, 6);
    drawColorAndTextureRect(matrixPtr, vertices, texcoords, colors, textureId);
}

void CRGLSupportImpl::drawColorAndTextureRect(float * matrixPtr, float vertices[], float texcoords[], float colors[], lUInt32 textureId) {
#ifdef QT_OPENGL_ES_2
    QMatrix4x4 matrix(matrixPtr);
    program_solid->setUniformValue("matrix", m);
#else
    CR_UNUSED(matrixPtr);
    glEnable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //LVGLSetColor(color);
    //glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    //glDisable(GL_LIGHTING);
    glActiveTexture(GL_TEXTURE0);
    checkError("glActiveTexture");
    glEnable(GL_TEXTURE_2D);
    checkError("glEnable(GL_TEXTURE_2D)");
    glBindTexture(GL_TEXTURE_2D, textureId);
    checkError("glBindTexture");

    glEnable(GL_BLEND);
    //GL_SRC_ALPHA
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    checkError("glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)");
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glEnableClientState(GL_COLOR_ARRAY);
    checkError("glEnableClientState(GL_COLOR_ARRAY)");
    glEnableClientState(GL_VERTEX_ARRAY);
    checkError("glEnableClientState(GL_VERTEX_ARRAY)");
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    checkError("glEnableClientState(GL_TEXTURE_COORD_ARRAY)");
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    checkError("glVertexPointer(3, GL_FLOAT, 0, vertices)");
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    checkError("glTexCoordPointer(2, GL_FLOAT, 0, texcoords)");
    glColorPointer(4, GL_FLOAT, 0, colors);
    checkError("glColorPointer(4, GL_FLOAT, 0, colors)");

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkError("glDrawArrays");

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_TEXTURE_2D);
#endif
}

void CRGLSupportImpl::init() {

#if QT_GL
    CRLog::trace("CRGLSupportImpl::init() -- calling initializeOpenGLFunctions()");
    initializeOpenGLFunctions();
    Q_ASSERT(QOpenGLFunctions::isInitialized(d_ptr));
#endif

#if QT_GL
#ifdef QT_OPENGL_ES_2


    // texture + color
    {
        QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
        const char *vsrc =
            "attribute highp vec4 vertex;\n"
            "attribute lowp vec4 colAttr;\n"
            "attribute mediump vec4 texCoord;\n"
            "varying lowp vec4 col;\n"
            "varying mediump vec4 texc;\n"
            "uniform mediump mat4 matrix;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = matrix * vertex;\n"
            "    col = colAttr;\n"
            "    texc = texCoord;\n"
            "}\n";
        vshader->compileSourceCode(vsrc);

        QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);
        const char *fsrc =
            "uniform sampler2D texture;\n"
            "varying lowp vec4 col;\n"
            "varying mediump vec4 texc;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = texture2D(texture, texc.st) * col;\n"
            "}\n";
        fshader->compileSourceCode(fsrc);

        program_texture = new QGLShaderProgram(this);
        program_texture->addShader(vshader);
        program_texture->addShader(fshader);
        program_texture->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
        program_texture->bindAttributeLocation("colAttr", PROGRAM_COLOR_ATTRIBUTE);
        program_texture->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
        program_texture->link();

        program_texture->bind();
        program_texture->setUniformValue("texture", 0);
    }

    // solid
    {
        QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
        const char *vsrc =
            "attribute highp vec4 vertex;\n"
            "attribute lowp vec4 colAttr;\n"
            "varying lowp vec4 col;\n"
            "uniform mediump mat4 matrix;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = matrix * vertex;\n"
            "    col = colAttr;\n"
            "}\n";
        vshader->compileSourceCode(vsrc);

        QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);
        const char *fsrc =
            "uniform sampler2D texture;\n"
            "varying lowp vec4 col;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = col;\n"
            "}\n";
        fshader->compileSourceCode(fsrc);

        program_texture = new QGLShaderProgram(this);
        program_texture->addShader(vshader);
        program_texture->addShader(fshader);
        program_texture->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
        program_texture->bindAttributeLocation("colAttr", PROGRAM_COLOR_ATTRIBUTE);
        program_texture->link();

        program_texture->bind();
        program_texture->setUniformValue("texture", 0);
    }

#endif
#endif
}

void CRGLSupportImpl::uninit() {
#if QT_GL
#ifdef QT_OPENGL_ES_2
    if (program_texture) {
        delete program_texture;
        program_texture = NULL;
    }
    if (program_solid) {
        delete program_solid;
        program_solid = NULL;
    }
#endif
#endif
}

bool CRGLSupportImpl::isTexture(lUInt32 textureId) {
    return glIsTexture(textureId) == GL_TRUE;
}

lUInt32 CRGLSupportImpl::genTexture() {
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    if (checkError("glGenTextures")) return 0;
    return textureId;
}

void CRGLSupportImpl::deleteTexture(lUInt32 textureId) {
    if (!textureId)
        return;
    if (glIsTexture(textureId) != GL_TRUE) {
        CRLog::error("Invalid texture %d", textureId);
        return;
    }
    GLuint id = textureId;
    glDeleteTextures(1, &id);
    checkError("~GLImageCachePage - glDeleteTextures");
}

bool CRGLSupportImpl::setTextureImage(lUInt32 textureId, int dx, int dy, lUInt8 * pixels) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    checkError("updateTexture - glBindTexture");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    checkError("updateTexture - glPixelStorei");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    checkError("updateTexture - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkError("updateTexture - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    checkError("updateTexture - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkError("updateTexture - glTexParameteri");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dx, dy, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    checkError("updateTexture - glTexImage2D");
    if (glGetError() != GL_NO_ERROR) {
        CRLog::error("Cannot set image for texture");
        return false;
    }
    return true;
}

bool CRGLSupportImpl::setTextureImageAlpha(lUInt32 textureId, int dx, int dy, lUInt8 * pixels) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    checkError("updateTexture - glBindTexture");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    checkError("updateTexture - glPixelStorei");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    checkError("updateTexture - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkError("updateTexture - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    checkError("updateTexture - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkError("updateTexture - glTexParameteri");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, dx, dy, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
    checkError("updateTexture - glTexImage2D");
    if (glGetError() != GL_NO_ERROR) {
        CRLog::error("Cannot set image for texture");
        return false;
    }
    return true;
}

/// returns texture ID for buffer, 0 if failed
bool CRGLSupportImpl::createFramebuffer(lUInt32 &textureId, lUInt32 &framebufferId, int dx, int dy) {
    bool res = true;
    textureId = framebufferId = 0;
    textureId = genTexture();
    if (!textureId)
        return false;
    GLuint fid = 0;
    glGenFramebuffersOES(1, &fid);
    if (checkError("createFramebuffer glGenFramebuffersOES")) return false;
    framebufferId = fid;
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebufferId);
    if (checkError("createFramebuffer glBindFramebuffer")) return false;

    glBindTexture(GL_TEXTURE_2D, textureId);
    checkError("glBindTexture(GL_TEXTURE_2D, _textureId)");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dx, dy, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    checkError("glTexImage2D");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    checkError("texParameter");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkError("texParameter");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    checkError("texParameter");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkError("texParameter");

    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, textureId, 0);
    checkError("glFramebufferTexture2DOES");
    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        CRLog::error("glFramebufferTexture2DOES failed");
        res = false;
    }
    checkError("glCheckFramebufferStatusOES");
    //glClearColor(0.5f, 0, 0, 1);
    glClearColor(0, 0, 0, 0);
    checkError("glClearColor");
    glClear(GL_COLOR_BUFFER_BIT);
    checkError("glClear");
    return res;
}

void CRGLSupportImpl::deleteFramebuffer(lUInt32 &textureId, lUInt32 &framebufferId) {
    //CRLog::debug("GLDrawBuf::deleteFramebuffer");
    if (textureId != 0) {
        deleteTexture(textureId);
    }
    if (framebufferId != 0) {
        glBindFramebufferOES( GL_FRAMEBUFFER_OES, 0);
        checkError("deleteFramebuffer - glBindFramebufferOES");
        GLuint fid = framebufferId;
        glDeleteFramebuffersOES(1, &fid);
        checkError("deleteFramebuffer - glDeleteFramebuffer");
    }
    textureId = 0;
    framebufferId = 0;
}

bool CRGLSupportImpl::bindFramebuffer(lUInt32 framebufferId) {
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebufferId);
    return !checkError("beforeDrawing glBindFramebufferOES");
}

void CRGLSupportImpl::flush() {
    glFlush();
    checkError("glFlush");
}

void myGlOrtho(float left, float right, float bottom, float top,
                                         float zNearPlane, float zFarPlane)
{
    float m[16];

    float r_l = 1.0f / (right - left);
    float t_b = 1.0f / (top - bottom);
    float f_n = 1.0f / (zFarPlane - zNearPlane);

    memset(m, 0, sizeof(m));
    m[0] = 2.0f * r_l;
    m[5] = 2.0f * t_b;
    m[10] = -2.0f * f_n;
    m[12] = (-(right + left)) * r_l;
    m[13] = (-(top + bottom)) * t_b;
    m[14] = (-(zFarPlane + zNearPlane)) * f_n;
    m[15] = 1.0f;
    glLoadMatrixf(m);
}

void CRGLSupportImpl::setOrthoProjection(int dx, int dy) {
    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    checkError("glPushMatrix");
    glLoadIdentity();
    myGlOrtho(0, dx, 0, dy, -1.0f, 5.0f);
    //glOrthof(0, _dx, 0, _dy, -1.0f, 1.0f);
    glViewport(0,0,dx,dy);
    checkError("glViewport");
    glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    checkError("glPushMatrix");
    glLoadIdentity();
}

void CRGLSupportImpl::setRotation(int x, int y, int rotationAngle) {
    if (!rotationAngle)
        return;
    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    //checkError("push matrix");
    checkError("matrix mode");
    glTranslatef(x, y, 0);
    glRotatef(rotationAngle, 0, 0, 1);
    glTranslatef(-x, -y, 0);
}

// utility function to fill 4-float array of vertex colors with converted CR 32bit color
void LVGLFillColor(lUInt32 color, float * buf, int count) {
    float r = ((color >> 16) & 255) / 255.0f;
    float g = ((color >> 8) & 255) / 255.0f;
    float b = ((color >> 0) & 255) / 255.0f;
    float a = (((color >> 24) & 255) ^ 255) / 255.0f;
    for (int i=0; i<count; i++) {
        *buf++ = r;
        *buf++ = g;
        *buf++ = b;
        *buf++ = a;
    }
}

