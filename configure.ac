#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
AC_INIT([libmylcd],[0.60.0],[okio@users.sourceforge.net])
AC_CONFIG_SRCDIR([src/lcd.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.9 foreign])

AS_IF([test "x${enable_maintainer_mode}" != "xno"], [enable_maintainer_mode="yes"])
AM_MAINTAINER_MODE

VERSION_MAJOR="0"
VERSION_MINOR="60"
VERSION_REVISION="0"

# AC_REQUIRE([AC_CANONICAL_HOST])_LT_SET_OPTION([LT_INIT],[win32-dll])
# AC_DIAGNOSE([obsolete],[AC_LIBTOOL_WIN32_DLL: Remove this warning and the call to _LT_SET_OPTION when you
# put the `win32-dll' option into LT_INIT's first parameter.])

LT_PREREQ([2.2.6])
LT_INIT([win32-dll], [shared])
LT_LANG([C])

AC_C_INLINE
AC_C_VOLATILE
AC_PROG_CC_C99
AC_DEFINE([_FORTIFY_SOURCE], 2, [Define to '2' to get glibc warnings.])
AC_DEFINE([_WIN32_WINNT], 0x0501, [Define to '0x0501' for Windows 2000 APIs.])

# Checks for programs.
# AC_PROG_CXX
# AC_PROG_CC
# AC_PROG_RANLIB

AC_CONFIG_MACRO_DIR([m4])


LDFLAGS = -module -avoid-version -no-undefined
CFLAGS="$CFLAGS -L"contribs/x32/lib/" -I"contribs/x32/include/" -L"../contribs/x32/lib/" -I"../contribs/x32/include/" -Werror -D_WIN32_WINNT=0x0501 -D__WIN32__=1 -DPTW32_STATIC_LIB -DLIBUSB_BLOCKS"
LIBS=$LIBS -L"contribs/x32/lib/" -L"../contribs/x32/lib/" 

# AC_CHECK_DECLS([__BUILD_PTHREADS_SUPPORT__], [AC_MSG_ERROR([found])], [AC_MSG_ERROR([not found])], [#include "src/mylcdconfig.h"])

# Checks for libraries.
#AC_CHECK_LIB([SDL], [SDL_Init], , [AC_MSG_ERROR([libsdl not found])])

# check for libpng
#AC_CHECK_LIB([png.dll], [png_destroy_write_struct], , [AC_MSG_ERROR([libpng not found])])

# check for libjpeg
#AC_CHECK_LIB([jpeg.dll], [jpeg_read_header], , [AC_MSG_ERROR([libjpeg not found])])

# check for lglcd.dll
# AC_CHECK_LIB([lglcd], [lg_LcdInit], , [AC_MSG_ERROR([liblglcd.a not found])])

# check for libusb0
#AC_CHECK_LIB([usb], [usb_open], , [AC_MSG_ERROR([libusb not found])])

#AC_CHECK_HEADERS([usb.h], , [AC_MSG_ERROR([libusb header usb.h not found])])

AC_CHECK_HEADERS([pthread.h sched.h], , [AC_MSG_ERROR([pthread.h or sched.h not found])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_C_VOLATILE

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_DIAGNOSE([obsolete],[your code may safely assume C89 semantics that RETSIGTYPE is void.
Remove this warning and the `AC_CACHE_CHECK' when you adjust the code.])dnl
AC_CACHE_CHECK([return type of signal handlers],[ac_cv_type_signal],[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <signal.h>
],
		 [return *(signal (0, 0)) (0) == 1;])],
		   [ac_cv_type_signal=int],
		   [ac_cv_type_signal=void])])
AC_DEFINE_UNQUOTED([RETSIGTYPE],[$ac_cv_type_signal],[Define as the return type of signal handlers
		    (`int' or `void').])

AC_FUNC_VPRINTF


AC_CHECK_FUNCS([swprintf vsprintf atexit gethostbyaddr gethostbyname gettimeofday memmove memset select socket sqrt strcasecmp strdup strerror strncasecmp strrchr strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

