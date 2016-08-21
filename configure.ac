#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([unicodeview], [0.03tmp], [mkiever@tlink.de])
AC_CONFIG_SRCDIR([unipageview.h])
AC_CONFIG_HEADERS([config.h])

# AM_INIT_AUTOMAKE added
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# Library `-lm':
AC_CHECK_LIB([m], [ceilf])
# Library `-lSDL':
AC_CHECK_LIB([SDL], [SDL_Init])
# Library `-lSDLmain':
AC_CHECK_LIB([SDLmain], [main])
# Library `-lSDL_draw':
AC_CHECK_LIB([SDL_draw], [Draw_Line])
# Library `-lfreetype':
AC_CHECK_LIB([freetype], [FT_Init_FreeType])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([stdio.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
