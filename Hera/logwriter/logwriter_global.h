#ifndef LOGWRITER_GLOBAL_H
#define LOGWRITER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LOGWRITER_LIBRARY)
#  define LOGWRITERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LOGWRITERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LOGWRITER_GLOBAL_H