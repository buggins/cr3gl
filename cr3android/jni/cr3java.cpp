#include "cr3java.h"

#include <dlfcn.h>

static JavaVM * jvm = NULL;

void SaveJVMPointer(JNIEnv *env) {
    int status = env->GetJavaVM(&jvm);
    if(status != 0) {
        // Fail!
    	CRLog::fatal("cannot get JVM");
    }
}

JNIEnv * CRJNIEnv::currentThreadEnv() {
	// get environment for current thread
	if (!jvm) {
		CRLog::fatal("JVM pointer is not initialized");
	}
    JNIEnv *env = NULL;
    int getEnvStat = jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, NULL) != 0) {
            CRLog::fatal("Failed to attach to current thread");
        }
    } else if (getEnvStat == JNI_OK) {
        //
    } else if (getEnvStat == JNI_EVERSION) {
        getEnvStat = jvm->GetEnv((void **)&env, JNI_VERSION_1_4);
        if (getEnvStat == JNI_EDETACHED) {
            if (jvm->AttachCurrentThread(&env, NULL) != 0) {
                CRLog::fatal("Failed to attach to current thread");
            }
        } else if (getEnvStat == JNI_OK) {
            //
        } else if (getEnvStat == JNI_EVERSION) {
            getEnvStat = jvm->GetEnv((void **)&env, JNI_VERSION_1_2);
            if (getEnvStat != JNI_OK) {
            	CRLog::fatal("Cannot get JNI environment");
            }
        }
    }
    return env;
}

lString16 CRJNIEnv::fromJavaString( jstring str )
{
	if ( !str )
        return lString16::empty_str;
	jboolean iscopy;
	const char * s = env()->GetStringUTFChars( str, &iscopy );
	lString16 res(s);
	env()->ReleaseStringUTFChars(str, s);
	return res;
}

jstring CRJNIEnv::toJavaString( const lString16 & str )
{
	return env()->NewStringUTF(UnicodeToUtf8(str).c_str());
}

void CRJNIEnv::fromJavaStringArray( jobjectArray array, lString16Collection & dst )
{
	dst.clear();
	int len = env()->GetArrayLength(array);
	for ( int i=0; i<len; i++ ) {
		jstring str = (jstring)env()->GetObjectArrayElement(array, i);
		dst.add(fromJavaString(str));
		env()->DeleteLocalRef(str);
	}
}

jobjectArray CRJNIEnv::toJavaStringArray( lString16Collection & src )
{
    int len = src.length();
	jobjectArray array = env()->NewObjectArray(len, env()->FindClass("java/lang/String"), env()->NewStringUTF(""));
	for ( int i=0; i<len; i++ ) {
		jstring local = toJavaString(src[i]); 
		env()->SetObjectArrayElement(array, i, local);
		env()->DeleteLocalRef(local);
	}
	return array;
}

CRPropRef CRJNIEnv::fromJavaProperties( jobject jprops )
{
	CRPropRef props = LVCreatePropsContainer();
    CRObjectAccessor jp(env(), jprops);
    CRMethodAccessor p_getProperty(jp, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
    jobject en = CRMethodAccessor( jp, "propertyNames", "()Ljava/util/Enumeration;").callObj();
    CRObjectAccessor jen(env(), en);
    CRMethodAccessor jen_hasMoreElements(jen, "hasMoreElements", "()Z");
    CRMethodAccessor jen_nextElement(jen, "nextElement", "()Ljava/lang/Object;");
    while ( jen_hasMoreElements.callBool() ) {
    	jstring key = (jstring)jen_nextElement.callObj();
    	jstring value = (jstring)p_getProperty.callObj(key);
    	props->setString(LCSTR(fromJavaString(key)),LCSTR(fromJavaString(value))); 
    	env()->DeleteLocalRef(key);
    	env()->DeleteLocalRef(value);
    }
	return props;
}

jobject CRJNIEnv::toJavaProperties( CRPropRef props )
{
    jclass cls = env()->FindClass("java/util/Properties");
    jmethodID mid = env()->GetMethodID(cls, "<init>", "()V");
    jobject obj = env()->NewObject(cls, mid);
    CRObjectAccessor jp(env(), obj);
    CRMethodAccessor p_setProperty(jp, "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");
    for ( int i=0; i<props->getCount(); i++ ) {
    	jstring key = toJavaString(lString16(props->getName(i)));
    	jstring value = toJavaString(lString16(props->getValue(i)));
    	p_setProperty.callObj(key, value);
		env()->DeleteLocalRef(key);
		env()->DeleteLocalRef(value);
    }
	return obj;
}

LVStreamRef CRJNIEnv::jbyteArrayToStream( jbyteArray array )
{
	if ( !array )
		return LVStreamRef();
	int len = env()->GetArrayLength(array);
	if ( !len )
		return LVStreamRef();
    lUInt8 * data = (lUInt8 *)env()->GetByteArrayElements(array, 0);
    LVStreamRef res = LVCreateMemoryStream(data, len, true, LVOM_READ);
    env()->ReleaseByteArrayElements(array, (jbyte*)data, 0);
    return res;
} 

jbyteArray CRJNIEnv::streamToJByteArray( LVStreamRef stream )
{
	if ( stream.isNull() )
		return NULL;
	unsigned sz = stream->GetSize();
	if ( sz<10 || sz>2000000 )
		return NULL;
    jbyteArray array = env()->NewByteArray(sz);
    lUInt8 * array_data = (lUInt8 *)env()->GetByteArrayElements(array, 0);
    lvsize_t bytesRead = 0;
    stream->Read(array_data, sz, &bytesRead);
   	env()->ReleaseByteArrayElements(array, (jbyte*)array_data, 0);
    if (bytesRead != sz)
    	return NULL;
    return array; 
}

static void ConvertCRColorsToAndroid( lUInt8 * buf, int dx, int dy )
{
	int sz = dx * dy;
	for ( lUInt8 * p = buf; --sz>=0; p+=4 ) {
		// invert A
		p[3] ^= 0xFF; 
		// swap R and B
		lUInt8 c = p[0];
		p[0] = p[2];
		p[2] = c;
	}
} 

class LVColorDrawBufEx : public LVColorDrawBuf {
public:
    lUInt8 * getData() { return _data; }
    void convert() {
    	if ( GetBitsPerPixel()==32 )
    		ConvertCRColorsToAndroid( _data, GetWidth(), GetHeight() );
    }
    
	LVColorDrawBufEx(int dx, int dy, lUInt8 * pixels, int bpp)
	: LVColorDrawBuf( dx, dy, pixels, bpp ) {
	}
};

class JNIGraphicsLib : public BitmapAccessorInterface
{
    void * _lib;

	int (*AndroidBitmap_getInfo)(JNIEnv* env, jobject jbitmap, AndroidBitmapInfo* info);
	int (*AndroidBitmap_lockPixels)(JNIEnv* env, jobject jbitmap, void** addrPtr);
	int (*AndroidBitmap_unlockPixels)(JNIEnv* env, jobject jbitmap);
public:
    virtual LVDrawBuf * lock(JNIEnv* env, jobject jbitmap) {
	    //CRLog::trace("JNIGraphicsLib::lock entered");
		AndroidBitmapInfo info;
		if ( ANDROID_BITMAP_RESUT_SUCCESS!=AndroidBitmap_getInfo(env, jbitmap, &info) ) {
			CRLog::error("BitmapAccessor : cannot get bitmap info");
			return NULL;
		}
		int width = info.width;
		int height = info.height;
		int stride = info.stride;
		int format = info.format;
		if ( format!=ANDROID_BITMAP_FORMAT_RGBA_8888 && format!=ANDROID_BITMAP_FORMAT_RGB_565  && format!=8 ) {
			CRLog::error("BitmapAccessor : bitmap format %d is not yet supported", format);
			return NULL;
		}
		int bpp = (format==ANDROID_BITMAP_FORMAT_RGBA_8888) ? 32 : 16;
	    //CRLog::trace("JNIGraphicsLib::lock info: %d (%d) x %d", width, stride, height);
		lUInt8 * pixels = NULL; 
		if ( ANDROID_BITMAP_RESUT_SUCCESS!=AndroidBitmap_lockPixels(env, jbitmap, (void**)&pixels) ) {
	        CRLog::error("AndroidBitmap_lockPixels failed");
		    pixels = NULL;
		}
	    //CRLog::trace("JNIGraphicsLib::lock pixels locked!" );
		return new LVColorDrawBufEx( width, height, pixels, bpp );
    } 
    virtual void unlock(JNIEnv* env, jobject jbitmap, LVDrawBuf * buf ) {
    	LVColorDrawBufEx * bmp = (LVColorDrawBufEx*)buf;
    	bmp->convert();
    	AndroidBitmap_unlockPixels(env, jbitmap);
    	delete buf;
    } 

    void * getProc( const char * procName )
    {
        return dlsym( _lib, procName );
    }
    bool load( const char * libName )
    {
        if ( !_lib ) {
            _lib = dlopen( libName, RTLD_NOW | RTLD_LOCAL );
        }
        if ( _lib ) {
        	AndroidBitmap_getInfo = (int (*)(JNIEnv* env, jobject jbitmap, AndroidBitmapInfo* info))
                 getProc( "AndroidBitmap_getInfo" );
            AndroidBitmap_lockPixels = (int (*)(JNIEnv* env, jobject jbitmap, void** addrPtr))
                 getProc( "AndroidBitmap_lockPixels" );
            AndroidBitmap_unlockPixels = (int (*)(JNIEnv* env, jobject jbitmap))
	             getProc( "AndroidBitmap_unlockPixels");
            if ( !AndroidBitmap_getInfo || !AndroidBitmap_lockPixels || !AndroidBitmap_unlockPixels )
                unload(); // not all functions found in library, fail
        }
        return ( _lib!=NULL );
    }
    bool unload()
    {
        bool res = false;
        if ( _lib ) {
            dlclose( _lib );
            res = true;
        }
        _lib = NULL;
        return res;
    }
    JNIGraphicsLib()
    : _lib(NULL) {
    }
    ~JNIGraphicsLib() {
        unload();
    }
};

