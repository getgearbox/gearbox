#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([gearbox], [0.1], [gearbox@corybennett.org])
AC_CONFIG_SRCDIR([apache/mod_gearbox/mod_gearbox.cc])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([auto])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

AC_TYPE_LONG_LONG_INT
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])

AC_ARG_VAR([SHASUM_BIN])
AC_PATH_PROGS([SHASUM_BIN], [sha1sum shasum])
if test -z "${SHASUM_BIN}"; then
   echo "sha1sum or shasum command not found, required for tests"
   exit 1
fi
AC_DEFINE_UNQUOTED([SHASUM_BIN], ["${SHASUM_BIN}"], [Path to sha1sum binary])

AC_ARG_VAR([TRUE_BIN])
AC_PATH_PROG([TRUE_BIN],    [true])
if test -z "${TRUE_BIN}"; then
   echo "true command not found, required for tests"
   exit 1
fi
AC_DEFINE_UNQUOTED([TRUE_BIN], ["${TRUE_BIN}"], [path to true binary])

AC_ARG_VAR([FALSE_BIN])
AC_PATH_PROG([FALSE_BIN],   [false])
if test -z "${FALSE_BIN}"; then
   echo "false command not found, required for tests"
   exit 1
fi
AC_DEFINE_UNQUOTED([FALSE_BIN], ["${FALSE_BIN}"], [path to false binary])

AC_DEFINE_UNQUOTED([SYSCONFDIR], ["`test "${prefix}" = "NONE" && prefix=; eval echo ${sysconfdir}`"], [path to system configuration files])
AC_DEFINE_UNQUOTED([DATADIR], ["`test "${prefix}" = "NONE" && prefix=; eval echo ${datarootdir}`"], [path to data directory])
AC_DEFINE_UNQUOTED([PLUGINSDIR], ["`test "${exec_prefix}" = "NONE" && exec_prefix=${prefix}; test "${exec_prefix}" = "NONE" && exec_prefix=; eval echo ${libexecdir}/gearbox/plugins`"], [path to data directory])

AC_DEFINE_UNQUOTED([SHLIB_EXT], ["`eval echo ${shrext_cmds}`"], [shared libary extension])

AX_BOOST_BASE([1.49], [], [exit -1])
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_GRAPH
AX_BOOST_REGEX
AX_BOOST_IOSTREAMS

PKG_CHECK_MODULES([LOG4CXX], [liblog4cxx], [], [exit 1])
PKG_CHECK_MODULES([YAJL], [yajl], [], [exit 1])
PKG_CHECK_MODULES([CURL], [libcurl], [], [exit 1])
# on OSX:
# export PKG_CONFIG_PATH=/usr/local/opt/ossp-uuid/lib/pkgconfig
PKG_CHECK_MODULES([UUID], [uuid], [], [exit 1])
PKG_CHECK_MODULES([GEARMAN], [gearmand], [], [exit 1])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcrypto':
AC_CHECK_LIB([crypto], [main])
# FIXME: Replace `main' with a function in `-lzookeeper_st':
AC_CHECK_LIB([zookeeper_st], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 floor gettimeofday memset mkfifo select socket strerror])

AM_INIT_AUTOMAKE([1.13 subdir-objects no-define foreign])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT


