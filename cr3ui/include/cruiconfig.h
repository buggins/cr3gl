#ifndef CRUICONFIG_H
#define CRUICONFIG_H

#include <lvstring.h>
#include <lvptrvec.h>

struct CRUIInterfaceLanguage {
    lString8 id;
    lString8 nameRes;
    lString16 name;
    lString8 fileName;
    lString16 getName();
    CRUIInterfaceLanguage(const char * _id, const char * _nameRes, lString8 _fileName)
        : id(_id), nameRes(_nameRes), fileName(_fileName) {

    }
};

struct CRUIHyphenationDictionary {
    lString8 id;
    lString8 langCode;
    lString8 nameRes;
    lString16 name;
    lString8 fileName;
    lString16 getName();
    CRUIHyphenationDictionary(const char * _id, const char * _langCode, const char * _nameRes, lString8 _fileName)
        : id(_id), langCode(_langCode), nameRes(_nameRes), fileName(_fileName) {

    }
};

typedef int TEXTURE_FORMAT;
enum {
    TEXTURE_RGBA = 0,
    TEXTURE_LUMINANCE_ALPHA = 1,
    TEXTURE_LUMINANCE = 2,
    TEXTURE_ALPHA = 3
};

struct CRUIConfig {
    lString8 logFile;
    lString8 dbFile;
    lString8 iniFile;
    lString8 hyphDir;
    lString8 resourceDir;
    lString8 coverCacheDir;
    lString8 docCacheDir;
    lString8 i18nDir;
    lString8 themesDir;
    lString8 manualsDir;
    lString8 manualFile;
    lString8 currentThemeDir;
    lString8 uiFontFace;
    lString8 fallbackFontFace;
    lString8 monoFontFace;
    lString8 cssDir;
    lString8 defaultDownloadsDir;

    lString8 systemLanguage;

    int docCacheMaxBytes;
    int coverDirMaxItems;
    int coverDirMaxFiles;
    int coverDirMaxSize;
    int coverRenderCacheMaxItems;
    int coverRenderCacheMaxBytes;

    int minFontSize;
    int maxFontSize;
    int defFontSize;

    int apiLevel;
    bool hasBattery;
    int batteryLevel;

    bool touchMode;
    bool enableOpenGl;
    bool einkMode;
    bool einkModeSettingsSupported;
    bool updateScreenModeInCurrentThread;
    TEXTURE_FORMAT textureFormat;
    TEXTURE_FORMAT getTextureFormat() { return !einkMode ? TEXTURE_RGBA : TEXTURE_ALPHA; }

    /// font directories to register all files from
    lString8Collection fontDirs;
    /// separate font files to register
    lString8Collection fontFiles;

    LVPtrVector<CRUIInterfaceLanguage> interfaceLanguages;
    LVPtrVector<CRUIHyphenationDictionary> hyphenationDictionaries;

    bool setHyphenationDictionary(lString8 id, lString8 fallbackId);

    void setInterfaceLanguage(lString8 id);

    CRUIHyphenationDictionary * findHyphenationDictionary(lString8 id);

    CRUIInterfaceLanguage * findInterfaceLanguage(lString8 id);

    /// sets resourceDir, i18ndir, hyphdir
    void setupResources(lString8 baseDir);

    /// loads theme from themesDirectory + "/" + theme + "/cr3theme.xml"; pass empty string to reload current theme
    void setTheme(lString8 themeName);
    /// sets coverCache, docCache, iniFile, dbFile
    void setupUserDir(lString8 baseDir);

    void setupResourcesForScreenSize();

    /// on GL context close, delete all GL objects
    void clearGraphicsCaches();

    void initEngine(bool setLogger = true);
    void uninitEngine();
    void startBackgroundThreads();

    CRUIConfig();
private:
    /// loads theme from themesDirectory + "/" + theme + "/cr3theme.xml"; pass empty string to reload current theme
    void loadTheme();
};

extern CRUIConfig crconfig;

#endif // CRUICONFIG_H
