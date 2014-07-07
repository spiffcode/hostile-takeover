/******************************************************************************
 *
 * Copyright (c) 2003 PalmSource, Inc. All rights reserved.
 *
 * File: AdnDebugMgr.h
 *
 * Release: DebugNub for Palm OS 5.x Native ARM debugging
 *
 * Description:
 *    API used to communicate with the Palm OS 5.x DebugNub.
 *
 *****************************************************************************/

#ifndef __ADNDEBUGMGR_H__
#define __ADNDEBUGMGR_H__

// Uses unsigned shorts and longs to avoid needing PalmTypes.h for UInt32 typedef

// Define creator ID and featureNum used by DebugNub for Palm OS 5.x
#define adnFtrCreator		'adbg'
#define adnFtrNumVersion	0


/***********************************************************************
 *	Palm OS SWI-related Definitions (largely from future Palm OS PUD.h)
 ***********************************************************************/

// Generic Semihosting SWI op-codes			(non-exhaustive list)
#define kAdnSemihostArmWrite0				0x0004	// Write a C-string to stdout
#define kAdnSemihostArmReportException		0x0018	// Angel SWI reason report exception

// Palm-specific Semihosting SWI op-codes	(non-exhaustive list)
#define kAdnSemihostPUDConnect				0x0101
#define kAdnSemihostPUDProcessCreate		0x0110
#define kAdnSemihostPUDProcessDestroy		0x0111
#define kAdnSemihostPUDThreadCreate			0x0120
#define kAdnSemihostPUDThreadDestroy		0x0121
#define kAdnSemihostPUDModulePostLoad		0x0130
#define kAdnSemihostPUDModulePostUnload		0x0131
#define kAdnSemihostPUDDebugBreak			0x0132
#define kAdnSemihostPUDFaultNotification	0x0133
#define kAdnSemihostPUDModuleTableUpdate	0x0134
#define kAdnSemihostPUDDownloadEvent		0x0140
#define kAdnSemihostPUDProfilerEvent		0x0141

// Palm Adn-specific Semihosting SWI op-codes
#define kAdnSemihostNativeRegister			0x0150
#define kAdnSemihostNativeUnregister		0x0151
#define kAdnSemihostDebugEnableSet			0x0152
#define kAdnSemihostDebugEnableGet			0x0153
#define kAdnSemihostLicenseeSpecific		0x0154
#define kAdnSemihostDebugEnableGetSupported 0x0155


/***********************************************************************
 *	Features to be enabled and disabled
 ***********************************************************************/

#define kAdnEnableMasterSwitch				0x00000001	// [off] Master switch.  Nothing works with this off, except
														// AdnDebugEnableSet/Get() which is used to turn it on.
														//
														// With the master switch turned ON, the nub will:
														//  - Catch (by entering the debugger) fatal (ARM) exceptions
														//    (illegal memory access, undefined instructions).
														//  - Enter the debugger if AdnDebugBreak() is called.  Even
														//    if a debugger is not initially present, one can later
														//    be connected for "after the crash" debugging.
														// [Some nub implementations may also:]
														//  - Catch ErrFatalDisplay() calls.  Note: currently, won't
														//    catch 68K fatal alerts if you've dropped in the 68K
														//    debugger at least once since the last reset.
														//
														// In addition, if the nub has already communicated with a
														// debugger, then additional commands which otherwise required
														// kAdnEnableFullDebugging will also be allowed.

#define kAdnEnableDebugIndicator			0x00000002	// [on]  Enable visual indicator that shows when the debugger
														// nub is performing (or waiting for) serial communications.

#define kAdnEnableFullDebugging				0x00000004	// [off] Allow FULL interactive debugging.
														//  - This MUST be turned ON in order for application calls to
														//    AdnDebugNativeRegister() to be processed correctly
														//    (i.e. sent to the debugger to register loaded PACE Native
														//    Object code for source level debugging).
														//  - If this option is off, then other native debugging calls
														//    (ModulePostLoad, ModulePostUnload, etc.) are also disabled.

#define kAdnEnableShowSafeFatalAlerts		0x00000008	// [off] If on, don't catch calls to ErrFatalDisplay()
														// iff the Reset/Debug/Continue options are displayed
														// and we're pretty sure they'll work. [May not be supported.]

 
/***********************************************************************
 *	Structure Definitions (largely from future Palm OS PUD.h)
 ***********************************************************************/
typedef struct DbgPostLoadParamsType {
	unsigned long		processID;
	unsigned long		moduleID;
	void				*codeAddr;
	void				*dataAddr;
	unsigned long		type;
	unsigned long		creator;
	unsigned long		rsrcType;
	unsigned short		rsrcID;
	unsigned short		reserved;
} DbgPostLoadParamsType;

typedef struct DbgPostUnloadParamsType {
	unsigned long		processID;
	unsigned long		moduleID;
} DbgPostUnloadParamsType;


/***********************************************************************
 *	Semihosting SWI number	(from future Palm OS ARM.h)
 ***********************************************************************/
#define SEMI_SWI_ARM 0x123456L
#define SEMI_SWI_THUMB 0xAB

#ifdef __thumb
	#define SEMI_SWI_NUM SEMI_SWI_THUMB
#else
	#define SEMI_SWI_NUM SEMI_SWI_ARM
#endif


/***********************************************************************
 *	Palm OS SWI-related Prototypes
 ***********************************************************************/

#if !defined(__arm)		// Included in non-ARM build; silently ignore everything

	#define AdnGetNativeCodeBaseAddr() 0
	#define _SemihostOp0(op)			
	#define _SemihostOp0r(op)			
	#define _SemihostOp1(op,p1)			
	#define _SemihostOp3r(op,p1,p2,p3)	
	#define _SemihostWrite0(op, s)		
	#define _SemihostPostLoad(op, p)	
	#define _SemihostPostUnload(op, p)	


#elif defined(__MWERKS__)			// CodeWarrior for Palm OS R9.2 (beta 2 or greater)

	// This function must be defined for the current toolset, to retrieve and/or
	// calculate the base address of our code resource.  This function is called
	// called by AdnDebugNativeRegister() which tells the desktop debugger where
	// exactly in memory the code is located (to enable source level debugging).
	// Ideally, we just reference the startup function, but unfortunately without
	// PIC, that resolves to zero (it's not automatically relocated at runtime).
	// Here is code that works for CodeWarrior with or without PIC support.
	#if __option(PIC)				// Post 9.2 - just reference it
		extern
		#ifdef __cplusplus
		"C"
		#endif
		void __ARMlet_Startup__(void);
		#define AdnGetNativeCodeBaseAddr() ((void *)__ARMlet_Startup__)
	#else							// otherwise, calculate it
		#pragma thumb off
		static void * AdnGetNativeCodeBaseAddr(void);
		static asm void * AdnGetNativeCodeBaseAddr(void)
		{
			sub		r0, pc, #8						// get real address of this function
			lda		r1, AdnGetNativeCodeBaseAddr	// get zero-based offset to this function
			sub		r0, r0, r1						// subtract to get real base of code
			bx		lr								// and return
		}
		#pragma thumb reset
	#endif

	// These declarations work, but the functions don't get
	// inlined as requested (and thus we need the "bx lr").
	// We could remove the "inline" directive but then we need function prototypes.
	inline __asm void _SemihostOp0(unsigned op)
		{ swi SEMI_SWI_NUM; bx lr; }

	inline __asm unsigned _SemihostOp0r(unsigned op)
		{ swi SEMI_SWI_NUM; bx lr; }

	inline __asm void _SemihostOp1(unsigned op, unsigned param1)
		{ swi SEMI_SWI_NUM; bx lr; }

	inline __asm unsigned _SemihostOp3r(unsigned op, unsigned param1, unsigned param2, unsigned param3)
		{ swi SEMI_SWI_NUM; bx lr; }

	inline __asm void _SemihostWrite0(unsigned op, const char *string)
		{ swi SEMI_SWI_NUM; bx lr; }

	inline __asm void _SemihostPostLoad(unsigned op, const DbgPostLoadParamsType *)
		{ swi SEMI_SWI_NUM; bx lr; }

	inline __asm void _SemihostPostUnload(unsigned op, const DbgPostUnloadParamsType *)
		{ swi SEMI_SWI_NUM; bx lr; }

	// You might think that something like the following would work, but it doesn't.
	// The SWI instruction does get inlined, but the required function parameters
	// do not get properly moved into registers r0, r1, etc.
	//		inline /*asm*/ void _SemihostOp0(unsigned op)
	//			{ asm { swi SEMI_SWI_NUM } }
	

#elif defined(__ARMCC_VERSION)	// ARM ADS

	// Note: In order to debug ADS-generated PACE Native Objects, we must have
	// a valid definition for the AdnGetNativeCodeBaseAddr() [see above].
	
	// Since we don't currently support building in ADS, we force a build error
	// if anyone attempts to call this (as yet undefined) function (which is
	// called by AdnDebugNativeRegister(), below).  If you need to use this in
	// ADS, either fix this, or if you know your base address, you can instead
	// call the AdnDebugNativeRegisterAddr() variation.
	#define AdnGetNativeCodeBaseAddr() AdnThisShouldCauseABuildError()

	__swi(SEMI_SWI_NUM) void	_SemihostOp0		(unsigned op);
	__swi(SEMI_SWI_NUM) unsigned _SemihostOp0r		(unsigned op);
	__swi(SEMI_SWI_NUM) void	_SemihostOp1		(unsigned op, unsigned param1);
	__swi(SEMI_SWI_NUM) unsigned _SemihostOp3r		(unsigned op, unsigned param1, unsigned param2, unsigned param3);
	__swi(SEMI_SWI_NUM) void	_SemihostWrite0		(unsigned op, const char *string);
	__swi(SEMI_SWI_NUM) void	_SemihostPostLoad	(unsigned op, const DbgPostLoadParamsType *);
	__swi(SEMI_SWI_NUM) void	_SemihostPostUnload	(unsigned op, const DbgPostUnloadParamsType *);


#elif defined(__GNUC__)  // gcc

		// Note: In order to debug gcc-generated PACE Native Objects, we must
		// either come up with a valid definition for the AdnGetNativeCodeBaseAddr()
		// function [see above] or else PNO code will have to use the alternate
		// registration function, AdnDebugNativeRegisterAddr(), instead of the
		// usual AdnDebugNativeRegister().  In order to do so, the PNO must be
		// passed-in the base address of its code resource from the 68K side of
		// the world, and that value can then in turn be registered with POD.  

        // preprocessor nastiness
        #define WRAP(x) #x
        #define PUT_QUOTES_AROUND(x) WRAP(x)
        #define SWI_NUM_WITH_QUOTES_AROUND_IT PUT_QUOTES_AROUND(SEMI_SWI_NUM)
 
        inline static void _SemihostOp0(unsigned op)
                { asm("swi " SWI_NUM_WITH_QUOTES_AROUND_IT); }
 
        inline static unsigned _SemihostOp0r(unsigned op)
                { asm("swi " SWI_NUM_WITH_QUOTES_AROUND_IT); }
 
        inline static void _SemihostOp1(unsigned op, unsigned param1)
                { asm("swi " SWI_NUM_WITH_QUOTES_AROUND_IT); }
 
        inline static unsigned _SemihostOp3r(unsigned op, unsigned param1, unsigned param2, unsigned param3)
                { asm("swi " SWI_NUM_WITH_QUOTES_AROUND_IT); }
 
        inline static void _SemihostWrite0(unsigned op, const char *string)
                { asm("swi " SWI_NUM_WITH_QUOTES_AROUND_IT); }
 
        inline static void _SemihostPostLoad(unsigned op, const DbgPostLoadParamsType *params)
                { asm("swi " SWI_NUM_WITH_QUOTES_AROUND_IT); }
 
        inline static void _SemihostPostUnload(unsigned op, const DbgPostUnloadParamsType *params)
                { asm("swi " SWI_NUM_WITH_QUOTES_AROUND_IT); }


#else	// Unrecognized ARM compiler; don't know how to do a SWI, so punt safely...

	// This function must be defined for the current toolset, to retrieve and/or
	// calculate the base address of our code resource.  This function is called
	// called by AdnDebugNativeRegister() which tells the desktop debugger where
	// exactly in memory the code is located (to enable source level debugging).
	#define AdnGetNativeCodeBaseAddr() AdnThisShouldCauseABuildError()

	// The preferred solution is to just reference the startup function, but if
	// that isn't relocated at runtime then something like the following can be used.
//	#pragma thumb off
//	static void * AdnGetNativeCodeBaseAddr(void);
//	static asm void * AdnGetNativeCodeBaseAddr(void)
//	{
//		sub		r0, pc, #8						// get real address of this function
//		lda		r1, AdnGetNativeCodeBaseAddr	// get zero-based offset to this function
//		sub		r0, r0, r1						// subtract to get real base of code
//		bx		lr								// and return
//	}
//	#pragma thumb reset

	// To support other compilers, inline SWI calls must be implemented:
	// (The AdnDebugMgr APIs should not be called for non-ARM builds.)
	#define _SemihostOp0(op)			AdnThisShouldCauseABuildError()
	#define _SemihostOp0r(op)			AdnThisShouldCauseABuildError()
	#define _SemihostOp1(op,p1)			AdnThisShouldCauseABuildError()
	#define _SemihostOp3r(op,p1,p2,p3)	AdnThisShouldCauseABuildError()
	#define _SemihostWrite0(op, s)		AdnThisShouldCauseABuildError()
	#define _SemihostPostLoad(op, p)	AdnThisShouldCauseABuildError()
	#define _SemihostPostUnload(op, p)	AdnThisShouldCauseABuildError()

#endif


/***********************************************************************
 * FUNCTION:    AdnDebugEnableSet
 * DESCRIPTION: Enable debugger nub features (kAdnEnable*).
 * PARAMETERS:  Flags indicating which features to enable
 * RETURNED:    nothing
 ***********************************************************************/
#define AdnDebugEnableSet(flags) \
		_SemihostOp1(kAdnSemihostDebugEnableSet, flags)


/***********************************************************************
 * FUNCTION:    AdnDebugEnableGet
 * DESCRIPTION: Get enabled debugger nub features (kAdnEnable*).
 * PARAMETERS:  none
 * RETURNED:    Flags indicating which features are enabled
 ***********************************************************************/
#define AdnDebugEnableGet() \
		_SemihostOp0r(kAdnSemihostDebugEnableGet)


/***********************************************************************
 * FUNCTION:    AdnDebugEnableGetSupported
 * DESCRIPTION: Get supported debugger nub features (kAdnEnable*).
 * PARAMETERS:  none
 * RETURNED:    Flags indicating which features are supported
 ***********************************************************************/
#define AdnDebugEnableGetSupported() \
		_SemihostOp0r(kAdnSemihostDebugEnableGetSupported)


/***********************************************************************
 * FUNCTION:    AdnDebugLicenseeSpecific
 * DESCRIPTION: Make licensee-specific call to ArmDebugNub.
 * PARAMETERS:  oemID    - PalmSource-registered OEM ID (creator code)
 *              selector - Licensee-specific function selector
 *              param    - Function-specific parameter
 * RETURNED:    result   - Function-specific result
 *                         (kAdnErrUnsupportedCall == not supported)
 ***********************************************************************/
#define kAdnErrUnsupportedCall	0xFFFFFFFF

#define AdnDebugLicenseeSpecific(oemID, selector, param) \
		_SemihostOp3r(kAdnSemihostLicenseeSpecific, oemID, selector, param)



/***********************************************************************
 * FUNCTION:    AdnDebugMessage
 * DESCRIPTION: Display a debug message in the desktop debugger.
 * PARAMETERS:  messageP - pointer to null-terminated string to display
 * RETURNED:    nothing
 ***********************************************************************/
#define AdnDebugMessage(messageP) \
		_SemihostWrite0(kAdnSemihostArmWrite0, messageP)

#define AdnDebugMessageIf(condition, messageP) \
	do {if (condition) AdnDebugMessage(messageP);} while (0)


/***********************************************************************
 * FUNCTION:    AdnDebugBreak
 * DESCRIPTION: Break into the desktop debugger.
 * PARAMETERS:  none
 * RETURNED:    nothing
 ***********************************************************************/
#define AdnDebugBreak() \
		_SemihostOp0(kAdnSemihostPUDDebugBreak)



/***********************************************************************
 * FUNCTION:    AdnDebugUpdateLoadedModules
 * DESCRIPTION: Notify debugger of any recently loaded/unloaded modules.
 * PARAMETERS:  none
 * RETURNED:    nothing
 ***********************************************************************/
#define AdnDebugUpdateLoadedModules() \
		_SemihostOp0(kAdnSemihostPUDModuleTableUpdate)


/***********************************************************************
 * FUNCTION:    AdnDebugNativeRegisterAddr
 * DESCRIPTION: Ask debugger to register PACE Native Object. This is
 *				useful in the case where the code making this call
 *				needs to register Native code that lives elsewhere (e.g.
 *				in a different chunk), and thus AdnGetNativeCodeBaseAddr()
 *				won't retrieve the correct code base address. Generally,
 *				PACE Native Objects which are registering themselves should
 *				use the simpler form, AdnDebugNativeRegister(), below.
 * PARAMETERS:  dbType    - application database type (e.g. 'appl')
 *              dbCreator - application database creator code
 *              rsrcType  - PACE Native Object resource type (e.g. 'ARMC')
 *              rsrcID    - PACE Native Object resource ID
 *              codeAddr  - PACE Native Object code base address
 * RETURNED:    nothing
 ***********************************************************************/
#define AdnDebugNativeRegisterAddr(_dbType, _dbCreator, _rsrcType, _rsrcID, _codeAddr) \
		{																			\
			DbgPostLoadParamsType _postLoadParams;									\
			_postLoadParams.processID = 0;											\
			_postLoadParams.moduleID = (unsigned long)_codeAddr /*sectionCookie*/;	\
			_postLoadParams.codeAddr = _codeAddr;									\
			_postLoadParams.dataAddr = 0;											\
			_postLoadParams.type = _dbType;											\
			_postLoadParams.creator = _dbCreator;									\
			_postLoadParams.rsrcType = _rsrcType;									\
			_postLoadParams.rsrcID = _rsrcID;										\
			_postLoadParams.reserved = 0;											\
			_SemihostPostLoad(kAdnSemihostNativeRegister, &_postLoadParams);		\
		}
		

/***********************************************************************
 * FUNCTION:    AdnDebugNativeRegister
 * DESCRIPTION: Ask debugger to register PACE Native Object.
 *				This should be done after locking the code.  Any
 *				previously unresolved breakpoints will be activated.
 * PARAMETERS:  dbType    - application database type (e.g. 'appl')
 *              dbCreator - application database creator code
 *              rsrcType  - PACE Native Object resource type (e.g. 'ARMC')
 *              rsrcID    - PACE Native Object resource ID
 * RETURNED:    nothing
 ***********************************************************************/
#define AdnDebugNativeRegister(_dbType, _dbCreator, _rsrcType, _rsrcID) \
		AdnDebugNativeRegisterAddr(_dbType, _dbCreator, _rsrcType, _rsrcID, AdnGetNativeCodeBaseAddr());


/***********************************************************************
 * FUNCTION:    AdnDebugNativeUnregisterAddr
 * DESCRIPTION: Ask debugger to unregister PACE Native Object. This is
 *				useful in the case where the code making this call
 *				needs to unregister Native code that lives elsewhere (e.g.
 *				in a different chunk), and thus AdnGetNativeCodeBaseAddr()
 *				won't retrieve the correct code base address. Generally,
 *				PACE Native Objects which are unregistering themselves should
 *				use the simpler form, AdnDebugNativeUnregister(), below.
 * PARAMETERS:  codeAddr  - PACE Native Object code base address
 * RETURNED:    nothing
 ***********************************************************************/
#define AdnDebugNativeUnregisterAddr(_codeAddr)											\
		{																				\
			DbgPostUnloadParamsType _postUnloadParams;									\
			_postUnloadParams.processID = 0;											\
			_postUnloadParams.moduleID = (unsigned long)_codeAddr /*sectionCookie*/;	\
			_SemihostPostUnload(kAdnSemihostNativeUnregister, &_postUnloadParams);		\
		}

/***********************************************************************
 * FUNCTION:    AdnDebugNativeUnregister
 * DESCRIPTION: Ask debugger to unregister PACE Native Object.
 *				This should be done prior to unlocking the code since
 *				breakpoints in the code may need to be removed.
 * PARAMETERS:  none
 * RETURNED:    nothing
 ***********************************************************************/
#define AdnDebugNativeUnregister() \
		AdnDebugNativeUnregisterAddr(AdnGetNativeCodeBaseAddr());


#endif // __ADNDEBUGMGR_H__
