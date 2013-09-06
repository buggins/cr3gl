#include "cr3mainwindow.h"

#include "cr3qt.h"
#include <QApplication>

using namespace CRUI;


int main(int argc, char *argv[])
{
    lString16 exePath = LVExtractPath(Utf8ToUnicode(argv[0]));
    LVAppendPathDelimiter(exePath);
    InitCREngine(exePath);
    QApplication a(argc, argv);
    OpenGLWindow w;
    w.show();
    
    int res = a.exec();
    crconfig.uninitEngine();
    return res;
}
