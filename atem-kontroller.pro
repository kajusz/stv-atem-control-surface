#-------------------------------------------------
#
# Project created by QtCreator 2016-03-21T17:53:08
#
#-------------------------------------------------

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
		serial \
		kdevice \
		libqatemcontrol \
		atem-kontroller

kdevice.depends = serial
atem-kontroller.depends = kdevice qatemcontrol
