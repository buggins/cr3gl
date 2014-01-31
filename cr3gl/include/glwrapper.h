#ifndef GLWRAPPER_H
#define GLWRAPPER_H

#include <lvtypes.h>

class CRGLSupport
{
public:
    CRGLSupport() {}
    ~CRGLSupport() {}
    virtual void drawSolidFillRect(float vertices[], lUInt32 color) = 0;
    virtual void drawSolidFillRect(float vertices[], float colors[]) = 0;
    virtual void drawColorAndTextureRect(float vertices[], float texcoords[], lUInt32 color, lUInt32 textureId) = 0;
    virtual void drawColorAndTextureRect(float vertices[], float txcoords[], float colors[], lUInt32 textureId) = 0;

    virtual lUInt32 genTexture() = 0;
    virtual bool isTexture(lUInt32 textureId) = 0;
    virtual void deleteTexture(lUInt32 & textureId) = 0;
    /// set texture image in RGBA format, returns false if failed
    virtual bool setTextureImage(lUInt32 textureId, int dx, int dy, unsigned char * pixels) = 0;
    /// sets texture image as ALPHA only, returns false if failed
    virtual bool setTextureImageAlpha(lUInt32 textureId, int dx, int dy, unsigned char * pixels) = 0;
    /// returns texture ID for buffer, 0 if failed
    virtual bool createFramebuffer(lUInt32 &textureId, lUInt32 &framebufferId, int dx, int dy) = 0;

    virtual void deleteFramebuffer(lUInt32 &framebufferId) = 0;

    virtual bool bindFramebuffer(lUInt32 framebufferId) = 0;
    virtual void setOrthoProjection(int dx, int dy) = 0;
    virtual void setRotation(int x, int y, int rotationAngle) = 0;
    virtual void flush() = 0;

    static CRGLSupport * instance();
};


void LVGLFillColor(lUInt32 color, float * buf, int count);

#define CRGL CRGLSupport::instance()

#endif // GLWRAPPER_H
