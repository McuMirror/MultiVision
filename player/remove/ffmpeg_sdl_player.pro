QT       += core gui multimedia
QT       += widgets
QT       += opengl
QT       += openglwidgets


CONFIG += c++17
#CONFIG+=precompile_header
#PRECOMPILED_HEADER=stable.h
DEFINES += __STDC_CONSTANT_MACROS

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32-msvc{
INCLUDEPATH += $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/include
LIBS += $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/avformat.lib   \
        $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/avcodec.lib    \
        $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/avdevice.lib   \
        $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/avfilter.lib   \
        $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/avutil.lib     \
       # $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/postproc.lib   \
        $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/swresample.lib \
        $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/swscale.lib    \

#INCLUDEPATH += $$PWD/SDL2-devel-2.30.9-VC/include
#LIBS += $$PWD/SDL2-devel-2.30.9-VC/lib/x64/SDL2.lib \
#        $$PWD/SDL2-devel-2.30.9-VC/lib/x64/SDL2main.lib

}

win32-g++ {
INCLUDEPATH +=  $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/include
LIBS +=         $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libavcodec.dll.a       \
                #$$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/libavdevice.dll.a      \
                $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libavfilter.dll.a      \
                $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libavformat.dll.a      \
                $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libavutil.dll.a        \
                #$$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libpostproc.dll.a      \
                $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libswresample.dll.a    \
                $$PWD/ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libswscale.dll.a       \

#INCLUDEPATH +=  $$PWD/SDL2-devel-2.30.9-mingw/x86_64-w64-mingw32/include/SDL2
#LIBS +=         $$PWD/SDL2-devel-2.30.9-mingw/x86_64-w64-mingw32/lib/libSDL2.dll.a \
                #$$PWD/SDL2-devel-2.30.9-mingw/x86_64-w64-mingw32/lib/libSDL2main.a



}
LIBS += -lopengl32




SOURCES += \
    #audioplaythread.cpp \
    avframequeue.cpp \
    avpacketqueue.cpp \
    decodethread.cpp \
    demuxthread.cpp \
    main.cpp \
    mainwindow.cpp \
    qaudioplayer.cpp \
    #sdlaudiooutput.cpp \
    #sdlvideoplayer.cpp \
    videoopenglwidget.cpp \
 \    #videoplaythread.cpp
    videoslider.cpp

HEADERS += \
    #audioplaythread.h \
    avframequeue.h \
    avpacketqueue.h \
    decodethread.h \
    demuxthread.h \
    mainwindow.h \
    qaudioplayer.h \
    queue.h \
    #sdlaudiooutput.h \
    #sdlvideoplayer.h \
    thread.h \
    videoopenglwidget.h \
 \    #videoplaythread.h
    videoslider.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    shaders.qrc
