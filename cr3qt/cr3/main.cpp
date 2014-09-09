#include "cr3mainwindow.h"

#include "cr3qt.h"
#include <QApplication>

using namespace CRUI;

//class MyApplication : public QApplication {

//};

int main(int argc, char *argv[])
{
    int res = 0;
    {

//        QString serverName = "CoolReader3";
//        QLocalSocket socket;
//        socket.connectToServer(serverName);
//        if (socket.waitForConnected(200))
//            return; // Exit already a process running

//        QLocalServer m_localServer(&a);
//        connect(&m_localServer, SIGNAL(newConnection()), &a, SLOT(newLocalSocketConnection()));
//        m_localServer->listen(serverName);


        lString16 exePath = LVExtractPath(Utf8ToUnicode(argv[0]));
        LVAppendPathDelimiter(exePath);
        InitCREngine(exePath);
        QApplication a(argc, argv);


        OpenGLWindow w;
        lString8 filename;
        if (argc > 1) {
            filename = argv[1];
            if (filename.startsWith("\"") && filename.endsWith("\""))
                filename = filename.substr(1, filename.length() - 2);
            w.setFileToOpenOnStart(filename);
        }
        bool fullscreen = w.getSettings()->getBoolDef(PROP_APP_FULLSCREEN, false);
        if (fullscreen)
            w.showFullScreen();
        else
            w.show();
        res = a.exec();
    }
    crconfig.uninitEngine();
    return res;
}
