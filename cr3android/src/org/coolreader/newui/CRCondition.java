package org.coolreader.newui;

import java.util.concurrent.locks.Condition;

public class CRCondition extends CRLock {
	private final Condition _condition;
	public CRCondition() {
		super();
		_condition = _lock.newCondition();
	}
	public void await() {
		for (;;) {
			try {
				_condition.await();
				break;
			} catch (InterruptedException e) {
				_lock.lock();
			}
		}
	}
	public void signal() {
		_condition.signal();
	}
	public void signalAll() {
		_condition.signalAll();
	}
}
