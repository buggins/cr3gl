/*
 * glfont.h
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#ifndef GLFONT_H_
#define GLFONT_H_

#include <lvtypes.h>
#include <lvfntman.h>
#include <lvhashtable.h>

// factory
bool LVInitGLFontManager(LVFontManager * base);

class GLFont;
class GLGlyphCachePage;

// GLFont glyph cache item
struct GLGlyphCacheItem {
	lInt8   originX;     ///< X origin for glyph
	lInt8   originY;     ///< Y origin for glyph
	lInt8   dx;          ///< width of glyph
	lInt8   dy;          ///< height of glyph
	lUInt8  width;       ///< full width of glyph (advance)
	// texture coordinates
	float x0;
	float y0;
	float x1;
	float y1;
	GLFont * font;
	GLGlyphCachePage * page;
	void draw(int x, int y, lUInt32 color, lvRect * clip);
};

class GLGlyphCache {
	LVHashTable<lUInt64, GLGlyphCacheItem*> _map;
	LVPtrVector<GLGlyphCachePage, true> _pages;
public:
	GLGlyphCache();
	~GLGlyphCache();
	/// remove all glyphs and textures
	void clear();
	/// remove glyphs for specified font
	void clearFontGlyphs(GLFont * font);
	/// get glyph for specified character of font, returns NULL if not found
	GLGlyphCacheItem * get(lChar16 ch, GLFont * font);
	/// stores glyph for specified character of font to cache
	GLGlyphCacheItem * put(lChar16 ch, GLFont * font, LVFontGlyphCacheItem * glyph);
};



#endif /* GLFONT_H_ */
