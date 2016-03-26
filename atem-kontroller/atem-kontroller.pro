QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = atem-kontroller
TEMPLATE = app

SOURCES += \
		main.cpp \
		mainwindow.cpp \
		control.cpp

HEADERS += mainwindow.h \
		control.h \
		libatem.h

FORMS += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../kdevice/release/ -lkdevice  -L$$OUT_PWD/../libqatemcontrol/release/ -lqatemcontrol
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../kdevice/debug/ -lkdevice  -L$$OUT_PWD/../libqatemcontrol/debug/ -lqatemcontrol
else:unix: LIBS += -L$$OUT_PWD/../kdevice/ -lkdevice -L$$OUT_PWD/../libqatemcontrol/ -lqatemcontrol

INCLUDEPATH += $$PWD/../kdevice $$PWD/../libqatemcontrol
DEPENDPATH += $$PWD/../kdevice $$PWD/../libqatemcontrol
