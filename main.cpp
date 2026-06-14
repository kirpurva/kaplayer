#include <QApplication>
#include <QFile>

#include "mainwindow.h"


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);


    QFile styleFile("themes/dark.qss");


    if(styleFile.open(QFile::ReadOnly))
    {
        QString style(styleFile.readAll());
        app.setStyleSheet(style);
    }


    MainWindow w;

    w.show();

    return app.exec();
}

