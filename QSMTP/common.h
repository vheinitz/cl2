/*!
Global definitions
*/
#ifndef _QSMTP_COMMON_HG_5344656345_H
#define _QSMTP_COMMON_HG_5344656345_H


#ifdef BUILDING_QSMTP_DLL
# define QSMTP_EXPORT Q_DECL_EXPORT
#else
# define QSMTP_EXPORT Q_DECL_IMPORT
#endif

#ifdef EMBEDDING_QSMTP
# undef QSMTP_EXPORT
# define QSMTP_EXPORT
#endif

#endif // HG
