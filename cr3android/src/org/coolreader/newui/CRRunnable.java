package org.coolreader.newui;

public class CRRunnable implements Runnable {
	private long ptr;
	public CRRunnable(long ptr) {
		this.ptr = ptr;
	}

	@Override
	public void run() {
		CRView.callCRRunnableInternal(ptr);
	}

}
