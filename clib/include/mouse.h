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

#ifndef _MOUSE_H_
#define _MOUSE_H_

#define	M_LEFT_DOWN	0x001
#define	M_LEFT_UP	0x002
#define	M_MIDDLE_DOWN	0x004
#define	M_MIDDLE_UP	0x008
#define	M_RIGHT_DOWN	0x010
#define	M_RIGHT_UP	0x020
#define	M_MOTION	0x040
#define	M_KEYPRESS	0x080
#define M_POLL		0x100
#define M_NOPAINT	0x200

#define M_BUTTON_DOWN	(M_LEFT_DOWN | M_MIDDLE_DOWN | M_RIGHT_DOWN)
#define M_BUTTON_UP	(M_LEFT_UP | M_MIDDLE_UP | M_RIGHT_UP)

#define	M_LEFT		1
#define	M_RIGHT		2
#define	M_MIDDLE	4

typedef struct {
  int flags;
  int x,y;
  int buttons;
  int key;
} MouseEvent;

#ifdef __cplusplus
extern "C" {
#endif

void MouseGetEvent(int flags, MouseEvent *event);
void MouseSetColors(int fg, int bg);
void MouseSetSpeed(int speed);
void MouseWarp(int x, int y);

#ifdef __cplusplus
}
#endif

#endif
