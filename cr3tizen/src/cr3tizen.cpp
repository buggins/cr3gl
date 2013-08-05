/*
 * cr3tizen.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: vlopatin
 */

#include "tizenx.h"
#include "cr3tizen.h"
#include "lvstring.h"


class CRTizenLogger : public CRLog
{
protected:
    virtual void log( const char * level, const char * msg, va_list args )
    {
        if (!strcmp("ERROR", level))
        	AppLogExceptionInternal("", 0, msg, args);
        else if (!strcmp("INFO", level))
        	AppLogInternal("", 0, msg, args);
        else
        	AppLogDebugInternal("", 0, msg, args);
    }
public:
    CRTizenLogger()
    {
        static unsigned char utf8sign[] = {0xEF, 0xBB, 0xBF};
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
