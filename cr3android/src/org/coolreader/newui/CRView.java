package org.coolreader.newui;

import java.io.IOException;
import java.io.InputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;

public class CRView extends GLSurfaceView implements GLSurfaceView.Renderer {

	public static final String TAG = "cr3v";
	public static final Logger log = L.create(TAG);
	
	public CRView(Context context) {
		super(context);
		mAssetManager = context.getAssets();
		setRenderer(this);
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
		drawInternal();
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int w, int h) {
		gl.glViewport(0, 0, w, h);
		log.i("Java: onSurfaceChanged(" + w + "," + h + ")");
		surfaceChangedInternal(w, h);
	}
	
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig cfg) {
		// do nothing
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		surfaceDestroyedInternal();
		super.surfaceDestroyed(holder);
	}
	
	

	@Override
	public void onPause() {
		super.onPause();
	}

	@Override
	public void onResume() {
		super.onResume();
	}

	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event) {
		// process in GL thread
		queueEvent(new Runnable() {
			@Override
			public void run() {
				handleKeyEventInternal(event);
			}
		});
		return true;
	}

	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event) {
		// process in GL thread
		queueEvent(new Runnable() {
			@Override
			public void run() {
				handleKeyEventInternal(event);
			}
		});
		return true;
	}

	@Override
	public boolean onTouchEvent(final MotionEvent event) {
		// process in GL thread
		queueEvent(new Runnable() {
			@Override
			public void run() {
				handleTouchEventInternal(event);
			}
		});
		return true;
	}

	// accessible from Java
	public boolean init(CRConfig config) {
		log.i("calling initInternal");
		return initInternal(config);
	}
	
	//=======================================================================================
	// JNI interfacing methods

	// accessible from Java
	native private boolean initInternal(CRConfig config);
	
	native private boolean uninitInternal();

	native private void drawInternal();

	native private void surfaceChangedInternal(int x, int y);
	
	native private void surfaceDestroyedInternal();

	native private boolean handleKeyEventInternal(KeyEvent event);

	native private boolean handleTouchEventInternal(MotionEvent event);

	
	// accessible from Java JNI calls
	
	/**
	 * Checks whether specified directlry or file is symbolic link.
	 * (thread-safe)
	 * @param pathName is path to check
	 * @return path link points to if specified directory is link (symlink), null for regular file/dir
	 */
	public native static String isLink(String pathName);

	/**
	 * Calls CRRunnable.run() method.
	 * @param ptr is actually C pointer to CRRunnable
	 */
	public native static void callCRRunnableInternal(long ptr);
	
	
	// accessible from JNI only
	
	/**
	 * Call from JNI to execute CRRunnable in GUI (GL) thread
	 * @param crRunnablePtr is value of C pointer to CRRunnable object 
	 */
	private final void runInGLThread(long crRunnablePtr) {
		log.d("runInGLThread - in Java");
		queueEvent(new CRRunnable(crRunnablePtr));
	}

	/**
	 * Call from JNI to create thread with JNI CRRunnable callback.
	 * @param crRunnablePtr
	 * @return newly created thread
	 */
	private final CRThread createCRThread(long crRunnablePtr) {
		return new CRThread(crRunnablePtr);
	}
	
	/**
	 * Call from JNI thread to sleep current thread for specified time
	 * @param ms is millis to sleep
	 */
	private final void sleepMs(long ms) {
		try {
			Thread.sleep(ms);
		} catch (InterruptedException e) {
			// ignore
		}
	}

	private final InputStream openResourceStream(String path) {
		try {
			return mAssetManager.open(path, AssetManager.ACCESS_RANDOM);
		} catch (IOException e) {
			return null;
		}
	}
	
	private final String[] listResourceDir(String path) {
		try {
			return mAssetManager.list(path);
		} catch (IOException e) {
			return null;
		}
	}
	
	private long mNativeObject; // holds pointer to native object instance
	private AssetManager mAssetManager;

	// ======================================================================================================================
	
	static {
		System.loadLibrary("crenginegl");
	}
}
