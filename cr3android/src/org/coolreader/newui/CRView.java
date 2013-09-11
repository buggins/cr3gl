package org.coolreader.newui;

import java.util.concurrent.locks.ReentrantLock;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;

public class CRView extends GLSurfaceView implements GLSurfaceView.Renderer {

	public CRView(Context context) {
		super(context);
		setRenderer(this);
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int w, int h) {
		gl.glViewport(0, 0, w, h);
		java.util.concurrent.Semaphore s;
		java.util.concurrent.locks.ReentrantLock l;
		java.lang.Thread t;
	}
	
	/**
	 * Call from JNI to execute CRRunnable in GUI (GL) thread
	 * @param crRunnablePtr is value of C pointer to CRRunnable object 
	 */
	public void runInGLThread(long crRunnablePtr) {
		queueEvent(new CRRunnable(crRunnablePtr));
	}
	
	public Thread createCRThread(long crRunnablePtr) {
		return new Thread(new CRRunnable(crRunnablePtr));
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig cfg) {
		// do nothing
	}
	
	private int mNativeObject; // holds pointer to native object instance
	native public boolean initInternal(CRConfig config);
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
	
	
	static {
		System.loadLibrary("crenginegl");
	}
}
