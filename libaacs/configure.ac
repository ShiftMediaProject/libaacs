# initilization
AC_INIT([libaacs],[0.1.0], [http://bd.videolan.org/])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER(config.h)

# messages
library_not_found="Could not find required library!"
function_not_found="Could not find required function!"

# configure options
AC_ARG_ENABLE([werror],
  [AS_HELP_STRING([--enable-werror],
  [set warnings as errors via -Werror (default is yes)])],
  [use_werror=$enableval],
  [use_werror=yes])

# required programs
AC_PROG_CC
AC_PROG_LIBTOOL

# required types
AC_TYPE_SIGNAL

# required headers
AC_CHECK_HEADERS([stdarg.h sys/types.h dirent.h errno.h libgen.h malloc.h])
AC_CHECK_HEADERS([stdlib.h mntent.h linux/cdrom.h inttypes.h])
AC_CHECK_HEADERS([sys/time.h time.h])

# required structures
AC_STRUCT_DIRENT_D_TYPE

# required system services
AC_SYS_LARGEFILE

# required functions
AC_CHECK_FUNC([snprintf],, [AC_MSG_ERROR($function_not_found)])

# openssl check
# check for ECDSA_do_sign, since it's needed
AC_CHECK_LIB([crypto], [ECDSA_do_sign],,
  [AC_MSG_ERROR($library_not_found)])

# use -Werror
if [[ $use_werror = "yes" ]]; then
  WERROR="-Werror"
fi

# generate output files
AC_SUBST(WERROR)
AC_CONFIG_FILES([Makefile src/Makefile src/libaacs.pc])
AC_OUTPUT
