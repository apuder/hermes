/* This is file graphics.h */
/*
** Copyright (C) 1991 DJ Delorie, 24 Kirsten Ave, Rochester NH 03867-2954
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* History:15,50 */
#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  GR_80_25_text,
  GR_default_text,
  GR_width_height_text,
  GR_biggest_text,
  GR_320_200_graphics,
  GR_default_graphics,
  GR_width_height_graphics,
  GR_biggest_noninterlaced_graphics,
  GR_biggest_graphics
} GR_graphics_modes;

#ifdef __cplusplus
void GrSetMode(int which, int width=0, int height=0);
#else
void GrSetMode();
#endif

void GrSetColor(int c, int r, int g, int b);
int GrAllocColor(int r, int g, int b); /* shared, read-only */
int GrAllocCell(void); /* unshared, read-write */
void GrQueryColor(int n, int *r, int *g, int *b);
void GrFreeColor(int c);
int GrWhite(void);
int GrBlack(void);

/* or a color with GrXOR to "xor" the color onto the screen */
#define GrXOR		0x100
/* GrNOCOLOR is used for "no" color */
#define GrNOCOLOR	0x100

void GrPlot(int x, int y, int c);
int GrPixel(int x, int y);

int GrMaxX(void);
int GrMaxY(void);
int GrSizeX(void);
int GrSizeY(void);

void GrLine(int x, int y, int x2, int y2, int c);

void GrTextXY(int x, int y, char *text, int fg, int bg);

#ifdef __cplusplus
}

typedef enum {
  BlitSrc,
  BlitDest,
  BlitXor,
} GrBlitFunc;

class GrRegion;

GrRegion *GrScreenRegion();

void Blit(GrRegion *src,
          GrRegion *dest, int dx, int dy,
          GrBlitFunc function);
void Blit(GrRegion *src, int sx, int sy, int sw, int sh,
          GrRegion *dest, int dx, int dy,
          GrBlitFunc function);

class GrRegion {
public:
  int flags;
  int color;

  GrRegion *parent;
  int rel_x, rel_y, abs_x, abs_y;

  int width;
  int height;
  int row_scale;
  unsigned char *data;  /* for read/write operations *EXCEPT* bcopy/memcpy */
  unsigned char *rdata; /* for read via bcopy/memcpy */
  unsigned char *wdata; /* for write via bcopy/memcpy */

  GrRegion();
  GrRegion(int width, int height); /* memory buffer */
 ~GrRegion();
  GrRegion *SubRegion(int x, int y, int w, int h);

  int  MaxX();
  int  MaxY();
  int  SizeX();
  int  SizeY();

  void Plot(int x, int y, int c=-1);
  void Line(int x1, int y1, int x2, int y2, int c=-1);
  void HLine(int x1, int x2, int y, int c=-1);
  void VLine(int x, int y1, int y2, int c=-1);
  void Rectangle(int x1, int y1, int x2, int y2, int c=-1); /* outline */
  void Box(int x, int y, int w, int h, int c=-1); /* filled */

  void Text(int x, int y, char *text, int fg=-1, int bg=-1);

  int  Point(int x, int y);
};

#endif

#endif
