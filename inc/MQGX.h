/* $Header:
 *
 * ========================================================================
 * Copyright (c) 2002 by MediaQ, Incorporated. All Rights Reserved.
 *
 * Confidential and Proprietary to MediaQ, Incorporated.
 * ========================================================================
 *
 * $Log:
 * 
 */


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MQGX_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MQGAPIEXT_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifndef _MQGX_H_
#define _MQGX_H_

#ifdef UNDER_CE
#include "windows.h"
#else /* Palm OS */
#endif

#include "mqcommon.h"

#define MQGX_MAJOR_VERSION 0x0000
#define MQGX_MINOR_VERSION 0x0006

#define MQ_GX_ERROR 0
#define MQ_GX_SUCCESS 1

#define MQ_ERROR MQ_GX_ERROR
#define MQ_SUCCESS MQ_GX_SUCCESS

#define MQ_GX_WAITFORVSYNC 0 // waits for new VSync
#define MQ_GX_IS_VSYNC 1	 // returns 1 if in VSync else 0
#define MQ_GX_GETSCANLINE 2  // returns current scanline
							 // being refreshed
typedef struct _MQPROPERTY
{
	UINT16  VendorID;		// vendorID (0x4D51)
	UINT16  DeviceID;
	UINT16  DeviceRev;
	UINT32  VideoMemSize;	// size in bytes
	UINT32  BuildNumber; // Build Number
} MQPROPERTY, *PMQPROPERTY;

typedef struct _MQRECT
{
	UINT16  top;
	UINT16  left;
	UINT16  right;
	UINT16  bottom;
} MQRECT, *PMQRECT;

typedef struct _MQGXPROPERTY
{
	MQPROPERTY	Property;
	UINT32  	GXVersion;	// Upper 16bit: Major number
							// Lower 16bit: Minor number
	UINT32  	GXCapFlags;	// see definition of CapFlags
} MQGXPROPERTY, *PMQGXPROPERTY;

// GXCapFlags definitions:
#define	MQ_GX_CAP_BLT			0x00000001	// MQGxFillRect(), MQGxCopyRect()
											// and MQGxBltRect()
#define	MQ_GX_CAP_LINE			0x00000002	// MQGxLine() is available
#define	MQ_GX_CAP_CLIP			0x00000004	// MQGxSetClip() is available
//#define	MQ_GX_CAP_STRETCHBLT	0x00000008

// Miscellaneous flag
#define	MQ_GX_BLT_CLIP					0x00000001
// enable clipping(MQSetClip must be called first)

// Transparent flags
#define	MQ_GX_BLT_TRANSPARENT_SRC_COLOR	0x00000004
// color source transparent blt (if source pixel		
//   matches colorCompare, do not overwrite)
#define	MQ_GX_BLT_TRANSPARENT_SRC_MONO	0x00000008
// mono source transparent blt (if source bit is 0, no
//   overwrite on corresponding destination location)
#define	MQ_GX_BLT_TRANSPARENT_SRC_MONO_INV	0x00000010
// mono source inverted transparent blt (if source bit
//   is 1, no overwrite on corresponding destination
//   location)
#define	MQ_GX_BLT_TRANSPARENT_DST		0x00000020
// destination transparent blt (if destination pixel
//   matches colorCompare, overwrite)

// Source flags
#define	MQ_GX_BLT_SRC_SYSMEM_COLOR		0x00000040
// color source in system memory
#define	MQ_GX_BLT_SRC_SYSMEM_MONO		0x00000080
// mono source in system memory
#define	MQ_GX_BLT_SRC_SCREEN_MONO		0x00000100
// mono source in video/screen memory
#define	MQ_GX_BLT_SRC_SOLID				0x00000400
// source is solid color

// Pattern flags
#define	MQ_GX_BLT_PAT_MONO				0x00000800
// pattern is mono 8x8 bitmap
#define	MQ_GX_BLT_PAT_SOLID				0x00001000
// pattern is solid color

#if defined(UNDER_CE)
#if !defined(MQAPI_STATIC_LIBRARY)

#ifdef MQGX_EXPORTS
#define MQAPI_FUNC __declspec(dllexport)
#else
#define MQAPI_FUNC __declspec(dllimport)
#endif
#endif

#if defined(MQAPI_STATIC_LIBRARY)
#define MQAPI_FUNC extern "C"
#endif

#else /* Palm OS */

//#ifdef BUILDING_MQGx_LIB
//#define MQAPI_FUNC
//#else
#define MQAPI_FUNC extern
//#endif

// PalmPilot common definitions
//#include <Common.h>
//#include <SystemMgr.rh>


/********************************************************************
 * Type and creator of MQGx Library database
 ********************************************************************/
#define		MQGxLibCreatorID	'MEDQ'				// MQGx Library database creator
#define		MQGxLibTypeID		'libr'				// Standard library database type


/********************************************************************
 * Internal library name which can be passed to SysLibFind()
 ********************************************************************/
#define		MQGxLibName			"MQGx.lib"		


/************************************************************
 * MQGx Library result codes
 * (appErrorClass is reserved for 3rd party apps/libraries.
 * It is defined in SystemMgr.h)
 *************************************************************/

#define MQGxErrParam			(appErrorClass | 1)		// invalid parameter
#define MQGxErrNotOpen			(appErrorClass | 2)		// library is not open
#define MQGxErrStillOpen		(appErrorClass | 3)		// returned from MQGxLibClose() if
														// the library is still open by others
#define MQGxErrMemory			(appErrorClass | 4)		// memory error occurred

#endif

#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

	
// Exported functions
MQAPI_FUNC MQAPI_OPEN_RETTYPE MQGxOpen(MQAPI_OPEN_HANDLE GXhandle)
								MQGx_LIB_TRAP(sysLibTrapOpen);

MQAPI_FUNC MQAPI_RETTYPE MQGxClose(MQAPI_HANDLE GXhandle)
										MQGx_LIB_TRAP(sysLibTrapClose);

MQAPI_FUNC MQAPI_RETTYPE MQGxGetProperty(MQAPI_HANDLE GXhandle, 
										PMQGXPROPERTY pGxProp)
										MQGx_LIB_TRAP(MQGxLibTrapGetProperty);
MQAPI_FUNC MQAPI_RETTYPE MQGxFillRect(MQAPI_HANDLE GXhandle,
										INT16 x, 
										INT16 y,
										INT16 w,
										INT16 h,
										UINT32 color)
										MQGx_LIB_TRAP(MQGxLibTrapFillRect);
MQAPI_FUNC MQAPI_RETTYPE MQGxCopyRect(MQAPI_HANDLE GXhandle,
										INT16 dx, 
										INT16 dy,
										INT16 w,
										INT16 h,
										INT16 sx,
										INT16 sy)
										MQGx_LIB_TRAP(MQGxLibTrapCopyRect);
MQAPI_FUNC MQAPI_RETTYPE MQGxBlt(MQAPI_HANDLE GXhandle,
										MQGXBLTPARAM *pBltParam)
										MQGx_LIB_TRAP(MQGxLibTrapBlt);

MQAPI_FUNC MQAPI_RETTYPE MQGxCopyMonoBitmap(MQAPI_HANDLE GXhandle,
										INT16 dx, INT16 dy,
										INT16 w, INT16 h,
										INT16 sx, INT16 sy,
										UINT16 fgColor, UINT16 bgColor,
										INT16 srcStride,
										UINT8 *pMonoBits)
										MQGx_LIB_TRAP(MQGxLibTrapCopyMonoBitmap);

MQAPI_FUNC MQAPI_RETTYPE MQGxCopyTransMonoBitmap(MQAPI_HANDLE GXhandle,
 										INT16 dx, INT16 dy,
 										INT16 w, INT16 h,
     									INT16 sx, INT16 sy,
										UINT16 color,
										INT16 srcStride,
										UINT8 *pMonoBits,
										INT16 selectTrans )
										MQGx_LIB_TRAP(MQGxLibTrapCopyTransMonoBitmap);

MQAPI_FUNC MQAPI_RETTYPE MQGxCopyColorBitmap(MQAPI_HANDLE GXhandle,
 										INT16 dx, INT16 dy,
 										INT16 w, INT16 h,
     									INT16 sx, INT16 sy,
										INT16 srcStride,
										UINT8 *pColorBits )
										MQGx_LIB_TRAP(MQGxLibTrapCopyColorBitmap);

MQAPI_FUNC MQAPI_RETTYPE MQGxLine(MQAPI_HANDLE GXhandle,
										UINT16 x1, 
										UINT16 y1,
										UINT16 x2,
										UINT16 y2,
										UINT16 rop2,
										UINT32 color, 
										UINT16 flags)
										MQGx_LIB_TRAP(MQGxLibTrapLine);
MQAPI_FUNC MQAPI_RETTYPE MQGxSetClip(MQAPI_HANDLE GXhandle,	PMQRECT pClipRect)
										MQGx_LIB_TRAP(MQGxLibTrapSetClip);
MQAPI_FUNC MQAPI_RETTYPE MQGxSetDisplayAddr(MQAPI_HANDLE GXhandle,
											UINT32 uiDispAddr32)
										MQGx_LIB_TRAP(MQGxLibTrapSetDisplayAddr);
MQAPI_FUNC MQAPI_RETTYPE MQGxVSync(MQAPI_HANDLE GXhandle, UINT16 option)
										MQGx_LIB_TRAP(MQGxLibTrapVSync);
MQAPI_FUNC MQAPI_RETTYPE MQGxSetPal(MQAPI_HANDLE GXhandle, 
										UINT32 palVal, UINT16 index)
										MQGx_LIB_TRAP(MQGxLibTrapSetPal);

MQAPI_FUNC MQAPI_RETTYPE MQGxSetPalRange(MQAPI_HANDLE GXhandle, 
										UINT32 * palRange, UINT16 startIndex,
										UINT16 endIndex)
										MQGx_LIB_TRAP(MQGxLibTrapSetPalRange);

MQAPI_FUNC MQAPI_RETTYPE MQGxGetPal(MQAPI_HANDLE GXhandle, 
										UINT32 * pPalVal, UINT16 index)
										MQGx_LIB_TRAP(MQGxLibTrapGetPal);
MQAPI_FUNC MQAPI_RETTYPE MQGxGetPalRange(MQAPI_HANDLE GXhandle, 
									UINT32 * pPalArray,
									UINT16 startIndex, UINT16 endIndex)
									MQGx_LIB_TRAP(MQGxLibTrapGetPalRange);
MQAPI_FUNC MQAPI_RETTYPE MQGxBltFullScreen(MQAPI_HANDLE GXhandle, 
									MQGXBLTPARAM *pBltParam)
									MQGx_LIB_TRAP(MQGxLibTrapBltFullScreen);
MQAPI_FUNC MQAPI_RETTYPE MQGxCopyRectDirect(MQAPI_HANDLE GXhandle,
									INT16 dx, 
									INT16 dy,
									INT16 w,
									INT16 h,
									INT16 sx,
									INT16 sy)
									MQGx_LIB_TRAP(MQGxLibTrapCopyRectDirect);
MQAPI_FUNC MQAPI_RETTYPE MQGxCopyPackedColorBitmap(MQAPI_HANDLE GXhandle, 
									UINT32 *pColorBits,
									UINT16 bitmapSize,
									MQRECT rect)
									MQGx_LIB_TRAP(MQGxLibTrapBltPackedBitmap);

#ifndef UNDER_CE
//--------------------------------------------------
// Standard library open, close, sleep and wake functions
//--------------------------------------------------

extern Err MQGxSleep(UInt16 refNum)
				MQGx_LIB_TRAP(sysLibTrapSleep);

extern Err MQGxWake(UInt16 refNum)
				MQGx_LIB_TRAP(sysLibTrapWake);


//--------------------------------------------------
// Custom library API functions
//--------------------------------------------------

// For loading the library in Palm OS Mac emulation mode
extern Err MQGxLibInstall(UInt16 refNum, SysLibTblEntryPtr entryP);

#endif

#ifdef __cplusplus
}
#endif

#endif // _MQGX_H_