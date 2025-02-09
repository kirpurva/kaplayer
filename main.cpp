#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORM", "xcb");
    
    qputenv("QT_MEDIA_BACKEND", "gstreamer");
    
    qputenv("GST_DEBUG", "2");
    
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}