QT -= qui

TARGET = serial
TEMPLATE = lib
CONFIG += staticlib

SOURCES += serial/serial.cc
win32:SOURCES += serial/impl/win.cc serial/impl/list_ports/list_ports_win.cc
unix:!macx:SOURCES += serial/impl/unix.cc serial/impl/list_ports/list_ports_linux.cc
macx:SOURCES += serial/impl/unix.cc serial/impl/list_ports/list_ports_osx.cc

HEADERS += \
		serial/serial.h \
		serial/v8stdint.h

win32:LIBS += -ladvapi32 -lsetupapi
unix:!macx:LIBS += -lrt -lpthread
macx:LIBS += -framework IOKit -framework Foundation
