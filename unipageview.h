#ifndef INC_UNIPAGEVIEW_H
#define INC_UNIPAGEVIEW_H

/* ------------------------------------------------------------------------- */
/* unipageview.h - this file is part of unicodeview                          */
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
/* shows 256 glyphs of a font in a 8x8 matrix */


#include <SDL/SDL.h>
#include "SDL_ttf.h"


struct unipageview {
  SDL_Surface * parent;
  SDL_Surface * view;
  SDL_Rect srect;
  int gridded;
  int dist;
  unsigned int page_base;
  unsigned int first_cpt;
  unsigned int first_index;
  unsigned int last_cpt;
  unsigned int last_index;
  TTF_Font * font;
};


extern struct unipageview * upv_create(
  SDL_Surface * parent, SDL_Rect * r);
extern int upv_init(struct unipageview * upv);
extern void upv_render(struct unipageview * upv);
extern void upv_setfont(struct unipageview * upv, TTF_Font * font);
extern unsigned int upv_getbase(struct unipageview * upv);
extern void upv_setbase(struct unipageview * upv, unsigned int base);
extern void upv_setgridded(struct unipageview * upv, int gridded);
extern int upv_gridded(struct unipageview * upv);
extern void upv_next(struct unipageview * upv);
extern void upv_setdistance(struct unipageview * upv, int dist);

#endif
