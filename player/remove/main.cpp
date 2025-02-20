#include "mainwindow.h"
#include <QOpenGLContext>
// #include "sdlvideoplayer.h"
#include <QApplication>
#include <iostream>
extern "C" {
// #include "SDL.h"
#include "libavutil/avutil.h"
}

int main(int argc, char* argv[])
{
    QSurfaceFormat format;
    qDebug() << "QSurfaceFormat format: " << format.version();
    QApplication a(argc, argv);

    MainWindow w;
    QThread* currentThread = QThread::currentThread();
    qDebug() << "main::thread_id()" << currentThread;
    w.show();

    return a.exec();
}
