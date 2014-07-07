/******************************************************************************
 *
 * Copyright (c) 2003 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: PalmPin.h
 *
 * Description:
 *      Public API for the Pen Input Manager Library
 *
 * History:
 *		Version 1.0		- Intial Revision (03/03/03)
 *
 *****************************************************************************/

#ifndef __PINLIB_h__
#define __PINLIB_H__

#include <PalmTypes.h>
#include <LibTraps.h>

/********************************************************************
 * Library type and creator
 ********************************************************************/
#define pinLibType				sysFileTLibrary
#define pinLibCreator			'pinM'
#define pinLibName				"PinLib"
#define pinFtrNumVersion		0

/********************************************************************
 * Pinlet IDs
 ********************************************************************/
#define pinClassic				"Classic"
#define pinTriCell				"Tri Cell"
#define pinStdKeyboard			"StdKB"
#define pinletStdKbNum			"StdKBNum"
#define pinletStdKbIntl			"StdKBIntl"

/********************************************************************
 * PIN Manager Errors
 ********************************************************************/
#define pinErrorClass			(appErrorClass | 0x0A00) 
#define pinErrInvalidState		(pinErrorClass + 1)
#define pinErrUnknownID			(pinErrorClass + 2)
#define pinErrInvalidInputMode	(pinErrorClass + 3)

/********************************************************************
 * Input area states
 ********************************************************************/
#define pinInputAreaNone		0
#define pinInputAreaShow		1
#define pinInputAreaHide		2
#define pinInputAreaLegacyMode	3
#define pinInputAreaFullScreen	4

/********************************************************************
 * Input Modes
 ********************************************************************/
#define pinInputModeNormal		0
#define pinInputModeShift		1
#define pinInputModeCapsLock	2
#define pinInputModePunctuation	3
#define pinInputModeNumeric		4
#define pinInputModeExtended	5
#define pinInputModeHiragana	6
#define pinInputModeKatakana	7

/********************************************************************
 * AIA event and notification
 ********************************************************************/
#define sysNotifyAiaEvent		'Aian'
#define AiaExtentChangedEvent	((eventsEnum)(firstUserEvent+2))

typedef struct AiaExtentChangedEventDataType
{
	RectangleType newDim;
	RectangleType oldDim;
} AiaExtentChangedEventDataType;

// Macro to simplify getting the data out of the event structure.
// Example:
// yDiff = AiaExtentChangedData(eventP)->newDim->extent.y -
//         aiaExtentChangedData(eventP)->oldDim->extent.y;
#define aiaExtentChangedData(eventP) ((AiaExtentChangedEventDataType *)(&((eventP)->data.generic)))


/********************************************************************
 * Library Traps
 ********************************************************************/
#define kPinLibTrapOpen						sysLibTrapOpen
#define kPinLibTrapClose					sysLibTrapClose
#define kPinLibTrapPinGetInputAreaState		(sysLibTrapCustom + 17)
#define kPinLibTrapPinSetInputAreaState		(sysLibTrapCustom + 18)
#define kPinLibTrapPinResetInputState		(sysLibTrapCustom + 24)
#define kPinLibTrapPinGetCurrentPinletID	(sysLibTrapCustom + 20)
#define kPinLibTrapPinSwitchToPinlet		(sysLibTrapCustom + 21)
#define kPinLibTrapPinGetInputMode			(sysLibTrapCustom + 23)
#define kPinLibTrapPinSetInputMode			(sysLibTrapCustom + 22)
#define kPinLibTrapPINShowReferenceDialog	(sysLibTrapCustom + 25)


/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Err	PinLibOpen(UInt16 refnum)
				SYS_TRAP(kPinLibTrapOpen);
Err	PinLibClose(UInt16 refnum)
				SYS_TRAP(kPinLibTrapClose);

UInt16 PinGetInputAreaState(UInt16 refnum)
				SYS_TRAP(kPinLibTrapPinGetInputAreaState);
				
Err PinSetInputAreaState(UInt16 refnum, UInt16 state)
				SYS_TRAP(kPinLibTrapPinSetInputAreaState);

void PinResetInputState(UInt16 refnum)
				SYS_TRAP(kPinLibTrapPinResetInputState);
				
const char* PinGetCurrentPinletID(UInt16 refnum)
				SYS_TRAP(kPinLibTrapPinGetCurrentPinletID);
				
Err PinSwitchToPinlet(UInt16 refnum, const char* pinletID, UInt16 initialInputMode)
				SYS_TRAP(kPinLibTrapPinSwitchToPinlet);

UInt16 PinGetInputMode(UInt16 refnum)
				SYS_TRAP(kPinLibTrapPinGetInputMode);
				
void PinSetInputMode(UInt16 refnum, UInt16 inputMode)
				SYS_TRAP(kPinLibTrapPinSetInputMode);
				
#ifdef __cplusplus
}
#endif


#endif  //__PINLIB_h__
