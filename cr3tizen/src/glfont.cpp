/*
 * glfont.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

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
    if ( fontMan ) {
        delete fontMan;
    }
    fontMan = new GLFontManager(base);
    return true;
}


    /// garbage collector frees unused fonts
void GLFontManager::gc()
{
	_base->gc();
}

/// returns most similar font
LVFontRef GLFontManager::GetFont(int size, int weight, bool italic, css_font_family_t family, lString8 typeface, int documentId)
{
	LVFontRef res = _base->GetFont(size, weight, italic, family, typeface, documentId);
	return res;
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
	return _base->UnregisterDocumentFonts(documentId);
}

/// initializes font manager
bool GLFontManager::Init( lString8 path )
{
	// nothing to do
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
GLFontManager::GLFontManager(LVFontManager * base) : _base(base)
{
	//
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
