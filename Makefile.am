

bin_PROGRAMS=unicodeview


unicodeview_SOURCES=unicodeview.c \
	 unipageview.c unipageview.h \
	 SDL_ttf.c SDL_ttf.h


unicodeview_CFLAGS=`freetype-config --cflags`
unicodeview_LDADD=-lfreetype -lSDL_draw -lSDLmain -lSDL


EXTRA_DIST = COPYING_SDL_ttf