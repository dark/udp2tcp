# Initialization
AC_PREREQ(2.61)
AC_INIT([udp2tcp], [0.0.3-alpha], BUG-REPORT-ADDRESS)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Check a random file for existence
AC_CONFIG_SRCDIR([src/library.c])

AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h sys/socket.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memmove memset socket strdup strerror strtol])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
