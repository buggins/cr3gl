/*
 * glfont.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#include <crengine.h>
#include "glfont.h"

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

/// font manager interface class
class GLFontManager : public LVFontManager
{
protected:
	LVFontManager * _base;
	LVHashTable<LVFont *, LVFontRef> _mapByBase;
	LVHashTable<LVFont *, LVFontRef> _mapByGl;
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

};

bool LVInitGLFontManager(LVFontManager * base) {
    if (fontMan && fontMan != base) {
        delete fontMan;
    }
    fontMan = new GLFontManager(base);
    return true;
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
                       lUInt32 flags=0, int letter_spacing=0 ) {
    	// TODO: implement OpenGL related behavior
    	_base->DrawTextString(buf, x, y, text, len, def_char, palette, addHyphen, flags, letter_spacing);
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
	for (;;) {
		LVHashTable<LVFont *, LVFontRef>::pair * item = iter.next();
		if (!item)
			break;
		if (item->value.getRefCount() <= 1) {
			removeFontInstance(item->value.get());
		}
	}
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
	CRLog::debug("Created GL Font Manager");
}

/// destructor
GLFontManager::~GLFontManager()
{
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
