package org.coolreader.newui;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Authenticator;
import java.net.HttpURLConnection;
import java.net.PasswordAuthentication;
import java.net.URL;
import java.util.LinkedList;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSession;

public class DownloadManager {
	
	public interface DownloadManagerCallback {
	    /// pass download result to window
	    void onDownloadResult(int downloadTaskId, String url, int result, String resultMessage, String mimeType, int size, byte[] data);

	    /// download progress
	    void onDownloadProgress(int downloadTaskId, String url, int result, String resultMessage, String mimeType, int size, int sizeDownloaded);
	}
	
	static int nextTaskId = 1;
	private DownloadManagerCallback callback;
	
	private class Task {
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
		
		private void error(int code, String message) {
			callback.onDownloadResult(id, url, code, message, "", 0, null);
		}
		
		public void execute() {
			if (cancelled)
				return;
			log.d("executing download for " + url);
	        InputStream input = null;
	        OutputStream output = null;
	        HttpURLConnection connection = null;
	        try {
	            URL url = new URL(this.url);
	            connection = (HttpURLConnection) url.openConnection();
	            if (connection instanceof HttpsURLConnection) {
	            	HttpsURLConnection https = (HttpsURLConnection)connection;
	            	// TODO: implement https stuff
	            	https.setHostnameVerifier(new HostnameVerifier() {
						@Override
						public boolean verify(String arg0, SSLSession arg1) {
							return true;
						}
					});
	            }
	            connection.setInstanceFollowRedirects(true);
	            connection.setUseCaches(false);
	            if (login != null && login.length() > 0) {
	            	Authenticator.setDefault(new Authenticator(){
	            	    protected PasswordAuthentication getPasswordAuthentication() {
	            	        return new PasswordAuthentication(login, password.toCharArray());
	            	    }});	            	
	            }
	            connection.connect();

	            // expect HTTP 200 OK, so we don't mistakenly save error report
	            // instead of the file
	            int code = connection.getResponseCode();
	            if (code != HttpURLConnection.HTTP_OK) {
	            	error(code, "Server returned HTTP " + code
	                        + " " + connection.getResponseMessage());
	            	return;
	            }

	            // this will be useful to display download percentage
	            // might be -1: server did not report the length
	            int fileLength = connection.getContentLength();
	            String mimeType = connection.getContentType();

	            // download the file
	            input = connection.getInputStream();
	            ByteArrayOutputStream buf = null;
	            if (saveAs != null && saveAs.length() > 0) {
	            	output = new FileOutputStream(new File(saveAs));
	            } else {
	            	buf = new ByteArrayOutputStream();
	            	output = buf;
	            }

	            byte data[] = new byte[4096];
	            long total = 0;
	            int count;
	            while ((count = input.read(data)) != -1) {
	                // allow canceling with back button
	                if (cancelled) {
	                    input.close();
	                    error(1, "Cancelled");
	                    break;
	                }
	                total += count;
	                // publishing the progress....
	                if (fileLength > 0) {// only if total length is known
	                	callback.onDownloadProgress(id, this.url, code, "", mimeType, (int)total, fileLength);
	                }
	                output.write(data, 0, count);
	            }
	            if (!cancelled) {
	            	byte[] res = null;
	            	if (buf != null) {
	            		res = buf.toByteArray();
	            	}
	            	callback.onDownloadResult(id, this.url, 0, "OK", mimeType, (int)total, res);
	            }
	        } catch (Exception e) {
            	error(2, "Error while downloading: " + e.getMessage());
            	return;
	        } finally {
	            try {
	                if (output != null)
	                    output.close();
	                if (input != null)
	                    input.close();
	            } catch (IOException ignored) {
	            }

	            if (connection != null)
	                connection.disconnect();
	        }
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
