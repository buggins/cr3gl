#include "glwrapper.h"
#include <lvstring.h>

CRGLSupport::CRGLSupport() {
#if QT_GL
    initializeOpenGLFunctions();
#endif
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
QGLShaderProgram *CRGLSupport::program_texture = NULL;
QGLShaderProgram *CRGLSupport::program_solid = NULL;
#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_COLOR_ATTRIBUTE 1
#define PROGRAM_TEXCOORD_ATTRIBUTE 2
#endif

void CRGLSupport::drawSolidFillRect(GLfloat * matrixPtr, GLfloat vertices[], GLfloat colors[]) {
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

void CRGLSupport::drawColorAndTextureRect(GLfloat * matrixPtr, GLfloat vertices[], GLfloat texcoords[], GLfloat colors[], GLint textureId) {
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

void CRGLSupport::init() {
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

void CRGLSupport::uninit() {
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

