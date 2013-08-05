/*
 * gldrawbuf.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#include "gldrawbuf.h"
#include <glext.h>


#define MIN_TEX_SIZE 64
#define MAX_TEX_SIZE 4096
static int nearestPOT(int n) {
	for (int i = MIN_TEX_SIZE; i <= MAX_TEX_SIZE; i++) {
		if (n >= i)
			return i;
	}
	return MIN_TEX_SIZE;
}

static bool checkError(const char * context) {
	if (glGetError() != GL_NO_ERROR) {
		CRLog::error("GLDrawBuf : GL Error at %s", context);
		return true;
	}
	return false;
}



/// rotates buffer contents by specified angle
void GLDrawBuf::Rotate( cr_rotate_angle_t angle ) {
	CRLog::error("GLDrawBuf::Rotate() is not implemented");
}

/// returns white pixel value
lUInt32 GLDrawBuf::GetWhiteColor()
{
	return 0xFFFFFF;
}

/// returns black pixel value
lUInt32 GLDrawBuf::GetBlackColor()
{
	return 0x000000;
}

/// returns current background color
lUInt32 GLDrawBuf::GetBackgroundColor()
{
	return _backgroundColor;
}

/// sets current background color
void GLDrawBuf::SetBackgroundColor( lUInt32 cl )
{
	_backgroundColor = cl;
}

/// returns current text color
lUInt32 GLDrawBuf::GetTextColor()
{
	return _textColor;
}

/// sets current text color
void GLDrawBuf::SetTextColor( lUInt32 cl )
{
	_textColor = cl;
}
/// gets clip rect
void GLDrawBuf::GetClipRect( lvRect * clipRect )
{
	*clipRect = _clipRect;
}
/// sets clip rect
void GLDrawBuf::SetClipRect( const lvRect * clipRect )
{
	_clipRect = *clipRect;
}
/// set to true for drawing in Paged mode, false for Scroll mode
void GLDrawBuf::setHidePartialGlyphs( bool hide )
{
	_hidePartialGlyphs = hide;
}
/// invert image
void  GLDrawBuf::Invert() {
	CRLog::error("GLDrawBuf::Invert() is not implemented");
	// not supported
}
/// get buffer width, pixels
int  GLDrawBuf::GetWidth() {
	return _dx;
}
/// get buffer height, pixels
int  GLDrawBuf::GetHeight() {
	return _dy;
}
/// get buffer bits per pixel
int  GLDrawBuf::GetBitsPerPixel()
{
	return _bpp;
}

/// fills buffer with specified color
int  GLDrawBuf::GetRowSize()
{
	return _dx * _bpp / 8;
}

/// fills buffer with specified color
void GLDrawBuf::Clear( lUInt32 color )
{
	FillRect(0, 0, _dx, _dy, color);
}

/// get pixel value
lUInt32 GLDrawBuf::GetPixel( int x, int y )
{
	CRLog::error("GLDrawBuf::GetPixel() is not implemented");
	return 0;
}

/// get average pixel value for area (coordinates are fixed floating points *16)
lUInt32 GLDrawBuf::GetAvgColor(lvRect & rc16)
{
	CRLog::error("GLDrawBuf::GetAvgColor() is not implemented");
	return 0;
}

/// get linearly interpolated pixel value (coordinates are fixed floating points *16)
lUInt32 GLDrawBuf::GetInterpolatedColor(int x16, int y16)
{
	CRLog::error("GLDrawBuf::GetInterpolatedColor() is not implemented");
	return 0;
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

/// fills rectangle with specified color
void GLDrawBuf::FillRect( int x0, int y0, int x1, int y1, lUInt32 color )
{
	beforeDrawing();
	GLfloat vertices[] = {x0,y0,0, x0,y1,0, x1,y1,0, x0,y0,0, x1,y1,0, x1,y0,0};
	GLfloat colors[6 * 4];
	LVGLFillColor(color, colors, 6);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(4, GL_FLOAT, 0, colors);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	afterDrawing();
}

/// draws rounded rectangle with specified line width, rounding radius, and color
void GLDrawBuf::RoundRect( int x0, int y0, int x1, int y1, int borderWidth, int radius, lUInt32 color, int cornerFlags)
{
	CRLog::error("GLDrawBuf::RoundRect() is not implemented");
}

/// fills rectangle with pattern
void GLDrawBuf::FillRectPattern( int x0, int y0, int x1, int y1, lUInt32 color0, lUInt32 color1, lUInt8 * pattern )
{
	CRLog::error("GLDrawBuf::FillRectPattern() is not implemented");
}

/// inverts image in specified rectangle
void GLDrawBuf::InvertRect(int x0, int y0, int x1, int y1)
{
	CRLog::error("GLDrawBuf::InvertRect() is not implemented");
}

/// sets new size
void GLDrawBuf::Resize( int dx, int dy )
{
	deleteFramebuffer();
	_dx = dx;
	_dy = dy;
	_tdx = nearestPOT(dx);
	_tdy = nearestPOT(dy);
	_prepareStage = 0;
}

/// draws bitmap (1 byte per pixel) using specified palette
void GLDrawBuf::Draw( int x, int y, const lUInt8 * bitmap, int width, int height, lUInt32 * palette )
{
	CRLog::error("GLDrawBuf::Draw(bitmap) is not implemented");
}

/// draws image
void GLDrawBuf::Draw( LVImageSourceRef img, int x, int y, int width, int height, bool dither)
{
	CRLog::error("GLDrawBuf::Draw(img) is not implemented");
}

/// draws buffer content to another buffer doing color conversion if necessary
void GLDrawBuf::DrawTo( LVDrawBuf * buf, int x, int y, int options, lUInt32 * palette )
{
	CRLog::error("GLDrawBuf::DrawTo() is not implemented");
}

/// draws rescaled buffer content to another buffer doing color conversion if necessary
void GLDrawBuf::DrawRescaled(LVDrawBuf * src, int x, int y, int dx, int dy, int options)
{
	CRLog::error("GLDrawBuf::DrawRescaled() is not implemented");
}

/// draws text string
/*
void DrawTextString( int x, int y, LVFont * pfont,
				   const lChar16 * text, int len,
				   lChar16 def_char, lUInt32 * palette, bool addHyphen=false ) = 0;
*/

/*
/// draws formatted text
void DrawFormattedText( formatted_text_fragment_t * text, int x, int y ) = 0;
*/
/// returns scanline pointer
lUInt8 * GLDrawBuf::GetScanLine( int y )
{
	CRLog::error("GLDrawBuf::GetScanLine() is not implemented");
	return NULL;
}

void GLDrawBuf::createFramebuffer()
{
	if (_textureBuf) {
		glGenTextures(1, &_textureId);
		if (checkError("createFramebuffer glGenTextures")) return;
		glGenFramebuffersOES(1, &_framebufferId);
		if (checkError("createFramebuffer glGenFramebuffers")) return;
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebufferId);
		if (checkError("createFramebuffer glBindFramebuffer")) return;
		glBindTexture(GL_TEXTURE_2D, _textureId);
		if (checkError("createFramebuffer glBindTexture")) return;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		if (checkError("createFramebuffer glPixelStorei")) return;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		if (checkError("createFramebuffer glTexParameteri")) return;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _tdx, _tdy, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		if (checkError("createFramebuffer glTexImage2D")) return;
	}
}

void GLDrawBuf::deleteFramebuffer()
{
	if (_textureBuf) {
		if (_textureId != 0) {
			glDeleteTextures(1, &_textureId);
			checkError("deleteFramebuffer - glDeleteTextures");
		}
		if (_framebufferId != 0) {
			glDeleteFramebuffersOES(1, &_framebufferId);
			checkError("deleteFramebuffer - glDeleteFramebuffer");
		}
		_textureId = 0;
		_framebufferId = 0;
	}
}

void GLDrawBuf::beforeDrawing()
{
	if (_prepareStage++ == 0) {
		if (_textureBuf) {
			if (_textureId == 0 || _framebufferId == 0) {
				createFramebuffer();
				glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, _textureId, 0);
				checkError("beforeDrawing - glFramebufferTexture2D");
			}
		}
	}
}

void GLDrawBuf::afterDrawing()
{
	if (--_prepareStage == 0) {
		if (_textureBuf) {
			//bind the base framebuffer
			glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
			checkError("afterDrawing - glBindFramebuffer");
		}
	}
}

/// create drawing texture of specified size
GLDrawBuf::GLDrawBuf(int width, int height, int bpp, bool useTexture)
: _dx(width), _dy(height),
		_tdx(nearestPOT(width)), _tdy(nearestPOT(height)), _bpp(bpp),
		_hidePartialGlyphs(false), _clipRect(0, 0, width, height),
		_textColor(0x000000), _backgroundColor(0xFFFFFF),
		_textureBuf(useTexture), _textureId(0), _framebufferId(0), _prepareStage(0)
{

}

/// destructor
GLDrawBuf::~GLDrawBuf()
{
	deleteFramebuffer();
}

