// Homegrown reporter talking code, as gleaned from POSE.

#include "..\ht.h"

void (__stdcall *tracer_init_outputport)		(HINSTANCE, const char*, const char*);
void (__stdcall *tracer_close_outputport)		(void);
void (__stdcall *tracer_output_VT)				(WORD, const char*, const va_list*);
void (__stdcall *tracer_output_VTL)				(WORD, const char*, const va_list*);
void (__stdcall *tracer_output_B)				(WORD, const BYTE*, size_t);
void (__stdcall *tracer_get_caps)				(char*,size_t*);
long (__stdcall *tracer_get_outputport_status)	(void);
HMODULE ghTracerLib;

void ReporterInit()
{
#ifdef INCL_TRACE
	ghTracerLib	= LoadLibrary ("palmtrace.dll");
	if (ghTracerLib == NULL)
		return;
	tracer_init_outputport	= (void (__stdcall *)(HINSTANCE, const char*,const char*)) GetProcAddress(ghTracerLib, "tracer_init_outputport");
	tracer_close_outputport	= (void (__stdcall *)(void)) GetProcAddress(ghTracerLib, "tracer_close_outputport");
	tracer_output_VT		= (void (__stdcall *)(unsigned short,const char *,const va_list * )) GetProcAddress(ghTracerLib, "tracer_output_VT");
	tracer_output_VTL		= (void (__stdcall *)(unsigned short,const char *,const va_list * )) GetProcAddress(ghTracerLib, "tracer_output_VTL");
	tracer_output_B			= (void (__stdcall *)(unsigned short,const unsigned char *,unsigned int))	GetProcAddress(ghTracerLib, "tracer_output_B");
	tracer_get_caps			= (void (__stdcall *) (char *,size_t*)) GetProcAddress(ghTracerLib, "tracer_get_capabilities");
	tracer_get_outputport_status	= (long (__stdcall *)(void)) GetProcAddress(ghTracerLib, "tracer_get_outputport_status");

	(*tracer_init_outputport) (GetModuleHandle(0), "tcp", "localhost");
#endif
}

void ReporterExit()
{
#ifdef INCL_TRACE
	if (ghTracerLib == NULL)
		return;
	(*tracer_close_outputport)();
	FreeLibrary(ghTracerLib);
#endif
}

void ReporterOutputDebugString(char *pszFormat, va_list va)
{
#ifdef INCL_TRACE
	if (ghTracerLib == NULL)
		return;
	(*tracer_output_VTL)(0, pszFormat, &va);
#endif
}
