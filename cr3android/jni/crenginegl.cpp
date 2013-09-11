#include <jni.h>
#include "cr3java.h"
#include "org_coolreader_newui_CRView.h"
#include "cruimain.h"
#include "crconcurrent.h"
#include "cruiconfig.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static jfieldID gNativeObjectID = 0;

class DocViewNative {
public:
    JNIEnv * _env;    
    CRUIMainWidget * _widget;
    DocViewNative(JNIEnv * env) : _env(env), _widget(NULL) {}    
};

static DocViewNative * getNative(JNIEnv * env, jobject _this)
{
	DocViewNative * res = (DocViewNative *)env->GetIntField(_this, gNativeObjectID);
	if (res == NULL)
		CRLog::warn("Native DocView is NULL");
	return res;
}

class JNICDRLogger : public CRLog
{
public:
    JNICDRLogger()
    {
    	curr_level = CRLog::LL_DEBUG;
    }
protected:

	virtual void log( const char * lvl, const char * msg, va_list args)
	{
	    #define MAX_LOG_MSG_SIZE 1024
		static char buffer[MAX_LOG_MSG_SIZE+1];
		vsnprintf(buffer, MAX_LOG_MSG_SIZE, msg, args);
		int level = ANDROID_LOG_DEBUG;
		//LOGD("CRLog::log is called with LEVEL %s, pattern %s", lvl, msg);
		if ( !strcmp(lvl, "FATAL") )
			level = ANDROID_LOG_FATAL;
		else if ( !strcmp(lvl, "ERROR") )
			level = ANDROID_LOG_ERROR;
		else if ( !strcmp(lvl, "WARN") )
			level = ANDROID_LOG_WARN;
		else if ( !strcmp(lvl, "INFO") )
			level = ANDROID_LOG_INFO;
		else if ( !strcmp(lvl, "DEBUG") )
			level = ANDROID_LOG_DEBUG;
		else if ( !strcmp(lvl, "TRACE") )
			level = ANDROID_LOG_VERBOSE;
		__android_log_write(level, LOG_TAG, buffer);
	}
};

//typedef void (lv_FatalErrorHandler_t)(int errorCode, const char * errorText );

void cr3androidFatalErrorHandler(int errorCode, const char * errorText )
{
	static char str[1001];
	snprintf(str, 1000, "CoolReader Fatal Error #%d: %s", errorCode, errorText);
	LOGE(str);
	LOGASSERTFAILED(errorText, str);
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    initInternal
 * Signature: (Lorg/coolreader/newui/CRConfig;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_coolreader_newui_CRView_initInternal
  (JNIEnv * _env, jobject _this, jobject _config) {
    CRJNIEnv env(_env);
    jclass rvClass = env->FindClass("org/coolreader/newui/CRView");
    gNativeObjectID = env->GetFieldID(rvClass, "mNativeObject", "I");
    DocViewNative * obj = new DocViewNative(_env);
    env->SetIntField(_this, gNativeObjectID, (jint)obj);

	LOGI("initInternal called");
	// set fatal error handler
	crSetFatalErrorHandler( &cr3androidFatalErrorHandler );
	LOGD("Redirecting CDRLog to Android");
	CRLog::setLogger( new JNICDRLogger() );
	CRLog::setLogLevel( CRLog::LL_TRACE );

	CRLog::trace("Setting crash handler");
	crSetSignalHandler();

    // init config
    CRObjectAccessor cfg(_env, _config);

    // set screen size
    int screenX = CRIntField(cfg,"screenX").get();
    int screenY = CRIntField(cfg,"screenY").get();
    int screenDPI = CRIntField(cfg,"screenDPI").get();
    deviceInfo.setScreenDimensions(screenX, screenY, screenDPI);

    crconfig.coverCacheDir = CRStringField(cfg,"coverCacheDir").get8();
    crconfig.cssDir = CRStringField(cfg,"cssDir").get8();
    crconfig.dbFile = CRStringField(cfg,"dbFile").get8();
    crconfig.iniFile = CRStringField(cfg,"iniFile").get8();
    crconfig.hyphDir = CRStringField(cfg,"hyphDir").get8();
    crconfig.resourceDir = CRStringField(cfg,"resourceDir").get8();
    crconfig.uiFontFace = CRStringField(cfg,"uiFontFace").get8();
    crconfig.docCacheDir = CRStringField(cfg,"docCacheDir").get8();
    crconfig.i18nDir = CRStringField(cfg,"i18nDir").get8();
    crconfig.systemLanguage = CRStringField(cfg,"systemLanguage").get8();

    crconfig.docCacheMaxBytes = CRIntField(cfg,"docCacheMaxBytes").get();
    crconfig.coverDirMaxItems = CRIntField(cfg,"coverDirMaxItems").get();
    crconfig.coverDirMaxFiles = CRIntField(cfg,"screenDPI").get();
    crconfig.coverDirMaxSize = CRIntField(cfg,"coverDirMaxSize").get();
    crconfig.coverRenderCacheMaxItems = CRIntField(cfg,"coverRenderCacheMaxItems").get();
    crconfig.coverRenderCacheMaxBytes = CRIntField(cfg,"coverRenderCacheMaxBytes").get();

    CRLog::info("Calling initEngine");
    crconfig.initEngine();
    CRLog::info("Done initEngine");

    return JNI_TRUE;
}

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    isLink
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_coolreader_newui_CRView_isLink
  (JNIEnv * env, jclass obj, jstring pathname)
{
	if ( !pathname )
		return NULL;
	int res = JNI_FALSE;
	jboolean iscopy;
	const char * s = env->GetStringUTFChars(pathname, &iscopy);
	struct stat st;
	lString8 path;
	if ( !lstat( s, &st) ) {
		if ( S_ISLNK(st.st_mode) ) {
			char buf[1024];
			int len = readlink(s, buf, sizeof(buf) - 1);
			if (len != -1) {
				buf[len] = 0;
				path = lString8(buf);
			}
		}
	}
	env->ReleaseStringUTFChars(pathname, s);
	return !path.empty() ? (jstring)env->NewGlobalRef(env->NewStringUTF(path.c_str())) : NULL;
}

//============================================================================================================
// register JNI methods

static JNINativeMethod sCRViewMethods[] = {
  /* name, signature, funcPtr */
  {"initInternal", "(Lorg/coolreader/newui/CRConfig;)Z", (void*)Java_org_coolreader_newui_CRView_initInternal},
  {"isLink", "(Ljava/lang/String;)Ljava/lang/String;", (void*)Java_org_coolreader_newui_CRView_isLink}
};

/*
 * Register native JNI-callable methods.
 *
 * "className" looks like "java/lang/String".
 */
static int jniRegisterNativeMethods(JNIEnv* env, const char* className,
    const JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    LOGV("Registering %s natives\n", className);
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'\n", className);
        return -1;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'\n", className);
        return -1;
    }
    return 0;
}


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
   JNIEnv* env = NULL;
   jint res = -1;
 
#ifdef JNI_VERSION_1_6
    if (res==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOGI("JNI_OnLoad: JNI_VERSION_1_6\n");
   	    res = JNI_VERSION_1_6;
    }
#endif
#ifdef JNI_VERSION_1_4
    if (res==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_4) == JNI_OK) {
        LOGI("JNI_OnLoad: JNI_VERSION_1_4\n");
   	    res = JNI_VERSION_1_4;
    }
#endif
#ifdef JNI_VERSION_1_2
    if (res==-1 && vm->GetEnv((void**) &env, JNI_VERSION_1_2) == JNI_OK) {
        LOGI("JNI_OnLoad: JNI_VERSION_1_2\n");
   	    res = JNI_VERSION_1_2;
    }
#endif
	if ( res==-1 )
		return res;
 
    jniRegisterNativeMethods(env, "org/coolreader/newui/CRView", sCRViewMethods, sizeof(sCRViewMethods)/sizeof(JNINativeMethod));
    LOGI("JNI_OnLoad: native methods are registered!\n");
    return res;
}
