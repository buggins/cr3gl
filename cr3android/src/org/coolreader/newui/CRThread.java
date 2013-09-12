package org.coolreader.newui;

public class CRThread extends Thread {
	long crrunnablePtr;
	public CRThread(long runnablePtr) {
		this.crrunnablePtr = runnablePtr;
	}
	public void run() {
		CRView.callCRRunnableInternal(crrunnablePtr);
	}
}
