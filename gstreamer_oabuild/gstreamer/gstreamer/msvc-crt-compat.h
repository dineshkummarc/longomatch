#ifndef __MSVC_CRT_COMPAT_H__
#define __MSVC_CRT_COMPAT_H__

/* Because of annoying CRTs */
#ifdef _MSC_VER
#  define open _open
#  define close _close
#  define read _read
#  define write _write
#  ifndef lseek
#    define lseek _lseek
#  endif
#  define fdopen _fdopen
#  define dup _dup
#  define strupr _strupr
#  define unlink _unlink
#endif

#endif /* __MSVC_CRT_COMPAT_H__ */
