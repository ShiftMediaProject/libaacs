dnl library version number
m4_define([aacs_major], 0)
m4_define([aacs_minor], 6)
m4_define([aacs_micro], 0)
m4_define([aacs_version],[aacs_major.aacs_minor.aacs_micro])

dnl library (.so) version
#
dnl update when making new release
#
dnl - If there are no ABI changes, increase revision.
dnl - If ABI is changed backwards-compatible way, increase current and age. Set revision to 0.
dnl - If ABI is broken, increase current and set age and revision to 0.
#
dnl Library file name will be libaacs.(current-age).age.revision
#
m4_define([lt_current],  3)
m4_define([lt_age],      3)
m4_define([lt_revision], 0)

dnl initilization
AC_INIT([libaacs], aacs_version, [http://www.videolan.org/developers/libaacs.html])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-dist-gzip])
AC_CONFIG_HEADERS(config.h)
AM_MAINTAINER_MODE

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
  [AS_HELP_STRING([--enable-werror],
  [set warnings as errors via -Werror (default is no)])],
  [use_werror=$enableval],
  [use_werror=no])

AC_ARG_ENABLE([extra-warnings],
  [AS_HELP_STRING([--enable-extra-warnings],
  [set extra warnings (default is yes)])],
  [use_extra_warnings=$enableval],
  [use_extra_warnings=yes])

AC_ARG_ENABLE([optimizations],
  [AS_HELP_STRING([--enable-optimizations],
  [enable optimizations (default is yes)])],
  [use_optimizations=$enableval],
  [use_optimizations=yes])

AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug],
  [enable debugging information (default is yes)])],
  [use_debug=$enableval],
  [use_debug=yes])

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

AC_CHECK_FUNC([realpath],[AC_DEFINE([HAVE_REALPATH],[1],[realpath])])

dnl pthread check (not on win32)
if test "${SYS}" != "mingw32" ; then
  AC_CHECK_HEADERS([pthread.h])
  AC_CHECK_LIB([pthread], [pthread_mutex_init],,
    [AC_MSG_ERROR($library_not_found)])
fi

dnl gcrypt check

AC_ARG_WITH(libgcrypt-prefix,
            AC_HELP_STRING([--with-libgcrypt-prefix=PFX],
                           [prefix where libgcrypt is installed (optional)]),
     libgcrypt_config_prefix="$withval", libgcrypt_config_prefix="")
if test x$libgcrypt_config_prefix != x ; then
   if test x${LIBGCRYPT_CONFIG+set} != xset ; then
      LIBGCRYPT_CONFIG=$libgcrypt_config_prefix/bin/libgcrypt-config
   fi
fi

AC_PATH_PROG(LIBGCRYPT_CONFIG, libgcrypt-config, no)
if test x"$LIBGCRYPT_CONFIG" = xno; then
  AC_MSG_ERROR([libgcrypt not found on system])
else
  LIBGCRYPT_CFLAGS=`$LIBGCRYPT_CONFIG --cflags`
  LIBGCRYPT_LIBS=`$LIBGCRYPT_CONFIG --libs`
  AC_SUBST(LIBGCRYPT_CFLAGS)
  AC_SUBST(LIBGCRYPT_LIBS)
fi
AC_FUNC_STRERROR_R
AC_CHECK_LIB([gpg-error], [gpg_strerror_r])

dnl function testing for supported compiler options
check_cc_options()
{
  local tmpfile=$(mktemp XXXXXXXXXX)
  local tmpcfile="$tmpfile.c"
  mv $tmpfile $tmpcfile
  local retval=1
  printf "Checking if compiler supports $@... "
  if $CC -c $@ $tmpcfile -o /dev/null >/dev/null 2>&1; then
    local retval=0
  fi
  rm $tmpcfile
  if [[ $retval = "0" ]]; then
    echo "yes"
  else
    echo "no"
  fi
  return $retval
}

dnl set default warnings if supported
check_cc_options -Wall && \
  SET_WARNINGS="$SET_WARNINGS -Wall"
check_cc_options -Wdisabled-optimization && \
  SET_WARNINGS="$SET_WARNINGS -Wdisabled-optimization"
check_cc_options -Wpointer-arith && \
  SET_WARNINGS="$SET_WARNINGS -Wpointer-arith"
check_cc_options -Wredundant-decls && \
  SET_WARNINGS="$SET_WARNINGS -Wredundant-decls"
check_cc_options -Wcast-qual && \
  SET_WARNINGS="$SET_WARNINGS -Wcast-qual"
check_cc_options -Wwrite-strings && \
  SET_WARNINGS="$SET_WARNINGS -Wwrite-strings"
check_cc_options -Wtype-limits && \
  SET_WARNINGS="$SET_WARNINGS -Wtype-limits"
check_cc_options -Wundef && \
  SET_WARNINGS="$SET_WARNINGS -Wundef"

dnl use -Werror
if [[ $use_werror = "yes" ]]; then
  check_cc_options -Werror && \
    SET_WARNINGS="$SET_WARNINGS -Werror"
fi

dnl use extra warnings
if [[ $use_extra_warnings = "yes" ]]; then
  check_cc_options -Wextra && \
    SET_WARNINGS="$SET_WARNINGS -Wextra"
  check_cc_options -Winline && \
    SET_WARNINGS="$SET_WARNINGS -Winline"
fi

dnl use optimizations
if [[ $use_optimizations = "yes" ]]; then
  check_cc_options -O3 && \
    SET_OPTIMIZATIONS="$SET_OPTIMIZATIONS -O3"
  check_cc_options -fomit-frame-pointer && \
    SET_OPTIMIZATIONS="$SET_OPTIMIZATIONS -fomit-frame-pointer"
fi

dnl use debug
if [[ $use_debug = "yes" ]]; then
  check_cc_options -g && \
    SET_DEBUG_OPTS="$SET_DEBUG_OPTS -g"
fi

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
AC_SUBST(SET_WARNINGS)
AC_SUBST(SET_OPTIMIZATIONS)
AC_SUBST(SET_DEBUG_OPTS)
AC_SUBST(SET_FEATURES)
AC_CONFIG_FILES([Makefile src/Makefile src/examples/Makefile src/libaacs.pc src/libaacs/aacs-version.h])
AC_OUTPUT
