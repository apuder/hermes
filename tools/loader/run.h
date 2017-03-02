/*
 * run.h
 *---------------------------------------------------------
 * Header file for run.c                           AP  2/91
 */

#ifndef __RUN_H__
#define __RUN_H__

#define BYTE				0xff

#define NUM_GLOBAL_PARAMS		11
#define GLOBAL_PARAM_BASE		0x00000100

#define GNU_MAGIC_NUMBER		0x00640107

#define TEXT_SEGMENT_BASE		0x00100000
#define GDT_BASE			0x00000000

#define SIZE_OF_GDT_ENTRY		8
#define ATTR_DATA			0x02
#define ATTR_CODE			0x0a


void run_hermes_executable( void );


#endif
