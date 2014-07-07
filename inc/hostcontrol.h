/* -*- mode: C++; tab-width: 4 -*- */
/* ====================================================================================== */
/* Copyright (c) 1998-2000 Palm Computing, Inc. or its subsidiaries. All rights reserved. */
/* ====================================================================================== */

#ifndef _HOSTCONTROL_H_
#define _HOSTCONTROL_H_

#if !defined (__SYSTRAPS_H_) && !defined (__CORETRAPS_H_)
#error "Please #include either <CoreTraps.h> or <SysTraps.h>, depending on what's in your Palm Includes directory."
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*
	Set the base value for selectors. Note that this value MUST be
	two bytes long and have the high byte be non-zero. The reason
	for this has to do with the way SysGremlins was originally
	declared. It took a GremlinsSelector enumerated value. Originally,
	there was only one value, and it was zero. The way the 68K compiler
	works, it decides that GremlinsSelectors are only one byte long,
	so a call to SysGremlins would push one byte onto the stack. Because
	all values on the stack need to be word-aligned, the processor
	subtracts 1 from the stack before pushing on the byte. Therefore,
	the stack looks like:

			previous contents
			garbage byte
			selector
			return address

	With this setup, we have two choices: leave the selector size at
	one byte and limit ourselves to 256 functions, or define the selector
	to be a two byte value, with the first 256 values (all those with 0x00
	in the upper byte) to be GremlinIsOn. The latter sounds preferable, so
	we start the new selectors at 0x0100.
*/

#define hostSelectorBase	0x0100

	// Host information selectors

#define hostSelectorGetHostVersion			0x0100
#define hostSelectorGetHostID				0x0101
#define hostSelectorGetHostPlatform			0x0102
#define hostSelectorIsSelectorImplemented	0x0103
#define hostSelectorGestalt					0x0104
#define hostSelectorIsCallingTrap			0x0105

	// Profiler selectors

#define hostSelectorProfileInit				0x0200
#define hostSelectorProfileStart			0x0201
#define hostSelectorProfileStop				0x0202
#define hostSelectorProfileDump				0x0203
#define hostSelectorProfileCleanup			0x0204
#define hostSelectorProfileDetailFn			0x0205

	// Std C Library wrapper selectors

#define hostSelectorErrNo					0x0300

#define hostSelectorFClose					0x0301
#define hostSelectorFEOF					0x0302
#define hostSelectorFError					0x0303
#define hostSelectorFFlush					0x0304
#define hostSelectorFGetC					0x0305
#define hostSelectorFGetPos					0x0306
#define hostSelectorFGetS					0x0307
#define hostSelectorFOpen					0x0308
#define hostSelectorFPrintF					0x0309		/* Floating point not yet supported in Poser */
#define hostSelectorFPutC					0x030A
#define hostSelectorFPutS					0x030B
#define hostSelectorFRead					0x030C
#define hostSelectorRemove					0x030D		/* Not yet implemented in Poser */
#define hostSelectorRename					0x030E		/* Not yet implemented in Poser */
#define hostSelectorFReopen					0x030F		/* Not yet implemented in Poser */
#define hostSelectorFScanF					0x0310		/* Not yet implemented */
#define hostSelectorFSeek					0x0311
#define hostSelectorFSetPos					0x0312
#define hostSelectorFTell					0x0313
#define hostSelectorFWrite					0x0314
#define hostSelectorTmpFile					0x0315
#define hostSelectorTmpNam					0x0316		/* Not yet implemented in Poser */
#define hostSelectorGetEnv					0x0317

#define hostSelectorMalloc					0x0318		/* Not yet implemented in Poser */
#define hostSelectorRealloc					0x0319		/* Not yet implemented in Poser */
#define hostSelectorFree					0x031A		/* Not yet implemented in Poser */

	// Gremlin selectors

#define hostSelectorGremlinIsRunning		0x0400
#define hostSelectorGremlinNumber			0x0401
#define hostSelectorGremlinCounter			0x0402
#define hostSelectorGremlinLimit			0x0403
#define hostSelectorGremlinNew				0x0404

	// Database selectors

#define hostSelectorImportFile				0x0500
#define hostSelectorExportFile				0x0501

	// Preferences selectors

#define hostSelectorGetPreference			0x0600
#define hostSelectorSetPreference			0x0601

	// Logging selectors

#define hostSelectorLogFile					0x0700
#define hostSelectorSetLogFileSize			0x0701

	// RPC selectors

#define hostSelectorSessionCreate			0x0800		/* Not yet implemented in Poser */
#define hostSelectorSessionOpen				0x0801		/* Not yet implemented in Poser */
#define hostSelectorSessionClose			0x0802
#define hostSelectorSessionQuit				0x0803
#define hostSelectorSignalSend				0x0804
#define hostSelectorSignalWait				0x0805
#define hostSelectorSignalResume			0x0806

	// External tracing tool support

#define hostSelectorTraceInit				0x0900
#define hostSelectorTraceClose				0x0901
#define hostSelectorTraceOutputT			0x0902
#define hostSelectorTraceOutputTL			0x0903
#define hostSelectorTraceOutputVT			0x0904
#define hostSelectorTraceOutputVTL			0x0905
#define hostSelectorTraceOutputB			0x0906

#define hostSelectorLastTrapNumber			0x0907

typedef UInt16 HostControlTrapNumber;


struct HostFILE
{
	long	_field;
};

typedef struct HostFILE HostFILE;


typedef long	HostBool;
typedef long	HostErr;
typedef long	HostID;
typedef long	HostPlatform;
typedef long	HostSignal;

#ifndef hostErrorClass
	#define	hostErrorClass				0x1C00	// Host Control Manager
#else
	#if hostErrorClass != 0x1C00
		#error "You cannot change hostErrorClass without telling us."
	#endif
#endif

enum	// HostErr values
{
	hostErrNone = 0,

	hostErrBase = hostErrorClass,

	hostErrUnknownGestaltSelector,
	hostErrDiskError,
	hostErrOutOfMemory,
	hostErrMemReadOutOfRange,
	hostErrMemWriteOutOfRange,
	hostErrMemInvalidPtr,
	hostErrInvalidParameter,
	hostErrTimeout,
	hostErrInvalidDeviceType,
	hostErrInvalidRAMSize,
	hostErrFileNotFound,
	hostErrRPCCall,				// Issued if the following functions are not called remotely:
								//		HostSessionCreate
								//		HostSessionOpen
								//		HostSessionClose
								//		HostSessionQuit
								//		HostSignalWait
								//		HostSignalResume
	hostErrSessionRunning,		// Issued by HostSessionCreate, HostSessionOpen, and
								// HostSessionQuit if a session is running.
	hostErrSessionNotRunning,	// Issued by HostSessionClose if no session is running.
	hostErrNoSignalWaiters,		// Issued by HostSendSignal if no one's waiting for a signal.
	hostErrSessionNotPaused		// Issued when HostSignalResume, but the session was not
								// halted from a HostSignalSend call.
};


enum	// HostID values
{
	hostIDPalmOS,			// The plastic thingy
	hostIDPalmOSEmulator,	// The Copilot thingy
	hostIDPalmOSSimulator	// The Mac libraries you link with thingy
};


enum	// HostPlatform values
{
	hostPlatformPalmOS,
	hostPlatformWindows,
	hostPlatformMacintosh,
	hostPlatformUnix
};

enum	// HostSignal values
{
	hostSignalReserved,
	hostSignalIdle,
	hostSignalQuit,
#if 0
	// (Proposed...not supported yet)
	hostSignalSessionStarted,
	hostSignalSessionStopped,
	hostSignalHordeStarted,
	hostSignalGremlinStarted,
	hostSignalGremlinSuspended,
	hostSignalGremlinResumed,
	hostSignalGremlinStopped,
	hostSignalHordeStopped,
#endif
	hostSignalUser	= 0x40000000	// User-defined values start here and go up.
};


// Use these to call FtrGet to see if you're running under the
// Palm OS Emulator.  If not, FtrGet will return ftrErrNoSuchFeature.

#define kPalmOSEmulatorFeatureCreator	('pose')
#define kPalmOSEmulatorFeatureNumber	(0)


struct HostGremlinInfo
{
	long		fFirstGremlin;
	long		fLastGremlin;
	long		fSaveFrequency;
	long		fSwitchDepth;
	long		fMaxDepth;
	char		fAppNames[200];	// Comma-seperated list of application names
								// to run Gremlins on.  If the string is empty,
								// all applications are fair game.  If the string
								// begins with a '-' (e.g., "-Address,Datebook"),
								// then all applications named in the list are
								// excluded instead of included.
};

typedef struct HostGremlinInfo HostGremlinInfo;


// Define this, since SysTraps.h doesn't have it.

#ifdef __SYSTRAPS_H_
#define sysTrapHostControl sysTrapSysGremlins
#endif


// Define HOST_TRAP

#if defined (_SYSTEM_API)

#define HOST_TRAP(selector)	\
	_SYSTEM_API(_CALL_WITH_16BIT_SELECTOR)(_SYSTEM_TABLE, sysTrapHostControl, selector)

#else

#define HOST_TRAP(selector)													\
	FIVEWORD_INLINE(														\
		0x3F3C, selector,					/* MOVE.W #selector, -(A7)	*/	\
		m68kTrapInstr + sysDispatchTrapNum,	/* TRAP $F					*/	\
		sysTrapHostControl,					/* sysTrapHostControl		*/	\
		0x544F)								/* ADD.Q #2, A7				*/ 
#endif


/* ==================================================================== */
/* Host environment-related calls										*/
/* ==================================================================== */

long			HostGetHostVersion(void)
						HOST_TRAP(hostSelectorGetHostVersion);

HostID			HostGetHostID(void)
						HOST_TRAP(hostSelectorGetHostID);

HostPlatform	HostGetHostPlatform(void)
						HOST_TRAP(hostSelectorGetHostPlatform);

HostBool		HostIsSelectorImplemented(long selector)
						HOST_TRAP(hostSelectorIsSelectorImplemented);

HostErr			HostGestalt(long gestSel, long* response)
						HOST_TRAP(hostSelectorGestalt);

HostBool		HostIsCallingTrap(void)
						HOST_TRAP(hostSelectorIsCallingTrap);


/* ==================================================================== */
/* Profiling-related calls												*/
/* ==================================================================== */

HostErr			HostProfileInit(long maxCalls, long maxDepth)
						HOST_TRAP(hostSelectorProfileInit);

HostErr			HostProfileDetailFn(void* addr, HostBool logDetails)
						HOST_TRAP(hostSelectorProfileDetailFn);

HostErr			HostProfileStart(void)
						HOST_TRAP(hostSelectorProfileStart);

HostErr			HostProfileStop(void)
						HOST_TRAP(hostSelectorProfileStop);

HostErr			HostProfileDump(const char* filename)
						HOST_TRAP(hostSelectorProfileDump);

HostErr			HostProfileCleanup(void)
						HOST_TRAP(hostSelectorProfileCleanup);


/* ==================================================================== */
/* Std C Library-related calls											*/
/* ==================================================================== */

long			HostErrNo(void)
						HOST_TRAP(hostSelectorErrNo);


long			HostFClose(HostFILE* f)
						HOST_TRAP(hostSelectorFClose);

long			HostFEOF(HostFILE* f)
						HOST_TRAP(hostSelectorFEOF);

long			HostFError(HostFILE* f)
						HOST_TRAP(hostSelectorFError);

long			HostFFlush(HostFILE* f)
						HOST_TRAP(hostSelectorFFlush);

long			HostFGetC(HostFILE* f)
						HOST_TRAP(hostSelectorFGetC);

long			HostFGetPos(HostFILE* f, long* posP)
						HOST_TRAP(hostSelectorFGetPos);

char*			HostFGetS(char* s, long n, HostFILE* f)
						HOST_TRAP(hostSelectorFGetS);

HostFILE*		HostFOpen(const char* name, const char* mode)
						HOST_TRAP(hostSelectorFOpen);

long			HostFPrintF(HostFILE* f, const char* fmt, ...)
						HOST_TRAP(hostSelectorFPrintF);

long			HostFPutC(long c, HostFILE* f)
						HOST_TRAP(hostSelectorFPutC);

long			HostFPutS(const char* s, HostFILE* f)
						HOST_TRAP(hostSelectorFPutS);

long			HostFRead(void* buffer, long size, long count, HostFILE* f)
						HOST_TRAP(hostSelectorFRead);

long			HostRemove(const char* name)
						HOST_TRAP(hostSelectorRemove);

long			HostRename(const char* oldName, const char* newName)
						HOST_TRAP(hostSelectorRename);

HostFILE*		HostFReopen(const char* name, const char* mode, HostFILE *f)
						HOST_TRAP(hostSelectorFReopen);

long			HostFScanF(HostFILE* f, const char *fmt, ...)
						HOST_TRAP(hostSelectorFScanF);

long			HostFSeek(HostFILE* f, long offset, long origin)
						HOST_TRAP(hostSelectorFSeek);

long			HostFSetPos(HostFILE* f, long* pos)
						HOST_TRAP(hostSelectorFSetPos);

long			HostFTell(HostFILE* f)
						HOST_TRAP(hostSelectorFTell);

long			HostFWrite(const void* buffer, long size, long count, HostFILE* f)
						HOST_TRAP(hostSelectorFWrite);

HostFILE*		HostTmpFile(void)
						HOST_TRAP(hostSelectorTmpFile);

char*			HostTmpNam(char *name)
						HOST_TRAP(hostSelectorTmpNam);

char*			HostGetEnv(const char*)
						HOST_TRAP(hostSelectorGetEnv);


void*			HostMalloc(long size)
						HOST_TRAP(hostSelectorMalloc);

void*			HostRealloc(void* p, long size)
						HOST_TRAP(hostSelectorRealloc);

void			HostFree(void* p)
						HOST_TRAP(hostSelectorFree);


/* ==================================================================== */
/* Gremlin-related calls												*/
/* ==================================================================== */

HostBool		HostGremlinIsRunning(void)
						HOST_TRAP(hostSelectorGremlinIsRunning);

long			HostGremlinNumber(void)
						HOST_TRAP(hostSelectorGremlinNumber);

long			HostGremlinCounter(void)
						HOST_TRAP(hostSelectorGremlinCounter);

long			HostGremlinLimit(void)
						HOST_TRAP(hostSelectorGremlinLimit);

HostErr			HostGremlinNew(const HostGremlinInfo*)
						HOST_TRAP(hostSelectorGremlinNew);


/* ==================================================================== */
/* Import/export-related calls											*/
/* ==================================================================== */

HostErr			HostImportFile(const char* fileName, long cardNum)
						HOST_TRAP(hostSelectorImportFile);

HostErr			HostExportFile(const char* fileName, long cardNum, const char* dbName)
						HOST_TRAP(hostSelectorExportFile);


/* ==================================================================== */
/* Preference-related calls												*/
/* ==================================================================== */

HostBool		HostGetPreference(const char*, char*)
						HOST_TRAP(hostSelectorGetPreference);

void			HostSetPreference(const char*, const char*)
						HOST_TRAP(hostSelectorSetPreference);


/* ==================================================================== */
/* Logging-related calls												*/
/* ==================================================================== */

HostFILE*		HostLogFile(void)
						HOST_TRAP(hostSelectorLogFile);

void			HostSetLogFileSize(long)
						HOST_TRAP(hostSelectorSetLogFileSize);


/* ==================================================================== */
/* RPC-related calls													*/
/* ==================================================================== */

HostErr			HostSessionCreate(const char* device, long ramSize, const char* romPath)
						HOST_TRAP(hostSelectorSessionCreate);

HostErr			HostSessionOpen(const char* psfFileName)
						HOST_TRAP(hostSelectorSessionOpen);

HostErr			HostSessionClose(const char* saveFileName)
						HOST_TRAP(hostSelectorSessionClose);

HostErr			HostSessionQuit(void)
						HOST_TRAP(hostSelectorSessionQuit);

HostErr			HostSignalSend(HostSignal signalNumber)
						HOST_TRAP(hostSelectorSignalSend);

HostErr			HostSignalWait(long timeout, HostSignal* signalNumber)
						HOST_TRAP(hostSelectorSignalWait);

HostErr			HostSignalResume(void)
						HOST_TRAP(hostSelectorSignalResume);

/* ==================================================================== */
/* Tracing calls														*/
/* ==================================================================== */


void			HostTraceInit(void)
						HOST_TRAP(hostSelectorTraceInit);

void			HostTraceClose(void)
						HOST_TRAP(hostSelectorTraceClose);

void			HostTraceOutputT(unsigned short, const char*, ...)
						HOST_TRAP(hostSelectorTraceOutputT);

void			HostTraceOutputTL(unsigned short, const char*, ...)
						HOST_TRAP(hostSelectorTraceOutputTL);

void			HostTraceOutputVT(unsigned short, const char*, char* /*va_list*/)
						HOST_TRAP(hostSelectorTraceOutputVT);

void			HostTraceOutputVTL(unsigned short, const char*, char* /*va_list*/)
						HOST_TRAP(hostSelectorTraceOutputVTL);

void			HostTraceOutputB(unsigned short, const unsigned char*, unsigned long/*size_t*/)
						HOST_TRAP(hostSelectorTraceOutputB);


#ifdef __cplusplus 
}
#endif

#endif /* _HOSTCONTROL_H_ */
