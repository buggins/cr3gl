package org.coolreader.newui;

import java.util.LinkedList;

public class DownloadManager {
	
	public interface DownloadManagerCallback {
	    /// pass download result to window
	    void onDownloadResult(int downloadTaskId, String url, int result, String resultMessage, String mimeType, int size, byte[] data, String file);

	    /// download progress
	    void onDownloadProgress(int downloadTaskId, String url, int result, String resultMessage, String mimeType, int size, int sizeDownloaded);
	}
	
	static int nextTaskId = 1;
	private DownloadManagerCallback callback;
	
	private static class Task {
		final int id;
		final String url;
		final String method;
		final String login;
		final String password;
		final String saveAs;
		boolean cancelled;
		Task(String url, String method, String login, String password, String saveAs) {
			synchronized(DownloadManager.class) {
				id = nextTaskId++;
			}
			this.url = url;
			this.method = method;
			this.login = login;
			this.password = password;
			this.saveAs = saveAs;
		}
		public void execute() {
			if (cancelled)
				return;
			log.d("executing download for " + url);
		}
		public void cancel() {
			cancelled = true;
		}
	}
	
	public static final String TAG = "cr3v";
	public static final Logger log = L.create(TAG);
	
	private final Object lock = new Object();
	private volatile boolean stopped = false;
	
	private Thread thread;
	private LinkedList<Task> queue = new LinkedList<Task>();
	
	private void eventLoop() {
		while (!stopped) {
			Task task = null;
			try {
				synchronized(lock) {
					lock.wait();
					if (queue.size() > 0) {
						task = queue.removeFirst();
					}
				}
			} catch (InterruptedException e) {
				if (stopped)
					break;
			}
			if (stopped)
				break;
			if (task != null)
				task.execute();
		}
	}
	
	public void start() {
		log.i("Starting download manager");
		synchronized(lock) {
			thread = new Thread(new Runnable() {
				@Override
				public void run() {
					eventLoop();
				}
			});
			thread.start();
		}
	}
	
	public void stop() {
		log.i("Stopping download manager");
		synchronized(lock) {
			stopped = true;
			lock.notify();
		}
		for (;;) {
			try {
				thread.join();
				break;
			} catch (InterruptedException e) {
			}
		}
	}
	
	public void setCallback(DownloadManagerCallback callback) {
		this.callback = callback;
	}
	
    /// returns 0 if not supported, task ID if download task is started
    public int openUrl(String url, String method, String login, String password, String saveAs) {
    	if (stopped)
    		return 0;
		log.i("openUrl " + url);
		Task task = new Task(url, method, login, password, saveAs);
		synchronized(lock) {
			queue.addLast(task);
			lock.notify();
		}
        return task.id;
    }
    
    /// cancel specified download task
    public void cancelDownload(int downloadTaskId) { 
		log.i("cancelDownload " + downloadTaskId);
		synchronized(lock) {
			for (Task item : queue) {
				if (item.id == downloadTaskId)
					item.cancel();
			}
		}
    }
    
}
