/*
 * glfont.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#include <gl.h>
#include <glext.h>
#include <crengine.h>
#include "glfont.h"
#include "glscene.h"
#include "gldrawbuf.h"

#if (USE_FREETYPE==1)

//#include <ft2build.h>

#ifdef ANDROID
#include "freetype/config/ftheader.h"
#include "freetype/freetype.h"
#else

#include <freetype/config/ftheader.h>
//#include FT_FREETYPE_H
#include <freetype/freetype.h>
#endif

#if (USE_FONTCONFIG==1)
    #include <fontconfig/fontconfig.h>
#endif

#endif

static bool checkError(const char * context) {
	if (glGetError() != GL_NO_ERROR) {
		CRLog::error("GLFont : GL Error at %s", context);
		return true;
	}
	return false;
}

class GLGlyphCachePage;
class GLGlyphCache;
class GLFont;

#define GL_GLYPH_CACHE_PAGE_SIZE 1024
class GLGlyphCachePage {
	GLGlyphCache * cache;
	LVGrayDrawBuf * drawbuf;
	int currentLine;
	int nextLine;
	int x;
	bool closed;
	bool needUpdateTexture;
	GLuint textureId;
public:
	GLGlyphCache * getCache() { return cache; }
	GLGlyphCachePage(GLGlyphCache * pcache) : cache(pcache), drawbuf(NULL), closed(false), needUpdateTexture(false), textureId(0) {
		// create drawing buffer
		//drawbuf = new LVGrayDrawBuf(GL_GLYPH_CACHE_PAGE_SIZE, GL_GLYPH_CACHE_PAGE_SIZE, 8, NULL);
		// init free lines
		currentLine = nextLine = x = 0;
	}
	virtual ~GLGlyphCachePage() {
		if (drawbuf)
			delete drawbuf;
		if (textureId != 0)
			glDeleteTextures(1, &textureId);
	}
	void updateTexture() {
		if (drawbuf == NULL)
			return; // no draw buffer!!!
	    if (textureId == 0) {
	    	CRLog::debug("updateTexture - new texture");
			glGenTextures(1, &textureId);
			if (glGetError() != GL_NO_ERROR)
				return;
	    }
    	CRLog::debug("updateTexture - setting image %dx%d", drawbuf->GetWidth(), drawbuf->GetHeight());
	    glBindTexture(GL_TEXTURE_2D, textureId);
	    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, drawbuf->GetWidth(), drawbuf->GetHeight(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, drawbuf->GetScanLine(0));
	    //glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, drawbuf->GetWidth(), drawbuf->GetHeight(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, drawbuf->GetScanLine(0));
//#define DP(x) (drawbuf->GetPixel(x,0)!=0?'1':'0')
    	//CRLog::debug("%c%c%c%c%c%c%c%c%c%c", DP(0),DP(1),DP(2),DP(3),DP(4),DP(5),DP(6),DP(7),DP(8),DP(9));
	    //glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, drawbuf->GetWidth(), drawbuf->GetHeight(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, drawbuf->GetScanLine(0));
	    checkError("updateTexture - glTexImage2D");
	    if (glGetError() != GL_NO_ERROR) {
	        glDeleteTextures(1, &textureId);
	        return;
	    }
	    needUpdateTexture = false;
	    if (closed) {
	    	delete drawbuf;
	    	drawbuf = NULL;
	    }
	}
	void drawItem(GLGlyphCacheItem * item, int x, int y, lUInt32 color, lvRect * clip) {
		if (needUpdateTexture)
			updateTexture();
		if (textureId != 0) {
			//CRLog::trace("drawing character at %d,%d", x, y);
			float dstx0 = x;
			float dsty0 = y;
			float dstx1 = x + (item->dx);
			float dsty1 = y - (item->dy);
			float srcx0 = item->x0;
			float srcy0 = item->y0;
			float srcx1 = item->x1;
			float srcy1 = item->y1;
			if (clip) {
				// correct clipping
				float txpp = 1 / 1024.0f; // texture coordinates per pixel
				dstx0 += clip->left;
				srcx0 += clip->left * txpp;
				dstx1 -= clip->right;
				srcx1 -= clip->right * txpp;
				dsty0 -= clip->top;
				srcy0 += clip->top * txpp;
				dsty1 += clip->bottom;
				srcy1 -= clip->bottom * txpp;
			}
	    	GLfloat vertices[] = {dstx0,dsty0,0, dstx0,dsty1,0, dstx1,dsty1,0, dstx0,dsty0,0, dstx1,dsty1,0, dstx1,dsty0,0};
	    	GLfloat texcoords[] = {srcx0,srcy0, srcx0,srcy1, srcx1,srcy1, srcx0,srcy0, srcx1,srcy1, srcx1,srcy0};

	    	LVGLSetColor(color);
	    	glActiveTexture(GL_TEXTURE0);
	    	glEnable(GL_TEXTURE_2D);
	    	glBindTexture(GL_TEXTURE_2D, textureId);

	    	glEnable(GL_BLEND);
	    	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	    	glEnableClientState(GL_VERTEX_ARRAY);
	    	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	    	glVertexPointer(3, GL_FLOAT, 0, vertices);
	    	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

	    	glDrawArrays(GL_TRIANGLES, 0, 6);

	    	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	    	glDisableClientState(GL_VERTEX_ARRAY);
	    	glDisable(GL_TEXTURE_2D);
	    	glDisable(GL_BLEND);
		}
	}
	GLGlyphCacheItem * addItem(GLFont * font, LVFontGlyphCacheItem * glyph) {
		if (closed)
			return NULL;
		// next line if necessary
		if (x + glyph->bmp_width > GL_GLYPH_CACHE_PAGE_SIZE) {
			// move to next line
			currentLine = nextLine;
			x = 0;
		}
		// check if no room left for glyph height
		if (currentLine + glyph->bmp_height > GL_GLYPH_CACHE_PAGE_SIZE) {
			closed = true;
			updateTexture();
			return NULL;
		}
		GLGlyphCacheItem * cacheItem = new GLGlyphCacheItem();
		cacheItem->x0 = x / (float)GL_GLYPH_CACHE_PAGE_SIZE;
		cacheItem->y0 = currentLine / (float)GL_GLYPH_CACHE_PAGE_SIZE;
		cacheItem->x1 = (x + glyph->bmp_width) / (float)GL_GLYPH_CACHE_PAGE_SIZE;
		cacheItem->y1 = (currentLine + glyph->bmp_height) / (float)GL_GLYPH_CACHE_PAGE_SIZE;
		cacheItem->dx = glyph->bmp_width;
		cacheItem->dy = glyph->bmp_height;
		cacheItem->originX = glyph->origin_x;
		cacheItem->originY = glyph->origin_y;
		cacheItem->width = glyph->advance;
		cacheItem->page = this;
		cacheItem->font = font;
		// draw glyph to buffer, if non empty
		if (glyph->bmp_height && glyph->bmp_width) {
			if (nextLine < currentLine + glyph->bmp_height)
				nextLine = currentLine + glyph->bmp_height;
			if (!drawbuf) {
				drawbuf = new LVGrayDrawBuf(GL_GLYPH_CACHE_PAGE_SIZE, GL_GLYPH_CACHE_PAGE_SIZE, 8, NULL);
				drawbuf->SetBackgroundColor(0x000000);
				drawbuf->SetTextColor(0xFFFFFF);
				drawbuf->Clear(0x000000);
			}
//#define BCH(x) (glyph->bmp[x] > 127 ?'1':'0')
//			CRLog::debug("Adding new glyph at %d,%d: %c%c%c%c%c%c%c%c", x, currentLine, BCH(0),BCH(1),BCH(2),BCH(3),BCH(4),BCH(5),BCH(6),BCH(7) );
			drawbuf->Draw(x, currentLine, glyph->bmp, glyph->bmp_width, glyph->bmp_height, NULL);
			x += glyph->bmp_width;
			needUpdateTexture = true;
		}
		return cacheItem;
	}
};

void GLGlyphCacheItem::draw(int x, int y, lUInt32 color, lvRect * clip) {
	page->drawItem(this, x, y, color, clip);
}

//============================================================================================
// GLGlyphCache implementation
#define LVGLMakeGlyphKey(ch, font) ((((lUInt64)ch) << 32) ^ ((lUInt64)font))

void GLGlyphCache::clear() {
	LVHashTable<lUInt64, GLGlyphCacheItem*>::iterator iter = _map.forwardIterator();
	for (;;) {
		LVHashTable<lUInt64, GLGlyphCacheItem*>::pair * item = iter.next();
		if (!item)
			break;
		delete item->value;
		item->value = NULL;
	}
	_map.clear();
}
void GLGlyphCache::clearFontGlyphs(GLFont * font) {
	LVHashTable<lUInt64, GLGlyphCacheItem*>::iterator iter = _map.forwardIterator();
	LVArray<lUInt64> keysForRemove(256, 0);
	for (;;) {
		LVHashTable<lUInt64, GLGlyphCacheItem*>::pair * item = iter.next();
		if (!item)
			break;
		if (item->value) {
			if (item->value->font == font)
				keysForRemove.add(item->key);
		}
	}
	for (int i = 0; i<keysForRemove.length(); i++) {
		GLGlyphCacheItem* removed = _map.get(keysForRemove[i]);
		_map.remove(keysForRemove[i]);
		if (removed)
			delete removed;
	}
}
GLGlyphCache::GLGlyphCache() : _map(32768) {

}
GLGlyphCache::~GLGlyphCache() {

}
GLGlyphCacheItem * GLGlyphCache::get(lChar16 ch, GLFont * font) {
	GLGlyphCacheItem *  res = _map.get(LVGLMakeGlyphKey(ch, font));
	return res;
}
GLGlyphCacheItem * GLGlyphCache::put(lChar16 ch, GLFont * font, LVFontGlyphCacheItem * glyph) {
	GLGlyphCachePage * page;
	if (_pages.length() == 0) {
		page = new GLGlyphCachePage(this);
		_pages.add(page);
	}
	page = _pages[_pages.length() - 1];
	GLGlyphCacheItem * item = page->addItem(font, glyph);
	if (!item) {
		page = new GLGlyphCachePage(this);
		_pages.add(page);
		item = page->addItem(font, glyph);
	}
	_map.set(LVGLMakeGlyphKey(ch, font), item);
	return item;
}


//=======================================================================================================
/// font manager interface class
class GLFontManager : public LVFontManager
{
protected:
	LVFontManager * _base;
	LVHashTable<LVFont *, LVFontRef> _mapByBase;
	LVHashTable<LVFont *, LVFontRef> _mapByGl;
	GLGlyphCache * _cache;
public:
    /// garbage collector frees unused fonts
    virtual void gc();
    /// returns most similar font
    virtual LVFontRef GetFont(int size, int weight, bool italic, css_font_family_t family, lString8 typeface, int documentId = -1);
    /// set fallback font face (returns true if specified font is found)
    virtual bool SetFallbackFontFace( lString8 face );
    /// get fallback font face (returns empty string if no fallback font is set)
    virtual lString8 GetFallbackFontFace();
    /// returns fallback font for specified size
    virtual LVFontRef GetFallbackFont(int /*size*/);
    /// registers font by name
    virtual bool RegisterFont( lString8 name );
    /// registers document font
    virtual bool RegisterDocumentFont(int /*documentId*/, LVContainerRef /*container*/, lString16 /*name*/, lString8 /*face*/, bool /*bold*/, bool /*italic*/);
    /// unregisters all document fonts
    virtual void UnregisterDocumentFonts(int /*documentId*/);
    /// initializes font manager
    virtual bool Init( lString8 path );
    /// get count of registered fonts
    virtual int GetFontCount();
    /// get hash of installed fonts and fallback font
    virtual lUInt32 GetFontListHash(int /*documentId*/);
    /// clear glyph cache
    virtual void clearGlyphCache();

    /// get antialiasing mode
    virtual int GetAntialiasMode();
    /// set antialiasing mode
    virtual void SetAntialiasMode( int mode );

    /// get kerning mode: true==ON, false=OFF
    virtual bool getKerning();

    /// get kerning mode: true==ON, false=OFF
    virtual void setKerning( bool kerningEnabled );

    /// notification from GLFont - instance is going to close
	virtual void removeFontInstance(LVFont * glFont);

    /// constructor
    GLFontManager(LVFontManager * base);
    /// destructor
    virtual ~GLFontManager();

    /// returns available typefaces
    virtual void getFaceList( lString16Collection & );

    /// sets current hinting mode
    virtual void SetHintingMode(hinting_mode_t /*mode*/);
    /// returns current hinting mode
    virtual hinting_mode_t  GetHintingMode();

    GLGlyphCache * getCache() {
    	return _cache;
    }

};

bool LVInitGLFontManager(LVFontManager * base) {
    if (fontMan && fontMan != base) {
        delete fontMan;
    }
    fontMan = new GLFontManager(base);
    return true;
}

class GLCharGlyphItem : public GLSceneItem {
	GLGlyphCacheItem * item;
	int x;
	int y;
	lUInt32 color;
	lvRect * clip;
public:
	GLCharGlyphItem(GLGlyphCacheItem * _item, int _x, int _y, lUInt32 _color, lvRect * _clip)
	: item(_item), x(_x), y(_y), color(_color), clip(_clip)
	{
	}
	virtual void draw() {
		item->draw(x, y, color, clip);
	}
	virtual ~GLCharGlyphItem() {
		if (clip)
			delete clip;
	}
};

/// returns non-NULL pointer to trimming values for 4 sides of rc, if clipping is necessary
lvRect * calcClipping(lvRect & rc, lvRect & cliprc) {
	if (rc.intersects(cliprc) && !cliprc.isRectInside(rc)) {
		lvRect * res = new lvRect();
		if (cliprc.left > rc.left)
			res->left = cliprc.left - rc.left;
		if (cliprc.top > rc.top)
			res->top = cliprc.top - rc.top;
		if (rc.right > cliprc.right)
			res->right = rc.right - cliprc.right;
		if (rc.bottom > cliprc.bottom)
			res->bottom = rc.bottom - cliprc.bottom;
		return res;
	} else {
		return NULL;
	}
}

/** \brief base class for fonts

    implements single interface for font of any engine
*/
class GLFont : public LVFont
{
	LVFontRef _base;
	GLFontManager * _fontMan;
public:

	GLFont(LVFontRef baseFont, GLFontManager * manager) {
		_base = baseFont;
		_fontMan = manager;
	}

	/// hyphenation character
    virtual lChar16 getHyphChar() { return _base->getHyphChar(); }

    /// hyphen width
    virtual int getHyphenWidth() { return _base->getHyphenWidth(); }

    /**
     * Max width of -/./,/!/? to use for visial alignment by width
     */
    virtual int getVisualAligmentWidth() { return _base->getVisualAligmentWidth(); }

    /** \brief get glyph info
        \param glyph is pointer to glyph_info_t struct to place retrieved info
        \return true if glyh was found
    */
    virtual bool getGlyphInfo( lUInt16 code, glyph_info_t * glyph, lChar16 def_char=0 ) { return _base->getGlyphInfo(code, glyph, def_char); }

    /** \brief measure text
        \param text is text string pointer
        \param len is number of characters to measure
        \param max_width is maximum width to measure line
        \param def_char is character to replace absent glyphs in font
        \param letter_spacing is number of pixels to add between letters
        \return number of characters before max_width reached
    */
    virtual lUInt16 measureText(
                        const lChar16 * text, int len,
                        lUInt16 * widths,
                        lUInt8 * flags,
                        int max_width,
                        lChar16 def_char,
                        int letter_spacing=0,
                        bool allow_hyphenation=true
                     ) {
    	return _base->measureText(text, len, widths, flags, max_width, def_char, letter_spacing, allow_hyphenation);
    }
    /** \brief measure text
        \param text is text string pointer
        \param len is number of characters to measure
        \return width of specified string
    */
    virtual lUInt32 getTextWidth(
                        const lChar16 * text, int len
        ) {
    	return _base->getTextWidth(text, len);
    }

//    /** \brief get glyph image in 1 byte per pixel format
//        \param code is unicode character
//        \param buf is buffer [width*height] to place glyph data
//        \return true if glyph was found
//    */
//    virtual bool getGlyphImage(lUInt16 code, lUInt8 * buf, lChar16 def_char=0) = 0;
    /** \brief get glyph item
        \param code is unicode character
        \return glyph pointer if glyph was found, NULL otherwise
    */
    virtual LVFontGlyphCacheItem * getGlyph(lUInt16 ch, lChar16 def_char=0) {
    	return _base->getGlyph(ch, def_char);
    }
    /// returns font baseline offset
    virtual int getBaseline() {
    	return _base->getBaseline();
    }
    /// returns font height including normal interline space
    virtual int getHeight() const {
    	return _base->getHeight();
    }
    /// returns font character size
    virtual int getSize() const {
    	return _base->getSize();
    }
    /// returns font weight
    virtual int getWeight() const {
    	return _base->getWeight();
    }
    /// returns italic flag
    virtual int getItalic() const {
    	return _base->getItalic();
    }
    /// returns char width
    virtual int getCharWidth( lChar16 ch, lChar16 def_char=0 ) {
    	return _base->getCharWidth(ch, def_char);
    }
    /// retrieves font handle
    virtual void * GetHandle() {
    	return _base->GetHandle();
    }
    /// returns font typeface name
    virtual lString8 getTypeFace() const {
    	return _base->getTypeFace();
    }
    /// returns font family id
    virtual css_font_family_t getFontFamily() const {
    	return _base->getFontFamily();
    }
    /// draws text string
    virtual void DrawTextString( LVDrawBuf * buf, int x, int y,
                       const lChar16 * text, int len,
                       lChar16 def_char, lUInt32 * palette = NULL, bool addHyphen = false,
                       lUInt32 flags=0, int letter_spacing=0 )
    {
		if (len <= 0)
			return;
    	GLDrawBuf * glbuf = dynamic_cast<GLDrawBuf *>(buf);
    	if (glbuf) {
    		// use specific rendering for GL buffer
        	GLGlyphCache * cache = _fontMan->getCache();
        	GLScene * scene = LVGLPeekScene();
        	if (!scene) {
        		CRLog::error("DrawTextString - no current scene");
        		return;
        	}
			if (letter_spacing < 0 || letter_spacing > 50)
				letter_spacing = 0;
			lvRect clip;
			buf->GetClipRect( &clip );
			int _height = _base->getHeight();
			int _size = _base->getSize();
			int _baseline = _base->getBaseline();
			lUInt32 color = glbuf->GetTextColor();
			if ( y + _height < clip.top || y >= clip.bottom )
				return;

	#if (ALLOW_KERNING==1)
			bool use_kerning = _base->kerningEnabled();
	#endif
			int i;

			lChar16 previous = 0;
			//lUInt16 prev_width = 0;
			lChar16 ch;
			// measure character widths
			bool isHyphen = false;
			int x0 = x;
			for ( i=0; i<=len; i++) {
				if ( i==len && (!addHyphen || isHyphen) )
					break;
				if ( i<len ) {
					ch = text[i];
					if ( ch=='\t' )
						ch = ' ';
					isHyphen = (ch==UNICODE_SOFT_HYPHEN_CODE) && (i<len-1);
				} else {
					ch = UNICODE_SOFT_HYPHEN_CODE;
					isHyphen = 0;
				}
				int kerning = 0;
	#if (ALLOW_KERNING==1)
				kerning = _base->getKerning(previous, ch, def_char);
	#endif
				GLGlyphCacheItem * item = cache->get(ch, this);
				if (!item) {
					LVFontGlyphCacheItem * glyph = getGlyph(ch, def_char);
					if (glyph) {
						item = cache->put(ch, this, glyph);
					}
				}
				if ( !item )
					continue;
				if ( (item && !isHyphen) || i>=len-1 ) { // avoid soft hyphens inside text string
					int w = item->width + (kerning >> 6);
					lvRect rc;
					rc.left = x + (kerning>>6) + item->originX;
					rc.top = (y + _baseline - item->originY);
					rc.right = rc.left + item->dx;
					rc.bottom = rc.top + item->dy;
//					clip.top = glbuf->GetHeight() - clip.top;
//					clip.bottom = glbuf->GetHeight() - clip.bottom;
					if (clip.intersects(rc)) {
						lvRect * clipInfo = NULL;
						if (!clip.isRectInside(rc))
							clipInfo = calcClipping(rc, clip);
						scene->add(new GLCharGlyphItem(item,
								rc.left,
								glbuf->GetHeight() - rc.top,
								color, clipInfo));
					}
					x  += w + letter_spacing;
					previous = ch;
				}
			}
			if ( flags & LTEXT_TD_MASK ) {
				// text decoration: underline, etc.
				int h = _size > 30 ? 2 : 1;
				lUInt32 cl = buf->GetTextColor();
				if ( flags & LTEXT_TD_UNDERLINE || flags & LTEXT_TD_BLINK ) {
					int liney = y + _baseline + h;
					buf->FillRect( x0, liney, x, liney+h, cl );
				}
				if ( flags & LTEXT_TD_OVERLINE ) {
					int liney = y + h;
					buf->FillRect( x0, liney, x, liney+h, cl );
				}
				if ( flags & LTEXT_TD_LINE_THROUGH ) {
	//                int liney = y + _baseline - _size/4 - h/2;
					int liney = y + _baseline - _size*2/7;
					buf->FillRect( x0, liney, x, liney+h, cl );
				}
			}
    	} else {
    		// use base font rendering for non-GL buffers
    		_base->DrawTextString(buf, x, y, text, len, def_char, palette, addHyphen, flags, letter_spacing);
    	}
    }

    /// get bitmap mode (true=monochrome bitmap, false=antialiased)
    virtual bool getBitmapMode() { return _base->getBitmapMode(); }
    /// set bitmap mode (true=monochrome bitmap, false=antialiased)
    virtual void setBitmapMode( bool mode ) { _base->setBitmapMode(mode); }

    /// get kerning mode: true==ON, false=OFF
    virtual bool getKerning() const { return _base->getKerning(); }
    /// get kerning mode: true==ON, false=OFF
    virtual void setKerning( bool kerning) { _base->setKerning(kerning); }

    /// sets current hinting mode
    virtual void setHintingMode(hinting_mode_t mode) { _base->setHintingMode(mode); }
    /// returns current hinting mode
    virtual hinting_mode_t  getHintingMode() const { return _base->getHintingMode(); }

    /// returns true if font is empty
    virtual bool IsNull() const {
    	return _base->IsNull();
    }
    virtual bool operator ! () const {
    	return _base->operator !();
    }
    virtual void Clear() {
    	_base->Clear();
    }
    virtual ~GLFont() { }
    /// set fallback font for this font
    void setFallbackFont( LVFastRef<LVFont> font ) { _base->setFallbackFont(font); }
    /// get fallback font for this font
    LVFont * getFallbackFont() { return _base->getFallbackFont(); }
};



/// garbage collector frees unused fonts
void GLFontManager::gc()
{
	// remove links from hash maps
	LVHashTable<LVFont *, LVFontRef>::iterator iter = _mapByBase.forwardIterator();
	LVArray<LVFont *> keysForRemove(32, 0);
	// prepare list of fonts to free
	for (;;) {
		LVHashTable<LVFont *, LVFontRef>::pair * item = iter.next();
		if (!item)
			break;
		if (item->value.getRefCount() <= 1) {
			keysForRemove.add(item->key);
		}
	}
	// free found unused fonts
	for (int i=0; i<keysForRemove.length(); i++)
		removeFontInstance(keysForRemove[i]);
	// free base font instances
	_base->gc();
}

/// returns most similar font
LVFontRef GLFontManager::GetFont(int size, int weight, bool italic, css_font_family_t family, lString8 typeface, int documentId)
{
	LVFontRef res = _base->GetFont(size, weight, italic, family, typeface, documentId);
	LVFontRef existing = _mapByBase.get(res.get());
	if (!existing) {
		LVFontRef f = LVFontRef(new GLFont(res, this));
		_mapByBase.set(res.get(), f);
		_mapByGl.set(f.get(), res);
		return f;
	} else {
		return existing;
	}
}

/// notification from GLFont - instance is going to close
void GLFontManager::removeFontInstance(LVFont * glFont) {
	LVFont * base = _mapByGl.get(glFont).get();
	_mapByGl.remove(glFont);
	_mapByBase.remove(base);
}

/// set fallback font face (returns true if specified font is found)
bool GLFontManager::SetFallbackFontFace( lString8 face )
{
	return _base->SetFallbackFontFace(face);
}

/// get fallback font face (returns empty string if no fallback font is set)
lString8 GLFontManager::GetFallbackFontFace()
{
	return _base->GetFallbackFontFace();
}

/// returns fallback font for specified size
LVFontRef GLFontManager::GetFallbackFont(int size)
{
	return _base->GetFallbackFont(size);
}

/// registers font by name
bool GLFontManager::RegisterFont( lString8 name )
{
	return _base->RegisterFont(name);
}

/// registers document font
bool GLFontManager::RegisterDocumentFont(int documentId, LVContainerRef container, lString16 name, lString8 face, bool bold, bool italic)
{
	return _base->RegisterDocumentFont(documentId, container, name, face, bold, italic);
}

/// unregisters all document fonts
void GLFontManager::UnregisterDocumentFonts(int documentId)
{
	_base->UnregisterDocumentFonts(documentId);
}

/// initializes font manager
bool GLFontManager::Init( lString8 path )
{
	// nothing to do
	return true;
}

/// get count of registered fonts
int GLFontManager::GetFontCount() {
	return _base->GetFontCount();
}

/// get hash of installed fonts and fallback font
lUInt32 GLFontManager::GetFontListHash(int documentId)
{
	return _base->GetFontListHash(documentId);
}
/// clear glyph cache
void GLFontManager::clearGlyphCache()
{
	_base->clearGlyphCache();
}

/// get antialiasing mode
int GLFontManager::GetAntialiasMode()
{
	return _base->GetAntialiasMode();
}
/// set antialiasing mode
void GLFontManager::SetAntialiasMode( int mode )
{
	_base->SetAntialiasMode(mode);
}

/// get kerning mode: true==ON, false=OFF
bool GLFontManager::getKerning()
{
	return _base->getKerning();
}
/// get kerning mode: true==ON, false=OFF
void GLFontManager::setKerning( bool kerningEnabled )
{
	_base->setKerning(kerningEnabled);
}

/// constructor
GLFontManager::GLFontManager(LVFontManager * base) : LVFontManager(), _base(base), _mapByBase(1000), _mapByGl(1000)
{
	_cache = new GLGlyphCache();
	CRLog::debug("Created GL Font Manager");
}

/// destructor
GLFontManager::~GLFontManager()
{
	delete _cache;
	delete _base;
}

/// returns available typefaces
void GLFontManager::getFaceList( lString16Collection & result)
{
	_base->getFaceList(result);
}


/// sets current hinting mode
void GLFontManager::SetHintingMode(hinting_mode_t mode)
{
	_base->SetHintingMode(mode);
}
/// returns current hinting mode
hinting_mode_t GLFontManager::GetHintingMode() {
	return _base->GetHintingMode();
}
