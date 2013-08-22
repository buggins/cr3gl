#include "cr3mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OpenGLWindow w;
    w.show();
    
    return a.exec();
}
