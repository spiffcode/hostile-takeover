/* $Header:
 *
 * ========================================================================
 * Copyright (c) 2002 by MediaQ, Incorporated. All Rights Reserved.
 *
 * Confidential and Proprietary to MediaQ, Incorporated.
 * ========================================================================
 *
 * $Log:   
 */

#ifndef _MQCOMMON_H_
#define _MQCOMMON_H_
/*-------------------------------------------------------*
 | Configurations that can be modified by API developers.|
 *-------------------------------------------------------*/
//#define	MQAPI_STATIC_LIBRARY

/*-------------------------------------------------------*
 | Stop! Don't modified anything below.                  |
 *-------------------------------------------------------*/

#ifdef UNDER_CE

#define MQVx_LIB_TRAP(trapNum)
#define MQGx_LIB_TRAP(trapNum)

#if !defined(QUERYESCSUPPORT) 
#define  QUERYESCSUPPORT 8	// Just in case it is not defined
#endif

#if !defined(MQAPI_RETTYPE)
typedef int MQAPI_RETTYPE;
#endif
#if !defined(PVOID)
typedef void        *           PVOID;
#endif

#if !defined(MQAPI_HANDLE)
typedef UINT32 MQAPI_HANDLE;
#endif

#if !defined(MQAPI_OPEN_HANDLE)
typedef MQAPI_HANDLE MQAPI_OPEN_HANDLE;
#endif

#if !defined(MQAPI_OPEN_RETTYPE)
typedef MQAPI_HANDLE MQAPI_OPEN_RETTYPE;
#endif

#else /* Palm OS*/

#include <PalmOS.h>
#include <PalmTypes.h>
#include <SystemResources.h>
#include <Rect.h>				// for RectangleType
#include <Window.h>			// for WinDrawRectangle

#if !defined(INT8)
typedef signed      char        INT8;
#endif

#if !defined(UINT8)
typedef unsigned    char        UINT8;
#endif

#if !defined(INT16)
typedef 			Int16		INT16;
#endif

#if !defined(UINT16)
typedef 			UInt16      UINT16;
#endif

#if !defined(INT32)
typedef 			Int32   	INT32;
#endif

#if !defined(UINT32)
typedef 			UInt32      UINT32;
#endif

#if !defined(BYTE)
typedef unsigned    char        BYTE;
#endif

#if !defined(PVOID)
typedef void        *           PVOID;
#endif

#if !defined(MQAPI_STATIC_LIBRARY)
#ifdef BUILDING_MQVx_LIB
#define MQVx_LIB_TRAP(trapNum) 
#define MQGx_LIB_TRAP(trapNum) 
#else
#define MQVx_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#define MQGx_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif
#endif

#if defined(MQAPI_STATIC_LIBRARY)
#define MQVx_LIB_TRAP(trapNum) 
#define MQGx_LIB_TRAP(trapNum) 
#endif

#if !defined(MQAPI_OPEN_HANDLE)
typedef UINT16 MQAPI_OPEN_HANDLE;
#endif

#if !defined(MQAPI_OPEN_RETTYPE)
typedef Err MQAPI_OPEN_RETTYPE;
#endif

#if !defined(MQAPI_RETTYPE)
typedef Err MQAPI_RETTYPE;
#endif

#if !defined(MQAPI_HANDLE)
typedef UINT16 MQAPI_HANDLE;
#endif

#if !defined(MQAPI_STATIC_LIBRARY)

#if 0
typedef enum {
	MQGxLibTrapGetProperty = sysLibTrapCustom,
	MQGxLibTrapFillRect,
	MQGxLibTrapCopyRect,
	MQGxLibTrapBlt,
	MQGxLibTrapCopyMonoBitmap,
	MQGxLibTrapCopyTransMonoBitmap,
	MQGxLibTrapCopyColorBitmap,
	MQGxLibTrapLine,
	MQGxLibTrapSetClip,
	MQGxLibTrapSetDisplayAddr,
	MQGxLibTrapVSync,
	MQGxLibTrapSetPal,
	MQGxLibTrapSetPalRange,
	MQGxLibTrapGetPal,
	MQGxLibTrapGetPalRange,
	MQGxLibTrapBltFullScreen,
	MQGxLibTrapCopyRectDirect,
	MQGxLibTrapBltPackedBitmap,
				
	MQGxLibTrapLast
	} MQGxLibTrapNumberEnum;

typedef enum {
	MQVxLibTrapGetProperty = sysLibTrapCustom,
	MQVxLibTrapSetVideo,
	MQVxLibTrapShowVideo,
	MQVxLibTrapConvertImage,
	MQVxLibTrapSetVIP,
	MQVxLibTrapVIPStart,
	MQVxLibTrapVIPStop,	
	MQVxLibTrapVIPCapture,
	MQVxLibTrapAllocSurface,
	MQVxLibTrapFreeSurface,
	MQVxLibTrapSurfaceLock,
	MQVxLibTrapSurfaceUnlock,
	
	MQVxLibTrapLast
	} MQVxLibTrapNumberEnum;
#else
#define MQGxLibTrapGetProperty (sysLibTrapCustom + 0)
#define MQGxLibTrapFillRect (sysLibTrapCustom + 1)
#define MQGxLibTrapCopyRect (sysLibTrapCustom + 2)
#define MQGxLibTrapBlt (sysLibTrapCustom + 3)
#define MQGxLibTrapCopyMonoBitmap (sysLibTrapCustom + 4)
#define MQGxLibTrapCopyTransMonoBitmap (sysLibTrapCustom + 5)
#define MQGxLibTrapCopyColorBitmap (sysLibTrapCustom + 6)
#define MQGxLibTrapLine (sysLibTrapCustom + 7)
#define MQGxLibTrapSetClip (sysLibTrapCustom + 8)
#define MQGxLibTrapSetDisplayAddr (sysLibTrapCustom + 9)
#define MQGxLibTrapVSync (sysLibTrapCustom + 10)
#define MQGxLibTrapSetPal (sysLibTrapCustom + 11)
#define MQGxLibTrapSetPalRange (sysLibTrapCustom + 12)
#define MQGxLibTrapGetPal (sysLibTrapCustom + 13)
#define MQGxLibTrapGetPalRange (sysLibTrapCustom + 14)
#define MQGxLibTrapBltFullScreen (sysLibTrapCustom + 15)
#define MQGxLibTrapCopyRectDirect (sysLibTrapCustom + 16)
#define MQGxLibTrapBltPackedBitmap (sysLibTrapCustom + 17)
#define MQGxLibTrapLas (sysLibTrapCustom + 18)

#define MQVxLibTrapGetProperty (sysLibTrapCustom + 0)
#define MQVxLibTrapSetVideo (sysLibTrapCustom + 1)
#define MQVxLibTrapShowVideo (sysLibTrapCustom + 2)
#define MQVxLibTrapConvertImage (sysLibTrapCustom + 3)
#define MQVxLibTrapSetVIP (sysLibTrapCustom + 4)
#define MQVxLibTrapVIPStart (sysLibTrapCustom + 5)
#define MQVxLibTrapVIPStop (sysLibTrapCustom + 6)
#define MQVxLibTrapVIPCapture (sysLibTrapCustom + 7)
#define MQVxLibTrapAllocSurface (sysLibTrapCustom + 8)
#define MQVxLibTrapFreeSurface (sysLibTrapCustom + 9)
#define MQVxLibTrapSurfaceLock (sysLibTrapCustom + 10)
#define MQVxLibTrapSurfaceUnlock (sysLibTrapCustom + 11)
#define MQVxLibTrapLas (sysLibTrapCustom + 12)
#endif

#endif

typedef struct _MQGXBLTPARAM
{
	// Destination location and Dimension parameters
	INT16  dx;			// destination x in pixels
						//   (relative to left)
	INT16  dy;			// destination y in scan lines
						//   (relative to top)
	INT16  w;			// width of the rectangle in pixels
	INT16  h;			// height of the rectangle in scan lines

	// Source bitmap parameters
	INT16  sx;			// source x in pixels (relative to left)
	INT16  sy;			// source y in scan lines
						//   (relative to top)
	INT16  sw;			// width of the source rectangle in
						//   pixels (for Stretch Blt only)
	INT16  sh;			// height of the source rectangle in
						//   scanlines (for Stretch Blt only)
	UINT16 bpp;			// color depth
	INT16  stride;		// number of bytes per scanline
						//   for bitmap
	UINT32 fgColor;		// foreground color (used if pixel data
						//   is 1 on 1bpp source bitmap)
	UINT32 bgColor;  	// background color (used if pixel data
						//   is 0 on 1bpp source bitmap)
	UINT8  *pBits;		// pointer to source bitmap

						// Pattern bitmap parameters
	INT16  px;			// pattern offset in x direction
	INT16  py; 			// pattern offset in y direction
	UINT32 patFgColor;	// foreground color (used if pixel data
						//   is 1 on 1bpp source bitmap)
	UINT32 patBgColor;  // background color (used if pixel data
						//   is 0 on 1bpp source bitmap)
	UINT32 pattern0;	// First 32 bits of Pattern data (1bpp)
	UINT32 pattern1;	// Second 32 bits of Pattern data (1bpp)

	// Miscellaneous parameters
	UINT32 colorCompare;// ColorKey for Transparent Blt
	UINT32 flags;		// (see below)
	UINT16 rop3;		// ROP3 code
} MQGXBLTPARAM;

#endif

#endif //_MQCOMMON_H_