package org.coolreader.newui;

public class DownloadManager {
	
	public interface DownloadManagerCallback {
		
	}
	
	static int nextTaskId = 1;
	
	private static class Task {
		final int id;
		final String url;
		final String method;
		final String login;
		final String password;
		final String saveAs;
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
	}
	
	public static final String TAG = "cr3v";
	public static final Logger log = L.create(TAG);
	
	private final Object lock = new Object();
	private volatile boolean stopped = false;
	
	public void start() {
		synchronized(lock) {
			
		}
	}
	
	public void stop() {
		synchronized(lock) {
			stopped = true;
		}
	}
	
    /// returns 0 if not supported, task ID if download task is started
    public int openUrl(String url, String method, String login, String password, String saveAs) {
		log.i("openUrl " + url);
		Task task = new Task(url, method, login, password, saveAs);
        return 0;
    }
    
    /// cancel specified download task
    public void cancelDownload(int downloadTaskId) { 
		log.i("cancelDownload " + downloadTaskId);
    }
    
}
