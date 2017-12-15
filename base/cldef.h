#ifndef __HG__HG_CL_DEF___H_
#define __HG__HG_CL_DEF___H_

//TODO: set the macro in build script
#ifndef BUILDING_CLIB_DLL
#define BUILDING_CLIB_DLL
#endif

#ifdef BUILDING_CLIB_DLL
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_EXPORT
# endif
#else
# ifndef CLIB_EXPORT
#   define CLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

#endif