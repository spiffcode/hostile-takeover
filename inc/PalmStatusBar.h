/******************************************************************************
 *
 * Copyright (c) 2003 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: PalmStatusBar.h
 *
 * Description:
 *      Public API for the Status Bar Library
 *
 * History:
 *		Version 1.0		- Intial Revision (03/03/03)
 *
 *****************************************************************************/

#ifndef __STATUSBARMGRLIB_H__
#define __STATUSBARMGRLIB_H__

#include <PalmTypes.h>
#include <LibTraps.h>

/********************************************************************
 * Library type and creator
 ********************************************************************/
#define statLibName			"StatusBarMgrLib"
#define statLibCreator		'sBar'
#define statLibType			sysFileTLibrary
#define statFtrNumVersion	(0)

/********************************************************************
 * Status Bar Errors
 ********************************************************************/
#define T3statErrorClass			(appErrorClass  | 0x0900)
#define T3statErrNoStatusBar		(statErrorClass | 1)
#define T3statErrInvalidSelector	(statErrorClass | 2)
#define T3statErrInputWindowOpen 	(statErrorClass | 3)
#define T3statErrBadParam			(statErrorClass | 101)
#define T3statErrInvalidState		(statErrorClass | 102)

/********************************************************************
 * Status Bar Attributes
 ********************************************************************/
typedef enum StatAttrTypeTag
{
    T3statAttrExists     = 0,         // device supports the status bar
    T3statAttrBarVisible,             // status bar is visible
    T3statAttrDimension               // bounds of status bar window
} StatAttrType;

/********************************************************************
 * Library Traps
 ********************************************************************/
#define kStatusBarMgrLibTrapOpen		sysLibTrapOpen
#define kStatusBarMgrLibTrapClose		sysLibTrapClose
#define kStatusBarMgrLibGetAttribute	(sysLibTrapCustom)
#define kStatusBarMgrLibHide			(sysLibTrapCustom + 1)
#define kStatusBarMgrLibShow			(sysLibTrapCustom + 2)


/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
   
Err T3StatLibOpen(UInt16 refnum)
				SYS_TRAP(kStatusBarMgrLibTrapOpen);
				
Err T3StatLibClose(UInt16 refnum)
				SYS_TRAP(kStatusBarMgrLibTrapClose);

Err T3StatGetAttribute(UInt16 refnum, StatAttrType selector, UInt32 *dataP)
				SYS_TRAP(kStatusBarMgrLibGetAttribute);
				
Err T3StatHide(UInt16 refnum)
				SYS_TRAP(kStatusBarMgrLibHide);
				
Err T3StatShow(UInt16 refnum)
				SYS_TRAP(kStatusBarMgrLibShow);

#ifdef __cplusplus
}
#endif

#endif  //__STATUSBARMGRLIB_H__
