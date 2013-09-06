/*
 * gldrawbuf.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#include "gldrawbuf.h"



#include <lvhashtable.h>
#include <lvarray.h>


class GLImageCachePage;

class GLImageCacheItem {
    GLImageCachePage * _page;
public:
    GLImageCachePage * getPage() { return _page; }
    CacheableObject * _objectPtr;
    // image size
    int _dx;
    int _dy;
    int _x0;
    int _y0;
    bool _deleted;
    GLImageCacheItem(GLImageCachePage * page, CacheableObject * obj) : _page(page), _objectPtr(obj), _deleted(false) {}
};

class GLImageCache : public CacheObjectListener {
    LVHashTable<CacheableObject*,GLImageCacheItem*> _map;
    LVPtrVector<GLImageCachePage> _pages;
    GLImageCachePage * _activePage;
    void removePage(GLImageCachePage * page);
public:
    GLImageCache();
    virtual ~GLImageCache();
    GLImageCacheItem * get(CacheableObject * obj);
    GLImageCacheItem * set(LVImageSourceRef img);
    GLImageCacheItem * set(LVDrawBuf * img);
    void clear();
    void drawItem(CacheableObject * obj, int x, int y, int dx, int dy, int srcx, int srcy, int srcwidth, int srcheight, lUInt32 color, int options, lvRect * clip, int rotationAngle);
    virtual void onCachedObjectDeleted(CacheableObject * obj);
    void removeDeletedItems();
};

extern GLImageCache * glImageCache;



#define MIN_TEX_SIZE 64
#define MAX_TEX_SIZE 4096
static int nearestPOT(int n) {
	for (int i = MIN_TEX_SIZE; i <= MAX_TEX_SIZE; i++) {
		if (n <= i)
			return i;
	}
	return MIN_TEX_SIZE;
}

static bool _checkError(const char *srcfile, int line, const char * context) {
    int err = glGetError();
    if (err != GL_NO_ERROR) {
        CRLog::error("GLDrawBuf : GL Error %04x at %s : %s : %d", err, context, srcfile, line);
		return true;
	}
	return false;
}

#define checkError(context) _checkError(__FILE__, __LINE__, context)

GLImageCache * glImageCache = NULL;

/// object deletion listener callback function type
static void onObjectDestroyedCallback(CacheObjectListener * pcache, CacheableObject * pobject) {
	if (glImageCache == pcache)
		glImageCache->onCachedObjectDeleted(pobject);
}

void LVGLCreateImageCache() {
	if (glImageCache != NULL)
		delete glImageCache;
	glImageCache = new GLImageCache();
}

class GLImageCachePage {
	GLImageCache * _cache;
	int _tdx;
	int _tdy;
	LVColorDrawBuf * _drawbuf;
	int _currentLine;
	int _nextLine;
	int _x;
	bool _closed;
	bool _needUpdateTexture;
	GLuint _textureId;
	int _itemCount;
public:
	GLImageCachePage(GLImageCache * cache, int dx, int dy) : _cache(cache), _drawbuf(NULL), _currentLine(0), _nextLine(0), _x(0), _closed(false), _needUpdateTexture(false), _textureId(0) {
        CRLog::trace("created image cache page %d x %d", dx, dy);
		_tdx = nearestPOT(dx);
		_tdy = nearestPOT(dy);
		_itemCount = 0;
	}

	virtual ~GLImageCachePage() {
		if (_drawbuf)
			delete _drawbuf;
        if (_textureId != 0) {
            if (glIsTexture(_textureId) != GL_TRUE) {
                CRLog::error("Invalid texture %d", _textureId);
                return;
            }
            glDeleteTextures(1, &_textureId);
            checkError("~GLImageCachePage - glDeleteTextures");
        }
	}

    void updateTexture() {
		if (_drawbuf == NULL)
			return; // no draw buffer!!!
	    if (_textureId == 0) {
	    	//CRLog::debug("updateTexture - new texture");
			glGenTextures(1, &_textureId);
			if (glGetError() != GL_NO_ERROR)
				return;
	    }
    	//CRLog::debug("updateTexture - setting image %dx%d", _drawbuf->GetWidth(), _drawbuf->GetHeight());
	    glBindTexture(GL_TEXTURE_2D, _textureId);
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
        lUInt8 * pixels = _drawbuf->GetScanLine(0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _drawbuf->GetWidth(), _drawbuf->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _drawbuf->GetScanLine(0));
	    checkError("updateTexture - glTexImage2D");
	    if (glGetError() != GL_NO_ERROR) {
	        glDeleteTextures(1, &_textureId);
            _textureId = 0;
	        return;
	    }
	    _needUpdateTexture = false;
	    if (_closed) {
	    	delete _drawbuf;
	    	_drawbuf = NULL;
	    }
	}
	void invertAlpha(GLImageCacheItem * item) {
		int x0 = item->_x0;
		int y0 = item->_y0;
		int x1 = x0 + item->_dx;
		int y1 = y0 + item->_dy;
	    for (int y = y0; y < y1; y++) {
	    	lUInt32 * row = (lUInt32 *)_drawbuf->GetScanLine(y);
	    	for (int x = x0; x < x1; x++) {
	    		lUInt32 cl = row[x];
	    		cl ^= 0xFF000000;
	    		lUInt32 r = (cl & 0x00FF0000) >> 16;
	    		lUInt32 b = (cl & 0x000000FF) << 16;
	    		row[x] = (cl & 0xFF00FF00) | r | b;
	    	}
	    }
	}
	GLImageCacheItem * reserveSpace(CacheableObject * obj, int width, int height) {
		GLImageCacheItem * cacheItem = new GLImageCacheItem(this, obj);
		if (_closed)
			return NULL;

		// next line if necessary
		if (_x + width > _tdx) {
			// move to next line
			_currentLine = _nextLine;
			_x = 0;
		}
		// check if no room left for glyph height
		if (_currentLine + height > _tdy) {
			_closed = true;
			return NULL;
		}
		cacheItem->_dx = width;
		cacheItem->_dy = height;
		cacheItem->_x0 = _x;
		cacheItem->_y0 = _currentLine;
		if (height && width) {
			if (_nextLine < _currentLine + height)
				_nextLine = _currentLine + height;
			if (!_drawbuf) {
				_drawbuf = new LVColorDrawBuf(_tdx, _tdy, 32);
				_drawbuf->SetBackgroundColor(0x000000);
				_drawbuf->SetTextColor(0xFFFFFF);
				_drawbuf->Clear(0xFF000000);
			}
			_x += width;
			_needUpdateTexture = true;
		}
		_itemCount++;
		return cacheItem;
	}
	int deleteItem(GLImageCacheItem* item) {
		_itemCount--;
		return _itemCount;
	}
	GLImageCacheItem * addItem(LVImageSourceRef img) {
		GLImageCacheItem * cacheItem = reserveSpace(img.get(), img->GetWidth(), img->GetHeight());
		if (cacheItem == NULL) {
			if (_closed && _needUpdateTexture)
				updateTexture();
			return NULL;
		}
		img->setOnObjectDestroyedCallback(onObjectDestroyedCallback, _cache);
		_drawbuf->Draw(img, cacheItem->_x0, cacheItem->_y0, cacheItem->_dx, cacheItem->_dy, false);
		invertAlpha(cacheItem);
		_needUpdateTexture = true;
		return cacheItem;
	}
	GLImageCacheItem * addItem(LVDrawBuf * buf) {
		GLImageCacheItem * cacheItem = reserveSpace(buf, buf->GetWidth(), buf->GetHeight());
		if (cacheItem == NULL)
			return NULL;
		buf->setOnObjectDestroyedCallback(onObjectDestroyedCallback, _cache);
		buf->DrawTo(_drawbuf, cacheItem->_x0, cacheItem->_y0, 0, NULL);
		invertAlpha(cacheItem);
		_needUpdateTexture = true;
		return cacheItem;
	}
    void drawItem(GLImageCacheItem * item, int x, int y, int dx, int dy, int srcx, int srcy, int srcdx, int srcdy, lUInt32 color, lUInt32 options, lvRect * clip, int rotationAngle) {
        //CRLog::trace("drawing item at %d,%d %dx%d <= %d,%d %dx%d ", x, y, dx, dy, srcx, srcy, srcdx, srcdy);
        if (_needUpdateTexture)
			updateTexture();
		if (_textureId != 0) {
            if (glIsTexture(_textureId) != GL_TRUE) {
                CRLog::error("Invalid texture %d", _textureId);
                return;
            }
            float dstx0 = x;
			float dsty0 = y;
			float dstx1 = x + dx;
			float dsty1 = y - dy;
			float txppx = 1 / (float)_tdx;
			float txppy = 1 / (float)_tdy;
			float srcx0 = (item->_x0 + srcx) * txppx;
			float srcy0 = (item->_y0 + srcy) * txppy;
			float srcx1 = (item->_x0 + srcx + srcdx) * txppx;
			float srcy1 = (item->_y0 + srcy + srcdy) * txppy;
			if (clip) {
                //CRLog::trace("before clipping dst (%d,%d,%d,%d) src (%f,%f,%f,%f)", (int)dstx0, (int)dsty0, (int)dstx1, (int)dsty1, srcx0, srcy0, srcx1, srcy1);
				// correct clipping
                float xscale = (srcx1-srcx0) / (dstx1 - dstx0);
                float yscale =  (srcy1-srcy0) / (dsty1 - dsty0);

                srcx0 += clip->left * xscale;
                srcx1 -= clip->right * xscale;
                dstx0 += clip->left;
				dstx1 -= clip->right;

                srcy0 -= clip->top * yscale;
                srcy1 += clip->bottom * yscale;
                dsty0 -= clip->top;
                dsty1 += clip->bottom;

                //CRLog::trace("after clipping dst (%d,%d,%d,%d) src (%f,%f,%f,%f)", (int)dstx0, (int)dsty0, (int)dstx1, (int)dsty1, srcx0, srcy0, srcx1, srcy1);
            }
	    	GLfloat vertices[] = {dstx0,dsty0,0, dstx0,dsty1,0, dstx1,dsty1,0, dstx0,dsty0,0, dstx1,dsty1,0, dstx1,dsty0,0};
	    	GLfloat texcoords[] = {srcx0,srcy0, srcx0,srcy1, srcx1,srcy1, srcx0,srcy0, srcx1,srcy1, srcx1,srcy0};
            //rotationAngle = 0;
            if (rotationAngle) {
                //rotationAngle = 0;
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                checkError("push matrix");
                int x = (dstx0 + dstx1) / 2;
                int y = (dsty0 + dsty1) / 2;
                checkError("matrix mode");
                glTranslatef(x, y, 0);
                glRotatef(rotationAngle, 0, 0, 1);
                glTranslatef(-x, -y, 0);
            }

	    	LVGLSetColor(color);
	    	//glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	    	//glDisable(GL_LIGHTING);
	    	glActiveTexture(GL_TEXTURE0);
            checkError("glActiveTexture");
            glEnable(GL_TEXTURE_2D);
            checkError("glEnable(GL_TEXTURE_2D)");
            glBindTexture(GL_TEXTURE_2D, _textureId);
            checkError("glBindTexture");

	    	glEnable(GL_BLEND);
	    	//GL_SRC_ALPHA
	    	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	    	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            checkError("glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)");
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	    	glDisableClientState(GL_COLOR_ARRAY);
            checkError("glDisableClientState(GL_COLOR_ARRAY)");
            glEnableClientState(GL_VERTEX_ARRAY);
            checkError("glEnableClientState(GL_VERTEX_ARRAY)");
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            checkError("glEnableClientState(GL_TEXTURE_COORD_ARRAY)");
            glVertexPointer(3, GL_FLOAT, 0, vertices);
            checkError("glVertexPointer(3, GL_FLOAT, 0, vertices)");
            glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
            checkError("glTexCoordPointer(2, GL_FLOAT, 0, texcoords)");

	    	glDrawArrays(GL_TRIANGLES, 0, 6);
            checkError("glDrawArrays");

            if (rotationAngle) {
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                checkError("pop matrix");
            }

	    	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	    	glDisableClientState(GL_VERTEX_ARRAY);
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
		}
	}
	void close() {
		_closed = true;
		if (_needUpdateTexture)
			updateTexture();
	}
};


//=======================================================================
// GLImageCache

#define GL_IMAGE_CACHE_PAGE_SIZE 1024
GLImageCacheItem * GLImageCache::get(CacheableObject * obj) {
	GLImageCacheItem * res = _map.get(obj);
	return res;
}

GLImageCacheItem * GLImageCache::set(LVImageSourceRef img) {
	GLImageCacheItem * res = NULL;
	if (img->GetWidth() <= GL_IMAGE_CACHE_PAGE_SIZE / 3 && img->GetHeight() <= GL_IMAGE_CACHE_PAGE_SIZE / 3) {
		// trying to reuse common page for small images
		if (_activePage)
			res = _activePage->addItem(img);
		if (!res) {
			_activePage = new GLImageCachePage(this, GL_IMAGE_CACHE_PAGE_SIZE, GL_IMAGE_CACHE_PAGE_SIZE);
			_pages.add(_activePage);
			res = _activePage->addItem(img);
		}
	} else {
		// use separate page for big image
		GLImageCachePage * page = new GLImageCachePage(this, img->GetWidth(), img->GetHeight());
		_pages.add(page);
		res = page->addItem(img);
		page->close();
	}
	_map.set(img.get(), res);
	return res;
}

GLImageCacheItem * GLImageCache::set(LVDrawBuf * img) {
	GLImageCacheItem * res = NULL;
	if (img->GetWidth() <= GL_IMAGE_CACHE_PAGE_SIZE / 3 && img->GetHeight() < GL_IMAGE_CACHE_PAGE_SIZE / 3) {
		// trying to reuse common page for small images
		if (_activePage == NULL) {
			_activePage = new GLImageCachePage(this, GL_IMAGE_CACHE_PAGE_SIZE, GL_IMAGE_CACHE_PAGE_SIZE);
			_pages.add(_activePage);
		}
		res = _activePage->addItem(img);
		if (!res) {
			_activePage = new GLImageCachePage(this, GL_IMAGE_CACHE_PAGE_SIZE, GL_IMAGE_CACHE_PAGE_SIZE);
			_pages.add(_activePage);
			res = _activePage->addItem(img);
		}
	} else {
		// use separate page for big image
		GLImageCachePage * page = new GLImageCachePage(this, img->GetWidth(), img->GetHeight());
		_pages.add(page);
		res = page->addItem(img);
		page->close();
	}
	_map.set(img, res);
	return res;
}

void GLImageCache::drawItem(CacheableObject * obj, int x, int y, int dx, int dy, int srcx, int srcy, int srcdx, int srcdy, lUInt32 color, int options, lvRect * clip, int rotationAngle)
{
	GLImageCacheItem* item = get(obj);
	if (item) {
        item->getPage()->drawItem(item, x, y, dx, dy, srcx, srcy, srcdx, srcdy, color, options, clip, rotationAngle);
	}
}

GLImageCache::GLImageCache() : _map(1024), _activePage(0)
{
	glImageCache = this;
}
GLImageCache::~GLImageCache() {
	clear();
	glImageCache = NULL;
}

void GLImageCache::clear() {
    CRLog::trace("Clearing image cache");
    LVHashTable<CacheableObject*,GLImageCacheItem*>::iterator iter = _map.forwardIterator();
	LVHashTable<CacheableObject*,GLImageCacheItem*>::pair * p;
	for (;;) {
		p = iter.next();
		if (!p)
			break;
		delete p->value;
	}
	_map.clear();
	_pages.clear();
}

void GLImageCache::removeDeletedItems() {
    LVArray<CacheableObject*> list;
    LVHashTable<CacheableObject*,GLImageCacheItem*>::iterator p = _map.forwardIterator();
    for (;;) {
        LVHashTable<CacheableObject*,GLImageCacheItem*>::pair * item = p.next();
        if (!item)
            break;
        if (item->value->_deleted)
            list.add(item->key);
    }
    for (int i = 0 ; i < list.length(); i++) {
        onCachedObjectDeleted(list[i]);
    }
}

void GLImageCache::onCachedObjectDeleted(CacheableObject * obj) {
    CRLog::trace("Cached object deleted");
	GLImageCacheItem* item = get(obj);
	if (item) {
        if (LVGLPeekScene()) {
            item->_deleted = true;
            CRLog::trace("item deleted while scene is active");
        } else {
            int itemsLeft = item->getPage()->deleteItem(item);
            CRLog::trace("itemsLeft = %d", itemsLeft);
            if (itemsLeft <= 0) {
                CRLog::trace("removing page");
                removePage(item->getPage());
            }
            _map.remove(obj);
            delete item;
        }
	}
}

void GLImageCache::removePage(GLImageCachePage * page) {
	if (_activePage == page)
		_activePage = NULL;
	page = _pages.remove(page);
	if (page)
		delete page;
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

//class ClipRectItem : public GLSceneItem {
//	lvRect _rc;
//public:
//	ClipRectItem(int left, int top, int right, int bottom) {
//		_rc.left = left;
//		_rc.right = right;
//		_rc.top = top;
//		_rc.bottom = bottom;
//	}
//	virtual void draw() {
//		//glViewport(_rc.left, _rc.top, _rc.width(), _rc.height());
//	}
//};

/// sets clip rect
void GLDrawBuf::SetClipRect( const lvRect * clipRect )
{
	bool changed = false;
	if (clipRect) {
		if (_clipRect != *clipRect) {
			_clipRect = *clipRect;
			changed = true;
		}
	} else {
		if (_clipRect.left != 0 || _clipRect.top != 0 || _clipRect.right != _dx || _clipRect.bottom != _dy) {
			_clipRect.left = 0;
			_clipRect.top = 0;
			_clipRect.right = _dx;
			_clipRect.bottom = _dy;
			changed = true;
		}
	}
//	if (_scene)
//		_scene->add(new ClipRectItem(_clipRect.left, GetHeight() - _clipRect.bottom, _clipRect.right, GetHeight() - _clipRect.top));
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

// converts color from CoolReader format and calls glColor4f
void LVGLSetColor(lUInt32 color) {
	float r = ((color >> 16) & 255) / 255.0f;
	float g = ((color >> 8) & 255) / 255.0f;
	float b = ((color >> 0) & 255) / 255.0f;
	float a = (((color >> 24) & 255) ^ 255) / 255.0f;
	glColor4f(r, g, b, a);
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

class GLFillRectItem : public GLSceneItem {
public:
	int x0;
	int y0;
	int x1;
    int y1;
    lUInt32 color;
    GLFillRectItem(int _x0, int _y0, int _x1, int _y1, lUInt32 _color) : x0(_x0), y0(_y0), x1(_x1), y1(_y1), color(_color) { }
    virtual void draw() {
    	GLfloat vertices[] = {
    			x0,y0,0,
    			x0,y1,0,
    			x1,y1,0,
    			x0,y0,0,
    			x1,y1,0,
    			x1,y0,0};
    	GLfloat colors[6 * 4];
    	LVGLFillColor(color, colors, 6);
    	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        checkError("glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)");
        glEnable(GL_BLEND);
        checkError("glEnable(GL_BLEND)");
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
    	glDisable(GL_BLEND);
    }
};

/// fills rectangle with specified color
void GLDrawBuf::FillRect( int x0, int y0, int x1, int y1, lUInt32 color )
{
	if (_scene) {
        lvRect rc(x0, y0, x1, y1);
        lvRect clip;
        GetClipRect(&clip);
        if (rc.intersect(clip)) {
            _scene->add(new GLFillRectItem(rc.left, _dy - rc.top, rc.right, _dy - rc.bottom, color));
        }
	}
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


class GLDrawImageSceneItem : public GLSceneItem {
    CacheableObject * img;
	int x;
	int y;
	int width;
	int height;
	int srcx;
	int srcy;
	int srcwidth;
	int srcheight;
	lUInt32 color;
	lUInt32 options;
	lvRect * clip;
    int rotationAngle;
public:
	virtual void draw() {
		if (glImageCache)
            glImageCache->drawItem(img, x, y, width, height, srcx, srcy, srcwidth, srcheight, color, options, clip, rotationAngle);
	}
    GLDrawImageSceneItem(CacheableObject * _img, int _x, int _y, int _width, int _height, int _srcx, int _srcy, int _srcw, int _srch, lUInt32 _color, lUInt32 _options, lvRect * _clip, int _rotationAngle)
	: img(_img), x(_x), y(_y), width(_width), height(_height),
	  srcx(_srcx), srcy(_srcy), srcwidth(_srcw), srcheight(_srch),
      color(_color), options(_options), clip(_clip), rotationAngle(_rotationAngle)
	{

	}
	virtual ~GLDrawImageSceneItem() {
		if (clip)
			delete clip;
	}
};

/// draws image
void GLDrawBuf::DrawRotated( LVImageSourceRef img, int x, int y, int width, int height, int rotationAngle)
{
    if (width <= 0 || height <= 0)
        return;
    GLImageCacheItem * item = glImageCache->get(img.get());
    if (item == NULL)
        item = glImageCache->set(img);
    if (item != NULL) {
        lvRect cliprect;
        GetClipRect(&cliprect);
        lvRect rc;
        rc.left = x;
        rc.top = y;
        rc.right = x + width;
        rc.bottom = y + height;
        if (!rc.intersects(cliprect))
            return; // out of bounds
        lvRect * clip = rc.clipBy(cliprect); // probably, should be clipped
        LVGLAddSceneItem(new GLDrawImageSceneItem(img.get(), x + 1, GetHeight() - y - 1, width - 2, height - 2, 1, 1, img->GetWidth() - 2, img->GetHeight() - 2, 0xFFFFFF, 0, clip, rotationAngle));
    }
}

/// draws image
void GLDrawBuf::Draw( LVImageSourceRef img, int x, int y, int width, int height, bool dither)
{
	if (width <= 0 || height <= 0)
		return;
	GLImageCacheItem * item = glImageCache->get(img.get());
	if (item == NULL)
		item = glImageCache->set(img);
	if (item != NULL) {
		lvRect cliprect;
		GetClipRect(&cliprect);
		lvRect rc;
		rc.left = x;
		rc.top = y;
		rc.right = x + width;
		rc.bottom = y + height;
		if (!rc.intersects(cliprect))
			return; // out of bounds
		const CR9PatchInfo * ninePatch = img->GetNinePatchInfo();
		if (!ninePatch) {
			lvRect * clip = rc.clipBy(cliprect); // probably, should be clipped
            LVGLAddSceneItem(new GLDrawImageSceneItem(img.get(), x, GetHeight() - y, width, height, 0, 0, img->GetWidth(), img->GetHeight(), 0xFFFFFF, 0, clip, 0));
		} else {
			lvRect srcitems[9];
			lvRect dstitems[9];
			lvRect src(1, 1, img->GetWidth()-1, img->GetHeight() - 1);
			ninePatch->calcRectangles(rc, src, dstitems, srcitems);
			for (int i=0; i<9; i++) {
				if (srcitems[i].isEmpty() || dstitems[i].isEmpty())
					continue; // empty
				if (!dstitems[i].intersects(cliprect))
					continue; // out of bounds
                //CRLog::trace("cliprect (%d, %d, %d, %d)", cliprect.left, cliprect.top, cliprect.right, cliprect.bottom);
                //CRLog::trace("nine-patch[%d] (%d, %d, %d, %d) -> (%d, %d, %d, %d)", i, srcitems[i].left, srcitems[i].top, srcitems[i].right, srcitems[i].bottom, dstitems[i].left, dstitems[i].top, dstitems[i].right, dstitems[i].bottom);
				// visible
				lvRect * clip = dstitems[i].clipBy(cliprect); // probably, should be clipped
                LVGLAddSceneItem(new GLDrawImageSceneItem(img.get(), dstitems[i].left, GetHeight() - dstitems[i].top, dstitems[i].width(), dstitems[i].height(), srcitems[i].left, srcitems[i].top, srcitems[i].width(), srcitems[i].height(), 0xFFFFFF, 0, clip, 0));
			}
		}
	}
}

class GLDrawTextureItem : public GLSceneItem {
	int textureId;
	int dstx0;
	int dsty0;
	int dstx1;
	int dsty1;
	float srcx0;
	float srcy0;
	float srcx1;
	float srcy1;
	lUInt32 color;
public:
	GLDrawTextureItem(int _textureId, int _dstx0, int _dsty0, int _dstx1, int _dsty1, float _srcx0, float _srcy0, float _srcx1, float _srcy1, lUInt32 _color)
	: textureId(_textureId),
	  dstx0(_dstx0), dsty0(_dsty0),
	  dstx1(_dstx1), dsty1(_dsty1),
	  srcx0(_srcx0), srcy0(_srcy0),
	  srcx1(_srcx1), srcy1(_srcy1),
	  color(_color)
	{

	}
    virtual void draw() {
    	GLfloat vertices[] = {dstx0,dsty0,0, dstx0,dsty1,0, dstx1,dsty1,0, dstx0,dsty0,0, dstx1,dsty1,0, dstx1,dsty0,0};
    	GLfloat texcoords[] = {srcx0,srcy0, srcx0,srcy1, srcx1,srcy1, srcx0,srcy0, srcx1,srcy1, srcx1,srcy0};
    	//GLfloat colors[6 * 4];
    	//LVGLFillColor(color, colors, 6);
    	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    	//LVGLSetColor(0xFFFFFF);
    	glColor4f(1,1,1,1);
    	glActiveTexture(GL_TEXTURE0);
        checkError("glActiveTexture");
        glBindTexture(GL_TEXTURE_2D, textureId);
        checkError("glBindTexture");
        glEnable(GL_TEXTURE_2D);
        checkError("glEnable(GL_TEXTURE_2D)");
        glEnableClientState(GL_VERTEX_ARRAY);
        checkError("glEnableClientState(GL_VERTEX_ARRAY)");
        //glEnableClientState(GL_COLOR_ARRAY);
    	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        checkError("glEnableClientState(GL_TEXTURE_COORD_ARRAY)");
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        checkError("glVertexPointer(3, GL_FLOAT, 0, vertices)");
        //glColorPointer(4, GL_FLOAT, 0, colors);
    	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
        checkError("glTexCoordPointer(2, GL_FLOAT, 0, texcoords)");

    	glDrawArrays(GL_TRIANGLES, 0, 6);
        checkError("glDrawArrays");

    	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    	//glDisableClientState(GL_COLOR_ARRAY);
    	glDisableClientState(GL_VERTEX_ARRAY);
    	glDisable(GL_TEXTURE_2D);
        checkError("glDisable(GL_TEXTURE_2D)");
    }
};

//void LVGLDrawTexture(int textureId, int dstx0, int dsty0, int dstx1, int dsty1, float srcx0, float srcy0, float srcx1, float srcy1, lUInt32 color) {
//	GLfloat vertices[] = {dstx0,dsty0,0, dstx0,dsty1,0, dstx1,dsty1,0, dstx0,dsty0,0, dstx1,dsty1,0, dstx1,dsty0,0};
//	GLfloat texcoords[] = {srcx0,srcy0, srcx0,srcy1, srcx1,srcy1, srcx0,srcy0, srcx1,srcy1, srcx1,srcy0};
//	GLfloat colors[6 * 4];
//	LVGLFillColor(color, colors, 6);
//	glActiveTexture(GL_TEXTURE0);
//	glEnable(GL_TEXTURE_2D);
//	glEnableClientState(GL_VERTEX_ARRAY);
//	//glEnableClientState(GL_COLOR_ARRAY);
//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glVertexPointer(3, GL_FLOAT, 0, vertices);
//	//glColorPointer(4, GL_FLOAT, 0, colors);
//	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
//
//	glDrawArrays(GL_TRIANGLES, 0, 6);
//
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	//glDisableClientState(GL_COLOR_ARRAY);
//	glDisableClientState(GL_VERTEX_ARRAY);
//	glDisable(GL_TEXTURE_2D);
//}

/// draws buffer content to another buffer doing color conversion if necessary
void GLDrawBuf::DrawTo( LVDrawBuf * buf, int x, int y, int options, lUInt32 * palette )
{
	GLDrawBuf * glbuf = dynamic_cast<GLDrawBuf*>(buf);
	if (glbuf) {
		if (_textureBuf && _textureId != 0) {
			if (glbuf->_scene)
				glbuf->_scene->add(new GLDrawTextureItem(_textureId, x, glbuf->_dy - y - _dy, x + _dx, glbuf->_dy - y, 0, 0, _dx / (float)_tdx, _dy / (float)_tdy, 0xFFFFFF));
		} else {
			CRLog::error("GLDrawBuf::DrawTo() - no texture buffer!");
		}
	} else {
		CRLog::error("GLDrawBuf::DrawTo() is not implemented for non-GL draw buffer targets");
	}
}

/// draws rescaled buffer content to another buffer doing color conversion if necessary
void GLDrawBuf::DrawRescaled(LVDrawBuf * src, int x, int y, int dx, int dy, int options)
{
    if (dx <= 0 || dy <= 0 || !src)
        return;
    GLDrawBuf * glbuf = dynamic_cast<GLDrawBuf*>(src);
	if (glbuf) {
		if (glbuf->_textureBuf && glbuf->_textureId != 0) {
			if (_scene)
				_scene->add(new GLDrawTextureItem(glbuf->_textureId, x, _dy - y - dy, x + dx, _dy - y, 0, 0, glbuf->_dx / (float)glbuf->_tdx, glbuf->_dy / (float)glbuf->_tdy, 0xFFFFFF));
		} else {
			CRLog::error("GLDrawBuf::DrawRescaled() - no texture buffer!");
		}
	} else {
        GLImageCacheItem * item = glImageCache->get(src);
        if (item == NULL)
            item = glImageCache->set(src);
        if (item != NULL) {
            lvRect cliprect;
            GetClipRect(&cliprect);
            lvRect rc;
            rc.left = x;
            rc.top = y;
            rc.right = x + dx;
            rc.bottom = y + dy;
            if (!rc.intersects(cliprect))
                return; // out of bounds
            lvRect * clip = rc.clipBy(cliprect); // probably, should be clipped
            LVGLAddSceneItem(new GLDrawImageSceneItem(src, x, GetHeight() - y, dx, dy, 0, 0, src->GetWidth(), src->GetHeight(), 0xFFFFFF, 0, clip, 0));
        }
	}
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
		// generate IDs
		//CRLog::debug("GLDrawBuf::createFramebuffer %dx%d", _tdx, _tdy);
		glGenTextures(1, &_textureId);
		if (checkError("createFramebuffer glGenTextures")) return;
		glGenFramebuffersOES(1, &_framebufferId);
		if (checkError("createFramebuffer glGenFramebuffersOES")) return;
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebufferId);
		if (checkError("createFramebuffer glBindFramebuffer")) return;

		glBindTexture(GL_TEXTURE_2D, _textureId);
        checkError("glBindTexture(GL_TEXTURE_2D, _textureId)");
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _tdx, _tdy, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
		checkError("glTexImage2D");

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        checkError("texParameter");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        checkError("texParameter");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        checkError("texParameter");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        checkError("texParameter");

		glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, _textureId, 0);
		checkError("glFramebufferTexture2DOES");
		// Always check that our framebuffer is ok
		if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
			CRLog::error("glFramebufferTexture2DOES failed");
		}
        checkError("glCheckFramebufferStatusOES");
        //glClearColor(0.5f, 0, 0, 1);
        glClearColor(0, 0, 0, 0);
        checkError("glClearColor");
        glClear(GL_COLOR_BUFFER_BIT);
        checkError("glClear");
    }
}

void GLDrawBuf::deleteFramebuffer()
{
	if (_textureBuf) {
		//CRLog::debug("GLDrawBuf::deleteFramebuffer");
		if (_textureId != 0) {
            if (glIsTexture(_textureId) != GL_TRUE) {
                CRLog::error("Invalid texture %d", _textureId);
                return;
            }
            glDeleteTextures(1, &_textureId);
			checkError("deleteFramebuffer - glDeleteTextures");
		}
		if (_framebufferId != 0) {
			glBindFramebufferOES( GL_FRAMEBUFFER_OES, 0);
            checkError("deleteFramebuffer - glBindFramebufferOES");
            glDeleteFramebuffersOES(1, &_framebufferId);
			checkError("deleteFramebuffer - glDeleteFramebuffer");
		}
		_textureId = 0;
		_framebufferId = 0;
	}
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


void GLDrawBuf::beforeDrawing()
{
	if (_prepareStage++ == 0) {
		if (_textureBuf) {
			if (_textureId == 0 || _framebufferId == 0) {
				createFramebuffer();
			}
			//CRLog::debug("Setting render to texture");
			glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebufferId);
			if (checkError("beforeDrawing glBindFramebufferOES")) return;
		}
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        checkError("glPushMatrix");
		glLoadIdentity();
		myGlOrtho(0, _dx, 0, _dy, -1.0f, 5.0f);
		//glOrthof(0, _dx, 0, _dy, -1.0f, 1.0f);
		glViewport(0,0,_dx,_dy);
        checkError("glViewport");
        _scene = LVGLPushScene(new GLScene());
		glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        checkError("glPushMatrix");
        glLoadIdentity();
    } else {
        CRLog::warn("Duplicate beforeDrawing/afterDrawing");
    }
}

void GLDrawBuf::afterDrawing()
{
    if (--_prepareStage == 0) {
		if (_scene) {
			_scene->draw();
			_scene->clear();
            GLScene * s = LVGLPopScene();
            if (s != _scene) {
				CRLog::error("Current scene does not match");
			}
			delete _scene;
			_scene = NULL;
            if (!LVGLPeekScene()) {
                glImageCache->removeDeletedItems();
            }
        }
		if (_textureBuf) {
			//bind the base framebuffer
			//CRLog::debug("Finished render to texture");
			glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
			checkError("afterDrawing - glBindFramebuffer");
		}
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        checkError("glPopMatrix");
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        checkError("glPopMatrix");
    } else {
        CRLog::warn("Duplicate beforeDrawing/afterDrawing");
    }
}

/// create drawing texture of specified size
GLDrawBuf::GLDrawBuf(int width, int height, int bpp, bool useTexture)
: _dx(width), _dy(height),
		_tdx(nearestPOT(width)), _tdy(nearestPOT(height)), _bpp(bpp),
		_hidePartialGlyphs(false), _clipRect(0, 0, width, height),
		_textColor(0x000000), _backgroundColor(0xFFFFFF),
		_textureBuf(useTexture), _textureId(0), _framebufferId(0),
		//_renderbufferId(0),
		_prepareStage(0),
		_scene(NULL)
{
    //if (_textureBuf) CRLog::trace("GLDrawBuf::GLDrawBuf");
}

/// destructor
GLDrawBuf::~GLDrawBuf()
{
    //if (_textureBuf) CRLog::trace("GLDrawBuf::~GLDrawBuf");
    deleteFramebuffer();
}

