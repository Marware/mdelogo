TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/local/include/opencv4

SOURCES += \
        src/ffmpeg.cpp \
        src/main.cpp \
        src/mdelogo.cpp \
        src/opencv.cpp \
        src/utils.cpp

HEADERS += \
    src/ffmpeg.h \
    src/mdelogo.h \
    src/opencv.h \
    src/utils.h


LIBS += -llept -ltesseract -lpthread -ltbb `pkg-config --libs opencv4` -lavdevice  -lavfilter -lpostproc -lavformat -lavcodec -ldl -lavutil -lswscale -lswresample
