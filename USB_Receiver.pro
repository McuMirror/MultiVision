QT       += core gui serialport multimedia
QT       += opengl
QT       += openglwidgets
DEFINES += __STDC_CONSTANT_MACROS

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

# QMAKE_CXXFLAGS_DEBUG += -gstabs+

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# hpp files
INCLUDEPATH += D:\openCV\build-opencv-4.10.0-Desktop_Qt_6_5_3_MinGW_64_bit-Debug\install\include
INCLUDEPATH += D:\openCV\build-opencv-4.10.0-Desktop_Qt_6_5_3_MinGW_64_bit-Debug\install\include\opencv2

INCLUDEPATH +=D:\openCV\build-freetype-2.13.3-Desktop_Qt_6_5_3_MinGW_64_bit-Debug\install\include\freetype2
INCLUDEPATH += D:\openCV\build-freetype-2.13.3-Desktop_Qt_6_5_3_MinGW_64_bit-Debug\install\include\freetype2\freetype

# dll.a files
LIBS += D:\openCV\build-opencv-4.10.0-Desktop_Qt_6_5_3_MinGW_64_bit-Debug\install\x64\mingw\lib\libopencv*
LIBS += D:\openCV\build-freetype-2.13.3-Desktop_Qt_6_5_3_MinGW_64_bit-Debug\install\lib\lib*


# ffmpeg
INCLUDEPATH +=  D:\Qt\ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/include
LIBS +=         D:\Qt\ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libavcodec.dll.a       \
                D:\Qt\ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libavfilter.dll.a      \
                D:\Qt\ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libavformat.dll.a      \
                D:\Qt\ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libavutil.dll.a        \
                D:\Qt\ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libswresample.dll.a    \
                D:\Qt\ffmpeg-n6.1-latest-win64-lgpl-shared-6.1/lib/libswscale.dll.a

SOURCES += \
    detectprocess.cpp \
    glwidget.cpp \
    main.cpp \
    mcv.cpp \
    player/avpacketqueue.cpp \
    player/decodethread.cpp \
    player/demuxthread.cpp \
    player/player.cpp \
    player/qaudioplayer.cpp \
    usb_receiver.cpp \
    xe_qtcvutils.cpp \
    xeimage.cpp

HEADERS += \
    detectprocess.h \
    glwidget.h \
    imagewidget.h \
    mcv.h \
    player/avpacketqueue.h \
    player/decodethread.h \
    player/demuxthread.h \
    player/player.h \
    player/qaudioplayer.h \
    player/queue.h \
    player/thread.h \
    usb_receiver.h \
    xe_qtcvutils.h \
    xeimage.h

FORMS += \
    imagewidget.ui \
    usb_receiver.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md \
    opencv_help.md \
    player/10_minutes.mp4 \
    player/billie_jean.mp4

RESOURCES += \
    res.qrc
