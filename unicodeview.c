
/* ------------------------------------------------------------------------- */
/* unicodeview.c - this file is part of unicodeview                          */
/* Copyright 2012, 2013                                                      */
/* Matthias Kievernagel                                                      */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License version 2 as         */
/* published by the Free Software Foundation.                                */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>.     */
/* ------------------------------------------------------------------------- */

/* main program for viewing unicode fonts */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include "SDL_ttf.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "unipageview.h"


SDL_Surface * _pm_screen = NULL;

/* unicode window */
struct unipageview * _upview = NULL;

/* The font that's going to be used */
TTF_Font * _pm_font = NULL;


static void _usage() {
    fprintf(stderr,
        "Usage: unicodeview "
        " [-g] [-c] [-p 'page'] [-m 'charmap'] 'fontfilename'\n");
    fprintf(stderr, " -g: display in gridded mode (default)\n");
    fprintf(stderr, " -c: display in continuous mode\n");
    fprintf(stderr,
        " -p 'page': Start display on page 'page' in gridded mode.\n");
    fprintf(stderr,
        "            'page' must be given in hex format, i.e. '0xnnn'\n");
    fprintf(stderr,
        " -m 'charmap': Select 'charmap' in the font for indexing.\n");
    fprintf(stderr,
        "               'charmap' has the format 'number,number'.\n");
    fprintf(stderr,
        "               See README for more details.\n");
}


static char * _init_sdl(
        char * fontfilename, int fontsize, int platform_id, int encoding_id) {
    int res;
    FT_Face face;
    FT_UInt index;
    FT_ULong charcode;

    /* Start SDL */
    res = SDL_Init(SDL_INIT_VIDEO);
    if ( res == -1 ) {
        return SDL_GetError();
    }

    /* setup screen */
    _pm_screen = SDL_SetVideoMode(360, 390, 32, SDL_SWSURFACE);
    if ( _pm_screen == NULL ) {
        return SDL_GetError();
    }

    /* Initialize SDL_ttf */
    if ( TTF_Init() == -1 ) {
        return TTF_GetError();
    }

    /* Open the font */
    /* _pm_font = TTF_OpenFont ( "DejaVuSansMono.ttf", 16 ); */
    /* _pm_font = TTF_OpenFont ( "LinBiolinum_Rah.ttf", 16 ); */
    /* _pm_font = TTF_OpenFont("unifont.ttf", 16); */
    _pm_font = TTF_OpenFont(fontfilename, fontsize);
    if ( _pm_font == NULL ) {
        return TTF_GetError();
    }

    /* set specified platform/encoding charmap, otherwise use default */
    if ( platform_id != -1 && encoding_id != -1 ) {
        res = TTF_SelectCharMap(_pm_font, platform_id, encoding_id);
        /* printf("TTF_SelectCharMap %d\n", res); */
    }

    /* debug output:
    printf("faces %ld\n", TTF_FontFaces(_pm_font));
    printf("fixed %d\n", TTF_FontFaceIsFixedWidth(_pm_font));
    printf("kerning %d\n", TTF_GetFontKerning(_pm_font));
    printf("height %d\n", TTF_FontHeight(_pm_font));
    printf("ascent %d\n", TTF_FontAscent(_pm_font));
    printf("descent %d\n", TTF_FontDescent(_pm_font));
    printf("lineskip %d\n", TTF_FontLineSkip(_pm_font));
    */

    /* test iterating char codes:
    face = TTF_GetFreeTypeFace(_pm_font);
    charcode = FT_Get_First_Char(face, &index);
    while ( index != 0 ) {
        printf("char %08lx/%d\n", charcode, index);
        charcode = FT_Get_Next_Char(face, charcode, &index);
    }
    */

    SDL_EnableUNICODE(1);

    return NULL;
}

static void _teardown_sdl () {
    /* Close the font that was used */
    TTF_CloseFont(_pm_font);
    
    /* Quit SDL_ttf + SDL */
    TTF_Quit();
    SDL_Quit();
}

static void _pm_mainloop () {
    SDL_Event event;
    int _quit = 0;
    int base;
    int cpt;
    int dirty = 1;
    int minx,miny,maxx,maxy,adv;

    while ( _quit != 1 ) {
        while ( SDL_PollEvent ( &event )) {
            switch ( event.type ) {
            case SDL_KEYDOWN:
                /*
                printf("key down unicode (%hd)\n", event.key.keysym.unicode);
                */
                if ( event.key.keysym.unicode == (int)'q' ) {
                    _quit = 1;
                }
                /* debug output glyph metrics
                else if ( event.key.keysym.unicode == (int)'p' ) {
                    base = upv_getbase(_upview);
                    cpt = (base << 8) + 32;
                    TTF_GlyphMetrics(
                        _pm_font, cpt, &minx, &maxx, &miny, &maxy, &adv);
                    printf("[%04x]x(%d/%d)y(%d/%d)a(%d)\n", cpt,
                        minx, maxx, miny, maxy, adv);
                } */
                else if ( event.key.keysym.unicode == (int)'<' ) {
                    base = upv_getbase(_upview);
                    upv_setbase(_upview, base - 1);
                    upv_render(_upview);
                    dirty = 1;
                }
                else if ( event.key.keysym.unicode == (int)'>' ) {
                    if ( upv_gridded(_upview)) {
                        base = upv_getbase(_upview);
                        upv_setbase(_upview, base + 1);
                    } else {
                        upv_next(_upview);
                    }

                    upv_render(_upview);
                    dirty = 1;
                }
                break;

            case SDL_QUIT:
                _quit = 1;
                break;

            default:
                break;
            }
        }

        if ( dirty == 1 ) {
            SDL_Flip ( _pm_screen );
            dirty = 0;
        }

        SDL_Delay ( 50 );
    }
}

int main ( int argc, char * argv[] ) {
    char * err;
    int platform_id = -1;
    int encoding_id = -1;
    int res;
    int page = 0;
    int opt;
    char * fontname = NULL;
    int gridded = 1;

    while (( opt = getopt ( argc, argv, "-gcp:m:" )) != -1 ) {
        switch ( opt ) {
        case 'g':
            gridded = 1;
            break;
        case 'c':
            gridded = 0;
            break;
        case 'p':
            res = sscanf(optarg, "%i", &page);
            if ( res != 1 || page < 0 || page > 0xFFF ) {
                fprintf(stderr, "Illegal page (%s)\n", optarg);
                _usage();
                exit(1);
            }
            break;
        case 'm':
            res = sscanf(optarg, "%d , %d", &platform_id, &encoding_id);
            if ( res != 2 || platform_id < 0 || encoding_id < 0 ) {
                fprintf(stderr,
                    "Illegal charmap specification (%s)\n", optarg);
                _usage();
                exit(1);
            }
            break;
        case 1:
            fontname = optarg;
            break;
        default:
            /* printf ( "opt (%c,%d)\n", opt, opt ); */
            _usage();
            exit(1);
            break;
        }
    }

    if ( fontname == NULL ) {
        printf ( "font missing\n" );
        _usage();
        exit(1);
    }

    err = _init_sdl(fontname, gridded ? 16 : 64, platform_id, encoding_id);
    if ( err != NULL ) {
        fprintf(stderr, "Init failed: %s\n", err);
        exit(1);
    }

    SDL_WM_SetCaption("Unicode font viewer", "Unicode font viewer");

    SDL_Rect offset;
    offset.x = 10;
    offset.y = 10;
    offset.w = 340;
    offset.h = 370;
    _upview = upv_create(_pm_screen, &offset);
    upv_setfont(_upview, _pm_font);
    upv_setgridded(_upview, gridded);
    upv_init(_upview);
    /* in gridded mode: set page */
    if ( gridded ) {
        upv_setbase(_upview, page);
    }
    upv_render(_upview);

    _pm_mainloop ();
    
    _teardown_sdl ();
    return 0;
}
