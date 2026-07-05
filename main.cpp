#include <QApplication>
#include <QFile>
#include <QIcon>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Window/taskbar icon; the exe icon comes from kaplayer.rc.
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/kaplayer.ico")));

    // Theme is compiled into the binary via resources.qrc, so this
    // load cannot fail due to working directory or filename case.
    QFile styleFile(":/themes/dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    MainWindow w;
    w.show();

    return app.exec();
}
