
/* ------------------------------------------------------------------------- */
/* unipageview.c - this file is part of unicodeview                          */
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

/* unicode page view */
/* shows 256 glyphs of a font in a 16x16 grid pages or in continuous lines */


#include <stdlib.h>
#include <SDL.h>
#include <SDL_draw.h>
#include "SDL_ttf.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "unipageview.h"



static void _get_utf8_bytes(char utfbytes[], unsigned int codepoint) {
    if ( codepoint <= 0x0000007f ) {
        /* printf("1 bytes utf\n"); */
        utfbytes[0] = (unsigned char)(codepoint & 0x0000007f);
        utfbytes[1] = '\0';
    }
    else if ( codepoint <= 0x000007ff ) {
        /* printf("2 bytes utf\n"); */
        utfbytes[0] = (unsigned char)(0xC0 | ((codepoint & 0x000007C0) >> 6));
        utfbytes[1] = (unsigned char)(0x80 | (codepoint & 0x0000003F));
        utfbytes[2] = '\0';
    }
    else if ( codepoint <= 0x0000ffff ) {
        /* printf("3 bytes utf\n"); */
        utfbytes[0] = (unsigned char)(0xE0 | ((codepoint & 0x0000F000) >> 12));
        utfbytes[1] = (unsigned char)(0x80 | ((codepoint & 0x00000FC0) >> 6));
        utfbytes[2] = (unsigned char)(0x80 | (codepoint & 0x0000003F));
        utfbytes[3] = '\0';
    }
    else if ( codepoint <= 0x001fffff ) {
        /* printf("4 bytes utf\n"); */
        utfbytes[0] = (unsigned char)(0xF0 | ((codepoint & 0x001C0000) >> 18));
        utfbytes[1] = (unsigned char)(0x80 | ((codepoint & 0x0003F000) >> 12));
        utfbytes[2] = (unsigned char)(0x80 | ((codepoint & 0x00000FC0) >> 6));
        utfbytes[3] = (unsigned char)(0x80 | (codepoint & 0x0000003F));
        utfbytes[4] = '\0';
    }
    else if ( codepoint <= 0x03ffffff ) {
        /* printf("5 bytes utf\n"); */
        utfbytes[0] = (unsigned char)(0xF8 | ((codepoint & 0x03000000) >> 24));
        utfbytes[1] = (unsigned char)(0x80 | ((codepoint & 0x00FC0000) >> 18));
        utfbytes[2] = (unsigned char)(0x80 | ((codepoint & 0x0003F000) >> 12));
        utfbytes[3] = (unsigned char)(0x80 | ((codepoint & 0x00000FC0) >> 6));
        utfbytes[4] = (unsigned char)(0x80 | (codepoint & 0x0000003F));
        utfbytes[5] = '\0';
    }
    else if ( codepoint <= 0x7fffffff ) {
        /* printf("6 bytes utf\n"); */
        utfbytes[0] = (unsigned char)(0xFC | ((codepoint & 0x40000000) >> 30));
        utfbytes[1] = (unsigned char)(0x80 | ((codepoint & 0x3F000000) >> 24));
        utfbytes[2] = (unsigned char)(0x80 | ((codepoint & 0x00FC0000) >> 18));
        utfbytes[3] = (unsigned char)(0x80 | ((codepoint & 0x0003F000) >> 12));
        utfbytes[4] = (unsigned char)(0x80 | ((codepoint & 0x00000FC0) >> 6));
        utfbytes[5] = (unsigned char)(0x80 | (codepoint & 0x0000003F));
        utfbytes[6] = '\0';
    }
    else {
        fprintf(stderr, "Overflow codepoint parameter\n");
    }
}


static struct unipageview * _upv_new() {
    struct unipageview * upv =
        (struct unipageview *) malloc ( sizeof ( struct unipageview ));

    if ( upv == NULL ) {
        return NULL;
    }

    upv->parent = NULL;
    upv->view = NULL;
    upv->srect.x = 0;
    upv->srect.y = 0;
    upv->srect.w = 0;
    upv->srect.h = 0;
    upv->gridded = 1;
    upv->dist = 2;
    upv->page_base = 0;
    upv->first_cpt = 0;
    upv->first_index = 0;
    upv->last_cpt = 0;
    upv->last_index = 0;
    upv->font = NULL;
    return upv;
}


struct unipageview * upv_create(SDL_Surface * parent, SDL_Rect * r) {
   struct unipageview * upv = _upv_new();
   upv->parent = parent;
   upv->srect.x = r->x;
   upv->srect.y = r->y;
   upv->srect.w = r->w;
   upv->srect.h = r->h;

   return upv;
}

int upv_init(struct unipageview * upv) {
    /* create the surface */
    upv->view = SDL_CreateRGBSurface(
        SDL_SWSURFACE, upv->srect.w, upv->srect.h,
        upv->parent->format->BitsPerPixel,
        upv->parent->format->Rmask,
        upv->parent->format->Gmask,
        upv->parent->format->Bmask,
        upv->parent->format->Amask);

    if ( upv->view == NULL ) {
        fprintf(stderr, "upv_init: SDL_CreateRGBSurface failed.\n");
        return 0;
    }

    /* test: draw border */
    Draw_Line ( upv->view, 0, 0, upv->srect.w - 1, 0,
        0xCCEE33FFL );
    Draw_Line ( upv->view, 0, 0, 0, upv->srect.h - 1,
        0xCCEE33FFL );
    Draw_Line ( upv->view,
        upv->srect.w - 1, 0, upv->srect.w - 1, upv->srect.h - 1,
        0xCCEE33FFL );
    Draw_Line ( upv->view,
        0, upv->srect.h - 1, upv->srect.w - 1, upv->srect.h - 1,
        0xCCEE33FFL );

    upv_render(upv);

    return 1;
}

static void _upv_render_clean_table(struct unipageview * upv) {
    SDL_Rect offset;

    /* clean table area */
    offset.x = 10;
    offset.y = 10;
    offset.w = 320;
    offset.h = 20 + 10 + 320;
    SDL_FillRect(upv->view, &offset, 0L);
}

static void _upv_render_gridded(struct unipageview * upv) {
    SDL_Surface * title;
    char titlebuf[20];
    SDL_Surface * glyph;
    SDL_Color textColor = { 255, 255, 255 };
    SDL_Rect offset;
    int i,j;
    char utfbytes[8];
    int minx,miny,maxx,maxy;
    int advance;
    int cpt;
    int gridx = 20;
    int gridy = 20;

    /* printf("upv_render page_base (%d)\n", upv->page_base); */

    sprintf(titlebuf, "Page %3.3x", upv->page_base);
    title = TTF_RenderText_Solid(upv->font, titlebuf, textColor);
    offset.x = 10;
    offset.y = 10;
    SDL_BlitSurface(title, NULL, upv->view, &offset);
    SDL_FreeSurface(title);

    for ( i = 0; i < 16; i++ ) {
        for ( j = 0; j < 16; j++ ) {
            cpt = (upv->page_base << 8) | (i * 16 + j);
            /*
            _get_utf8_bytes(utfbytes, cpt);
            glyph = TTF_RenderUTF8_Solid(upv->font, utfbytes, textColor);
            */
            /**/
            /* printf("cpt (%08lx)\n", cpt); */
            glyph = TTF_RenderGlyph_Solid(
                upv->font, (upv->page_base << 8) | (i * 16 + j), textColor);
            /**/

            /*
            TTF_GlyphMetrics(upv->font, cpt,
                &minx, &maxx, &miny, &maxy, &advance);
            printf("%08x %d/%d/%d/%d/%d\n",
                cpt, minx, maxx, miny, maxy, advance);
            */
            /*
            if ( advance == 0 ) {
                advance = 16;
            }
            offset.x = 10 + j * 20 + ((16 - advance) >> 2);
            offset.y = 10 + 20 + 10 + i * 20 + ((16 - advance) >> 2);
            */

            offset.x = 10 + j * gridx;
            offset.y = 10 + 20 + 10 + i * gridy;
            SDL_BlitSurface(glyph, NULL, upv->view, &offset);
            SDL_FreeSurface(glyph);
        }
    }

    SDL_BlitSurface(upv->view, NULL, upv->parent, &(upv->srect));
}

static void _upv_render_continuous(struct unipageview * upv) {
    SDL_Surface * title;
    char titlebuf[20];
    SDL_Surface * glyph;
    SDL_Color textColor = { 255, 255, 255 };
    SDL_Rect offset;
    FT_Face face;
    FT_UInt index;
    FT_ULong charcode;
    int smaxx, smaxy;
    int gx, gy;
    int linesx;
    int lineskip;
    int minx,miny,maxx,maxy;
    int advance;

    face = TTF_GetFreeTypeFace(upv->font);
    if ( upv->first_index == 0 ) {
        charcode = FT_Get_First_Char(face, &index);
    } else {
        index = upv->first_index;
        charcode = upv->first_cpt;
        charcode = FT_Get_Next_Char(face, charcode, &index);
        /*printf("page base char %08lx/%d\n", charcode, index);*/
    }

    /*
    sprintf(titlebuf, "Index %d Code Pt. %x", index, charcode);
    title = TTF_RenderText_Solid(upv->font, titlebuf, textColor);
    offset.x = 10;
    offset.y = 10;
    SDL_BlitSurface(title, NULL, upv->view, &offset);
    SDL_FreeSurface(title);
    */

    gx = 10;
    linesx = gx;
    gy = 10 + 20 + 10;
    smaxx = gx + 16 * 20;
    smaxy = gy + 16 * 20;

    lineskip = TTF_FontLineSkip(upv->font);

    while ( index != 0 ) {

        TTF_GlyphMetrics(upv->font, charcode,
            &minx, &maxx, &miny, &maxy, &advance);
        /*
        printf("char %08lx/%d\n", charcode, index);
        printf("  %d/%d/%d/%d/%d\n",
            minx, maxx, miny, maxy, advance);
        */
        if ( advance < maxx - minx ) {
            advance = maxx - minx;
        }

        if ( gx + advance + upv->dist > smaxx ) {
            gx = linesx;
            gy += lineskip + upv->dist;
            if ( gy + lineskip + upv->dist > smaxy ) {
                break;
            }
        }

        glyph = TTF_RenderGlyph_Solid(
            upv->font, charcode, textColor);

        offset.x = gx;
        offset.y = gy;
        SDL_BlitSurface(glyph, NULL, upv->view, &offset);
        SDL_FreeSurface(glyph);

        gx += advance + upv->dist;

        charcode = FT_Get_Next_Char(face, charcode, &index);
    }

    upv->last_cpt = charcode;
    upv->last_index = index;
    /*printf("last: %08x/%d\n", upv->last_cpt, upv->last_index);*/

    SDL_BlitSurface(upv->view, NULL, upv->parent, &(upv->srect));
}


void upv_render(struct unipageview * upv) {
    _upv_render_clean_table(upv);

    if ( upv->gridded == 1 ) {
        _upv_render_gridded(upv);
    } else {
        _upv_render_continuous(upv);
    }
}


void upv_setfont(struct unipageview * upv, TTF_Font * font) {
    upv->font = font;
}

unsigned int upv_getbase(struct unipageview * upv) {
    return upv->page_base;
}
void upv_setbase(struct unipageview * upv, unsigned int base) {
    /* ignore for non-gridded */
    if ( upv->gridded == 1 ) {
        upv->page_base = base & 0x0fff;
    }
}

void upv_setgridded(struct unipageview * upv, int gridded) {
    upv->gridded = gridded;
}
int upv_gridded(struct unipageview * upv) {
    return upv->gridded;
}

void upv_next(struct unipageview * upv) {
    /* printf("upv_next %x/%d\n", upv->last_cpt, upv->last_index); */
    upv->first_index = upv->last_index;
    upv->first_cpt = upv->last_cpt;
}

void upv_setdistance(struct unipageview * upv, int dist) {
    upv->dist = dist;
}
