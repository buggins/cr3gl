#ifndef CRUICONFIG_H
#define CRUICONFIG_H

#include <lvstring.h>

struct CRUIConfig {
    lString8 logFile;
    lString8 dbFile;
    lString8 iniFile;
    lString8 hyphDir;
    lString8 resourceDir;
    lString8 coverCacheDir;
    lString8 docCacheDir;
    lString8 i18nDir;
    lString8 uiFontFace;
    lString8 cssDir;

    lString8 systemLanguage;

    int docCacheMaxBytes;
    int coverDirMaxItems;
    int coverDirMaxFiles;
    int coverDirMaxSize;
    int coverRenderCacheMaxItems;
    int coverRenderCacheMaxBytes;

    /// font directories to register all files from
    lString8Collection fontDirs;
    /// separate font files to register
    lString8Collection fontFiles;

    /// sets resourceDir, i18ndir, hyphdir
    void setupResources(lString8 baseDir);
    /// sets coverCache, docCache, iniFile, dbFile
    void setupUserDir(lString8 baseDir);

    void setupResourcesForScreenSize();

    void initEngine();
    void uninitEngine();

    CRUIConfig();
};

extern CRUIConfig crconfig;

#endif // CRUICONFIG_H
