package org.coolreader.newui;

import java.util.concurrent.locks.ReentrantLock;

public class CRLock {
	protected final ReentrantLock _lock;
	public CRLock() {
		_lock = new ReentrantLock();
	}
	public void lock() {
		_lock.lock();
	}
	public void unlock() {
		_lock.unlock();
	}
}
