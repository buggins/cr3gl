#include "litresplugin.h"

#define AUTHORIZE_URL "http://robot.litres.ru/pages/catalit_authorise/"
#define REGISTER_URL "http://robot.litres.ru/pages/catalit_register_user/"
#define GENRES_URL "http://robot.litres.ru/pages/catalit_genres/"
#define AUTHORS_URL "http://robot.litres.ru/pages/catalit_persons/"
#define CATALOG_URL "http://robot.litres.ru/pages/catalit_browser/"
#define TRIALS_URL "http://robot.litres.ru/static/trials/"
#define PURCHASE_URL "http://robot.litres.ru/pages/purchase_book/"
#define DOWNLOAD_BOOK_URL "http://robot.litres.ru/pages/catalit_download_book/"
#define P_ID "8786915";
const int CONNECT_TIMEOUT = 60000;
const int READ_TIMEOUT = 60000;
const int MAX_CONTENT_LEN_TO_BUFFER = 1000000;

LitresConnection::LitresConnection(CRUIMainWidget * main)
    : _main(main) {
}

LitresConnection::~LitresConnection() {

}

void LitresConnection::sendXMLRequest(lString8 url, LVHashTable<lString8, lString8> params, LVXMLParserCallback * contentCallback) {

}

/// download result
void LitresConnection::onDownloadResult(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, LVStreamRef stream) {

}

/// download progress
void LitresConnection::onDownloadProgress(int downloadTaskId, lString8 url, int result, lString8 resultMessage, lString8 mimeType, int size, int sizeDownloaded) {

}

LitresPlugin::LitresPlugin()
{

}

LitresPlugin::~LitresPlugin()
{

}

