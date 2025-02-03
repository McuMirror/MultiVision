QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
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


SOURCES += \
    detectprocess.cpp \
    main.cpp \
    usb_receiver.cpp \
    xe_qtcvutils.cpp \
    xeimage.cpp

HEADERS += \
    detectprocess.h \
    imagewidget.h \
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
    opencv_help.md

RESOURCES += \
    icon.qrc
