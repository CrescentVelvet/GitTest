#ifndef NETRECEIVE_GLOBAL_H
#define NETRECEIVE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(NETRECEIVE_LIBRARY)
#  define NETRECEIVESHARED_EXPORT Q_DECL_EXPORT
#else
#  define NETRECEIVESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // NETRECEIVE_GLOBAL_H