package org.coolreader.newui;

import java.io.File;
import java.io.FileInputStream;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Map;

import android.app.Activity;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;

public class CoolReader extends Activity {
	
	public CRView crview;
	public static final String TAG = "cr3";
	public static final Logger log = L.create(TAG);


	private CRConfig createConfig() {
		CRConfig cfg = new CRConfig();
		DisplayMetrics metrics = getResources().getDisplayMetrics();
		cfg.screenDPI = (int)(metrics.density * 160);
		cfg.screenX = metrics.widthPixels;
		cfg.screenY = metrics.heightPixels;
		
		initMountRoots();
		
		String dataDir = null;
		PackageManager m = getPackageManager();
		String s = getPackageName();
	    String externalFilesDir = getExternalFilesDir(null).getAbsolutePath();
		try {
		    PackageInfo p = m.getPackageInfo(s, 0);
		    
		    dataDir = p.applicationInfo.dataDir;
		    log.i("DataDir = " + dataDir);
		    log.i("ExternalFilesDir = " + dataDir);
		} catch (NameNotFoundException e) {
		    Log.w(TAG, "Error Package name not found ", e);
		}
		
		cfg.assetManager = getAssets();

		String externalStorageDir = Environment.getExternalStorageDirectory().getAbsolutePath();

		cfg.coverCacheDir = externalStorageDir + "/.cr3/coverpages";
		cfg.cssDir = externalStorageDir + "/.cr3/css";
		cfg.docCacheDir = externalStorageDir + "/.cr3/doccache";
		cfg.dbFile = externalStorageDir + "/.cr3/cr3new.sqlite";
		cfg.iniFile = externalStorageDir + "/.cr3/cr3new.ini";
		
		
		cfg.hyphDir = externalFilesDir + "/hyph";
		cfg.i18nDir = externalFilesDir + "/i18n";
		cfg.resourceDir = externalFilesDir + "/.cr3/res"; // TODO
		
		cfg.systemLanguage = "en"; // TODO
		cfg.uiFontFace = "Droid Sans";
		cfg.fontFiles = findFonts();
		
		return cfg;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		crview = new CRView(this);
		crview.init(createConfig());
		setContentView(crview);
	}

    @Override
    protected void onPause() {
        super.onPause();
        crview.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        crview.onResume();
    }	
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.cool_reader, menu);
		return true;
	}

	
	
	
	//=================================================================================================
	
	
	/**
	 * Returns array of writable data directories on external storage
	 * 
	 * @param subdir
	 * @param createIfNotExists
	 * @return
	 */
	public static File[] getDataDirectories(String subdir,
			boolean createIfNotExists, boolean writableOnly) {
		File[] roots = getStorageDirectories(writableOnly);
		ArrayList<File> res = new ArrayList<File>(roots.length);
		for (File dir : roots) {
			File dataDir = getSubdir(dir, ".cr3", createIfNotExists,
					writableOnly);
			if (subdir != null)
				dataDir = getSubdir(dataDir, subdir, createIfNotExists,
						writableOnly);
			if (dataDir != null)
				res.add(dataDir);
		}
		return res.toArray(new File[] {});
	}

	/**
	 * Get or create writable subdirectory for specified base directory
	 * 
	 * @param dir
	 *            is base directory
	 * @param subdir
	 *            is subdirectory name, null to use base directory
	 * @param createIfNotExists
	 *            is true to force directory creation
	 * @return writable directory, null if not exist or not writable
	 */
	public static File getSubdir(File dir, String subdir,
			boolean createIfNotExists, boolean writableOnly) {
		if (dir == null)
			return null;
		File dataDir = dir;
		if (subdir != null) {
			dataDir = new File(dataDir, subdir);
			if (!dataDir.isDirectory() && createIfNotExists)
				dataDir.mkdir();
		}
		if (dataDir.isDirectory() && (!writableOnly || dataDir.canWrite()))
			return dataDir;
		return null;
	}


	/**
	 * Get storage root directories.
	 * 
	 * @return array of r/w storage roots
	 */
	public static File[] getStorageDirectories(boolean writableOnly) {
		Collection<File> res = new HashSet<File>(2);
		for (File dir : mountedRootsList) {
			if (dir.isDirectory() && (!writableOnly || dir.canWrite()))
				res.add(dir);
		}
		return res.toArray(new File[res.size()]);
	}
	
	private static String[] findFonts() {
		ArrayList<File> dirs = new ArrayList<File>();
		File[] dataDirs = getDataDirectories("fonts", false, false);
		for (File dir : dataDirs)
			dirs.add(dir);
		File[] rootDirs = getStorageDirectories(false);
		for (File dir : rootDirs)
			dirs.add(new File(dir, "fonts"));
		dirs.add(new File(Environment.getRootDirectory(), "fonts"));
		ArrayList<String> fontPaths = new ArrayList<String>();
		for (File fontDir : dirs) {
			if (fontDir.isDirectory()) {
				log.v("Scanning directory " + fontDir.getAbsolutePath()
						+ " for font files");
				// get font names
				String[] fileList = fontDir.list(new FilenameFilter() {
					public boolean accept(File dir, String filename) {
						String lc = filename.toLowerCase();
						return (lc.endsWith(".ttf") || lc.endsWith(".otf")
								|| lc.endsWith(".pfb") || lc.endsWith(".pfa"))
//								&& !filename.endsWith("Fallback.ttf")
								;
					}
				});
				// append path
				for (int i = 0; i < fileList.length; i++) {
					String pathName = new File(fontDir, fileList[i])
							.getAbsolutePath();
					fontPaths.add(pathName);
					log.v("found font: " + pathName);
				}
			}
		}
		Collections.sort(fontPaths);
		return fontPaths.toArray(new String[] {});
	}

	
	private static File[] mountedRootsList;
	private static Map<String, String> mountedRootsMap;
	private static MountPathCorrector pathCorrector;
	
	private static boolean addMountRoot(Map<String, String> list, String path, String name) {
		if (list.containsKey(path))
			return false;
		for (String key : list.keySet()) {
			if (path.equals(key)) { // path.startsWith(key + "/")
				log.w("Skipping duplicate path " + path + " == " + key);
				return false; // duplicate subpath
			}
		}
		try {
			File dir = new File(path);
			if (dir.isDirectory()) {
//				String[] d = dir.list();
//				if ((d!=null && d.length>0) || dir.canWrite()) {
					log.i("Adding FS root: " + path + " " + name);
					list.put(path, name);
//					return true;
//				} else {
//					log.i("Skipping mount point " + path + " : no files or directories found here, and writing is disabled");
//				}
			}
		} catch (Exception e) {
			// ignore
		}
		return false;
	}
	
	public static String loadFileUtf8(File file) {
		try {
			InputStream is = new FileInputStream(file);
			return loadResourceUtf8(is);
		} catch (Exception e) {
			log.w("cannot load resource from file " + file);
			return null;
		}
	}

	public static String loadResourceUtf8(InputStream is) {
		try {
			int available = is.available();
			if (available <= 0)
				return null;
			byte buf[] = new byte[available];
			if (is.read(buf) != available)
				throw new IOException("Resource not read fully");
			is.close();
			String utf8 = new String(buf, 0, available, "UTF8");
			return utf8;
		} catch (Exception e) {
			log.e("cannot load resource");
			return null;
		}
	}

	public static byte[] loadResourceBytes(File f) {
		if (f == null || !f.isFile() || !f.exists())
			return null;
		FileInputStream is = null;
		try {
			is = new FileInputStream(f);
			byte[] res = loadResourceBytes(is);
			return res;
		} catch (IOException e) {
			log.e("Cannot open file " + f);
		}
		return null;
	}

	public static byte[] loadResourceBytes(InputStream is) {
		try {
			int available = is.available();
			if (available <= 0)
				return null;
			byte buf[] = new byte[available];
			if (is.read(buf) != available)
				throw new IOException("Resource not read fully");
			is.close();
			return buf;
		} catch (Exception e) {
			log.e("cannot load resource");
			return null;
		}
	}

	private static void initMountRoots() {
		Map<String, String> map = new LinkedHashMap<String, String>();

		// standard external directory
		String sdpath = Environment.getExternalStorageDirectory().getAbsolutePath();
		// dirty fix
		if ("/nand".equals(sdpath) && new File("/sdcard").isDirectory())
			sdpath = "/sdcard";
		// main storage
		addMountRoot(map, sdpath, "SD CARD");

		// retrieve list of mount points from system
		String[] fstabLocations = new String[] {
			"/system/etc/vold.conf",
			"/system/etc/vold.fstab",
			"/etc/vold.conf",
			"/etc/vold.fstab",
		};
		String s = null;
		String fstabFileName = null;
		for (String fstabFile : fstabLocations) {
			fstabFileName = fstabFile;
			s = loadFileUtf8(new File(fstabFile));
			if (s != null)
				log.i("found fstab file " + fstabFile);
		}
		if (s == null)
			log.w("fstab file not found");
		if ( s!= null) {
			String[] rows = s.split("\n");
			int rulesFound = 0;
			for (String row : rows) {
				if (row != null && row.startsWith("dev_mount")) {
					log.d("mount rule: " + row);
					rulesFound++;
					String[] cols = Utils.splitByWhitespace(row);
					if (cols.length >= 5) {
						String name = Utils.ntrim(cols[1]);
						String point = Utils.ntrim(cols[2]);
						String mode = Utils.ntrim(cols[3]);
						String dev = Utils.ntrim(cols[4]);
						if (Utils.empty(name) || Utils.empty(point) || Utils.empty(mode) || Utils.empty(dev))
							continue;
						String label = null;
						boolean hasusb = dev.indexOf("usb") >= 0;
						boolean hasmmc = dev.indexOf("mmc") >= 0;
						log.i("*** mount point '" + name + "' *** " + point + "  (" + dev + ")");
						if ("auto".equals(mode)) {
							// assume AUTO is for externally automount devices
							if (hasusb)
								label = "USB Storage";
							else if (hasmmc)
								label = "External SD";
							else
								label = "External Storage";
						} else {
							if (hasmmc)
								label = "Internal SD";
							else
								label = "Internal Storage";
						}
						if (!point.equals(sdpath)) {
							// external SD
							addMountRoot(map, point, label + " (" + point + ")");
						}
					}
				}
			}
			if (rulesFound == 0)
				log.w("mount point rules not found in " + fstabFileName);
		}

		// TODO: probably, hardcoded list is not necessary after /etc/vold parsing 
		String[] knownMountPoints = new String[] {
			"/system/media/sdcard", // internal SD card on Nook
			"/media",
			"/nand",
			"/PocketBook701", // internal SD card on PocketBook 701 IQ
			"/mnt/extsd",
			"/mnt/external1",
			"/mnt/external_sd",
			"/mnt/udisk",
			"/mnt/sdcard2",
			"/mnt/ext.sd",
			"/ext.sd",
			"/extsd",
			"/sdcard",
			"/sdcard2",
			"/mnt/udisk",
			"/sdcard-ext",
			"/sd-ext",
			"/mnt/external1",
			"/mnt/external2",
			"/mnt/sdcard1",
			"/mnt/sdcard2",
			"/mnt/usb_storage",
			"/mnt/external_sd",
			"/emmc",
			"/external",
			"/Removable/SD",
			"/Removable/MicroSD",
			"/Removable/USBDisk1", 
		};
		for (String point : knownMountPoints) {
			String link = CRView.isLink(point);
			if (link != null) {
				log.d("standard mount point path is link: " + point + " > " + link);
				addMountRoot(map, link, link);
			} else {
				addMountRoot(map, point, point);
			}
		}
		
		// auto detection
		//autoAddRoots(map, "/", SYSTEM_ROOT_PATHS);
		//autoAddRoots(map, "/mnt", new String[] {});
		
		mountedRootsMap = map;
		Collection<File> list = new ArrayList<File>();
		log.i("Mount ROOTS:");
		for (String f : map.keySet()) {
			File path = new File(f);
			list.add(path);
			String label = map.get(f);
			log.i("*** " + f + " '" + label + "' isDirectory=" + path.isDirectory() + " canRead=" + path.canRead() + " canWrite=" + path.canRead() + " isLink=" + CRView.isLink(f));
		}
		mountedRootsList = list.toArray(new File[] {});
		pathCorrector = new MountPathCorrector(mountedRootsList);

		for (String point : knownMountPoints) {
			String link = CRView.isLink(point);
			if (link != null)
				pathCorrector.addRootLink(point, link);
		}
		
		Log.i("cr3", "Root list: " + list + ", root links: " + pathCorrector);
//		testPathNormalization("/sdcard/books/test.fb2");
//		testPathNormalization("/mnt/sdcard/downloads/test.fb2");
//		testPathNormalization("/mnt/sd/dir/test.fb2");
	}

}
