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

dnl SDL2 option
AC_ARG_WITH(sdl2, [AC_HELP_STRING([--with-sdl2@<:@=no@:>@],
                  [Build using SDL 2.0 instead of SDL 1.2.])],
                  [], [with_sdl2=no])

# Checks for libraries.
# Library `-lm':
AC_CHECK_LIB([m], [ceilf])

AS_IF([test "x$with_sdl2" != xno],
    # Library `-lSDL2':
    [AC_CHECK_LIB([SDL2], [SDL_Init]),
     AC_SUBST(SDLCFLAGS, "`sdl2-config --cflags`")],
    [# Library `-lSDL':
     AC_CHECK_LIB([SDL], [SDL_Init]),
     # Library `-lSDLmain':
     AC_CHECK_LIB([SDLmain], [main]),
     AC_SUBST(SDLCFLAGS, "`sdl-config --cflags`")]
)

# Library `-lSDL_draw':
AC_CHECK_LIB([SDL_draw], [Draw_Line])
# Library `-lfreetype':
AC_CHECK_LIB([freetype], [FT_Init_FreeType])
AC_SUBST(FTCFLAGS, "`freetype-config --cflags`")

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([stdio.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
