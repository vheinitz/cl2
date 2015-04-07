/*!
Global or temporary definitions
*/
#ifndef _COMMON__H
#define _COMMON__H

#ifdef BUILDING_EIKJU_DLL
# define EIKJU_EXPORT Q_DECL_EXPORT
#else
# define EIKJU_EXPORT Q_DECL_IMPORT
#endif

#ifdef EMBEDDING_EIKJU_DLL
# undef EIKJU_EXPORT
# define EIKJU_EXPORT
#endif

#endif // _COMMON__H
