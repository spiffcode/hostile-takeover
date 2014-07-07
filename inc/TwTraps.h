/* Copyright (c) 2002-2003 Tapwave, Inc. All rights reserved. */

#ifndef __TWTRAPS_H__
#define __TWTRAPS_H__

#include <PalmOS.h>

#ifndef sysTrapOEMDispatch2
#define sysTrapOEMDispatch2             0xA443
#endif

/* this actually is the oem dispatch trap 2 */
#define sysTrapTwDispatch               sysTrapOEMDispatch2

#if CPU_TYPE == CPU_68K

#if defined(__GNUC__)
#if 0
	#define TAL_TRAP(selector)                              \
        __attribute__ ((__callseq__ (                       \
            "move.w #" _Str(selector) ",%%d2; "             \
            "trap   #15; "                                  \
            "dc.w   " _Str(sysTrapTwDispatch) ";")))
#else
	#define _TW_CALL_WITH_SELECTOR(table, vector, selector)\
        __attribute__ ((__callseq__ (\
                "move.w #" _Str(selector) ",%%d2; "\
                "trap #" _Str(table) "; dc.w " _Str(vector))))
	#define TAL_TRAP(selector) _TW_CALL_WITH_SELECTOR(15, sysTrapTwDispatch, selector)
#endif
#elif defined(__MWERKS__)
    #define TAL_TRAP(selector)                              \
        = { 0x343C, selector, 0x4E40 + 15, sysTrapTwDispatch }
#endif

#else

#define TAL_TRAP(selector)

#endif

#if 0
#define _TW_CALL_WITH_SELECTOR(table, vector, selector)\
        __attribute__ ((__callseq__ (\
                "move.w #" _Str(selector) ",%%d2; "\
                "trap #" _Str(table) "; dc.w " _Str(vector))))
#define TAL_TRAP(selector) _TW_CALL_WITH_SELECTOR(15, sysTrapTwDispatch, selector)
#endif

/* this gives the selector base for tapwave interface */
#define trapTwSelectorBase              256

/* selector numbers for 68K calls */
#define trapTwDeviceOpen                256
#define trapTwDeviceClose               257
#define trapTwDeviceRead                258
#define trapTwDeviceWrite               259
#define trapTwDeviceGetProperty         260
#define trapTwDeviceSetProperty         261
#define trapTwDeviceControl             262

#define trapTwDisplayGetState           263
#define trapTwDisplaySetState           264

#define trapTwHighScoreRegister         265
#define trapTwHighScoreUnregister       266
#define trapTwHighScoreReport           267
#define trapTwHighScoreGetSummary       268
#define trapTwHighScoreGetDetails       269

#define trapTwInputOpen                 270
#define trapTwInputClose                271
#define trapTwInputActivate             272
#define trapTwInputDeactivate           273
#define trapTwInputGetPeriod            274
#define trapTwInputSetPeriod            275
#define trapTwInputGetCapacity          276
#define trapTwInputSetCapacity          277
#define trapTwInputGetFormat            278
#define trapTwInputSetFormat            279
#define trapTwInputPeek                 280
#define trapTwInputRead                 281
#define trapTwInputPoll                 282
#define trapTwInputControl              283

#define trapTwNavResetCalibration       284
#define trapTwNavCalibrate              285

#define trapTwSndPlaySystemSound        286
#define trapTwSndGetVolume              287
#define trapTwSndSetMute                288
#define trapTwSndSetVolume              289
#define trapTwSndSetBassBoost           290
#define trapTwSndGetMute                291
#define trapTwSndGetBassBoost           292

#define trapTwCtlSetFrameStyle          293
#define trapWinGetBitmapDimensions      294
#define trapTwOSReserved1               295
#define trapTwOSReserved2               296
#define trapTwOSReserved3               297
#define trapTwOSReserved4               298
#define trapTwOSReserved5               299

#define trapTwDrawTitleBar              300
#define trapTwSetTapwaveScrollBar       301
#define trapTwGetSlotNumberForVolume    302

#define trapTwGfxOpen                   303
#define trapTwGfxClose                  304
#define trapTwGfxGetInfo                305
#define trapTwGfxGetMemoryUsage         306
#define trapTwGfxGetDisplaySurface      307
#define trapTwGfxGetPalmDisplaySurface  308
#define trapTwGfxInVBlank               309
#define trapTwGfxWaitForVBlank          310
#define trapTwGfxAllocSurface           311
#define trapTwGfxFreeSurface            312
#define trapTwGfxSetClip                313
#define trapTwGfxGetClip                314
#define trapTwGfxGetSurfaceInfo         315
#define trapTwGfxLockSurface            316
#define trapTwGfxUnlockSurface          317
#define trapTwGfxReadSurface            318
#define trapTwGfxWriteSurface           319
#define trapTwGfxIsSurfaceReady         320
#define trapTwGfxBitBlt                 321
#define trapTwGfxStretchBlt             322
#define trapTwGfxTransformBlt           323
#define trapTwGfxDrawPoints             324
#define trapTwGfxDrawColorPoints        325
#define trapTwGfxDrawLines              326
#define trapTwGfxDrawLineSegments       327
#define trapTwGfxDrawRect               328
#define trapTwGfxFillRect               329
#define trapTwGfxDrawSpans              330
#define trapTwGfxDrawBitmap             331
#define trapTwGfxReadSurfaceRegion      332
#define trapTwGfxWriteSurfaceRegion     333
#define trapTwGfxBlendBlt               334
#define trapTwGfxTileBlt                335
#define trapTwGfxMaskBlt                336
#define trapTwGfxAsyncBlt               337
#define trapTwGfxMaskBlendBlt           338
#define trapTwGfxTransparentBlt         339
#define trapTwGfxReserved3              340
#define trapTwGfxReserved4              341
#define trapTwGfxReserved5              342
#define trapTwGfxReserved6              343
#define trapTwGfxReserved7              344
#define trapTwGfxReserved8              345
#define trapTwGfxReserved9              346
#define trapTwGfxReserved10             347
#define trapTwGfxReserved11             348
#define trapTwGfxReserved12             349
#define trapTwGfxReserved13             350
#define trapTwGfxReserved14             351
#define trapTwGfxReserved15             352
#define trapTwGfxReserved16             353
#define trapTwGfxReserved17             354
#define trapTwGfxReserved18             355
#define trapTwGfxReserved19             356
#define trapTwGfxDrawPalmBitmap         357

#define trapTwSecGetFunctions			358
#define trapTwSecReserved1				359
#define trapTwSecReserved2				360
#define trapTwSecReserved3				361
#define trapTwSecReserved4				362
#define trapTwSecReserved5				363
#define trapTwSecReserved6				364
#define trapTwSecReserved7				365
#define trapTwSecReserved8				366
#define trapTwSecReserved9				367
#define trapTwSecReserved10				368
#define trapTwSecReserved11				369
#define trapTwSecReserved12				370
#define trapTwSecReserved13				371
#define trapTwSecReserved14				372
#define trapTwSecReserved15				373
#define trapTwSecReserved16				374

#define trapTwCreateDatabaseFromImage   375
#define trapTwGetGraphicForButton       376
#define trapTwHighScoreGetTournament    377
#define trapTwBlendMask                 378
#define trapTwGetPRCDataDirectory       379
#define trapTwGetDBDataDirectory        380
#define trapTwGetSlotRefNumForSlot      381
#define trapTwGetMicroSeconds           382
#define trapTwPickColor                 383

// ADD NEW TRAPS ABOVE THIS LINE AND THEN RENUMBER THE ONES BELOW

#define trapTwFutureReserved0           384
#define trapTwFutureReserved1           (trapTwFutureReserved0 + 1)
#define trapTwFutureReserved2           (trapTwFutureReserved0 + 2)
#define trapTwFutureReserved3           (trapTwFutureReserved0 + 3)
#define trapTwFutureReserved4           (trapTwFutureReserved0 + 4)
#define trapTwFutureReserved5           (trapTwFutureReserved0 + 5)
#define trapTwFutureReserved6           (trapTwFutureReserved0 + 6)
#define trapTwFutureReserved7           (trapTwFutureReserved0 + 7)
#define trapTwFutureReserved8           (trapTwFutureReserved0 + 8)
#define trapTwFutureReserved9           (trapTwFutureReserved0 + 9)

#endif /* __TWTRAPS_H__ */
