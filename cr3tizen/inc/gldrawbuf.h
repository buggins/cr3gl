/*
 * gldrawbuf.h
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#ifndef GLDRAWBUF_H_
#define GLDRAWBUF_H_

#include <lvdrawbuf.h>

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
public:
    /// rotates buffer contents by specified angle
    virtual void Rotate( cr_rotate_angle_t angle ) = 0;
    /// returns white pixel value
    virtual lUInt32 GetWhiteColor() = 0;
    /// returns black pixel value
    virtual lUInt32 GetBlackColor() = 0;
    /// returns current background color
    virtual lUInt32 GetBackgroundColor() = 0;
    /// sets current background color
    virtual void SetBackgroundColor( lUInt32 cl ) = 0;
    /// returns current text color
    virtual lUInt32 GetTextColor() = 0;
    /// sets current text color
    virtual void SetTextColor( lUInt32 cl ) = 0;
    /// gets clip rect
    virtual void GetClipRect( lvRect * clipRect ) = 0;
    /// sets clip rect
    virtual void SetClipRect( const lvRect * clipRect ) = 0;
    /// set to true for drawing in Paged mode, false for Scroll mode
    virtual void setHidePartialGlyphs( bool hide ) = 0;
    /// invert image
    virtual void  Invert() = 0;
    /// get buffer width, pixels
    virtual int  GetWidth() = 0;
    /// get buffer height, pixels
    virtual int  GetHeight() = 0;
    /// get buffer bits per pixel
    virtual int  GetBitsPerPixel() = 0;
    /// fills buffer with specified color
    virtual int  GetRowSize() = 0;
    /// fills buffer with specified color
    virtual void Clear( lUInt32 color ) = 0;
    /// get pixel value
    virtual lUInt32 GetPixel( int x, int y ) = 0;
    /// get average pixel value for area (coordinates are fixed floating points *16)
    virtual lUInt32 GetAvgColor(lvRect & rc16) = 0;
    /// get linearly interpolated pixel value (coordinates are fixed floating points *16)
    virtual lUInt32 GetInterpolatedColor(int x16, int y16) = 0;
    /// fills rectangle with specified color
    virtual void FillRect( int x0, int y0, int x1, int y1, lUInt32 color ) = 0;
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
    virtual void FillRectPattern( int x0, int y0, int x1, int y1, lUInt32 color0, lUInt32 color1, lUInt8 * pattern ) = 0;
    /// inverts image in specified rectangle
    virtual void InvertRect(int x0, int y0, int x1, int y1) = 0;
    /// sets new size
    virtual void Resize( int dx, int dy ) = 0;
    /// draws bitmap (1 byte per pixel) using specified palette
    virtual void Draw( int x, int y, const lUInt8 * bitmap, int width, int height, lUInt32 * palette ) = 0;
    /// draws image
    virtual void Draw( LVImageSourceRef img, int x, int y, int width, int height, bool dither=true ) = 0;
    /// draws buffer content to another buffer doing color conversion if necessary
    virtual void DrawTo( LVDrawBuf * buf, int x, int y, int options, lUInt32 * palette ) = 0;
    /// draws rescaled buffer content to another buffer doing color conversion if necessary
    virtual void DrawRescaled(LVDrawBuf * src, int x, int y, int dx, int dy, int options) = 0;
#if !defined(__SYMBIAN32__) && defined(_WIN32)
    /// draws buffer content to another buffer doing color conversion if necessary
    virtual void DrawTo( HDC dc, int x, int y, int options, lUInt32 * palette ) = 0;
#endif
    /// draws text string
    /*
    virtual void DrawTextString( int x, int y, LVFont * pfont,
                       const lChar16 * text, int len,
                       lChar16 def_char, lUInt32 * palette, bool addHyphen=false ) = 0;
    */

/*
    /// draws formatted text
    virtual void DrawFormattedText( formatted_text_fragment_t * text, int x, int y ) = 0;
*/
    /// returns scanline pointer
    virtual lUInt8 * GetScanLine( int y );

    /// create drawing texture of specified size
    GLDrawBuf(int width, int height, int bpp);

    /// virtual destructor
    virtual ~GLDrawBuf();
};




#endif /* GLDRAWBUF_H_ */
