/* -*- mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * Copyright (c) 2003 Tapwave, Inc. All rights reserved.
 *
 * All rights reserved. This is private and not for public distribution.
 */
#ifndef __TWGFX_H__
#define __TWGFX_H__

#include "TwError.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Constants
 */

/*
 * TwGfx specific error codes
 */
#define twGfxErrorLibraryOpen           (0 + twGfxErrorBase)
#define twGfxErrorNullPointer           (1 + twGfxErrorBase)
#define twGfxErrorBadObjectVersion      (2 + twGfxErrorBase)
#define twGfxErrorSurfaceAllocFailed    (3 + twGfxErrorBase)
#define twGfxErrorLibraryClosed         (4 + twGfxErrorBase)

#define twGfxErrorInvalidAlignment      (9 + twGfxErrorBase)
#define twGfxErrorInvalidHandle         (10 + twGfxErrorBase)
#define twGfxErrorInvalidPixelFormat    (11 + twGfxErrorBase)
#define twGfxErrorInvalidLocation       (12 + twGfxErrorBase)
#define twGfxErrorInvalidSize           (13 + twGfxErrorBase)
#define twGfxErrorInvalidRotation       (14 + twGfxErrorBase)
#define twGfxErrorInvalidMirror         (15 + twGfxErrorBase)
#define twGfxErrorInvalidCount          (16 + twGfxErrorBase)
#define twGfxErrorInvalidSurface        (17 + twGfxErrorBase)
#define twGfxErrorInvalidCoord          (18 + twGfxErrorBase)
#define twGfxErrorInvalidFlags          (19 + twGfxErrorBase)

#define twGfxErrorSurfaceNotLocked      (20 + twGfxErrorBase)
#define twGfxErrorOperationInProgress   (21 + twGfxErrorBase)

/* Memory location for surfaces */
#define twGfxLocationAcceleratorMemory  1

/* Pixel formats. Note that most api's support a subset of the formats. */
#define twGfxPixelFormatMonochrome      0                       /* 1 bit per pixel (colormap) */
#define twGfxPixelFormat1bpp            0                       /* 1 bit per pixel (colormap) */
#define twGfxPixelFormatRGB565          1                       /* little endian */
#define twGfxPixelFormatRGB565_LE       1                       /* little endian */
#define twGfxPixelFormatRGB565_BE       2                       /* big endian */
#define twGfxPixelFormat2bpp            3                       /* 2 bits per pixel (colormap) */
#define twGfxPixelFormat4bpp            4                       /* 4 bits per pixel (colormap) */
#define twGfxPixelFormat8bpp            5                       /* 8 bits per pixel (colormap) */

/* Rotation for TwGfxTransformBlt */
#define twGfxRotateNone                 0
#define twGfxRotateCW90                 1                       /* clockwise 90 degrees */
#define twGfxRotateCW180                2                       /* clockwise 180 degrees */
#define twGfxRotateCW270                3                       /* clockwise 270 degrees */
#define twGfxRotateCCW90                twGfxRotateCW270        /* counter-clockwise 90 degrees */
#define twGfxRotateCCW180               twGfxRotateCW180        /* counter-clockwise 180 degrees */
#define twGfxRotateCCW270               twGfxRotateCW90         /* counter-clockwise 270 degrees */

/* Mirror for TwGfxTransformBlt */
#define twGfxMirrorNone                 0
#define twGfxMirrorHorizontal           1
#define twGfxMirrorVertical             2
#define twGfxMirrorBoth                 3

/***********************************************************************/

/*
 * Macro to construct a frame buffer compatible rgb565 color given
 * three 8 bit components. This is a manually optimized version that
 * is consistent with the other color macros here.
 *
 * Note that the 68k version of this macro generates data that is
 * byte-swapped into little-endian format. This means that data passed
 * to TwGfxDrawBitmap and TwGfxWriteSurface will be properly arranged
 * if you use this macro.
 */
#define TwGfxMakeDisplayRGB_BigEndian(_r,_g,_b) \
  ( (((_g) & 0xFC) << 11) | (((_b) & 0xF8) << 5) | ((_r) & 0xF8) | (((_g) & 0xFF) >> 5) )

#define TwGfxMakeDisplayRGB_LittleEndian(_r,_g,_b) \
  ( (((_r) & 0xF8) << 8) | (((_g) & 0xFC) << 3) | (((_b) & 0xF8) >> 3) )

#if CPU_TYPE == CPU_68K
#define TwGfxMakeDisplayRGB(_r,_g,_b) TwGfxMakeDisplayRGB_BigEndian(_r,_g,_b)
#else
#define TwGfxMakeDisplayRGB(_r,_g,_b) TwGfxMakeDisplayRGB_LittleEndian(_r,_g,_b)
#endif

/*
 * This macro converts a RGBColorType structure to a frame buffer
 * compatible display value.
 */
#define TwGfxRGBToDisplayRGB(_rgb) TwGfxMakeDisplayRGB((_rgb).r, (_rgb).g, (_rgb).b)

/*
 * These macros take an 8 bit color component and adjust them to be
 * sized to match the rgb565 display framebuffer
 */
#define TwGfxRComponentToDisplayComponent(_r) (((_r) & 0xF8) >> 3)
#define TwGfxGComponentToDisplayComponent(_g) (((_g) & 0xFC) >> 2)
#define TwGfxBComponentToDisplayComponent(_b) (((_b) & 0xF8) >> 3)

/*
 * Format information for the rgb565 display buffer
 */
#define twGfxRShift     11
#define twGfxRMask      0xF800
#define twGfxGShift     5
#define twGfxGMask      0x07E0
#define twGfxBShift     0
#define twGfxBMask      0x001F

/*
 * This macro converts from packed component RGB to packed display RGB
 */
#define TwGfxPackedRGBToDisplayRGB(_rgb)                               \
  ( (TwGfxRComponentToDisplayComponent((_rgb) >> 16) << twGfxRShift) | \
    (TwGfxGComponentToDisplayComponent((_rgb) >> 8) << twGfxGShift) |  \
    (TwGfxBComponentToDisplayComponent((_rgb)) << twGfxBShift) )

/*
 * This macro converts a RGBColorType structure to a
 * TwGfxPackedRGBType value.
 */
#define TwGfxRGBToPackedRGB(_rgb)                      \
  ((TwGfxPackedRGBType) ( (((UInt32)(_rgb).r) << 16) | \
                          ((UInt32)((_rgb).g) << 8) |  \
                          ((UInt32)(_rgb).b) ) )

/*
 * This macro converts rgb components to a TwGfxPackedRGBType
 */
#define TwGfxComponentsToPackedRGB(_r,_g,_b)                \
  ((TwGfxPackedRGBType) ( ((((UInt32)(_r)) & 0xFF) << 16) | \
                          ((((UInt32)(_g)) & 0xFF) << 8) |  \
                          (((UInt32)(_b)) & 0xFF) ) )

/*
 * This macro helps fill in a TwGfxPointType
 */
#define TwGfxMakePoint(_point, _x, _y) \
  ((_point).x = (_x), (_point).y = (_y))

/*
 * This macro helps fill in a TwGfxRectType
 */
#define TwGfxMakeRect(_rect, _x, _y, _w, _h) \
  ((_rect).x = (_x), (_rect).y = (_y), (_rect).w = (_w), (_rect).h = (_h))

/***********************************************************************/

/*
 * Opaque structures
 */
typedef struct TwGfxTypeTag TwGfxType;
typedef struct TwGfxSurfaceTypeTag TwGfxSurfaceType;

/*
 * Structures
 */


/*
 * Color information. This is the "abstract" RGB color value. This is
 * not the same as the display pixel format. This type is used for the
 * API entry points. Use the TwGfxComponentsToPackedRGB to construct
 * instances of this type.
 */
typedef UInt32 TwGfxPackedRGBType;

/*
 * TwGfx library state information structure
 */
typedef struct TwGfxInfoTypeTag {
  Int32 size;                                   /* caller MUST set this to sizeof(TwGfxInfoType) */

  Int32 displayWidth, displayHeight;            /* current dimensions of display */
  Int32 displayRowBytes;                        /* byte width of entire row */
  Int32 displayPixelFormat;                     /* format of display */

  Int32 freeAcceleratorMemory;                  /* free accelerator memory */
  Int32 totalAcceleratorMemory;                 /* total accelerator memory */
} TwGfxInfoType;

/*
 * Surface information structure
 */
typedef struct TwGfxSurfaceInfoTypeTag {
  Int32 size;                                   /* caller MUST set this to sizeof(TwGfxSurfaceInfoType) */

  Int32 width, height;                          /* dimensions */
  Int32 rowBytes;                               /* byte width of entire row */

  Int32 location;                               /* which memory the surface is located */
  Int32 pixelFormat;                            /* format of the surface */
} TwGfxSurfaceInfoType;

/*
 * Bitmap structure
 */
typedef struct TwGfxBitmapTypeTag {
  Int32 size;                                   /* caller MUST set this to sizeof(TwGfxBitmapType) */

  Int32 width, height;                          /* size of bitmap */
  Int32 rowBytes;                               /* bytes per row */
  Int32 pixelFormat;                            /* format of data */
  void* data;                                   /* actual data */

  UInt16* palette;                              /* In native display format: twGfxPixelFormatRGB565_LE */
} TwGfxBitmapType;

typedef struct TwGfxPointTypeTag {
  Int32 x, y;
} TwGfxPointType;

typedef struct TwGfxRectTypeTag {
  Int32 x, y, w, h;
} TwGfxRectType;

typedef struct TwGfxSpanTypeTag {
  Int32 x, y, w;
} TwGfxSpanType;

/***********************************************************************/


/*
 * Methods
 */

/*
 * Open access to the graphics library. This must be called at least
 * once. aResult is filled in with an opaque handle to the
 * library. aInfoResult is filled in (if not NULL) with information
 * describing the current state of the display.
 *
 * Note that the display state will be affected by the
 * TwSetOrientationState call as well as the presence/absence/size of
 * the pen input area and the status bar.
 *
 * Errors:
 *      twGfxErrorLibraryOpen
 *      twGfxErrorBadObjectVersion
 *
 * See also:
 *      TwSetOrientationState
 *      PINSetInputAreaState
 *      StatHide
 *      StatShow
 */
Err TwGfxOpen(TwGfxType** aResult,
              TwGfxInfoType* aInfoResult)
  TAL_TRAP(trapTwGfxOpen);

/*
 * Release an instance of the TwGfx library. This call should be
 * called to free all memory resources associated with the instance.
 * This call will also call TwFreeSurface for all allocated surfaces
 * that have not yet been freed that were allocated using this
 * instance of the library.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 */
Err TwGfxClose(TwGfxType* aGfx)
  TAL_TRAP(trapTwGfxClose);

/*
 * Query the current state of the TwGfx library. This can be used, for
 * example, to determine how much free memory is available as well as
 * determine the display state.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorBadObjectVersion
 */
Err TwGfxGetInfo(TwGfxType* aGfx,
                 TwGfxInfoType* aResult)
  TAL_TRAP(trapTwGfxGetInfo);

/*
 * Query the current usage of a given memory location and return how
 * much of the total memory is allocated. For example, to determine
 * how much memory is available on the graphics accelerator set
 * aLocation to twGfxLocationAcceleratorMemory.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 */
Err TwGfxGetMemoryUsage(TwGfxType* aGfx,
                        Int32 aLocation,
                        Int32* aUsedResult)
  TAL_TRAP(trapTwGfxGetMemoryUsage);

/*
 * Return the surface used by the display
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 */
Err TwGfxGetDisplaySurface(TwGfxType* aGfx,
                           TwGfxSurfaceType** aResult)
  TAL_TRAP(trapTwGfxGetDisplaySurface);

/*
 * Return the surface used by PalmOS. This will be a surface that
 * represents the subset of the display used by PalmOS.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 */
Err TwGfxGetPalmDisplaySurface(TwGfxType* aGfx,
                               TwGfxSurfaceType** aResult)
  TAL_TRAP(trapTwGfxGetPalmDisplaySurface);

/*
 * See if the display is currently in the vertical blanking
 * period. *aInVBlankResult will be set to true if it is, false
 * otherwise.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 */
Err TwGfxInVBlank(TwGfxType* aGfx,
                  Boolean* aInVBlankResult)
  TAL_TRAP(trapTwGfxInVBlank);

/*
 * Wait until the display is is in the vertical blanking period.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 */
Err TwGfxWaitForVBlank(TwGfxType* aGfx)
  TAL_TRAP(trapTwGfxWaitForVBlank);

/******************************************/

/*
 * Attempt to allocate a surface using the parameters defined in
 * aDescription.
 *
 * The aDescription.size field should be set to sizeof(TwSurfaceInfo).
 *
 * aDescription.width and aDescription.height should be to the desired dimensions.
 *
 * aDescription.pixelFormat should be set to the desired pixel
 * format. The following pixel formats are supported:
 *
 *      twGfxPixelFormatRGB565_LE
 *
 * aDescription.location should be set to the choice of memory to allocate from.
 *
 * All other fields of aDescription are ignored. Upon success
 * aDescription will be filled in with exact information describing
 * the surface.
 *
 * Notes:
 *
 * Only surfaces that are allocated in the same format as the display
 * will work. There are no pixel format conversions done by the blt
 * operations.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorOutOfMemory
 *      twGfxErrorInvalidPixelFormat
 *      twGfxErrorInvalidLocation
 *      twGfxErrorInvalidSize
 *      twGfxErrorBadObjectVersion
 *      twGfxErrorSurfaceAllocFailed
 */
Err TwGfxAllocSurface(TwGfxType* aGfx,
                      TwGfxSurfaceType** aResult,
                      TwGfxSurfaceInfoType* aDescription)
  TAL_TRAP(trapTwGfxAllocSurface);

/*
 * Release the surface memory. If an operation is pending for the
 * surface, this call will wait until the surface is idle before
 * releasing it.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorInvalidSurface
 */
Err TwGfxFreeSurface(TwGfxSurfaceType* aSurface)
  TAL_TRAP(trapTwGfxFreeSurface);

/*
 * Set the clipping rectangle used when rendering with the given
 * surface. If you pass in a NULL pointer for aClipRect then clipping
 * is disabled for the given surface.
 *
 * The clipping rectangle for a surface applies to the following
 * drawing operations:
 *
 *      TwGfxDrawPoints
 *      TwGfxDrawColorPoints
 *      TwGfxDrawLines
 *      TwGfxDrawLineSegments
 *      TwGfxDrawRect
 *      TwGfxFillRect
 *      TwGfxDrawSpans
 *
 * The clipping rectangle for a surface does NOT apply to the following
 * drawing operations:
 *
 *      TwGfxWriteSurface
 *      TwGfxWriteSurfaceRegion
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 */
Err TwGfxSetClip(TwGfxSurfaceType* aSurface,
                 const TwGfxRectType* aClipRect)
  TAL_TRAP(trapTwGfxSetClip);

/*
 * Get the current clipping rectangle for the surface. When clipping
 * is disabled for the surface the fields in aClipRectResult will be
 * zero.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 */
Err TwGfxGetClip(TwGfxSurfaceType* aSurface,
                 TwGfxRectType* aClipRectResult)
  TAL_TRAP(trapTwGfxGetClip);

/*
 * Query the surface information.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorBadObjectVersion
 */
Err TwGfxGetSurfaceInfo(TwGfxSurfaceType* aSurface,
                        TwGfxSurfaceInfoType* aResult)
  TAL_TRAP(trapTwGfxGetSurfaceInfo);

/*
 * Lock the surface down, returning a pointer to the surface bitmap
 * data. The caller can then manipulate the data directly. Direct
 * manipulation of the surface data on the internal or external memory
 * will be slow compared to using DMA, but doesn't suffer any of the
 * latency issues that DMA has.
 *
 * The lock call can be nested; there should be a matching unlock for
 * every lock, however. The API only guarantees that the modifications
 * to the surface are valid when the final unlock is peformed.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 */
Err TwGfxLockSurface(TwGfxSurfaceType* aSurface, void** aAddressResult)
  TAL_TRAP(trapTwGfxLockSurface);

/*
 * Release the lock on the surface. When the lock counter goes to zero
 * the surface will be guaranteed to be updated to either the internal
 * or external memory if aUpdateFlags is true.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorSurfaceNotLocked
 */
Err TwGfxUnlockSurface(TwGfxSurfaceType* aSurface,
                       Boolean aUpdate)
  TAL_TRAP(trapTwGfxUnlockSurface);

/*
 * Make a copy of a surface. The surface bitmap data is copied to
 * aDestPixels which must be aligned on a 16 bit boundary. It is the
 * callers responsibility to provide enough valid memory for the
 * copy. If aAsync is true then the copy is made asynchronously. You
 * can invoke TwGfxIsSurfaceReady to see when the operation has
 * completed. It is an error to attempt a second copy operation on a
 * surface while one is pending.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorOperationInProgress
 *      twGfxErrorInvalidAlignment
 */
Err TwGfxReadSurface(TwGfxSurfaceType* aSurface,
                     void* aDestPixels,
                     Boolean aAsync)
  TAL_TRAP(trapTwGfxReadSurface);

/*
 * Make a copy of a subset of a surface. The surface bitmap data is
 * copied to aDestPixels which must be aligned on a 16 bit
 * boundary. It is the callers responsibility to provide enough valid
 * memory for the copy. If aAsync is true then the copy is made
 * asynchronously. You can invoke TwGfxIsSurfaceReady to see when the
 * operation has completed. It is an error to attempt a second copy
 * operation on a surface while one is pending.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorOperationInProgress
 *      twGfxErrorInvalidAlignment
 */
Err TwGfxReadSurfaceRegion(TwGfxSurfaceType* aSurface,
                           const TwGfxRectType* aBounds,
                           void* aDestPixels,
                           UInt32 aDestRowBytes,
                           Boolean aAsync)
  TAL_TRAP(trapTwGfxReadSurfaceRegion);

/*
 * Copy memory to a surface. The memory is copied to the surface
 * bitmap.
 *
 * Source data is arranged linearly (left to right, top to bottom)
 * with the rowbytes being aDestRect->w * sizeof(pixel).
 *
 * If aAsync is true then the copy is made asynchronously. You can
 * invoke TwGfxIsSurfaceReady to see when the operation has
 * completed. It is an error to attempt a second copy operation on a
 * surface while one is pending.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorOperationInProgress
 */
Err TwGfxWriteSurface(TwGfxSurfaceType* aSurface,
                      const void* aSourcePixels,
                      Boolean aAsync)
  TAL_TRAP(trapTwGfxWriteSurface);

/*
 * Copy memory to a subset of a surface. The memory is copied to the
 * surface bitmap at aDestPoint.
 *
 * Source data is arranged linearly (left to right, top to bottom)
 * with the rowbytes being specified by aSourceRowBytes. There will be
 * a performance impact if aSourceRowBytes is not the same as
 * aDestRect->w * sizeof(pixel).
 *
 * If aAsync is true then the copy is made asynchronously. You can
 * invoke TwGfxIsSurfaceReady to see when the operation has
 * completed. It is an error to attempt a second copy operation on a
 * surface while one is pending.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorOperationInProgress
 */
Err TwGfxWriteSurfaceRegion(TwGfxSurfaceType* aSurface,
                            const TwGfxRectType* aBounds,
                            const void* aSourcePixels,
                            UInt32 aSourceRowBytes,
                            Boolean aAsync)
  TAL_TRAP(trapTwGfxWriteSurfaceRegion);

/*
 * See if a surface is ready for an asynchronous operation. If an
 * operation is in progress with the surface then
 * twGfxErrorOperationInProgress is returned. If no operation is in
 * progress and no other error occurs then zero is returned.
 *
 * Surfaces are busy when:
 *
 *      An async TwGfxWriteSurface/TwGfxWriteSurfaceRegion is in progress.
 *      An async TwGfxReadSurface/TwGfxReadSurfaceRegion is in progress.
 *      An async blt is in progress (TwGfxAsyncBlt).
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorOperationInProgress
 */
Err TwGfxIsSurfaceReady(TwGfxSurfaceType* aSurface)
  TAL_TRAP(trapTwGfxIsSurfaceReady);

/******************************************/

/*
 * Basic bitblt routine.
 *
 * The source surface will be copied to the destination surface at the
 * destination point using the source rectangle.
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorOperationInProgress
 */
Err TwGfxBitBlt(TwGfxSurfaceType* aDestSurface,
                const TwGfxPointType* aDestPoint,
                TwGfxSurfaceType* aSourceSurface,
                const TwGfxRectType* aSourceRect)
  TAL_TRAP(trapTwGfxBitBlt);

/*
 * Asynchronous bitblt routine. Same idea as TwGfxBitBlt except that
 * the transfer is queued and occurs during the next vertical blanking
 * period that the graphics accelerator is idle. If the graphics
 * accelerator is kept constantly busy then when the bitblt occurs is
 * undefined.
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * It is safe to queue the same surface for an async blt multiple
 * times before the next vblank. Once the vblank occurs, the async blt
 * is done and the surface is no longer busy.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorOperationInProgress
 */
Err TwGfxAsyncBlt(TwGfxSurfaceType* aDestSurface,
                  const TwGfxPointType* aDestPoint,
                  TwGfxSurfaceType* aSourceSurface,
                  const TwGfxRectType* aSourceRect)
  TAL_TRAP(trapTwGfxAsyncBlt);

/*
 * Transparent bitblt routine.
 *
 * The source surface will be copied to the destination surface at the
 * destination point from the source surface using the source
 * rectangle.
 *
 * Pixels in the source surface whose value matches aTransparentColor
 * will not be copied to the destination surface.
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorOperationInProgress
 */
Err TwGfxTransparentBlt(TwGfxSurfaceType* aDestSurface,
                        const TwGfxPointType* aDestPoint,
                        TwGfxSurfaceType* aSourceSurface,
                        const TwGfxRectType* aSourceRect,
                        TwGfxPackedRGBType aTransparentColor)
  TAL_TRAP(trapTwGfxTransparentBlt);

/*
 * Masking bitblt routine.
 *
 * The source surface will be copied to the destination surface at the
 * destination point from the source surface using the source
 * rectangle. The source pixels will be masked using the mask bitmap;
 * one bits in the mask will allow the source surface pixel to draw
 * through to the destination surface while zero bits in the mask will
 * have the source surface pixel ignored.
 *
 * The only acceptable pixel format for the mask bitmap is
 * twGfxPixelFormatMonochrome.
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * The width and height of the source mask must equal the width and
 * height of the aSourceRect otherwise a twGfxErrorInvalidSize error
 * will be returned.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorInvalidSize
 *      twGfxErrorInvalidPixelFormat
 *      twGfxErrorOperationInProgress
 */
Err TwGfxMaskBlt(TwGfxSurfaceType* aDestSurface,
                 const TwGfxPointType* aDestPoint,
                 TwGfxSurfaceType* aSourceSurface,
                 const TwGfxRectType* aSourceRect,
                 const TwGfxBitmapType* aMask)
  TAL_TRAP(trapTwGfxMaskBlt);

/*
 * Blending bitblt routine.
 *
 * The source surface will be copied to the destination surface at the
 * destination point from the source surface using the source
 * rectangle. The source pixels will be blended with the destination
 * pixels using the aSourceAlpha value.
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorOperationInProgress
 */
Err TwGfxBlendBlt(TwGfxSurfaceType* aDestSurface,
                  const TwGfxPointType* aDestPoint,
                  TwGfxSurfaceType* aSourceSurface,
                  const TwGfxRectType* aSourceRect,
                  TwGfxPackedRGBType aSourceAlpha)
  TAL_TRAP(trapTwGfxBlendBlt);

/*
 * Masking blending bitblt routine.
 *
 * The source surface will be copied to the destination surface at the
 * destination point from the source surface using the source
 * rectangle. The source pixels will be blended with the destination
 * pixels using the provided alpha mask bitmap.
 *
 * Alpha values are provided using an index bitmap (for example, 4
 * bits per pixel). A zero value in the index data means no source
 * color is used, while the largest index value indicates all of the
 * color is used. At this time the following index pixel formats are
 * supported:
 *
 *      twGfxPixelFormat4bpp
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * The width and height of the alpha mask must equal the width and
 * height of the aSourceRect otherwise a twGfxErrorInvalidSize error
 * will be returned.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorInvalidSize
 *      twGfxErrorInvalidPixelFormat
 *      twGfxErrorOperationInProgress
 */
Err TwGfxMaskBlendBlt(TwGfxSurfaceType* aDestSurface,
                      const TwGfxPointType* aDestPoint,
                      TwGfxSurfaceType* aSourceSurface,
                      const TwGfxRectType* aSourceRect,
                      const TwGfxBitmapType* aAlphaMask)
  TAL_TRAP(trapTwGfxMaskBlendBlt);

/*
 * Stretching bitblt routine.
 *
 * The source surface will be copied to the destination surface at the
 * destination rectangle from the source surface using the source
 * rectangle.
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorOperationInProgress
 */
Err TwGfxStretchBlt(TwGfxSurfaceType* aDestSurface,
                    const TwGfxRectType* aDestRect,
                    TwGfxSurfaceType* aSourceSurface,
                    const TwGfxRectType* aSourceRect)
  TAL_TRAP(trapTwGfxStretchBlt);

/*
 * Tiling bitblt routine.
 *
 * The source surface will be tiled to the destination surface in the
 * destination rectangle area, using the source alignment point to
 * define a relative origin for the tiling.
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorOperationInProgress
 */
Err TwGfxTileBlt(TwGfxSurfaceType* aDestSurface,
                 const TwGfxRectType* aDestRect,
                 TwGfxSurfaceType* aSourceSurface,
                 const TwGfxPointType* aSourceAlignmentPoint)
  TAL_TRAP(trapTwGfxTileBlt);

/*
 * Transformation bitblt routine. Can rotate (none,90,180,27) and/or
 * mirror (none, horz, vert, both) the source surface during the
 * transfer to the destination surface.
 *
 * The source surface will be copied to the destination surface at the
 * destination point from the source surface using the source
 * rectangle.
 *
 * If the coordinate values address pixels outside the source surface
 * a twGfxErrorInvalidCoord error will be returned. It is ok to access
 * pixels outside the destination surface: clipping will ensure that
 * nothing improper happens.
 *
 * When mixing rotation and mirroring, the mirroring is applied first
 * then the rotation.
 *
 * Assuming the following example source bitmap:
 *
 *      123
 *      456
 *
 * Here are all the combinations of rotations and mirrorings:
 *
 * +----------+--------+--------+--------+--------+
 * + Rot/Mir  + None   + Horz   + Vert   + Both   +
 * +----------+--------+--------+--------+--------+
 * +          + 123    + 321    + 456    + 654    +
 * + None     + 456    + 654    + 123    + 321    +
 * +          +        +        +        +        +
 * +----------+--------+--------+--------+--------+
 * +          + 41     + 63     + 14     + 36     +
 * + CW90     + 52     + 52     + 25     + 25     +
 * +          + 63     + 41     + 36     + 14     +
 * +----------+--------+--------+--------+--------+
 * +          + 654    + 456    + 321    + 123    +
 * + CW180    + 321    + 123    + 654    + 456    +
 * +          +        +        +        +        +
 * +----------+--------+--------+--------+--------+
 * +          + 36     + 14     + 63     + 41     +
 * + CW270    + 25     + 25     + 52     + 52     +
 * +          + 14     + 36     + 41     + 63     +
 * +----------+--------+--------+--------+--------+
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCoord
 *      twGfxErrorInvalidRotation
 *      twGfxErrorInvalidMirror
 *      twGfxErrorOperationInProgress
 */
Err TwGfxTransformBlt(TwGfxSurfaceType* aDestSurface,
                      const TwGfxPointType* aDestPoint,
                      TwGfxSurfaceType* aSourceSurface,
                      const TwGfxRectType* aSourceRect,
                      Int32 aRotationFlags,
                      Int32 aMirrorFlags)
  TAL_TRAP(trapTwGfxTransformBlt);

/*
 * Draw a set of points with a single color into the surface. This is
 * equivalent to locking the surface and plotting the points, but more
 * efficiently. Invalid coordinates are clipped.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCount
 *      twGfxErrorOperationInProgress
 */
Err TwGfxDrawPoints(TwGfxSurfaceType* aDestSurface,
                    const TwGfxPointType* aPoints,
                    Int32 aNumberOfPoints,
                    TwGfxPackedRGBType aColor)
  TAL_TRAP(trapTwGfxDrawPoints);

/*
 * Same idea as TwGfxDrawPoints except that each point has a distinct
 * color. Invalid coordinates are clipped.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCount
 *      twGfxErrorOperationInProgress
 */
Err TwGfxDrawColorPoints(TwGfxSurfaceType* aDestSurface,
                         const TwGfxPointType* aPoints,
                         Int32 aNumberOfPoints,
                         const TwGfxPackedRGBType* aColors)
  TAL_TRAP(trapTwGfxDrawColorPoints);

/* TODO: describe which pixels are rendered */
/* TODO: the chip can draw stippled lines; does anybody except PalmOS care? */

/*
 * Draw one or more connected lines using the given color. There must
 * be 2 or more vertices provided. Lines are clipped to the
 * destination surface.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCount
 *      twGfxErrorOperationInProgress
 */
Err TwGfxDrawLines(TwGfxSurfaceType* aDestSurface,
                   const TwGfxPointType* aPoints,
                   Int32 aNumberOfPoints,
                   TwGfxPackedRGBType aColor)
  TAL_TRAP(trapTwGfxDrawLines);

/*
 * Draw one or more line segments. There must be 2 or more vertices
 * provided, and the count must be even. Lines are clipped to the
 * destination surface.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCount
 *      twGfxErrorOperationInProgress
 */
Err TwGfxDrawLineSegments(TwGfxSurfaceType* aDestSurface,
                          const TwGfxPointType* aPoints,
                          Int32 aNumberOfPoints,
                          TwGfxPackedRGBType aColor)
  TAL_TRAP(trapTwGfxDrawLineSegments);

// TODO: describe which pixels are rendered
/*
 * Draw the outline of a rectangle. The resulting lines are clipped to
 * the destination surface.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorOperationInProgress
 */
Err TwGfxDrawRect(TwGfxSurfaceType* aDestSurface,
                  const TwGfxRectType* aRect,
                  TwGfxPackedRGBType aColor)
  TAL_TRAP(trapTwGfxDrawRect);

// TODO: describe which pixels are rendered
/*
 * Fill a rectangle. The rectangle is clipped to the destination
 * surface.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorOperationInProgress
 */
Err TwGfxFillRect(TwGfxSurfaceType* aDestSurface,
                  const TwGfxRectType* aRect,
                  TwGfxPackedRGBType aColor)
  TAL_TRAP(trapTwGfxFillRect);

// TODO: describe which pixels are rendered
/*
 * Draw a set of constant colored horizontal spans. This is similar to
 * drawing a set of horizontal lines, but with smaller data overhead
 * and higher efficiency. The spans are clipped to the destination
 * surface.
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidCount
 *      twGfxErrorOperationInProgress
 */
Err TwGfxDrawSpans(TwGfxSurfaceType* aDestSurface,
                   const TwGfxSpanType* aSpans,
                   Int32 aNumberOfSpans,
                   TwGfxPackedRGBType aColor)
  TAL_TRAP(trapTwGfxDrawSpans);

// TODO: describe which pixels are rendered
/*
 * Render a bitmap to the destination surface. The bitmap is clipped
 * to the destination surface.
 *
 * The following pixel formats are supported:
 *
 * twGfxPixelFormatMonochrome
 * twGfxPixelFormatRGB565_LE
 * twGfxPixelFormatRGB565_BE
 *
 * Errors:
 *      twGfxErrorInvalidHandle
 *      twGfxErrorNullPointer
 *      twGfxErrorInvalidPixelFormat
 *      twGfxErrorBadObjectVersion
 *      twGfxErrorOperationInProgress
 */
Err TwGfxDrawBitmap(TwGfxSurfaceType* aDestSurface,
                    const TwGfxPointType* aDestPoint,
                    const TwGfxBitmapType* aBitmap)
  TAL_TRAP(trapTwGfxDrawBitmap);

#ifdef __cplusplus
}
#endif

#endif /* __TWGFX_H__ */
