package org.coolreader.newui;

import java.io.IOException;
import java.io.InputStream;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.text.InputType;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

public class CRView extends GLSurfaceView implements GLSurfaceView.Renderer {

	public static final String TAG = "cr3v";
	public static final Logger log = L.create(TAG);
	
	private CoolReader activity; 
	private volatile boolean surfaceOk = false;
	
	public CRView(Context context) {
		super(context);
		this.activity = (CoolReader)context;
		mAssetManager = context.getAssets();
		setRenderer(this);
		setFocusable(true);
		setFocusableInTouchMode(true);
		requestFocus();
		getHolder().addCallback(this);
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
		surfaceOk = true;
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		super.surfaceDestroyed(holder);
		log.i("CRView.surfaceDestroyed");
		surfaceOk = false;
		surfaceDestroyedInternal();
	}
	
	@Override
	public void onPause() {
		log.i("CRView.onPause");
		queueEvent(new Runnable() {
			@Override
			public void run() {
				// clear GL caches
				log.i("CRView.onPause - calling surfaceDestroyedInternal()");
				surfaceDestroyedInternal();
			}
		});
		super.onPause();
	}
	
	int lastBatteryLevel = 100;
	public void setBatteryLevel(final int level) {
		if (level == lastBatteryLevel)
			return;
		lastBatteryLevel = level;
		log.i("CRView.setBatteryLevel " + level);
		queueEvent(new Runnable() {
			@Override
			public void run() {
				setBatteryLevelInternal(level);
			}
		});
	}
	
	public void loadBook(final String pathName) {
		log.i("CRView.loadBook " + pathName);
		queueEvent(new Runnable() {
			@Override
			public void run() {
				loadBookInternal(pathName);
			}
		});
	}
	
	@Override
	public void onResume() {
		super.onResume();
	}
	
	/// call when application is being closed
	public void uninit() {
		uninitInternal();
	}

	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event) {
		// process in GL thread
		queueEvent(new Runnable() {
			@Override
			public void run() {
				handleKeyEventInternal(event);
				requestRender();
			}
		});
		return true;
	}

	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event) {
		// process in GL thread
		FutureTask<Boolean> f = new FutureTask<Boolean>(new Callable<Boolean>() {
			@Override
			public Boolean call() throws Exception {
				return handleKeyEventInternal(event);
			}
		});
		queueEvent(f);
		for (;;) {
			try {
				return f.get();
			} catch (InterruptedException e) {
				// retry
			} catch (ExecutionException e) {
				return false;
			}
		}
	}

	@Override
	public boolean onTouchEvent(final MotionEvent event) {
		// process in GL thread
		FutureTask<Boolean> f = new FutureTask<Boolean>(new Callable<Boolean>() {
			@Override
			public Boolean call() throws Exception {
				return handleTouchEventInternal(event);
			}
		});
		queueEvent(f);
		for (;;) {
			try {
				return f.get();
			} catch (InterruptedException e) {
				// retry
			} catch (ExecutionException e) {
				return false;
			}
		}
	}


	class CRInputConnection extends BaseInputConnection {
		private final KeyEvent delKeyDownEvent = new KeyEvent(
				KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL);
		private final KeyEvent delKeyUpEvent = new KeyEvent(KeyEvent.ACTION_UP,
				KeyEvent.KEYCODE_DEL);

		public CRInputConnection(View view) {
			super(view, false);
			this.setSelection(0, 0);
		}
		
		

		@Override
		public boolean sendKeyEvent(KeyEvent event) {
			log.d("sendKeyEvent " + event);
			return onKeyUp(event.getKeyCode(), event);
			//return onKeyUp(event.getKeyCode(), event);
			//return super.sendKeyEvent(event);
		}



		@Override
		public boolean commitText(CharSequence text, int newCursorPosition) {
			// TODO Auto-generated method stub
			log.d("commitText " + text);
			KeyEvent ev = new KeyEvent(System.currentTimeMillis(), text.toString(), 0, 0);
			return onKeyUp(0, ev);
			//return super.commitText(text, newCursorPosition);
		}



		@Override
		public boolean deleteSurroundingText(int leftLength, int rightLength) {
			// Android SDK 16+ doesn't send key events for backspace but calls
			// this method
			log.d("deleteSurroundingText " + leftLength + ", " + rightLength);
			onKeyDown(KeyEvent.KEYCODE_DEL,
					this.delKeyDownEvent);
			onKeyUp(KeyEvent.KEYCODE_DEL,
					this.delKeyUpEvent);
			return super.deleteSurroundingText(leftLength, rightLength);
		}
	}

	@Override
	public InputConnection onCreateInputConnection(EditorInfo outAttributes) // required for creation of soft keyboard
	{
/*		outAttributes.actionId = EditorInfo.IME_ACTION_DONE;
		outAttributes.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI;
		outAttributes.inputType = InputType.TYPE_CLASS_TEXT;
		return new CRInputConnection(this);
*/	
		outAttributes.actionLabel = "";
		outAttributes.hintText = "";
		outAttributes.initialCapsMode = 0;
		outAttributes.initialSelEnd = outAttributes.initialSelStart = -1;
		outAttributes.label = "";
		outAttributes.imeOptions = EditorInfo.IME_ACTION_NONE; // | IME_ACTION_DONE EditorInfo.IME_FLAG_NO_EXTRACT_UI;        
		outAttributes.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS ; //NULL; //TEXT;        

		return new CRInputConnection(this);
	    //return  new BaseInputConnection(this, false);  
	}

	@Override
	public boolean onCheckIsTextEditor() // required for creation of soft keyboard
	{
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

	native private void loadBookInternal(String pathName);
	
	native private void setBatteryLevelInternal(int level);

	
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
		//log.d("runInGLThread - in Java");
		queueEvent(new CRRunnable(crRunnablePtr));
	}

	/**
	 * Call from JNI to execute CRRunnable in GUI (GL) thread
	 * @param crRunnablePtr is value of C pointer to CRRunnable object 
	 */
	private final void runInGLThreadDelayed(final long crRunnablePtr, long delayMillis) {
		log.d("runInGLThreadDelayed - in Java");
		postDelayed(new Runnable() {
			public void run() {
				if (surfaceOk)
					queueEvent(new CRRunnable(crRunnablePtr));
			}
		}, delayMillis);
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
	
	private final void updateScreen(boolean updateNow, boolean animation) {
		if (updateNow)
			requestRender();
		if (!animation)
			setRenderMode(RENDERMODE_WHEN_DIRTY);
		else
			setRenderMode(RENDERMODE_CONTINUOUSLY);
	}
	
	private final void exitApp() {
		activity.finish();
	}

	private final void minimizeApp() {
		Intent intent = new Intent(Intent.ACTION_MAIN);
		intent.addCategory(Intent.CATEGORY_HOME);
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		activity.startActivity(intent);
	}
	
	private final void copyToClipboard(String s) {
		activity.copyToClipboard(s);
	}
	
	private final void showVirtualKeyboard() {
		post(new Runnable() {
			public void run() {
				setFocusable(true);
				setFocusableInTouchMode(true);
				requestFocus();
				activity.showVirtualKeyboard();
			}
		});
	}

	private final void hideVirtualKeyboard() {
		post(new Runnable() {
			public void run() {
				activity.hideVirtualKeyboard();
			}
		});
	}

    /// returns 0 if not supported, task ID if download task is started
    private int openUrl(String url, String method, String login, String password, String saveAs) {
		log.i("openUrl " + url);
        return activity.getDownloadManager().openUrl(url, method, login, password, saveAs);
    }
    
    /// cancel specified download task
    private void cancelDownload(int downloadTaskId) { 
		log.i("cancelDownload " + downloadTaskId);
		activity.getDownloadManager().cancelDownload(downloadTaskId);
    }
    
	private boolean fullscreen = false;
	private boolean isFullscreen() {
		return fullscreen;
	}
	
	private void setFullscreen(final boolean fullscreen) {
		if (this.fullscreen != fullscreen) {
			this.fullscreen = fullscreen;
			post(new Runnable() {
				public void run() {
					activity.setFullscreen(fullscreen);
				}
			});
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
