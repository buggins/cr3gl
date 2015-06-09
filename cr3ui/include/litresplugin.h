#ifndef LITRESPLUGIN_H
#define LITRESPLUGIN_H

#include "lvstring.h"
#include "cruimain.h"

class LitresPlugin
{
public:
    LitresPlugin();
    virtual ~LitresPlugin();
};

class LitresConnection
{
    CRUIMainWidget * _main;
public:
    LitresConnection(CRUIMainWidget * _main);
    void sendXMLRequest(lString8 url, LVHashTable<lString8, lString8> params, LVXMLParserCallback * contentCallback);
    /// download result
    virtual void onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream);
    /// download progress
    virtual void onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded);

    virtual ~LitresConnection();
};

#endif // LITRESPLUGIN_H
