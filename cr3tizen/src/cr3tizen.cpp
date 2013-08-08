/*
 * cr3tizen.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#include "tizenx.h"
#include "cr3tizen.h"
#include "lvstring.h"
#include "glfont.h"
#include "gldrawbuf.h"


class CRTizenLogger : public CRLog
{
protected:
    virtual void log( const char * level, const char * msg, va_list args )
    {
    	char buf[4096];
    	vsnprintf(buf, 4095, msg, args);
        if (!strcmp("ERROR", level))
        	AppLogExceptionInternal("", 0, "%s", buf);
        else if (!strcmp("INFO", level))
        	AppLogInternal("", 0, "%s", buf);
        else
        	AppLogDebugInternal("", 0, "%s", buf);
    }
public:
    CRTizenLogger()
    {
        static const char * log_level_names[] = {
        "FATAL",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "TRACE",
        };
        info( "Started logging. Level=%s", log_level_names[getLogLevel()] );
    }

    virtual ~CRTizenLogger() {
    }
};

void LVSetTizenLogger() {
	CRLog::setLogger(new CRTizenLogger());
}

void LVInitCoolReaderTizen() {
	LVSetTizenLogger();
	CRLog::info("Starting CoolReader");
	CRLog::setLogLevel(CRLog::LL_TRACE);
	InitFontManager(lString8());
	LVInitGLFontManager(fontMan);
	fontMan->RegisterFont(lString8("/usr/share/fonts/TizenSansMeduim.ttf"));
	fontMan->RegisterFont(lString8("/usr/share/fonts/TizenSansRegular.ttf"));
	fontMan->RegisterFont(lString8("/usr/share/fallback_fonts/TizenSansFallback.ttf"));
	fontMan->SetFallbackFontFace(lString8("Tizen Sans Fallback"));
}