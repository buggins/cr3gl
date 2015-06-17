#ifndef GLWRAPPER_H
#define GLWRAPPER_H

#include <lvtypes.h>
#include <lvarray.h>

class CRGLSupport
{
protected:
    //virtual void drawColorAndTextureRect(float vertices[], float texcoords[], lUInt32 color, lUInt32 textureId) = 0;
    virtual void drawSolidFillRect(float vertices[], lUInt32 color) = 0;
    virtual void drawSolidFillRect(float vertices[], float colors[]) = 0;
    virtual void drawColorAndTextureRect(float vertices[], float txcoords[], float colors[], lUInt32 textureId, bool linear) = 0;

    virtual void drawColorAndTextureRect(lUInt32 textureId, int tdx, int tdy, int srcx, int srcy, int srcdx, int srcdy, int xx, int yy, int dx, int dy, lUInt32 color, bool linear) = 0;

public:
    CRGLSupport() {}
    virtual ~CRGLSupport() {}
    virtual int getMaxTextureSize() = 0;

    virtual void drawSolidFillRect(lvRect & rc, lUInt32 color1, lUInt32 color2, lUInt32 color3, lUInt32 color4) = 0;
    virtual void drawColorAndTextureRect(lUInt32 textureId, int tdx, int tdy, lvRect & srcrc, lvRect & dstrc, lUInt32 color, bool linear) = 0;

    virtual lUInt32 genTexture() = 0;
    virtual bool isTexture(lUInt32 textureId) = 0;
    virtual void deleteTexture(lUInt32 & textureId) = 0;
    /// set texture image in RGBA format, returns false if failed
    virtual bool setTextureImage(lUInt32 textureId, int dx, int dy, unsigned char * pixels) = 0;
    /// sets texture image as ALPHA only, returns false if failed
    virtual bool setTextureImageAlpha(lUInt32 textureId, int dx, int dy, unsigned char * pixels) = 0;
    /// sets texture image as Luminance + Alpha, returns false if failed
    virtual bool setTextureImageLuminanceAlpha(lUInt32 textureId, int dx, int dy, lUInt8 * pixels) = 0;
    /// sets texture image as GRAY only, returns false if failed
    virtual bool setTextureImageGray(lUInt32 textureId, int dx, int dy, lUInt8 * pixels) = 0;
    /// returns texture ID for buffer, 0 if failed
    virtual bool createFramebuffer(lUInt32 &textureId, lUInt32 &framebufferId, int dx, int dy) = 0;

    virtual void deleteFramebuffer(lUInt32 &framebufferId) = 0;

    virtual bool bindFramebuffer(lUInt32 framebufferId) = 0;
    virtual void setOrthoProjection(int dx, int dy) = 0;
    virtual void setRotation(int x, int y, int rotationAngle) = 0;
    virtual void flush() = 0;

    static CRGLSupport * instance();
    static void close();
};


void LVGLFillColor(lUInt32 color, float * buf, int count);

#define CRGL CRGLSupport::instance()

#endif // GLWRAPPER_H
