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
#include "crui.h"

using namespace CRUI;

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

void LVInitCoolReaderTizen(const wchar_t * resourceDir) {
	LVSetTizenLogger();
	CRLog::info("Starting CoolReader");
	CRLog::setLogLevel(CRLog::LL_TRACE);
	InitFontManager(lString8());
	LVInitGLFontManager(fontMan);
	fontMan->RegisterFont(lString8("/usr/share/fonts/TizenSansMeduim.ttf"));
	fontMan->RegisterFont(lString8("/usr/share/fonts/TizenSansRegular.ttf"));
	fontMan->RegisterFont(lString8("/usr/share/fallback_fonts/TizenSansFallback.ttf"));
	fontMan->SetFallbackFontFace(lString8("Tizen Sans Fallback"));
	lString8Collection dirs;
	//dirs.add(UnicodeToUtf8(resourceDir));
	dirs.add(UnicodeToUtf8(resourceDir) + "screen-density-xhigh");
	LVCreateResourceResolver(dirs);
	LVGLCreateImageCache();

	currentTheme = new CRUITheme(lString8("BLACK"));
	currentTheme->setTextColor(0x000000);
	currentTheme->setFontForSize(CRUI::FONT_SIZE_XSMALL, fontMan->GetFont(16, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setFontForSize(CRUI::FONT_SIZE_SMALL, fontMan->GetFont(20, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setFontForSize(CRUI::FONT_SIZE_MEDIUM, fontMan->GetFont(26, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setFontForSize(CRUI::FONT_SIZE_LARGE, fontMan->GetFont(34, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setFontForSize(CRUI::FONT_SIZE_XLARGE, fontMan->GetFont(44, 400, false, css_ff_sans_serif, lString8("Tizen Sans Medium"), 0));
	currentTheme->setListDelimiterVertical(resourceResolver->getIcon("divider_light_v3.png"));
	CRUIStyle * buttonStyle = currentTheme->addSubstyle(lString8("BUTTON"));
	//keyboard_key_feedback_background.9
	buttonStyle->setBackground("btn_default_normal.9")->setFontSize(FONT_SIZE_LARGE);
	//buttonStyle->setBackground("keyboard_key_feedback_background.9")->setFontSize(FONT_SIZE_LARGE)->setPadding(10);
	//buttonStyle->setBackground("btn_default_normal.9")->setFontSize(FONT_SIZE_LARGE)->setPadding(10);
	buttonStyle->addSubstyle(STATE_PRESSED, STATE_PRESSED)->setBackground("btn_default_pressed.9");
	buttonStyle->addSubstyle(STATE_FOCUSED, STATE_FOCUSED)->setBackground("btn_default_selected.9");
	buttonStyle->addSubstyle(STATE_DISABLED, STATE_DISABLED)->setTextColor(0x80000000);
}
