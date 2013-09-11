#include <jni.h>
#include "cr3java.h"
#include "org_coolreader_newui_CRView.h"
#include "cruimain.h"

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


/*
 * Class:     org_coolreader_newui_CRView
 * Method:    initInternal
 * Signature: (Lorg/coolreader/newui/CRConfig;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_coolreader_newui_CRView_initInternal
  (JNIEnv * _env, jobject _this, jobject _config) {
    CRJNIEnv env(_env);
    jclass rvClass = env->FindClass("org/coolreader/crengine/DocView");
    gNativeObjectID = env->GetFieldID(rvClass, "mNativeObject", "I");
    DocViewNative * obj = new DocViewNative(_env);
    env->SetIntField(_this, gNativeObjectID, (jint)obj);
    // init config
    CRObjectAccessor cfg(_env, _config);
    int screenX = CRIntField(cfg,"screenX").get();
    int screenY = CRIntField(cfg,"screenY").get();
    int screenDPI = CRIntField(cfg,"screenDPI").get();
    return JNI_TRUE;
}


//============================================================================================================
// register JNI methods

static JNINativeMethod sCRViewMethods[] = {
  /* name, signature, funcPtr */
  {"initInternal", "(Lorg/coolreader/newui/CRConfig;)Z", (void*)Java_org_coolreader_newui_CRView_initInternal}
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
