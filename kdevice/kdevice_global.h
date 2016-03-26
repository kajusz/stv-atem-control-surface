#ifndef KDEVICE_GLOBAL_H
#define KDEVICE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(KDEVICE_LIBRARY)
#  define KDEVICESHARED_EXPORT Q_DECL_EXPORT
#else
#  define KDEVICESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // KDEVICE_GLOBAL_H
