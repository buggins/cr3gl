package org.coolreader.newui;

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
		
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig cfg) {
		// do nothing
	}

}
