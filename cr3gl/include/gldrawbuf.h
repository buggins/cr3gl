/*
 * gldrawbuf.h
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#ifndef GLDRAWBUF_H_
#define GLDRAWBUF_H_

#include <lvdrawbuf.h>
#include <lvhashtable.h>
#include <lvptrvec.h>

#include "glwrapper.h"

#include "glscene.h"

class GLDrawBuf;

/// Abstract drawing buffer
class TiledGLDrawBuf : public LVDrawBuf
{
protected:
    int _dx;
    int _dy;
    int _tiledx;
    int _tiledy;
    int _xtiles;
    int _ytiles;
    int _bpp;
    GLDrawBuf ** _tiles;
    bool _hidePartialGlyphs;
    lvRect _clipRect;
    lUInt32 _textColor;
    lUInt32 _backgroundColor;
    //lUInt32 _renderbufferId;
    //int _prepareStage;
    //GLScene * _scene;
    int _alpha;

    void init(int dx, int dy);
    void cleanup();

public:
    virtual GLDrawBuf * asGLDrawBuf() { return NULL; }

    virtual void beforeDrawing();
    virtual void afterDrawing();

    // tiles support
    /// returns true if drawing buffer is tiled
    virtual bool isTiled() { return true; }
    /// returns tile width (or just width if no tiles)
    virtual int tileWidth() { return _tiledx; }
    /// returns tile height (or just height if no tiles)
    virtual int tileHeight() { return _tiledy; }
    /// returns tile drawbuf for tiled image, returns this for non tiled draw buffer
    virtual LVDrawBuf * getTile(int x, int y);
    /// returns tile rectangle
    virtual void getTileRect(lvRect & rc, int x, int y);
    /// returns number of tiles in row
    virtual int getXtiles() {
        return _xtiles;
    }
    /// returns number of tiles in column
    virtual int getYtiles() {
        return _ytiles;
    }


    // general DrawBuf methods

    /// rotates buffer contents by specified angle
    virtual void Rotate( cr_rotate_angle_t angle );
    /// returns white pixel value
    virtual lUInt32 GetWhiteColor();
    /// returns black pixel value
    virtual lUInt32 GetBlackColor();
    /// returns current background color
    virtual lUInt32 GetBackgroundColor();
    /// sets current background color
    virtual void SetBackgroundColor( lUInt32 cl );
    /// returns current text color
    virtual lUInt32 GetTextColor();
    /// sets current text color
    virtual void SetTextColor( lUInt32 cl );
    /// gets clip rect
    virtual void GetClipRect( lvRect * clipRect );
    /// sets clip rect
    virtual void SetClipRect( const lvRect * clipRect );
    /// set to true for drawing in Paged mode, false for Scroll mode
    virtual void setHidePartialGlyphs( bool hide );
    /// invert image
    virtual void  Invert();
    /// get buffer width, pixels
    virtual int  GetWidth();
    /// get buffer height, pixels
    virtual int  GetHeight();
    /// get buffer bits per pixel
    virtual int  GetBitsPerPixel();
    /// fills buffer with specified color
    virtual int  GetRowSize();
    /// fills buffer with specified color
    virtual void Clear( lUInt32 color );
    /// get pixel value
    virtual lUInt32 GetPixel( int x, int y );
    /// get average pixel value for area (coordinates are fixed floating points *16)
    virtual lUInt32 GetAvgColor(lvRect & rc16);
    /// get linearly interpolated pixel value (coordinates are fixed floating points *16)
    virtual lUInt32 GetInterpolatedColor(int x16, int y16);


    /// draw gradient filled rectangle with colors for top-left, top-right, bottom-right, bottom-left
    virtual void GradientRect(int x0, int y0, int x1, int y1, lUInt32 color1, lUInt32 color2, lUInt32 color3, lUInt32 color4);
    /// fills rectangle with specified color
    virtual void FillRect( int x0, int y0, int x1, int y1, lUInt32 color );
    /// fills rectangle with specified color
    inline void FillRect( const lvRect & rc, lUInt32 color )
    {
        FillRect( rc.left, rc.top, rc.right, rc.bottom, color );
    }
    /// draws rectangle with specified color
    inline void Rect( int x0, int y0, int x1, int y1, lUInt32 color )
    {
        FillRect( x0, y0, x1-1, y0+1, color );
        FillRect( x0, y0, x0+1, y1-1, color );
        FillRect( x1-1, y0, x1, y1, color );
        FillRect( x0, y1-1, x1, y1, color );
    }
    /// draws rectangle with specified width and color
    inline void Rect( int x0, int y0, int x1, int y1, int borderWidth, lUInt32 color )
    {
        FillRect( x0, y0, x1-1, y0+borderWidth, color );
        FillRect( x0, y0, x0+borderWidth, y1-1, color );
        FillRect( x1-borderWidth, y0, x1, y1, color );
        FillRect( x0, y1-borderWidth, x1, y1, color );
    }
    /// draws rounded rectangle with specified line width, rounding radius, and color
    void RoundRect( int x0, int y0, int x1, int y1, int borderWidth, int radius, lUInt32 color, int cornerFlags=0x0F  );
    /// draws rectangle with specified color
    inline void Rect( const lvRect & rc, lUInt32 color )
    {
        Rect( rc.left, rc.top, rc.right, rc.bottom, color );
    }
    /// draws rectangle with specified color
    inline void Rect( const lvRect & rc, int borderWidth, lUInt32 color )
    {
        Rect( rc.left, rc.top, rc.right, rc.bottom, borderWidth, color );
    }
    /// fills rectangle with pattern
    virtual void FillRectPattern( int x0, int y0, int x1, int y1, lUInt32 color0, lUInt32 color1, lUInt8 * pattern );
    /// inverts image in specified rectangle
    virtual void InvertRect(int x0, int y0, int x1, int y1);
    /// sets new size
    virtual void Resize( int dx, int dy );
    /// draws bitmap (1 byte per pixel) using specified palette
    virtual void Draw( int x, int y, const lUInt8 * bitmap, int width, int height, lUInt32 * palette );
    /// draws image
    virtual void Draw( LVImageSourceRef img, int x, int y, int width, int height, bool dither=true );
    /// for GL buf only - rotated drawing
    virtual void DrawRotated( LVImageSourceRef img, int x, int y, int width, int height, int rotationAngle);
    /// draws buffer content to another buffer doing color conversion if necessary
    virtual void DrawTo( LVDrawBuf * buf, int x, int y, int options, lUInt32 * palette );
    /// draws rescaled buffer content to another buffer doing color conversion if necessary
    virtual void DrawRescaled(LVDrawBuf * src, int x, int y, int dx, int dy, int options);
    /// draws rescaled buffer content to another buffer doing color conversion if necessary
    virtual void DrawFragment(LVDrawBuf * src, int srcx, int srcy, int srcdx, int srcdy, int x, int y, int dx, int dy, int options);
#if !defined(__SYMBIAN32__) && defined(_WIN32) && !defined(NO_WIN32_DRAWING)
    /// draws buffer content to another buffer doing color conversion if necessary
    virtual void DrawTo( HDC dc, int x, int y, int options, lUInt32 * palette ) {}
#endif
    /// draws text string
    /*
    virtual void DrawTextString( int x, int y, LVFont * pfont,
                       const lChar16 * text, int len,
                       lChar16 def_char, lUInt32 * palette, bool addHyphen=false );
    */

/*
    /// draws formatted text
    virtual void DrawFormattedText( formatted_text_fragment_t * text, int x, int y );
*/
    /// returns scanline pointer
    virtual lUInt8 * GetScanLine( int y );

    virtual int getAlpha() { return _alpha; }
    virtual void setAlpha(int alpha);
    virtual lUInt32 applyAlpha(lUInt32 cl);

    /// create drawing texture of specified size
    TiledGLDrawBuf(int width, int height, int bpp, int tilex, int tiley);

    /// virtual destructor
    virtual ~TiledGLDrawBuf();
};

/// Abstract drawing buffer
class GLDrawBuf : public LVDrawBuf
{
protected:
	int _dx;
	int _dy;
	int _tdx;
	int _tdy;
	int _bpp;
	bool _hidePartialGlyphs;
	lvRect _clipRect;
	lUInt32 _textColor;
	lUInt32 _backgroundColor;
	bool _textureBuf;
    lUInt32 _textureId;
    lUInt32 _framebufferId;
    //lUInt32 _renderbufferId;
	int _prepareStage;
	GLScene * _scene;
    int _alpha;
public:
    GLScene * getScene() { return _scene; }
    virtual GLDrawBuf * asGLDrawBuf() { return this; }

	virtual void createFramebuffer();
	virtual void deleteFramebuffer();
	virtual void beforeDrawing();
	virtual void afterDrawing();

    /// rotates buffer contents by specified angle
    virtual void Rotate( cr_rotate_angle_t angle );
    /// returns white pixel value
    virtual lUInt32 GetWhiteColor();
    /// returns black pixel value
    virtual lUInt32 GetBlackColor();
    /// returns current background color
    virtual lUInt32 GetBackgroundColor();
    /// sets current background color
    virtual void SetBackgroundColor( lUInt32 cl );
    /// returns current text color
    virtual lUInt32 GetTextColor();
    /// sets current text color
    virtual void SetTextColor( lUInt32 cl );
    /// gets clip rect
    virtual void GetClipRect( lvRect * clipRect );
    /// sets clip rect
    virtual void SetClipRect( const lvRect * clipRect );
    /// set to true for drawing in Paged mode, false for Scroll mode
    virtual void setHidePartialGlyphs( bool hide );
    /// invert image
    virtual void  Invert();
    /// get buffer width, pixels
    virtual int  GetWidth();
    /// get buffer height, pixels
    virtual int  GetHeight();
    /// get buffer bits per pixel
    virtual int  GetBitsPerPixel();
    /// fills buffer with specified color
    virtual int  GetRowSize();
    /// fills buffer with specified color
    virtual void Clear( lUInt32 color );
    /// get pixel value
    virtual lUInt32 GetPixel( int x, int y );
    /// get average pixel value for area (coordinates are fixed floating points *16)
    virtual lUInt32 GetAvgColor(lvRect & rc16);
    /// get linearly interpolated pixel value (coordinates are fixed floating points *16)
    virtual lUInt32 GetInterpolatedColor(int x16, int y16);
    /// draw gradient filled rectangle with colors for top-left, top-right, bottom-right, bottom-left
    virtual void GradientRect(int x0, int y0, int x1, int y1, lUInt32 color1, lUInt32 color2, lUInt32 color3, lUInt32 color4);
    /// fills rectangle with specified color
    virtual void FillRect( int x0, int y0, int x1, int y1, lUInt32 color );
    /// fills rectangle with specified color
    inline void FillRect( const lvRect & rc, lUInt32 color )
    {
        FillRect( rc.left, rc.top, rc.right, rc.bottom, color );
    }
    /// draws rectangle with specified color
    inline void Rect( int x0, int y0, int x1, int y1, lUInt32 color )
    {
        FillRect( x0, y0, x1-1, y0+1, color );
        FillRect( x0, y0, x0+1, y1-1, color );
        FillRect( x1-1, y0, x1, y1, color );
        FillRect( x0, y1-1, x1, y1, color );
    }
    /// draws rectangle with specified width and color
    inline void Rect( int x0, int y0, int x1, int y1, int borderWidth, lUInt32 color )
    {
        FillRect( x0, y0, x1-1, y0+borderWidth, color );
        FillRect( x0, y0, x0+borderWidth, y1-1, color );
        FillRect( x1-borderWidth, y0, x1, y1, color );
        FillRect( x0, y1-borderWidth, x1, y1, color );
    }
    /// draws rounded rectangle with specified line width, rounding radius, and color
    void RoundRect( int x0, int y0, int x1, int y1, int borderWidth, int radius, lUInt32 color, int cornerFlags=0x0F  );
    /// draws rectangle with specified color
    inline void Rect( const lvRect & rc, lUInt32 color )
    {
        Rect( rc.left, rc.top, rc.right, rc.bottom, color );
    }
    /// draws rectangle with specified color
    inline void Rect( const lvRect & rc, int borderWidth, lUInt32 color )
    {
        Rect( rc.left, rc.top, rc.right, rc.bottom, borderWidth, color );
    }
    /// fills rectangle with pattern
    virtual void FillRectPattern( int x0, int y0, int x1, int y1, lUInt32 color0, lUInt32 color1, lUInt8 * pattern );
    /// inverts image in specified rectangle
    virtual void InvertRect(int x0, int y0, int x1, int y1);
    /// sets new size
    virtual void Resize( int dx, int dy );
    /// draws bitmap (1 byte per pixel) using specified palette
    virtual void Draw( int x, int y, const lUInt8 * bitmap, int width, int height, lUInt32 * palette );
    /// draws image
    virtual void Draw( LVImageSourceRef img, int x, int y, int width, int height, bool dither=true );
    /// for GL buf only - rotated drawing
    virtual void DrawRotated( LVImageSourceRef img, int x, int y, int width, int height, int rotationAngle);
    /// draws buffer content to another buffer doing color conversion if necessary
    virtual void DrawTo( LVDrawBuf * buf, int x, int y, int options, lUInt32 * palette );
    /// draws rescaled buffer content to another buffer doing color conversion if necessary
    virtual void DrawRescaled(LVDrawBuf * src, int x, int y, int dx, int dy, int options);
    /// draws rescaled buffer content to another buffer doing color conversion if necessary
    virtual void DrawFragment(LVDrawBuf * src, int srcx, int srcy, int srcdx, int srcdy, int x, int y, int dx, int dy, int options);
#if !defined(__SYMBIAN32__) && defined(_WIN32) && !defined(NO_WIN32_DRAWING)
    /// draws buffer content to another buffer doing color conversion if necessary
	virtual void DrawTo( HDC dc, int x, int y, int options, lUInt32 * palette ) {}
#endif
    /// draws text string
    /*
    virtual void DrawTextString( int x, int y, LVFont * pfont,
                       const lChar16 * text, int len,
                       lChar16 def_char, lUInt32 * palette, bool addHyphen=false );
    */

/*
    /// draws formatted text
    virtual void DrawFormattedText( formatted_text_fragment_t * text, int x, int y );
*/
    /// returns scanline pointer
    virtual lUInt8 * GetScanLine( int y );

    virtual int getAlpha() { return _alpha; }
    virtual void setAlpha(int alpha) { _alpha = alpha; }
    virtual lUInt32 applyAlpha(lUInt32 cl);

    /// create drawing texture of specified size
    GLDrawBuf(int width, int height, int bpp, bool textureBuffer);

    /// virtual destructor
    virtual ~GLDrawBuf();
};


//// utility function to fill 4-float array of vertex colors with converted CR 32bit color
//void LVGLFillColor(lUInt32 color, float * buf, int count);
//// converts color from CoolReader format and calls glColor4f
//void LVGLSetColor(lUInt32 color);

/// creates OpenGL based image cache
void LVGLCreateImageCache();

/// clears OpenGL based image cache on GL context destroy
void LVGLClearImageCache();

bool translateRect(lvRect & srcrc, lvRect & dstrc, lvRect & dstcrop);

#endif /* GLDRAWBUF_H_ */
