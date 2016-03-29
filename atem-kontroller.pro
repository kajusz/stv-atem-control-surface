TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
		serial \
		kdevice \
		libqatemcontrol \
		atem-kontroller

kdevice.depends = serial
atem-kontroller.depends = kdevice qatemcontrol
