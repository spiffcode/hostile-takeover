#include "..\ht.h"
#include <WsTransport.h>

#ifdef CE
// Can't make this work on Windows without spending another $1,400 for the BTW for Windows SDK

static TransportHost s_trah;
static HMODULE s_hmodTransport = NULL;
#endif

TransportMgr gtram;

//---------------------------------------------------------------------------
// TransportMgr implementation

int TransportMgr::GetTransportDescriptions(TransportDescription *atrad, int ctradMax)
{
	int ctrad = 0;
#ifdef CE
	// First try our favorite PocketPC 2003, BTW-CE 1.4 compatible transport

	if (s_hmodTransport == NULL)
		s_hmodTransport = LoadLibrary(TEXT("Wc14Transport.dll"));

	// No? Then try the BTW-CE 1.3 compatible transport

	if (s_hmodTransport == NULL)
		s_hmodTransport = LoadLibrary(TEXT("Wc13Transport.dll"));

	// Process cleanup will unload the DLL loaded above (whichever it is)

	if (s_hmodTransport != NULL) {
		int (*pfnGetTransportDescriptions)(TransportHost *ptrah, TransportDescription *atrad, int ctradMax);
		pfnGetTransportDescriptions = (int (*)(TransportHost *ptrah, TransportDescription *atrad, int ctradMax))
				GetProcAddress(s_hmodTransport, TEXT("GetTransportDescriptions"));
		if (pfnGetTransportDescriptions != NULL) {
			s_trah.New = (void *(*)(int))::operator new;
			s_trah.Delete = ::operator delete;
			s_trah.strncpyz = strncpyz;
			s_trah.HtMessageBox = HtMessageBox;
			s_trah.Status = Status;
			s_trah.ptimm = &gtimm;

			ctrad += pfnGetTransportDescriptions(&s_trah, atrad + ctrad, ctradMax - ctrad);
		}
	}
#endif

	ctrad += WsTransport::GetTransportDescriptions(atrad + ctrad, ctradMax - ctrad);

	return ctrad;
}
