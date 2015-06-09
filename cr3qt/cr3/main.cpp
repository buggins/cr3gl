#include "cr3mainwindow.h"

#include "cr3qt.h"
#include <QApplication>
#include <QtSingleApplication>

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


        lString8 filename;
        QString qfilename = "";
        if (argc > 1) {
            filename = argv[1];
            if (filename.startsWith("\"") && filename.endsWith("\"")) {
                filename = filename.substr(1, filename.length() - 2);
            }
            qfilename = filename.c_str();
        }

        QtSingleApplication a(argc, argv);

        if (a.isRunning()) {
            //CRLog::info("App is already running. Sending parameter: %s", filename.c_str());
            res = a.sendMessage(qfilename);
        } else {

            lString16 exePath = LVExtractPath(Utf8ToUnicode(argv[0]));
            LVAppendPathDelimiter(exePath);
            InitCREngine(exePath);

            OpenGLWindow w;
            a.setActivationWindow(&w);
            a.setMessageHandler(&w);
            if (!filename.empty()) {
                w.setFileToOpenOnStart(filename);
            }
            w.restorePositionAndShow();
            res = a.exec();

            crconfig.uninitEngine();
        }
    }
    return res;
}
