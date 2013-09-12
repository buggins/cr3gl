package org.coolreader.newui;

import android.content.res.AssetManager;

public class CRConfig {
	public int screenX;
	public int screenY;
	public int screenDPI;
	
    public String logFile;
    public String dbFile;
    public String iniFile;
    public String hyphDir;
    public String resourceDir;
    public String coverCacheDir;
    public String docCacheDir;
    public String i18nDir;
    public String uiFontFace;
    public String fallbackFontFace;
    public String cssDir;
    
    public String internalStorageDir;
    public String sdcardDir;

    public String systemLanguage;

    public int docCacheMaxBytes;
    public int coverDirMaxItems;
    public int coverDirMaxFiles;
    public int coverDirMaxSize;
    public int coverRenderCacheMaxItems;
    public int coverRenderCacheMaxBytes;

    /// font directories to register all files from
    public String[] fontDirs;
    /// separate font files to register
    public String[] fontFiles;
    
    public AssetManager assetManager;
    
}
