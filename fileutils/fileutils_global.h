#ifndef FILEUTILS_GLOBAL_H
#define FILEUTILS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FILEUTILS_LIBRARY)
#  define FILEUTILSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define FILEUTILSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // FILEUTILS_GLOBAL_H
