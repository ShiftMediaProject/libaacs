dnl library version number
m4_define([aacs_major], 0)
m4_define([aacs_minor], 9)
m4_define([aacs_micro], 0)
m4_define([aacs_version],[aacs_major.aacs_minor.aacs_micro])

dnl shared library version (.so version)
dnl
dnl update when making new release
dnl
dnl - If there are no ABI changes, increase revision.
dnl - If ABI is changed backwards-compatible way, increase current and age. Set revision to 0.
dnl - If ABI is broken, increase current and set age and revision to 0.
dnl
dnl Library file name will be libaacs.(current-age).age.revision

m4_define([lt_current],  6)
m4_define([lt_age],      6)
m4_define([lt_revision], 0)

dnl initilization
AC_INIT([libaacs], aacs_version, [http://www.videolan.org/developers/libaacs.html])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([foreign tar-ustar dist-bzip2 no-dist-gzip subdir-objects])
AC_CONFIG_HEADERS(config.h)
AM_MAINTAINER_MODE

dnl Enable silent rules only when available (automake 1.11 or later).
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

SET_FEATURES="-D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112L"

case "${host_os}" in
  "")
    SYS=unknown
    ;;
  *mingw32* | *cygwin* | *wince* | *mingwce*)
    case "${host_os}" in
      *wince* | *mingwce* | *mingw32ce*)
        SYS=mingwce
        ;;
      *mingw32*)
        SYS=mingw32
        AC_DEFINE([_WIN32_WINNT], 0x0501, [Define to '0x0500' for Windows XP APIs.])
        AC_DEFINE([_WIN32_IE], 0x0501, [Define to '0x0501' for IE 5.01.])
        ;;
    esac
    ;;
  *darwin*)
    SYS=darwin
    AC_DEFINE([USE_IOKIT], 1, [Use IOKit for MMC access])
    LDFLAGS="${LDFLAGS} -lobjc -Wl,-framework,IOKit,-framework,Cocoa,-framework,DiskArbitration"
    SET_FEATURES="${SET_FEATURES} -D_DARWIN_C_SOURCE"
    ;;
  *)
    SYS="${host_os}"
    ;;
esac

AM_CONDITIONAL(HAVE_WIN32,   test "${SYS}" = "mingw32")
AM_CONDITIONAL(HAVE_DARWIN,  test "${SYS}" = "darwin")

dnl messages
library_not_found="Could not find required library!"
function_not_found="Could not find required function!"

dnl configure options
AC_ARG_ENABLE([werror],
  [AS_HELP_STRING([--enable-werror], [set warnings as errors via -Werror @<:@default=disabled@:>@])])

AC_ARG_ENABLE([extra-warnings],
  [AS_HELP_STRING([--disable-extra-warnings], [set extra warnings @<:@default=enabled@:>@])])

AC_ARG_ENABLE([optimizations],
  [AS_HELP_STRING([--disable-optimizations], [disable optimizations @<:@default=enabled@:>@])])

dnl override platform specific check for dependent libraries
dnl otherwise libtool linking of shared libraries will
dnl fail on anything other than pass_all.
AC_CACHE_VAL(lt_cv_deplibs_check_method,
    [lt_cv_deplibs_check_method=pass_all])

dnl required programs
AC_PROG_CC
AC_PROG_LIBTOOL
AC_LIBTOOL_WIN32_DLL
AM_PROG_LEX
AC_PROG_YACC

dnl required types
AC_TYPE_SIGNAL

dnl required headers
AC_CHECK_HEADERS([stdarg.h sys/types.h dirent.h errno.h libgen.h malloc.h])
AC_CHECK_HEADERS([stdlib.h mntent.h linux/cdrom.h inttypes.h])
AC_CHECK_HEADERS([sys/time.h time.h sys/select.h limits.h sys/param.h])
AC_CHECK_HEADERS([sys/mount.h])

dnl required structures
AC_STRUCT_DIRENT_D_TYPE

dnl required system services
AC_SYS_LARGEFILE

dnl required functions
AC_CHECK_FUNC([snprintf],, [AC_MSG_ERROR($function_not_found)])

dnl required libraries

dnl gcrypt check
AM_PATH_LIBGCRYPT([1.6.0], [have_gcrypt=yes])
AS_IF([test "x$have_gcrypt" != xyes],[
    dnl older gcrypt requires pthread
    AM_PATH_LIBGCRYPT([1.1.94],,
                      AC_MSG_ERROR([libgcrypt not found on system]))
    dnl pthread check (not on win32)
    AS_IF([test "${SYS}" != "mingw32"], [
        AC_CHECK_HEADERS([pthread.h], ,
            [AC_MSG_ERROR([pthread required])])
        AC_SEARCH_LIBS([pthread_create], [pthread], ,
            [AC_MSG_ERROR([pthread required])])
        ])
    ])

dnl use re-entrant version of gcrypt_error() from gpg-error
AC_FUNC_STRERROR_R
AM_PATH_GPG_ERROR([0.5],
                  AC_DEFINE(HAVE_LIBGPG_ERROR, 1, [Define to 1 if you have the gpg-error library]))

CC_CHECK_CFLAGS_APPEND([-Wall -Wdisabled-optimization -Wpointer-arith ]dnl
[-Wredundant-decls -Wcast-qual -Wwrite-strings -Wtype-limits -Wundef ]dnl
[-Wmissing-prototypes -Wshadow])

CC_CHECK_CFLAGS_APPEND([-Werror=implicit-function-declaration ]dnl
                       [-Werror-implicit-function-declaration],
                       [break;])

CC_CHECK_WERROR
AS_IF([test "x$enable_werror" = "xyes"], [
  CFLAGS="${CFLAGS} $cc_cv_werror"
])

AS_IF([test "x$enable_extra_warnings" != "xno"], [
  CC_CHECK_CFLAGS_APPEND([-Wextra -Winline])
])

AS_IF([test "x$enable_optimizations" != "xno"], [
  CC_CHECK_CFLAGS_APPEND([-O3 -fomit-frame-pointer])
])


dnl export library version number
AACS_VERSION_MAJOR=aacs_major()
AC_SUBST(AACS_VERSION_MAJOR)
AACS_VERSION_MINOR=aacs_minor()
AC_SUBST(AACS_VERSION_MINOR)
AACS_VERSION_MICRO=aacs_micro()
AC_SUBST(AACS_VERSION_MICRO)

dnl export library (.so) version
LT_VERSION_INFO="lt_current:lt_revision:lt_age"
AC_SUBST(LT_VERSION_INFO)

dnl generate output files
AC_SUBST(SET_FEATURES)
AC_CONFIG_FILES([Makefile src/libaacs.pc src/libaacs/aacs-version.h])
AC_OUTPUT
