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
    CRJNIEnv _env;
    CRObjectAccessor _obj;
    CRMethodAccessor _openResourceStreamMethod;
    CRMethodAccessor _listResourceDirMethod;
    CRUIMainWidget * _widget;
    LVContainerRef _assets;
public:
    DocViewNative(jobject obj)
    : _obj(obj)
    , _openResourceStreamMethod(_obj, "openResourceStream", "(Ljava/lang/String;)Ljava/io/InputStream;")
    , _listResourceDirMethod(_obj, "listResourceDir", "(Ljava/lang/String;)[Ljava/lang/String;")
    , _widget(NULL) {}
    bool create() {
    	_widget = new CRUIMainWidget();
    	return true;
    }

    LVStreamRef openResource(const lString16 & path);

    LVContainerRef openResourceDir(const lString16 & path);

    ~DocViewNative() {
    	delete _widget;
    }
};

class CRAssetDir : public LVContainer {
	DocViewNative * _native;
	lString16 _path;
	LVPtrVector<LVContainerItemInfo> _items;
public:
	class ItemInfo : public LVContainerItemInfo {
		lString16 _name;
	public:
		ItemInfo(lString16 name) : _name(name) { }
		virtual lvsize_t        GetSize() const { return 0; }
	    virtual const lChar16 * GetName() const {
	    	return _name.c_str();
	    }
	    virtual lUInt32         GetFlags() const {
	    	return 0;
	    }
	    virtual bool            IsContainer() const {
	    	return false;
	    }
	};
	CRAssetDir(DocViewNative * native, lString16 path, lString16Collection & items) : _native(native), _path(path) {
		for (int i = 0; i < items.length(); i++) {
			ItemInfo * item = new ItemInfo(items[i]);
			_items.add(item);
		}
	}
    virtual LVContainer * GetParentContainer() { return NULL; }
    //virtual const LVContainerItemInfo * GetObjectInfo(const wchar_t * pname);
    virtual const LVContainerItemInfo * GetObjectInfo(int index) {
    	if (index >= 0 && index < _items.length())
    		return _items[index];
    	return NULL;
    }
    virtual int GetObjectCount() const {
    	return _items.length();
    }
    virtual LVStreamRef OpenStream( const lChar16 * fname, lvopen_mode_t mode ) {
    	if (mode != LVOM_READ)
    		return LVStreamRef();
    	lString16 path(fname);
    	return _native->openResource(path);
    }
    /// returns object size (file size or directory entry count)
    virtual lverror_t GetSize( lvsize_t * pSize ) {
    	if (pSize)
    		*pSize = _items.length();
    	return LVERR_OK;
    }
};

class CRInputStream : public LVStream {
	lvoffset_t pos;
	lvoffset_t size;
	bool eof;
public:

	/// Seek (change file pos)
    /**
        \param offset is file offset (bytes) relateve to origin
        \param origin is offset base
        \param pNewPos points to place to store new file position
        \return lverror_t status: LVERR_OK if success
    */
    virtual lverror_t Seek( lvoffset_t offset, lvseek_origin_t origin, lvpos_t * pNewPos ) {
    	lvoffset_t newpos = pos;
    	switch (origin) {
    	case LVSEEK_CUR:
    		newpos = pos + offset;
    		break;
    	case LVSEEK_SET:
    		newpos = offset;
    		break;
    	case LVSEEK_END:
    		newpos = size + offset;
    		break;
    	}
    	if (newpos < pos) {
    		_resetMethod.callVoid();
    		pos = 0;
    		eof = (size == 0);
    	}
    	if (newpos > pos) {
    		lvoffset_t toSkip = newpos - pos;
        	while (toSkip > 0) {
        		jlong skipped = _skipMethod.callLong(toSkip);
        		if (skipped <= 0)
        			break;
        		pos += skipped;
        		toSkip -= skipped;
    		}
    	}
    	if (pNewPos)
    		*pNewPos = pos;
		eof = (pos >= size);
    	return LVERR_OK;
    }

    /// Get file position
    /**
        \return lvpos_t file position
    */
    virtual lvpos_t GetPos()
    {
        return pos;
    }

    virtual lverror_t SetSize( lvsize_t size ) {
    	return LVERR_NOTIMPL;
    }

    /// Read
    /**
        \param buf is buffer to place bytes read from stream
        \param count is number of bytes to read from stream
        \param nBytesRead is place to store real number of bytes read from stream
        \return lverror_t status: LVERR_OK if success
    */
    virtual lverror_t Read( void * buf, lvsize_t count, lvsize_t * nBytesRead ) {
    	lUInt8 * pbuf = (lUInt8*)buf;
    	lvsize_t bytesRead = 0;
    	if (count <= 0) {
    		if (nBytesRead)
    			*nBytesRead = 0;
    		return LVERR_OK;
    	}
    	int bufSize = count > 32768 ? 32768 : 0;
    	jbyteArray array = _obj->NewByteArray(bufSize);
    	while (count > 0 && pos < size) {
    		int read = _readMethod.callInt(array);
    		if (read <= 0)
    			break;
    		jboolean isCopy;
    		jbyte* data = _obj->GetByteArrayElements(array, &isCopy);
    		memcpy(pbuf, data, read);
    	    _obj->ReleaseByteArrayElements(array, data, 0);
    		bytesRead += read;
    		count -= read;
    		pbuf += read;
    		pos += read;
    	}
    	_obj->DeleteLocalRef(array);
    	eof = (pos >= size);
    	return LVERR_OK;
    }

    virtual lverror_t Write( const void * buf, lvsize_t count, lvsize_t * nBytesWritten ) {
    	return LVERR_NOTIMPL;
    }

    /// Check whether end of file is reached
    /**
        \return true if end of file reached
    */
    virtual bool Eof() {
    	return eof;
    }

    /// Constructor
    CRInputStream(jobject obj)
    : _obj(obj)
    , _closeMethod(_obj, "close", "()V")
    , _readMethod(_obj, "read", "([B)I")
    , _resetMethod(_obj, "reset", "()V")
    , _skipMethod(_obj, "skip", "(J)J")
    {
    	pos = 0;
    	size = 0;
    	eof = false;
    	for (;;) {
    		jlong skipped = _skipMethod.callLong(65536);
    		if (skipped <= 0)
    			break;
    		size += skipped;
    	}
    	_resetMethod.callVoid();
		eof = (size == 0);
    }
    /// Destructor
    virtual ~CRInputStream() {
    	_closeMethod.callVoid();
    }
private:
    CRObjectAccessor _obj;
    CRMethodAccessor _closeMethod;
    CRMethodAccessor _readMethod;
    CRMethodAccessor _resetMethod;
    CRMethodAccessor _skipMethod;
    /*
 void  close()
Closes this stream.

 int  read(byte[] buffer)
Reads at most length bytes from this stream and stores them in the byte array b starting at offset.

 synchronized void  reset()
Resets this stream to the last marked location.

 long  skip(long byteCount)
Skips at most n bytes in this stream.
     *
     *
     */
};

LVStreamRef DocViewNative::openResource(const lString16 & path) {
	jstring str = _env.toJavaString(path);
	jobject obj = _openResourceStreamMethod.callObj((jobject)str);
	_env->DeleteLocalRef(str);
	if (!obj)
		return LVStreamRef();
	return LVStreamRef(new CRInputStream(obj));
}

LVContainerRef DocViewNative::openResourceDir(const lString16 & path) {
	jstring str = _obj.toJavaString(path);
	jobjectArray array = _listResourceDirMethod.callObjArray(str);
	if (!array)
		return LVContainerRef();
	lString16Collection items;
	_obj.fromJavaStringArray(array, items);
	LVContainerRef res(new CRAssetDir(this, path, items));
	_obj->DeleteLocalRef(str);
	_obj->DeleteLocalRef(array);
	return res;
}

DocViewNative::DocViewNative(jobject obj)
	: _obj(obj)
	, _openResourceStreamMethod(_obj, "openResourceStream", "(Ljava/lang/String;)Ljava/io/InputStream;")
	, _listResourceDirMethod(_obj, "listResourceDir", "(Ljava/lang/String;)[Ljava/lang/String;")
	, _widget(NULL)
{
	_assets = openResourceDir(lString16());
}

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

class AndroidGuiExecutorObject;

class AndroidConcurrencyProvider : public CRConcurrencyProvider {
    CRObjectAccessor crviewObject;
    CRMethodAccessor crviewRunInGLThread;
    CRMethodAccessor crviewCreateCRThread;
    CRMethodAccessor crviewSleepMs;
    CRClassAccessor lockClass;
    CRClassAccessor monitorClass;
    CRClassAccessor threadClass;
public:

    class AndroidMutex : public CRMutex {
        CRObjectAccessor mutex;
        CRMethodAccessor lock;
        CRMethodAccessor unlock;
    public:
        AndroidMutex(jobject object)
        		: mutex(object),
        		  lock(mutex, "lock", "()V"),
        		  unlock(mutex, "unlock", "()V") {}
        virtual void acquire() { lock.callVoid(); }
        virtual void release() { unlock.callVoid(); }
    };

    class AndroidMonitor : public CRMonitor {
        CRObjectAccessor monitor;
        CRMethodAccessor lock;
        CRMethodAccessor unlock;
        CRMethodAccessor waitMethod;
        CRMethodAccessor notifyMethod;
        CRMethodAccessor notifyAllMethod;
    public:
        AndroidMonitor(jobject object) :
        	monitor(object),
        	lock(monitor, "lock", "()V"),
        	unlock(monitor, "unlock", "()V"),
        	waitMethod(monitor, "await", "()V"),
        	notifyMethod(monitor, "signal", "()V"),
        	notifyAllMethod(monitor, "signalAll", "()V")
        	{}
        virtual void acquire() { lock.callVoid(); }
        virtual void release() { unlock.callVoid(); }
        virtual void wait() { waitMethod.callVoid(); }
        virtual void notify() { notifyMethod.callVoid(); }
        virtual void notifyAll() { notifyAllMethod.callVoid(); }
    };

    class AndroidThread : public CRThread {
        CRObjectAccessor thread;
        CRMethodAccessor startMethod;
        CRMethodAccessor joinMethod;
    public:
        AndroidThread(jobject object) :
        	thread(object),
        	startMethod(thread, "start", "()V"),
        	joinMethod(thread, "join", "()V")
        	{}
        virtual ~AndroidThread() {
        }
        virtual void start() {
        	startMethod.callVoid();
        }
        virtual void join() {
        	joinMethod.callVoid();
        }
    };

public:
    virtual CRMutex * createMutex() {
        return new AndroidMutex(lockClass.newObject());
    }

    virtual CRMonitor * createMonitor() {
        return new AndroidMonitor(monitorClass.newObject());
    }

    virtual CRThread * createThread(CRRunnable * threadTask) {
        return new AndroidThread(crviewCreateCRThread.callObj((jlong)threadTask));
    }
    virtual void executeGui(CRRunnable * task) {
    	CRLog::trace("executeGui - enter in JNI");
    	crviewRunInGLThread.callVoid((jlong)task);
    	CRLog::trace("executeGui - exit in JNI");
    }

    AndroidConcurrencyProvider(jobject _crviewObject) :
    		crviewObject(_crviewObject),
    		crviewRunInGLThread(crviewObject, "runInGLThread", "(J)V"),
    		crviewCreateCRThread(crviewObject, "createCRThread", "(J)Lorg/coolreader/newui/CRThread;"),
    		crviewSleepMs(crviewObject, "sleepMs", "(J)V"),
    		lockClass("org/coolreader/newui/CRLock"),
    		monitorClass("org/coolreader/newui/CRCondition"),
    		threadClass("org/coolreader/newui/CRThread")

    {
    }
    /// sleep current thread
    virtual void sleepMs(int durationMs) {
    	crviewSleepMs.callVoid((jlong)durationMs);
    }

    virtual ~AndroidConcurrencyProvider() {}
};


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

    DocViewNative * obj = new DocViewNative(_this);
    env->SetIntField(_this, gNativeObjectID, (jint)obj);

	LOGI("initInternal called");
	// set fatal error handler
	crSetFatalErrorHandler( &cr3androidFatalErrorHandler );
	LOGD("Redirecting CDRLog to Android");
	CRLog::setLogger( new JNICDRLogger() );
	CRLog::setLogLevel( CRLog::LL_TRACE );

	CRLog::trace("Setting crash handler");
	crSetSignalHandler();

	SaveJVMPointer(_env);

	concurrencyProvider = new AndroidConcurrencyProvider(_this);

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
    lString16Collection fonts;
    CRStringArrayField fontFilesField(cfg, "fontFiles");
    for (int i = 0; i < fontFilesField.length(); i++) {
    	crconfig.fontFiles.add(fontFilesField.get8(i));
    }
    //env.fromJavaStringArray()

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

/*
 * Class:     org_coolreader_newui_CRView
 * Method:    callCRRunnableInternal
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_coolreader_newui_CRView_callCRRunnableInternal
	(JNIEnv * env, jclass obj, jlong ptr)
{
	CRRunnable * runnable = (CRRunnable*)ptr;
	runnable->run();
}

//============================================================================================================
// register JNI methods

static JNINativeMethod sCRViewMethods[] = {
  /* name, signature, funcPtr */
  {"initInternal", "(Lorg/coolreader/newui/CRConfig;)Z", (void*)Java_org_coolreader_newui_CRView_initInternal},
  {"isLink", "(Ljava/lang/String;)Ljava/lang/String;", (void*)Java_org_coolreader_newui_CRView_isLink},
  {"callCRRunnableInternal", "(J)V", (void*)Java_org_coolreader_newui_CRView_callCRRunnableInternal}
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
