/////////////////////////////////////////////////////////////////////////////
// rip.cpp
/////////////////////////////////////////////////////////////////////////////
// This module contains support for communicating errors or messages to a
// developer or tester.
/////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) || defined(DEBUG)

namespace wi {
char *gpszRipFile;
int giRipLine;
}

#if defined(WIN) && !defined(CE)

#include "windows.h"
#include "rip.h"
#ifdef USE_PALM_UNIX_HEADERS
#include <unix\unix_stdio.h>
#else
#include <stdio.h>
#endif

#define RIP_DEFINED

namespace wi {
    
/////////////////////////////////////////////////////////////////////////////
// DoRip
//
// Gets called to output a message to a developer or tester.
/////////////////////////////////////////////////////////////////////////////

// Turn off c++ exception handler unwind semantics not enabled warning

#pragma warning(disable : 4530)

void DoRip(TCHAR *pszTitle, TCHAR *psz, va_list va)
{
    int idT;
    static bool gfRipActive = FALSE;

    // If gpszRipFile is not NULL, Format the string
    // "File: name.cpp, Line: nnn, ..."
    // Otherwise, format the string without the File/Line info.

    TCHAR szT[256];
    TCHAR szT1[128];
    wvsprintf(szT1, psz, va);

    if (gpszRipFile != NULL)
        wsprintf(szT, TEXT("\n%s: File: %s, Line: %d. %s\n"), pszTitle, gpszRipFile, giRipLine, szT1);
    else
        wsprintf(szT, TEXT("%s: %s"), pszTitle, szT1);

    // This'll break into the debugger if the debugger is present,
    // otherwise it'll fall through to the message box case.

    OutputDebugString(szT);

#if 0
    bool fBeingDebugged = TRUE;
#if defined(WIN) && !defined(CE)
    try {
		DebugBreak();
    } catch (...) {
        fBeingDebugged = FALSE;
    }
#else
	DebugBreak();
#endif

    // If we are not being debugged, bring up the message box

    if (fBeingDebugged)
		return;
#endif

    // Bring up a message box with these options:
    // Abort: terminate app
    // Retry: break
    // Ignore: keep running        

    // We don't want to recurse, which may happen when this message
    // box comes up due to WM_ACTIVATE messages and other being
    // sent back to the app

    if (gfRipActive)
        return;

    gfRipActive = TRUE;
    idT = MessageBox(NULL, szT, pszTitle,
			MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);
    gfRipActive = FALSE;

    switch (idT) {
    case IDABORT:
        // The process should exit now
        // Terminate the process so we don't go through all that
        // .dll cleanup code. This'll avoid mfc dumping heap
        // allocation info to the debugger, making termination
        // faster.

        TerminateProcess(GetCurrentProcess(), 0);
        return;

    case IDRETRY:
        // This'll break into the debugger or cause the just
        // in time debugger to load. If neither are present, the app
        // will terminate.

        OutputDebugString(szT);

		DebugBreak();

        // Just continue and hope things get better

        return;

	case IDIGNORE:
        // Output assertion string. Can also be used to ignore
		// asserts.

        OutputDebugString(szT);
        return;
    }
}

void DoAssertRip(int fNoAssert, TCHAR *psz, ...)
{
    if (fNoAssert)
        return;

    va_list va;
    va_start(va, psz);
    DoRip(TEXT("Assertion failed!"), psz, va);
    va_end(va);
}

void DoAssertRip(int fNoAssert)
{
    if (fNoAssert)
		return;

    DoRip(TEXT("Assertion failed!"), TEXT(""), NULL);
}

void DoAssertRip(TCHAR *psz, ...)
{
    va_list va;
    va_start(va, psz);
    DoRip(TEXT("Assert!"), psz, va);
    va_end(va);
}

void dvprintf(TCHAR *psz, va_list va)
{
    TCHAR szT1[2048];
    wvsprintf(szT1, psz, va);
    OutputDebugString(szT1);
}

void dprintf(TCHAR *psz, ...)
{
    va_list va;
    va_start(va, psz);
    dvprintf(psz, va);
    va_end(va);
}

void DoAssertRip()
{
	DoAssertRip(TEXT("Unknown error"));
}
    
} // namespace wi

#endif // WIN

#ifdef PIL

#define RIP_DEFINED

#include <palmos.h>
#include "rip.h"

namespace wi {
    
#ifdef PNO
void DebugBreak()
{
}
#else
extern "C" void DebugBreak() secCode14;
#endif

void Break()
{
	ErrDisplayFileLineMsg(gpszRipFile, giRipLine, "Assert!");
//	DebugBreak();
}

void DoAssertRip(int fNoAssert, char *psz, ...)
{
    if (fNoAssert)
        return;
    Break();
}

void DoAssertRip(int fNoAssert)
{
	if (!fNoAssert)
		Break();
}

void DoAssertRip(char *psz, ...)
{
	Break();
}

void DoAssertRip()
{
	Break();
}

} // namespace wi
    
#endif // PIL

#ifdef CE

#define RIP_DEFINED

#include "windows.h"

#include "rip.h"

namespace wi {

void Break()
{
	WCHAR wszT[300];
	WCHAR wszT2[300];

	MultiByteToWideChar(CP_ACP, 0, gpszRipFile, -1, wszT2, sizeof(wszT) - 1);
	wsprintf(wszT, TEXT("File: %s, Line: %d"), wszT2, giRipLine);
	MessageBox(NULL, wszT, TEXT("Assert"), MB_OK);
	DebugBreak();
}

void DoRip(char *psz, va_list va)
{
	char szT[300];
 	sprintf(szT, "File: %s, Line: %d. ", gpszRipFile, giRipLine);
	vsprintf(szT + strlen(szT), psz, va);
	
	WCHAR wszT[300];
	MultiByteToWideChar(CP_ACP, 0, szT, -1, wszT, sizeof(wszT) - 1);
	MessageBox(NULL, wszT, TEXT("Assert"), MB_OK);

	DebugBreak();
}

void DoAssertRip(int fNoAssert, char *psz, ...)
{
    if (fNoAssert)
        return;
    va_list va;
    va_start(va, psz);
	DoRip(psz, va);
    va_end(va);
}

void DoAssertRip(int fNoAssert)
{
	if (fNoAssert)
		return;
	DoRip("", NULL);
}

void DoAssertRip(char *psz, ...)
{
    va_list va;
    va_start(va, psz);
	DoRip(psz, va);
    va_end(va);
}

void DoAssertRip()
{
	Break();
}

} // namespace wi

#endif // CE

#ifdef IPHONE

#define RIP_DEFINED

#include <stdarg.h>
#include "rip.h"
#include "iphone.h"

namespace wi {

void Break()
{
    IPhone::Log("Break:");
    IPhone::Log("File: %s, Line: %d", gpszRipFile, giRipLine);
    IPhone::Break();
}

void DoRip(char *psz, va_list va)
{
    IPhone::Log("Assert:");
    IPhone::Log("File: %s, Line: %d", gpszRipFile, giRipLine);
    IPhone::Log(psz, va);
    IPhone::Break();
}

void DoAssertRip(int fNoAssert, char *psz, ...)
{
    if (fNoAssert)
        return;
    va_list va;
    va_start(va, psz);
	DoRip(psz, va);
    va_end(va);
}

void DoAssertRip(int fNoAssert)
{
	if (fNoAssert)
		return;
	DoRip("", 0);
}

void DoAssertRip(char *psz, ...)
{
    va_list va;
    va_start(va, psz);
	DoRip(psz, va);
    va_end(va);
}

void DoAssertRip()
{
	Break();
}

void dprintf(const char *psz, ...)
{
    va_list va;
    va_start(va, psz);
    vprintf(psz, va);
    va_end(va);
}

} // namespace wi
    
#endif // IPHONE

#ifndef RIP_DEFINED

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "inc/rip.h"

namespace wi {

void Break()
{
    printf("Break:\n");
    printf("File: %s, Line: %d\n", gpszRipFile, giRipLine);
    abort();
}

void DoRip(char *psz, va_list va)
{
    printf("Assert:\n");
    printf("File: %s, Line: %d\n", gpszRipFile, giRipLine);
    vprintf(psz, va);
    Break();
}

void DoAssertRip(int fNoAssert, char *psz, ...)
{
    if (fNoAssert)
        return;
    va_list va;
    va_start(va, psz);
	DoRip(psz, va);
    va_end(va);
}

void DoAssertRip(int fNoAssert)
{
	if (fNoAssert)
		return;
	DoRip("", 0);
}

void DoAssertRip(char *psz, ...)
{
    va_list va;
    va_start(va, psz);
	DoRip(psz, va);
    va_end(va);
}

void DoAssertRip()
{
	Break();
}

void dprintf(const char *psz, ...)
{
    va_list va;
    va_start(va, psz);
    vprintf(psz, va);
    va_end(va);
}

} // namespace wi
    
#endif // !RIP_DEFINED

#endif // defined(_DEBUG) || defined(DEBUG)
