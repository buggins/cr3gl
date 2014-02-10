#include "glwrapper.h"
#include <lvstring.h>


#if QT_GL

#ifdef QT_NO_OPENGL
#error "no opengl"
#endif


#include <QtGui/QOpenGLFunctions>
#include <QtOpenGLExtensions/QOpenGLExtensions>


//#define glActiveTexture glActiveTexture
#define glGenFramebuffersOES glGenFramebuffers
#define glBindFramebufferOES glBindFramebuffer
#define glFramebufferTexture2DOES glFramebufferTexture2D
#define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER
#define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0
#define glCheckFramebufferStatusOES glCheckFramebufferStatus
#define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE
//#define glOrthof glOrthofOES
#define glDeleteFramebuffersOES glDeleteFramebuffers
//#define glActiveTexture glActiveTexture

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


#ifdef QT_OPENGL_ES_2
QT_FORWARD_DECLARE_CLASS(QGLShaderProgram);
#endif

class CRGLSupportImpl :
#if QT_GL
        public QObject,
#endif
        public CRGLSupport
#if QT_GL
    , protected QOpenGLFunctions
#endif
{
#ifdef QT_OPENGL_ES_2
    QGLShaderProgram *program_texture;
    QGLShaderProgram *program_solid;
#endif
    float m[4 * 4];
    lUInt32 currentFramebufferId;
    int bufferDx;
    int bufferDy;
    int maxTextureSize;
    int rotationX;
    int rotationY;
    int rotationAngle;
    void init();
    void uninit();
    void myGlOrtho(float left, float right, float bottom, float top,
                                             float zNearPlane, float zFarPlane);
protected:
    //void drawColorAndTextureRect(float vertices[], float texcoords[], lUInt32 color, lUInt32 textureId);
    void drawColorAndTextureRect(float vertices[], float txcoords[], float colors[], lUInt32 textureId, bool linear);
    void drawSolidFillRect(float vertices[], lUInt32 color);
    void drawSolidFillRect(float vertices[], float colors[]);
public:
    CRGLSupportImpl();
    ~CRGLSupportImpl();

    void drawSolidFillRect(lvRect & rc, lUInt32 color1, lUInt32 color2, lUInt32 color3, lUInt32 color4);
    virtual void drawColorAndTextureRect(lUInt32 textureId, int tdx, int tdy, int srcx, int srcy, int srcdx, int srcdy, int xx, int yy, int dx, int dy, lUInt32 color, bool linear);
    virtual void drawColorAndTextureRect(lUInt32 textureId, int tdx, int tdy, lvRect & srcrc, lvRect & dstrc, lUInt32 color, bool linear);

    virtual int getMaxTextureSize();

    lUInt32 genTexture();
    bool isTexture(lUInt32 textureId);
    void deleteTexture(lUInt32 & textureId);
    /// set texture image in RGBA format, returns false if failed
    bool setTextureImage(lUInt32 textureId, int dx, int dy, unsigned char * pixels);
    /// sets texture image as ALPHA only, returns false if failed
    bool setTextureImageAlpha(lUInt32 textureId, int dx, int dy, unsigned char * pixels);
    /// returns texture ID for buffer, 0 if failed
    bool createFramebuffer(lUInt32 &textureId, lUInt32 &framebufferId, int dx, int dy);

    void deleteFramebuffer(lUInt32 &framebufferId);

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




CRGLSupportImpl::CRGLSupportImpl() : currentFramebufferId(0), bufferDx(0), bufferDy(0), maxTextureSize(0),
    rotationX(0), rotationY(0), rotationAngle(0)
{
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

static const char * vsrc_TEXTURE =
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

const char *fsrc_TEXTURE =
    "uniform sampler2D texture;\n"
    "varying lowp vec4 col;\n"
    "varying mediump vec4 texc;\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = texture2D(texture, texc.st) * col;\n"
    "}\n";

const char *vsrc_SOLID =
    "attribute highp vec4 vertex;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform mediump mat4 matrix;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = matrix * vertex;\n"
    "    col = colAttr;\n"
    "}\n";

const char *fsrc_SOLID =
    "uniform sampler2D texture;\n"
    "varying lowp vec4 col;\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = col;\n"
    "}\n";

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_COLOR_ATTRIBUTE 1
#define PROGRAM_TEXCOORD_ATTRIBUTE 2
#define PROGRAM_VERTEX_ATTRIBUTE_SOLID 0
#define PROGRAM_COLOR_ATTRIBUTE_SOLID 1
#endif

#define Z_2D -1.0f
void CRGLSupportImpl::drawSolidFillRect(lvRect & rc, lUInt32 color1, lUInt32 color2, lUInt32 color3, lUInt32 color4) {
    float colors[6 * 4];
    LVGLFillColor(color1, colors + 4*0, 1);
    LVGLFillColor(color4, colors + 4*1, 1);
    LVGLFillColor(color3, colors + 4*2, 1);
    LVGLFillColor(color1, colors + 4*3, 1);
    LVGLFillColor(color3, colors + 4*4, 1);
    LVGLFillColor(color2, colors + 4*5, 1);
    float x0 = (float)(rc.left);
    float y0 = (float)(bufferDy-rc.top);
    float x1 = (float)(rc.right);
    float y1 = (float)(bufferDy-rc.bottom);

    // don't flip for framebuffer
    if (currentFramebufferId) {
        y0 = (float)(rc.top);
        y1 = (float)(rc.bottom);
    }

    float vertices[] = {
            x0,y0,Z_2D,
            x0,y1,Z_2D,
            x1,y1,Z_2D,
            x0,y0,Z_2D,
            x1,y1,Z_2D,
            x1,y0,Z_2D};
    drawSolidFillRect(vertices, colors);
}

void CRGLSupportImpl::drawSolidFillRect(float vertices[], lUInt32 color) {
    float colors[6*4];
    LVGLFillColor(color, colors, 6);
    drawSolidFillRect(vertices, colors);
}

void CRGLSupportImpl::drawSolidFillRect(float vertices[], float colors[]) {
//    CRLog::trace("CRGLSupportImpl::drawSolidFillRect(fb=%d\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f)"
//            , currentFramebufferId
//            , vertices[0], vertices[1], vertices[2]
//            , vertices[3], vertices[4], vertices[5]
//            , vertices[6], vertices[7], vertices[8]
//            , vertices[9], vertices[10], vertices[11]
//            , vertices[12], vertices[13], vertices[14]
//            , vertices[15], vertices[16], vertices[17]
//            , colors[0], colors[1], colors[2], colors[3]
//            , colors[4], colors[5], colors[6], colors[7]
//            , colors[8], colors[9], colors[10], colors[11]
//            , colors[12], colors[13], colors[14], colors[15]
//            , colors[16], colors[17], colors[18], colors[19]
//            , colors[20], colors[21], colors[22], colors[23]
//            );
    checkError("before CRGLSupportImpl::drawSolidFillRect");
#ifdef QT_OPENGL_ES_2
    QMatrix4x4 matrix(m);
    if (!program_solid->bind())
        CRLog::error("error while binding texture program");
    glDisable(GL_CULL_FACE);
    //glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program_solid->setUniformValue("matrix", matrix);
    program_solid->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE_SOLID);
    program_solid->enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE_SOLID);
    program_solid->setAttributeArray
        (PROGRAM_VERTEX_ATTRIBUTE_SOLID, vertices, 3);
    program_solid->setAttributeArray
        (PROGRAM_COLOR_ATTRIBUTE_SOLID, colors, 4);
//    program_solid->setAttributeArray
//        (PROGRAM_TEXCOORD_ATTRIBUTE, texcoords);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkError("glDrawArrays");
    program_solid->disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE_SOLID);
    program_solid->disableAttributeArray(PROGRAM_COLOR_ATTRIBUTE_SOLID);
    program_solid->release();
#else
    glColor4f(1,1,1,1);
    glDisable(GL_CULL_FACE);
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
    checkError("after CRGLSupportImpl::drawSolidFillRect");
}

//void CRGLSupportImpl::drawColorAndTextureRect(float vertices[], float texcoords[], lUInt32 color, lUInt32 textureId) {
//    float colors[6*4];
//    LVGLFillColor(color, colors, 6);
//    drawColorAndTextureRect(vertices, texcoords, colors, textureId);
//}

void CRGLSupportImpl::drawColorAndTextureRect(lUInt32 textureId, int tdx, int tdy, lvRect & srcrc, lvRect & dstrc, lUInt32 color, bool linear) {
    drawColorAndTextureRect(textureId, tdx, tdy, srcrc.left, srcrc.top, srcrc.width(), srcrc.height(), dstrc.left, dstrc.top, dstrc.width(), dstrc.height(), color, linear);
}

void CRGLSupportImpl::drawColorAndTextureRect(lUInt32 textureId, int tdx, int tdy, int srcx, int srcy, int srcdx, int srcdy, int xx, int yy, int dx, int dy, lUInt32 color, bool linear) {
    float colors[6*4];
    LVGLFillColor(color, colors, 6);
    float dstx0 = (float)xx;
    float dsty0 = (float)(bufferDy - (yy));
    float dstx1 = (float)(xx + dx);
    float dsty1 = (float)(bufferDy - (yy + dy));

    // don't flip for framebuffer
    if (currentFramebufferId) {
        dsty0 = (float)((yy));
        dsty1 = (float)((yy + dy));
    }

    float srcx0 = srcx / (float)tdx;
    float srcy0 = srcy / (float)tdy;
    float srcx1 = (srcx + srcdx) / (float)tdx;
    float srcy1 = (srcy + srcdy) / (float)tdy;
    float vertices[] = {dstx0,dsty0,Z_2D,
                        dstx0,dsty1,Z_2D,
                        dstx1,dsty1,Z_2D,
                        dstx0,dsty0,Z_2D,
                        dstx1,dsty1,Z_2D,
                        dstx1,dsty0,Z_2D};
    float texcoords[] = {srcx0,srcy0, srcx0,srcy1, srcx1,srcy1, srcx0,srcy0, srcx1,srcy1, srcx1,srcy0};
    drawColorAndTextureRect(vertices, texcoords, colors, textureId, linear);
//    x, _dy - y - dy, x + dx, _dy - y,
//    srcx / (float)glbuf->_tdx,
//    srcy / (float)glbuf->_tdy,
//    (srcx + srcdx) / (float)glbuf->_tdx,
//    (srcy + srcdy) / (float)glbuf->_tdy,

}

void CRGLSupportImpl::drawColorAndTextureRect(float vertices[], float texcoords[], float colors[], lUInt32 textureId, bool linear) {
//    CRLog::trace("CRGLSupportImpl::drawColorAndTextureRect(fb=%d texture=%08x\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,%f,\n\t%f,%f,\n\t%f,%f,\n\t%f,%f,\n\t%f,%f,\n\t%f,%f,\n\t%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f\n\t%f,%f,%f,%f)"
//            , currentFramebufferId
//            , textureId
//            , vertices[0], vertices[1], vertices[2]
//            , vertices[3], vertices[4], vertices[5]
//            , vertices[6], vertices[7], vertices[8]
//            , vertices[9], vertices[10], vertices[11]
//            , vertices[12], vertices[13], vertices[14]
//            , vertices[15], vertices[16], vertices[17]
//            , texcoords[0], texcoords[1]
//            , texcoords[2], texcoords[3]
//            , texcoords[4], texcoords[5]
//            , texcoords[6], texcoords[7]
//            , texcoords[8], texcoords[9]
//            , texcoords[10], texcoords[11]
//            , colors[0], colors[1], colors[2], colors[3]
//            , colors[4], colors[5], colors[6], colors[7]
//            , colors[8], colors[9], colors[10], colors[11]
//            , colors[12], colors[13], colors[14], colors[15]
//            , colors[16], colors[17], colors[18], colors[19]
//            , colors[20], colors[21], colors[22], colors[23]
//            );
    checkError("before CRGLSupportImpl::drawColorAndTextureRect");

    if (!glIsTexture(textureId)) {
        CRLog::error("invalid texture passed to CRGLSupportImpl::drawColorAndTextureRect");
    }
    //LVGLSetColor(color);
    //glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    //glDisable(GL_LIGHTING);

#ifdef QT_OPENGL_ES_2

    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    checkError("glEnable(GL_BLEND)");
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    checkError("glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)");

//    int maxt = 0;
//    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxt);
//    CRLog::trace("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS=%d, GL_TEXTURE0=%d", maxt, GL_TEXTURE0);

    glActiveTexture(GL_TEXTURE0);
    checkError("glActiveTexture GL_TEXTURE0");
    glBindTexture(GL_TEXTURE_2D, textureId);
    checkError("glBindTexture");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    checkError("drawColorAndTextureRect - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    checkError("drawColorAndTextureRect - glTexParameteri");


    QMatrix4x4 matrix(m);
    if (!program_texture->bind())
        CRLog::error("error while binding texture program");
    program_texture->setUniformValue("matrix", matrix);
    program_texture->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    program_texture->enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
    program_texture->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    program_texture->setAttributeArray
        (PROGRAM_VERTEX_ATTRIBUTE, vertices, 3);
    program_texture->setAttributeArray
        (PROGRAM_COLOR_ATTRIBUTE, colors, 4);
    program_texture->setAttributeArray
        (PROGRAM_TEXCOORD_ATTRIBUTE, texcoords, 2);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkError("glDrawArrays");
    program_texture->disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    program_texture->disableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
    program_texture->disableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    program_texture->release();

    //glFlush();
    //checkError("glFlush");
    glBindTexture(GL_TEXTURE_2D, 0);
    checkError("glBindTexture");
#else

    glDisable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    checkError("glActiveTexture");
    glEnable(GL_TEXTURE_2D);
    checkError("glEnable(GL_TEXTURE_2D)");
    glBindTexture(GL_TEXTURE_2D, textureId);
    checkError("glBindTexture");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    checkError("drawColorAndTextureRect - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    checkError("drawColorAndTextureRect - glTexParameteri");

    glColor4f(1,1,1,1);
    glDisable(GL_ALPHA_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    checkError("glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)");

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
    checkError("after CRGLSupportImpl::drawColorAndTextureRect");
}

void CRGLSupportImpl::init() {

#if QT_GL
    CRLog::trace("CRGLSupportImpl::init() -- calling initializeOpenGLFunctions()");
    initializeOpenGLFunctions();
#endif

#if QT_GL
#ifdef QT_OPENGL_ES_2

    program_solid = NULL;
    program_texture = NULL;

    // texture + color
    {
        QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
        if (!vshader->compileSourceCode(vsrc_TEXTURE))
            CRLog::error("error while compiling shader %s", vsrc_TEXTURE);

        QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);
        if (!fshader->compileSourceCode(fsrc_TEXTURE))
            CRLog::error("error while compiling shader %s", fsrc_TEXTURE);

        program_texture = new QGLShaderProgram(this);
        program_texture->addShader(vshader);
        program_texture->addShader(fshader);
        program_texture->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
        program_texture->bindAttributeLocation("colAttr", PROGRAM_COLOR_ATTRIBUTE);
        program_texture->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
        if (!program_texture->link())
            CRLog::error("error while linking texture program");
        if (!program_texture->bind())
            CRLog::error("error while binding texture program");
//        program_texture->setUniformValue("texture", 0);
    }

    // solid
    {
        QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
        if (!vshader->compileSourceCode(vsrc_SOLID))
            CRLog::error("error while compiling shader %s", vsrc_SOLID);

        QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);
        if (!fshader->compileSourceCode(fsrc_SOLID))
            CRLog::error("error while compiling shader %s", fsrc_SOLID);

        program_solid = new QGLShaderProgram(this);
        program_solid->addShader(vshader);
        program_solid->addShader(fshader);
        program_solid->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE_SOLID);
        program_solid->bindAttributeLocation("colAttr", PROGRAM_COLOR_ATTRIBUTE_SOLID);
        if (!program_solid->link())
            CRLog::error("error while linking solid program");
        if (!program_solid->bind())
            CRLog::error("error while binding solid program");
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

int CRGLSupportImpl::getMaxTextureSize() {
    if (maxTextureSize)
        return maxTextureSize;
    GLint _maxTextureSize[1];
    GLint _maxFramebufferSize[2];
    _maxTextureSize[0] = 1024;
    _maxFramebufferSize[0] = 1024;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, _maxTextureSize);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, _maxFramebufferSize);
    maxTextureSize = _maxTextureSize[0];
    if (maxTextureSize > _maxFramebufferSize[0])
        maxTextureSize = _maxFramebufferSize[0];
    if (maxTextureSize > _maxFramebufferSize[1])
        maxTextureSize = _maxFramebufferSize[1];
    CRLog::info("Max OpenGL texture size: %d", maxTextureSize);
    return maxTextureSize;
}

lUInt32 CRGLSupportImpl::genTexture() {
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    //if (checkError("glGenTextures")) return 0;
    return textureId;
}

void CRGLSupportImpl::deleteTexture(lUInt32 & textureId) {
    if (!textureId)
        return;
    if (glIsTexture(textureId) != GL_TRUE) {
        CRLog::error("Invalid texture %d", textureId);
        return;
    }
    GLuint id = textureId;
    glDeleteTextures(1, &id);
    checkError("~GLImageCachePage - glDeleteTextures");
    textureId = 0;
}

bool CRGLSupportImpl::setTextureImage(lUInt32 textureId, int dx, int dy, lUInt8 * pixels) {
    checkError("before setTextureImage");
    glActiveTexture(GL_TEXTURE0);
    checkError("updateTexture - glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, 0);
    checkError("updateTexture - glBindTexture(0)");
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

    if (!glIsTexture(textureId))
        CRLog::error("second test - invalid texture passed to CRGLSupportImpl::setTextureImage");

    // ORIGINAL: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dx, dy, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dx, dy, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    checkError("updateTexture - glTexImage2D");
    if (glGetError() != GL_NO_ERROR) {
        CRLog::error("Cannot set image for texture");
        return false;
    }
    checkError("after setTextureImage");
    return true;
}

bool CRGLSupportImpl::setTextureImageAlpha(lUInt32 textureId, int dx, int dy, lUInt8 * pixels) {
    checkError("before setTextureImageAlpha");
    glActiveTexture(GL_TEXTURE0);
    checkError("updateTexture - glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, 0);
    checkError("updateTexture - glBindTexture(0)");
    glBindTexture(GL_TEXTURE_2D, textureId);
    checkError("setTextureImageAlpha - glBindTexture");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    checkError("setTextureImageAlpha - glPixelStorei");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    checkError("setTextureImageAlpha - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkError("setTextureImageAlpha - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    checkError("setTextureImageAlpha - glTexParameteri");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkError("setTextureImageAlpha - glTexParameteri");

    if (!glIsTexture(textureId))
        CRLog::error("second test: invalid texture passed to CRGLSupportImpl::setTextureImageAlpha");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, dx, dy, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
    checkError("setTextureImageAlpha - glTexImage2D");
    if (glGetError() != GL_NO_ERROR) {
        CRLog::error("Cannot set image for texture");
        return false;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    checkError("updateTexture - glBindTexture(0)");
    checkError("after setTextureImageAlpha");
    return true;
}

/// returns texture ID for buffer, 0 if failed
bool CRGLSupportImpl::createFramebuffer(lUInt32 &textureId, lUInt32 &framebufferId, int dx, int dy) {
    checkError("before createFramebuffer");
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
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    checkError("glClearColor");
    glClear(GL_COLOR_BUFFER_BIT);
    checkError("glClear");
    checkError("after createFramebuffer");
    //CRLog::trace("CRGLSupportImpl::createFramebuffer %d,%d  texture=%d, buffer=%d", dx, dy, textureId, framebufferId);
    currentFramebufferId = framebufferId;

    glBindTexture(GL_TEXTURE_2D, 0);
    checkError("createFramebuffer - glBindTexture(0)");
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
    checkError("createFramebuffer - glBindFramebufferOES(0)");

    return res;
}

void CRGLSupportImpl::deleteFramebuffer(lUInt32 &framebufferId) {
    //CRLog::debug("GLDrawBuf::deleteFramebuffer");
    if (framebufferId != 0) {
        glBindFramebufferOES( GL_FRAMEBUFFER_OES, 0);
        checkError("deleteFramebuffer - glBindFramebufferOES");
        GLuint fid = framebufferId;
        glDeleteFramebuffersOES(1, &fid);
        checkError("deleteFramebuffer - glDeleteFramebuffer");
    }
    //CRLog::trace("CRGLSupportImpl::deleteFramebuffer(%d)", framebufferId);
    framebufferId = 0;
    checkError("after deleteFramebuffer");
    currentFramebufferId = 0;
}

bool CRGLSupportImpl::bindFramebuffer(lUInt32 framebufferId) {
    //CRLog::trace("CRGLSupportImpl::bindFramebuffer(%d)", framebufferId);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebufferId);
    currentFramebufferId = framebufferId;
    return !checkError("glBindFramebufferOES");
}

void CRGLSupportImpl::flush() {
    glFlush();
    checkError("glFlush");
    //CRLog::trace("CRGLSupportImpl::flush()");
}

void CRGLSupportImpl::myGlOrtho(float left, float right, float bottom, float top,
                                         float zNearPlane, float zFarPlane)
{
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
}

void CRGLSupportImpl::setOrthoProjection(int dx, int dy) {
    //myGlOrtho(0, dx, 0, dy, -1.0f, 5.0f);
    bufferDx = dx;
    bufferDy = dy;
    //myGlOrtho(0, dx, 0, dy, -0.1f, 5.0f);

#ifdef QT_OPENGL_ES_2
    QMatrix4x4 matrix2;
    matrix2.ortho(0, dx, 0, dy, 0.5f, 5.0f);
    matrix2.copyDataTo(m);
#else
    myGlOrtho(0, dx, 0, dy, 0.1f, 5.0f);

    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    checkError("glPushMatrix");
    //glLoadIdentity();
    glLoadMatrixf(m);
    //glOrthof(0, _dx, 0, _dy, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    //checkError("glPushMatrix");
    glLoadIdentity();
#endif
    glViewport(0,0,dx,dy);
    checkError("glViewport");
}

void CRGLSupportImpl::setRotation(int x, int y, int rotationAngle) {
    this->rotationAngle = rotationAngle;
    rotationX = x;
    rotationY = y;
    if (!rotationAngle)
        return;
#ifdef QT_OPENGL_ES_2
    QMatrix4x4 matrix2;
    matrix2.ortho(0, bufferDx, 0, bufferDy, 0.5f, 5.0f);
    matrix2.translate(rotationX, rotationY, 0);
    matrix2.rotate(rotationAngle, 0, 0, 1);
    matrix2.translate(-rotationX, -rotationY, 0);
    matrix2.copyDataTo(m);
#else
    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    //checkError("push matrix");
    checkError("matrix mode");
    glLoadMatrixf(m);
    if (rotationAngle) {
        glTranslatef(rotationX, rotationY, 0);
        glRotatef(rotationAngle, 0, 0, 1);
        glTranslatef(-rotationX, -rotationY, 0);
    }
#endif
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

