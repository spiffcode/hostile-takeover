/******************************************************************************
 * Copyright (c) 2003 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
 
/**
 * @file 	PalmGoLCD.h
 * @version 1.0
 * @date 	03/03/2003
 *
 * Public API for the GoLCD Library.
 *
 * You can use the functions in this API to enable or disable fullscreen
 * writing, enable and disable the Graffiti 2 shift indicator (GSI) as a control for
 * full-screen writing, enable and disable Graffiti 2 inking, and even change the colors
 * of Graffiti 2 inking and the GSI.
 *
 * You can check whether the GoLCD Manager is installed by examining its feature set.
 * Call FtrGet as follows:
 * @sample
 * @code err = FtrGet (goLcdCreator, goLcdFtrNumVersion, &value); @endcode
 *
 * <hr>
 */

#ifndef __PALMGOLCD_H__
#define __PALMGOLCD_H__

#include <PalmTypes.h>
#include <LibTraps.h>

/********************************************************************
 * Library type and creator
 ********************************************************************/

#define goLcdLibName          "GoLcdLib"	/**< GoLCD library name. */
#define goLcdLibType          'libr'		/**< GoLCD library type. */
#define goLcdLibCreator       'GAny'		/**< GoLCD creator ID. */
#define goLcdLibFtrNum         0			/**< GoLCd feature number. */


/********************************************************************
 * Constants
 ********************************************************************/
 
/**
 * GoLCD Status.
 * One of the following:
 * - goLcdNotAvailable
 * - goLcdDisabled
 * - goLcdEnabled
 */
typedef Int32 GoLcdStatusType;

#define	goLcdNotAvailable	   -1	/**< GoLCD not available on this device. */
#define goLcdDisabled			0	/**< GoLCD functionality is disabled. */
#define goLcdEnabled			1	/**< GoLCD functionality is enabled. */


/**
 * GoLCD Ink State.
 * One of the following:
 * - goLcdInkDisabled
 * - goLcdInkEnabled
 */
typedef Int32 GoLcdInkStateType;

#define goLcdInkDisabled		0	/**< GoLCD inking is disabled. */
#define goLcdInkEnabled			1	/**< GoLCD inking is enabled. */


/**
 * GoLCD Timeout Mode.
 * One of the following:
 * - goLcdGraffitiMode
 * - goLcdPenTapMode
 */
typedef UInt32 GoLcdModeType;

/**
 * When it enters goLcdGraffitiMode, GoLCD continuously
 * interprets all pen events as Graffiti 2 strokes. There is a timeout
 * value associated with goLcdGraffitiMode, however. If it
 * does not receive a new pen event within the allotted time, it
 * returns to goLcdPenTapMode.
 * <br>
 * The default time-out value for goLcdGraffitiMode is
 * 150 system ticks, or 1500 milliseconds.
 */
#define goLcdGraffitiMode		0
/**
 * GoLCD starts in goLcdPenTapMode. When a penDownEvent is
 * received, a timer is started. If a penUpEvent is received before
 * the timer reaches the time-out value for this mode, the pen
 * events are passed on to the event-handler for the application
 * control. Otherwise, if the pen events exceed the time-out
 * value and the x, y coordinates change significantly, GoLCD
 * enters goLcdGraffitiMode and treats the pen events as a
 * Graffiti 2 stroke.
 * <br>
 * The default time-out value for goLcdPenTapMode is 15 system
 * ticks, or 150 milliseconds.
 */
#define goLcdPenTapMode			1
#define goLcdModeCount			2


/**
 * GoLCD GSI State.
 * One of the following:
 * - goLcdGsiNormal
 * - goLcdGsiOverride
 */
typedef UInt32 GoLcdGsiStateType;

#define goLcdGsiNormal			0 	/**< GSI is handled as normal. */
#define goLcdGsiOverride		1 	/**< GoLCD overrides the GSI with its own control. */


/**
 * GoLCD Color Mode.
 * One of the following:
 * - goLcdColorDefault
 * - goLcdColorOverride
 * - goLcdColorInverted
 */	
typedef UInt32 GoLcdColorModeType;

#define goLcdColorDefault		0	/**< Use the default color scheme. */
#define goLcdColorOverride		1	/**< Use the specified color (passed in as a separate argument). */
#define goLcdColorInverted		2  	/**< Use only with ink color. Inverts the color of the display area beneath the Graffiti 2 stroke. */


/********************************************************************
 * Traps
 ********************************************************************/
#define kGoLcdLibTrapOpen				sysLibTrapOpen
#define kGoLcdLibTrapClose				sysLibTrapClose

#define kGoLcdLibTrapGetStatus			(sysLibTrapCustom)
#define kGoLcdLibTrapSetStatus			(sysLibTrapCustom+1)
#define kGoLcdLibTrapGetInkState		(sysLibTrapCustom+2)
#define kGoLcdLibTrapSetInkState		(sysLibTrapCustom+3)
#define kGoLcdLibTrapGetTimeout			(sysLibTrapCustom+4)
#define kGoLcdLibTrapSetTimeout			(sysLibTrapCustom+5)
#define kGoLcdLibTrapGetBounds			(sysLibTrapCustom+6)
#define kGoLcdLibTrapSetBounds			(sysLibTrapCustom+7)
#define kGoLcdLibTrapGetGsiState		(sysLibTrapCustom+8)
#define kGoLcdLibTrapSetGsiState		(sysLibTrapCustom+9)

#define GoLcdLibAPIVersion				(sysMakeROMVersion(3, 6, 0, sysROMStageBeta, 0))

/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opens the GoLCD library.
 *
 * This function should be called prior to calling the other GoLCD functions.
 * 
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @return	The error code returned from the library. If this is errNone, the
 *			function was sucessful.
 *
 * @sample
 * @code
 * UInt16 gGoLcdLibRefNum = 0;
 * err = SysLibFind(goLcdLibName, &gGoLcdLibRefNum);
 * if( err == sysErrLibNotFound )
 * {
 *     err = SysLibLoad(goLcdLibType, goLcdLibCreator, &gGoLcdLibRefNum);
 *         if( !err ) {
 *             err = GoLcdLibOpen(gGoLcdLibRefNum);
 *         }
 * } @endcode
 *
 * @see GoLcdLibClose
 */
Err GoLcdLibOpen(UInt16 libRefNum)
				SYS_TRAP(kGoLcdLibTrapOpen);

/**
 * Closes the GoLCD library.
 *
 * This function should be called after your application has finished with the GoLCD
 * library.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @return	The error code returned from the library. If this is errNone, the
 *			function was sucessful.
 * 
 * @sample
 * @code
 * err = GoLcdLibClose(gGoLcdLibRefNum);
 * if( err == errNone )
 *     err = SysLibRemove(gGoLcdLibRefNum); @endcode
 *
 * @see GoLcdLibOpen
 */
Err GoLcdLibClose(UInt16 libRefNum)
				SYS_TRAP(kGoLcdLibTrapClose);

/**
 * Returns whether the GoLCD functionality is enabled, disabled, or not available on
 * this device.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @return	Returns the current GoLCD status.
 *
 * @see GoLcdStatusType
 * 
 * @sample
 * @code
 * GoLcdStatusType gCurrentGoLcdStatus = goLcdNotAvailable;
 * gCurrentGoLcdStatus = GoLcdGetStatus(gGoLcdLibRefNum);
 * switch(gCurrentGoLcdStatus) {
 *     case goLcdDisabled:
 *         // GoLcd is disabled
 *         break;
 *     case goLcdEnabled:
 *         // GoLcd is Enabled
 *         break;
 *     case goLcdNotAvailable:
 *     default:
 *         // Not available
 *         break;
 * } @endcode
 *
 * @see GoLcdSetStatus
 */
GoLcdStatusType GoLcdGetStatus(UInt16 libRefNum)
				SYS_TRAP(kGoLcdLibTrapGetStatus);

/**
 * Enables or disables the GoLCD functionality.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	status		Sets the new GoLCD status.
 * @return	Returns the previous GoLCD status.
 * 
 * @sample
 * @code
 * GoLcdStatusType gPreviousGoLcdStatus = goLcdNotAvailable;
 * gPreviousGoLcdStatus = GoLcdSetStatus(gGoLcdLibRefNum, goLcdEnabled); @endcode
 *
 * @see GoLcdGetStatus
 */
GoLcdStatusType GoLcdSetStatus(UInt16 libRefNum, GoLcdStatusType status)
				SYS_TRAP(kGoLcdLibTrapSetStatus);

/**
 * Returns the current state of GoLCD inking.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	stateP		Points to the returned GoLCD ink state.
 * @param	colorModeP	Points to the returned GoLCD color mode for ink.
 * @param	rgbP		Valid only if colorMode is set to goLcdColorOverride. Points to an RGB color
 * 						value corresponding to the current color used for inking. (The RGBColorType is
 * 						defined in the header file Bitmap.h.)
 *
 * @sample
 * @code
 * GoLcdInkStateType gCurrentGoLcdInkState = goLcdInkDisabled;
 * GoLcdColorModeType gCurrentGoLcdColorMode = goLcdColorDefault;
 * RGBColorType gCurrentColor;
 * GoLcdGetInkState(gGoLcdLibRefNum, &gCurrentGoLcdInkState, &gCurrentGoLcdColorMode, &gCurrentColor); @endcode
 *
 * @see GoLcdSetInkState
 */
void GoLcdGetInkState(UInt16 libRefNum, GoLcdInkStateType *stateP, GoLcdColorModeType *colorModeP, RGBColorType *rgbP)
				SYS_TRAP(kGoLcdLibTrapGetInkState);

/**
 * Sets the state of GoLCD inking.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	state		Sets the new GoLCD ink state.
 * @param	colorMode	Sets the new GoLCD color mode for ink.
 * @param	rgbP		Valid only if colorMode is set to goLcdColorOverride. Points to an RGB color
 * 						value corresponding to the current color used for inking. (The RGBColorType is
 * 						defined in the header file Bitmap.h.)
 *
 * @remarks	<em>Inking</em>, also known as <em>echoing</em>, refers to the drawing of Graffiti 2 strokes in the
 *			application area of the display.
 *
 * @sample
 * @code
 * RGBColorType gNewColor = { 0, 255, 0, 0}; // Red
 * GoLcdSetInkState(gGoLcdLibRefNum, goLcdInkEnabled, goLcdColorOverride, &gNewColor); @endcode
 *
 * @see GoLcdGetInkState
 */
void GoLcdSetInkState(UInt16 libRefNum, GoLcdInkStateType state, GoLcdColorModeType colorMode, RGBColorType *rgbP)
				SYS_TRAP(kGoLcdLibTrapSetInkState);

/**
 * Returns the GSI state associated with GoLCD.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	stateP		Points to the returned GoLCD GSI state.
 * @param	colorModeP	Points to the returned GoLCD color mode for GSI.
 * @param	rgbP		Valid only if colorMode is set to goLcdColorOverride. Points to an RGB color
 * 						value corresponding to the current color used for inking. (The RGBColorType is
 * 						defined in the header file Bitmap.h.)
 *
 * @remarks	In normal operation, the GSI is drawn in the lower-right portion of the screen when
 * 			the user enters the shift keystroke. The functionality of the GSI can be changed into
 * 			an enable and disable control for GoLCD. This function returns the current state of
 * 			the GSI.
 *
 * @sample
 * @code
 * GoLcdGsiStateType gCurrentGoLcdGsiState = goLcdGsiNormal;
 * GoLcdColorModeType gCurrentGoLcdColorMode = goLcdColorDefault;
 * RGBColorType gCurrentColor;
 * GoLcdGetGsiState(gGoLcdLibRefNum, &gCurrentGoLcdGsiState, &gCurrentGoLcdColorMode, &gCurrentColor); @endcode
 *
 * @see GoLcdSetGsiState
 */
void GoLcdGetGsiState(UInt16 libRefNum, GoLcdGsiStateType *stateP, GoLcdColorModeType *colorModeP, RGBColorType *rgbP)
				SYS_TRAP(kGoLcdLibTrapGetGsiState);

/**
 * Sets the GSI state associated with GoLCD.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	state		Sets the GoLCD GSI state.
 * @param	colorMode	Sets the GoLCD color mode for GSI.
 * @param	rgbP		Valid only if colorMode is set to goLcdColorOverride. Points to an RGB color
 * 						value corresponding to the current color used for inking. (The RGBColorType is
 * 						defined in the header file Bitmap.h.)
 *
 * @remarks In normal operation, the GSI is drawn in the lower-right portion of the screen when
 * 			the user enters the shift keystroke. The functionality of the GSI can be changed into
 * 			an enable and disable control for GoLCD. This function determines whether the
 * 			GSI is converted into a GoLCD control. This setting will apply to all GSIs in any
 * 			active form.
 *
 * @sample
 * @code
 * RGBColorType gNewColor = { 0, 255, 0, 0}; // Red
 * GoLcdSetGsiState(gGoLcdLibRefNum, goLcdGsiOverride, goLcdColorOverride, &gNewColor); @endcode
 *
 * @see GoLcdGetGsiState
 */
void GoLcdSetGsiState(UInt16 libRefNum, GoLcdGsiStateType state, GoLcdColorModeType colorMode, RGBColorType *rgbP)
				SYS_TRAP(kGoLcdLibTrapSetGsiState);

/**
 * Returns the length, in system ticks, of the time-out value of GoLCD.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	mode		Specifies the GoLCD mode.
 * @return	Returns the length (in system ticks) of the time-out value.
 *
 * @remarks GoLCD starts in goLcdPenTapMode. When a penDownEvent is received, a timer is
 * 			started. If a penUpEvent is received before the timer reaches the time-out value for
 * 			this mode, the pen events are passed on to the event handler for the application
 * 			control. Otherwise, if the pen events exceed the time-out value and the x, y
 * 			coordinates change significantly, GoLCD enters goLcdGraffitiMode and treats the
 * 			pen events as a Graffiti 2 stroke.
 * 			<br> The default time-out value for goLcdPenTapMode is 15 system ticks, or
 * 			150 milliseconds.
 * 			<br> When it enters goLcdGraffitiMode, GoLCD continuously interprets all pen events
 * 			as Graffiti 2 strokes. There is a time-out value associated with goLcdGraffitiMode,
 * 			however. If it does not receive a new pen event within the allotted time, it returns
 * 			to goLcdPenTapMode.
 * 			<br> The default time-out value for goLcdGraffitiMode is 150 system ticks, or
 * 			1500 milliseconds.
 * 			
 * @sample
 * @code
 * UInt32 gCurrentTimeout = GoLcdGetTimeout(gGoLcdLibRefNum, goLcdPenTapMode); @endcode
 *
 * @see GoLcdSetTimeout
 */
UInt32 GoLcdGetTimeout(UInt16 libRefNum, GoLcdModeType mode)
				SYS_TRAP(kGoLcdLibTrapGetTimeout);

/**
 * Sets the length, in system ticks, of the time-out value of GoLCD.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	mode		Specifies the GoLCD mode.
 * @param	ticks		The new time-out length in system ticks.
 *
 * @return	Returns the length (in system ticks) of the previous time-out value.
 *
 * @remarks GoLCD starts in goLcdPenTapMode. When a penDownEvent is received, a timer is
 * 			started. If a penUpEvent is received before the timer reaches the time-out value for
 * 			this mode, the pen events are passed on to the event handler for the application
 * 			control. Otherwise, if the pen events exceed the time-out value and the x, y
 * 			coordinates change significantly, GoLCD enters goLcdGraffitiMode and treats the
 * 			pen events as a Graffiti 2 stroke.
 * 			<br> The default time-out value for goLcdPenTapMode is 15 system ticks, or
 * 			150 milliseconds.
 * 			<br> When it enters goLcdGraffitiMode, GoLCD continuously interprets all pen events
 * 			as Graffiti 2 strokes. There is a time-out value associated with goLcdGraffitiMode,
 * 			however. If it does not receive a new pen event within the allotted time, it returns
 * 			to goLcdPenTapMode.
 * 			<br> The default time-out value for goLcdGraffitiMode is 150 system ticks, or
 * 			1500 milliseconds.
 *
 * @sample
 * @code
 * UInt32 gPreviousTimeout = GoLcdSetTimeout(gGoLcdLibRefNum, goLcdPenTapMode, 100); @endcode
 *
 * @see GoLcdGetTimeout
 */
UInt32 GoLcdSetTimeout(UInt16 libRefNum, GoLcdModeType mode, UInt32 ticks)
				SYS_TRAP(kGoLcdLibTrapSetTimeout);

/**
 * Returns the current bounds defined for use with GoLCD.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	rectP		Points to the returned rectangle.
 *
 * @remarks GoLCD ignores any succession of pen events (a series of penDownEvents followed
 * 			by a penUpEvent) if the initial penDownEvent occurs outside its bounds. By default,
 * 			its bounds are the entire display and users can start drawing full-screen writing
 * 			characters anywhere except for the right-most scroll bar and the menu bar area.
 *
 * @sample
 * @code
 * RectangleType gCurrentBounds;
 * GoLcdGetBounds(gGoLcdLibRefNum, &gCurrentBounds); @endcode
 *
 * @see GoLcdSetBounds
 */
void GoLcdGetBounds(UInt16 libRefNum, RectangleType *rectP)
				SYS_TRAP(kGoLcdLibTrapGetBounds);

/**
 * Sets the bounds defined for use with GoLCD.
 *
 * @param  	libRefNum	Reference number of the GoLCD library.
 * @param	rectP		Points to the rectangle value to set.
 *
 * @remarks GoLCD ignores any succession of pen events (a series of penDownEvents followed
 * 			by a penUpEvent) if the initial penDownEvent occurs outside its bounds. By default,
 * 			its bounds are the entire display and users can start drawing full-screen writing
 * 			characters anywhere except for the right-most scroll bar and the menu bar area.
 *
 * @sample
 * @code
 * RectangleType gNewBounds = { { 0, 0 } , { 50, 50 } };
 * GoLcdGetBounds(gGoLcdLibRefNum, &gNewBounds); @endcode
 *
 * @see GoLcdGetBounds
 */
void GoLcdSetBounds(UInt16 libRefNum, RectangleType *rectP)
				SYS_TRAP(kGoLcdLibTrapSetBounds);

#ifdef __cplusplus 
}
#endif

#endif 	//__PALMGOLCD_H__
