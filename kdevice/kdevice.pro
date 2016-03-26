QT -= qui

TARGET = kdevice
TEMPLATE = lib

DEFINES += KDEVICE_LIBRARY USE_SERIAL

SOURCES += device.cpp

HEADERS += \
		kdevice.h \
		kdevice_global.h \
		rgbf.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../serial/release/ -lserial -ladvapi32 -lsetupapi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../serial/debug/ -lserial -ladvapi32 -lsetupapi
else:unix: LIBS += -L$$OUT_PWD/../serial/ -lserial

INCLUDEPATH += $$PWD/../serial
DEPENDPATH += $$PWD/../serial
