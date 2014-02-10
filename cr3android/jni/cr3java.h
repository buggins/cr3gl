/*
 * CoolReader 3 Java Port helpers.
 */

#ifndef CR3_JAVA_H
#define CR3_JAVA_H

#include <jni.h>
#include <android/log.h>
#include "cruievent.h"
#include "lvstring.h"
#include "cruiconfig.h"

#define  LOG_TAG    "cr3eng"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGASSERTFAILED(cond,...)  __android_log_assert(cond,LOG_TAG,__VA_ARGS__)

#include "../../crengine/include/lvstring.h"
#include "../../crengine/include/lvdrawbuf.h"
#include "../../crengine/include/props.h"
#include "../../crengine/include/lvtinydom.h"

//====================================================================
// libjnigraphics replacement for pre-2.2 SDKs 
enum AndroidBitmapFormat {
    ANDROID_BITMAP_FORMAT_NONE      = 0,
    ANDROID_BITMAP_FORMAT_RGBA_8888 = 1,
    ANDROID_BITMAP_FORMAT_RGB_565   = 4,
    ANDROID_BITMAP_FORMAT_RGBA_4444 = 7,
    ANDROID_BITMAP_FORMAT_A_8       = 8,
};
//====================================================================
#define ANDROID_BITMAP_RESUT_SUCCESS            0
#define ANDROID_BITMAP_RESULT_BAD_PARAMETER     -1
#define ANDROID_BITMAP_RESULT_JNI_EXCEPTION     -2
#define ANDROID_BITMAP_RESULT_ALLOCATION_FAILED -3

typedef struct {
    uint32_t    width;
    uint32_t    height;
    uint32_t    stride;
    int32_t     format;
//    uint32_t    flags;      // 0 for now
} AndroidBitmapInfo;

class BitmapAccessorInterface {
public:
    virtual LVDrawBuf * lock(JNIEnv* env, jobject jbitmap) = 0;
    virtual void unlock(JNIEnv* env, jobject jbitmap, LVDrawBuf * buf ) = 0;
	static BitmapAccessorInterface * getInstance();
	virtual ~BitmapAccessorInterface() {}
};

void SaveJVMPointer(JNIEnv *env);

//====================================================================

class CRJNIEnv {
	JNIEnv * __env;
public:
	// pass NULL to get environment for current thread automatically
    CRJNIEnv(JNIEnv * pEnv) : __env(pEnv) { }
    // to get environment for current thread
    CRJNIEnv() : __env(NULL) { }
    JNIEnv * env() {
    	if (__env)
    		return __env;
    	else
    		return currentThreadEnv();
    }

    static JNIEnv * currentThreadEnv();

    JNIEnv * operator -> () { return env(); }
	lString16 fromJavaString( jstring str );
	jstring toJavaString( const lString16 & str );
	void fromJavaStringArray( jobjectArray array, lString16Collection & dst );
	jobjectArray toJavaStringArray( lString16Collection & dst );
	LVStreamRef jbyteArrayToStream( jbyteArray array ); 
	jbyteArray streamToJByteArray( LVStreamRef stream ); 
	CRPropRef fromJavaProperties( jobject jprops );
	jobject toJavaProperties( CRPropRef props );
};

class CRClassAccessor : public CRJNIEnv {
protected:
	jclass cls;
public:
	jclass getClass() { return cls; }
    CRClassAccessor(JNIEnv * pEnv, jclass _class) : CRJNIEnv(pEnv)
    {
    	cls = (jclass)env()->NewGlobalRef(_class);
    }
    CRClassAccessor(jclass _class) : CRJNIEnv(NULL)
    {
    	cls = (jclass)currentThreadEnv()->NewGlobalRef(_class);
    }
    CRClassAccessor(JNIEnv * pEnv, const char * className) : CRJNIEnv(pEnv)
    {
    	cls = (jclass)env()->NewGlobalRef(env()->FindClass(className));
    }
    CRClassAccessor(const char * className) : CRJNIEnv(NULL)
    {
    	cls = (jclass)currentThreadEnv()->NewGlobalRef(currentThreadEnv()->FindClass(className));
    }
    ~CRClassAccessor() {
    	env()->DeleteGlobalRef(cls);
    }
    jobject newObject() {
        jmethodID mid = env()->GetMethodID(cls, "<init>", "()V");
        jobject obj = env()->NewObject(cls, mid);
        return obj;
    }
    jobject newObject(jlong v) {
        jmethodID mid = env()->GetMethodID(cls, "<init>", "(J)V");
        jobject obj = env()->NewObject(cls, mid, v);
        return obj;
    }
};

class CRObjectAccessor : public CRClassAccessor {
	jobject obj;
public:
	jobject getObject() { return obj; }
	CRObjectAccessor(JNIEnv * pEnv, jobject _obj)
    : CRClassAccessor(pEnv, pEnv->GetObjectClass(_obj))
    {
    	obj = env()->NewGlobalRef(_obj);
    }
	CRObjectAccessor(jobject _obj)
    : CRClassAccessor(NULL, currentThreadEnv()->GetObjectClass(_obj))
    {
    	obj = env()->NewGlobalRef(_obj);
    }
	~CRObjectAccessor() {
    	env()->DeleteGlobalRef(obj);
	}
};

class CRFieldAccessor {
protected:
	CRObjectAccessor & objacc;
	jfieldID fieldid;
public:
	CRFieldAccessor( CRObjectAccessor & acc, const char * fieldName, const char * fieldType )
	: objacc(acc)
	{
		fieldid = objacc->GetFieldID( objacc.getClass(), fieldName, fieldType );
	}
	jobject getObject()
	{
		 return objacc->GetObjectField(objacc.getObject(), fieldid); 
	}
	void setObject( jobject obj )
	{
		 return objacc->SetObjectField(objacc.getObject(), fieldid, obj); 
	}
};

class CRMethodAccessor {
protected:
	CRObjectAccessor & objacc;
	jmethodID methodid;
	int apiLevel;
public:
	CRMethodAccessor( CRObjectAccessor & acc, const char * methodName, const char * signature, int _apiLevel = 0 )
	: objacc(acc), apiLevel(_apiLevel)
	{
		if (supported())
			methodid = objacc->GetMethodID( objacc.getClass(), methodName, signature );
		else
			methodid = 0;
	}
	bool supported() {
		return crconfig.apiLevel >= apiLevel;
	}
	jobject callObj()
	{
		return objacc->CallObjectMethod( objacc.getObject(), methodid ); 
	}
	jobject callObj(jlong v)
	{
		return objacc->CallObjectMethod( objacc.getObject(), methodid, v );
	}
	jobject callObj(jobject obj)
	{
		return objacc->CallObjectMethod( objacc.getObject(), methodid, obj ); 
	}
	jobjectArray callObjArray(jobject obj)
	{
		return (jobjectArray)objacc->CallObjectMethod( objacc.getObject(), methodid, obj );
	}
	jobject callObj(jobject obj1, jobject obj2)
	{
		return objacc->CallObjectMethod( objacc.getObject(), methodid, obj1, obj2 ); 
	}
	jboolean callBool()
	{
		return objacc->CallBooleanMethod( objacc.getObject(), methodid ); 
	}
	jint callInt()
	{
		return objacc->CallIntMethod( objacc.getObject(), methodid ); 
	}
	jint callInt(jint v)
	{
		return objacc->CallIntMethod( objacc.getObject(), methodid, v );
	}
	jfloat callFloat()
	{
		return objacc->CallFloatMethod( objacc.getObject(), methodid);
	}
	jfloat callFloat(jint v)
	{
		return objacc->CallFloatMethod( objacc.getObject(), methodid, v );
	}
	jint callInt(jbyteArray array)
	{
		return objacc->CallIntMethod( objacc.getObject(), methodid, array );
	}
	jlong callLong()
	{
		return objacc->CallLongMethod( objacc.getObject(), methodid );
	}
	jlong callLong(jlong v)
	{
		return objacc->CallLongMethod( objacc.getObject(), methodid, v );
	}
	void callVoid()
	{
		return objacc->CallVoidMethod( objacc.getObject(), methodid );
	}
	void callVoid(jboolean p1, jboolean p2)
	{
		return objacc->CallVoidMethod( objacc.getObject(), methodid, p1, p2 );
	}
	void callVoidLongLong(jlong p1, jlong p2)
	{
		return objacc->CallVoidMethod( objacc.getObject(), methodid, p1, p2 );
	}
	void callVoid(jlong v)
	{
		return objacc->CallVoidMethod( objacc.getObject(), methodid, v);
	}
	void callVoid(jstring s)
	{
		return objacc->CallVoidMethod( objacc.getObject(), methodid, s);
	}
};

class CRStaticMethodAccessor {
protected:
	CRClassAccessor & objacc;
	jmethodID methodid;
public:
	CRStaticMethodAccessor( CRClassAccessor & acc, const char * methodName, const char * signature )
	: objacc(acc)
	{
		methodid = objacc->GetStaticMethodID( objacc.getClass(), methodName, signature );
	}
	jobject callObj()
	{
		return objacc->CallStaticObjectMethod( objacc.getClass(), methodid );
	}
	jobject callObj(jlong v)
	{
		return objacc->CallStaticObjectMethod( objacc.getClass(), methodid, v );
	}
	jobject callObj(jobject obj)
	{
		return objacc->CallStaticObjectMethod( objacc.getClass(), methodid, obj );
	}
	jobjectArray callObjArray(jobject obj)
	{
		return (jobjectArray)objacc->CallStaticObjectMethod( objacc.getClass(), methodid, obj );
	}
	jobject callObj(jobject obj1, jobject obj2)
	{
		return objacc->CallStaticObjectMethod( objacc.getClass(), methodid, obj1, obj2 );
	}
	jboolean callBool()
	{
		return objacc->CallStaticBooleanMethod( objacc.getClass(), methodid );
	}
	jint callInt()
	{
		return objacc->CallStaticIntMethod( objacc.getClass(), methodid );
	}
	jint callInt(jint v)
	{
		return objacc->CallStaticIntMethod( objacc.getClass(), methodid, v );
	}
	jint callInt(jbyteArray array)
	{
		return objacc->CallStaticIntMethod( objacc.getClass(), methodid, array );
	}
	jlong callLong()
	{
		return objacc->CallStaticLongMethod( objacc.getClass(), methodid );
	}
	jlong callLong(jlong v)
	{
		return objacc->CallStaticLongMethod( objacc.getClass(), methodid, v );
	}
	void callVoid()
	{
		return objacc->CallStaticVoidMethod( objacc.getClass(), methodid );
	}
	void callVoid(jlong v)
	{
		return objacc->CallStaticVoidMethod( objacc.getClass(), methodid, v);
	}
};

class CRStringField : public CRFieldAccessor {
public:
	CRStringField( CRObjectAccessor & acc, const char * fieldName )
	: CRFieldAccessor( acc, fieldName, "Ljava/lang/String;" ) 
	{
	}
	lString16 get() {
		jstring str = (jstring)objacc->GetObjectField(objacc.getObject(), fieldid);
		lString16 res = objacc.fromJavaString(str);
		objacc.env()->DeleteLocalRef(str);
		return res;
	}
	lString8 get8() {
		return UnicodeToUtf8(get());
	}
	void set( const lString16& str) { objacc->SetObjectField(objacc.getObject(), fieldid, objacc.toJavaString(str)); } 
	void set( const lString8& str) { set(Utf8ToUnicode(str)); }
};

class CRStringArrayField : public CRFieldAccessor {
public:
	CRStringArrayField( CRObjectAccessor & acc, const char * fieldName )
	: CRFieldAccessor( acc, fieldName, "[Ljava/lang/String;" )
	{
	}
	lString16 get(int index) {
		jobjectArray array = (jobjectArray)objacc->GetObjectField(objacc.getObject(), fieldid);
		jstring str = (jstring)objacc->GetObjectArrayElement(array, index);
		lString16 res = objacc.fromJavaString(str);
		objacc.env()->DeleteLocalRef(str);
		objacc.env()->DeleteLocalRef(array);
		return res;
	}
	lString8 get8(int index) {
		return UnicodeToUtf8(get(index));
	}
	void set(int index, const lString16& str) {
		jobjectArray array = (jobjectArray)objacc->GetObjectField(objacc.getObject(), fieldid);
		jstring local = objacc.toJavaString(str);
		objacc->SetObjectArrayElement(array, index, local);
		objacc->DeleteLocalRef(local);
		objacc->DeleteLocalRef(array);
	}
	void set(int index, const lString8& str) { set(index, Utf8ToUnicode(str)); }
	int length() {
		jobjectArray array = (jobjectArray)objacc->GetObjectField(objacc.getObject(), fieldid);
		int res = objacc->GetArrayLength(array);
		objacc->DeleteLocalRef(array);
		return res;
	}
};

class CRIntField : public CRFieldAccessor {
public:
	CRIntField( CRObjectAccessor & acc, const char * fieldName )
	: CRFieldAccessor( acc, fieldName, "I" ) 
	{
	}
	int get() { return objacc->GetIntField(objacc.getObject(), fieldid); } 
	void set(int v) { objacc->SetIntField(objacc.getObject(), fieldid, v); } 
};

class CRLongField : public CRFieldAccessor {
public:
	CRLongField( CRObjectAccessor & acc, const char * fieldName )
	: CRFieldAccessor( acc, fieldName, "J" ) 
	{
	}
	lInt64 get() { return objacc->GetLongField(objacc.getObject(), fieldid); } 
	void set(lInt64 v) { objacc->SetLongField(objacc.getObject(), fieldid, v); } 
};

/// wrapper for Android KeyEvent
class CRKeyEventWrapper {
	CRObjectAccessor event;
	CRMethodAccessor getCharactersMethod;
	CRMethodAccessor getActionMethod;
	CRMethodAccessor getKeyCodeMethod;
	CRMethodAccessor getModifiersMethod;
public:
	CRKeyEventWrapper(JNIEnv * jni, jobject obj)
	: event(jni, obj)
	, getCharactersMethod(event, "getCharacters", "()Ljava/lang/String;", 3)  // API 3
	, getActionMethod(event, "getAction", "()I", 1) // API 1
	, getKeyCodeMethod(event, "getKeyCode", "()I", 1) // API 1
	, getModifiersMethod(event, "getModifiers", "()I", 13) // API 13 Android 3.2
	{

	}
	int getAction() {
		return getActionMethod.callInt();
	}
	int getKeyCode() {
		return getKeyCodeMethod.callInt();
	}
	int getModifiers() {
		if (getModifiersMethod.supported())
			return getModifiersMethod.callInt();
		return 0;
	}
	lString16 getCharacters() {
		jstring s = (jstring)getCharactersMethod.callObj();
		lString16 res = event.fromJavaString(s);
		event.env()->DeleteLocalRef(s);
		return res;
	}
};

/// wrapper for Android TouchEvent
class CRTouchEventWrapper {
	CRObjectAccessor event;
	CRMethodAccessor getPointerCountMethod;
	CRMethodAccessor getXMethod;
	CRMethodAccessor getYMethod;
	CRMethodAccessor getPointerIdMethod;
	CRMethodAccessor getToolTypeMethod;
	CRMethodAccessor getButtonStateMethod;
	CRMethodAccessor getActionMethod;
	CRMethodAccessor getActionMaskedMethod;
	CRMethodAccessor getActionIndexMethod;
	CRMethodAccessor getEventTimeMethod;
public:
	CRTouchEventWrapper(JNIEnv * _env, jobject obj)
	: event(_env, obj)
	, getPointerCountMethod(event, "getPointerCount", "()I", 5) // API 5  Android 2.0
	, getXMethod(event, "getX", "(I)F", 5) // API 5  Android 2.0
	, getYMethod(event, "getY", "(I)F", 5) // API 5  Android 2.0
	, getPointerIdMethod(event, "getPointerId", "(I)I", 5) // API 5  Android 2.0
	, getToolTypeMethod(event, "getToolType", "(I)I", 14)   // API 14 Android 4.0
	, getButtonStateMethod(event, "getButtonState", "()I", 14) // API 14 14 Android 4.0
	, getActionMethod(event, "getAction", "()I", 1) // API 1
	, getActionMaskedMethod(event, "getActionMasked", "()I", 8) // API 8  Android 2.2
	, getActionIndexMethod(event, "getActionIndex", "()I", 8)   // API 8  Android 2.2
	, getEventTimeMethod(event, "getEventTime", "()J", 16)  // API 16  Android 4.1
	{

	}
	int getPointerCount() {
		return getPointerCountMethod.callInt();
	}
	int getX(jint index) {
		return (int)getXMethod.callFloat(index);
	}
	int getY(jint index) {
		return (int)getYMethod.callFloat(index);
	}
	int getPointerId(jint index) {
		return getPointerIdMethod.callInt(index);
	}
	int getToolType(jint index) {
		if (getToolTypeMethod.supported())
			return getToolTypeMethod.callInt(index);
		return 1; // TOOL_TYPE_FINGER
	}
	int getAction() {
		return getActionMethod.callInt();
	}
	int getActionMasked() {
		if (getActionMaskedMethod.supported())
			return getActionMaskedMethod.callInt();
		// TODO:
		return 0;
	}
	int getActionIndex() {
		if (getActionIndexMethod.supported())
			return getActionIndexMethod.callInt();
		// TODO
		return 0;
	}
	lUInt64 getEventTime() {
		// convert uptime to system time
		if (getEventTimeMethod.supported()) {
			jlong ts = getEventTimeMethod.callLong();
			CRClassAccessor systemClockClass(event.env(), "android/os/SystemClock");
			CRStaticMethodAccessor uptimeMillisMethod(systemClockClass, "uptimeMillis", "()J");
			jlong currentUptimeMillis = uptimeMillisMethod.callLong();
			jlong currentTimeMillis = GetCurrentTimeMillis();
			return (lUInt64)(currentTimeMillis - (currentUptimeMillis - ts));
		} else {
			return GetCurrentTimeMillis();
		}
	}
};

#endif
