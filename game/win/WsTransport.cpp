#include "..\ht.h"
#include "..\Multiplayer.h"
#include <WsTransport.h>

#ifdef DEBUG
const TCHAR *PszFromSocError();
const TCHAR *PszFromSocError(int nError);
#endif

//---------------------------------------------------------------------------
// WsTransport implementation

int WsTransport::GetTransportDescriptions(TransportDescription *atrad, int ctradMax)
{
	char szHostName[100];
	gethostname(szHostName, sizeof(szHostName));

	HOSTENT *phste = gethostbyname(szHostName);

	TransportDescription *ptrad = atrad;
	for (int i = 0; phste->h_addr_list[i] != NULL && i < ctradMax; i++, ptrad++) {
		in_addr *pina = (in_addr *)phste->h_addr_list[i];
		ptrad->trat = ktratIP;
		sprintf(ptrad->szName, "IP %s", inet_ntoa(*pina));
		ptrad->pfnOpen = WsTransport::Open;
		ptrad->dwTransportSpecific = ntohl(pina->S_un.S_addr);
	}
	return i;

// This is a good start for finding the Bluetooth transport on a device that has
// Microsoft's Bluetooth stack. Haven't encountered one yet.

#if 0 
	WSAQUERYSET qs;
	memset(&qs, 0, sizeof(qs));
	qs.dwSize = sizeof(qs);
	// UNDONE: namespace?

	HANDLE hls = NULL;
	int err = WSALookupServiceBegin(&qs, 
			LUP_FLUSHCACHE | LUP_NEAREST | LUP_RETURN_NAME | LUP_RETURN_TYPE | LUP_RETURN_COMMENT | LUP_RETURN_ADDR,
			&hls);
	if (err != 0) {
#ifdef DEBUG
		HostMessageBox(TEXT("WSALookupServiceBegin err: %s"), PszFromSocError());
#else
		HostMessageBox(TEXT("Unable to enumerate transports"));
#endif
		return 0;
	}

	TransportDescription *ptrad = atrad;
	while (true) {
		DWORD dwSize = sizeof(qs);

		int err = WSALookupServiceNext(hls,
				LUP_FLUSHCACHE | LUP_NEAREST | LUP_RETURN_NAME | LUP_RETURN_TYPE | LUP_RETURN_COMMENT | LUP_RETURN_ADDR,
				&dwSize, &qs);
		if (err == WSA_E_NO_MORE || err == WSAENOMORE)
			break;

		if (err != 0) {
#ifdef DEBUG
			HostMessageBox(TEXT("WSALookupServiceNet err: %s"), PszFromSocError(err));
#else
			HostMessageBox(TEXT("Unable to enumerate transports"));
#endif
			return 0;
		}

		// UNDONE: translate qs.lpServiceClassId into a TransportType
		strncpyz(ptrad->szName, qs.lpszServiceInstanceName, sizeof(ptrad->szName));

		ctrad++;
		ptrad++;
	}

	return ctrad;
#endif
}

Transport *WsTransport::Open(TransportDescription *ptrad)
{
	Transport *ptra = new WsTransport(ptrad->dwTransportSpecific);
	if (ptra == NULL)
		return NULL;

	if (!ptra->Open())
		return NULL;
	return ptra;
}

bool WsTransport::Open()
{
	WSADATA wsad;
	int err = WSAStartup(MAKEWORD(1, 1), &wsad);
	if (err != 0) {

		// unrecoverable low-level problem (e.g., socket limit reached, network 
		// subsystem not ready, requested version not supported)

#ifdef DEBUG
		HostMessageBox(TEXT("WSAStartup err: %s"), PszFromSocError(err));
#else
		HostMessageBox(TEXT("Unable to initialize network"));
#endif
		return false;
	}

	return SocTransport::Open();
}

void WsTransport::Close()
{
	SocTransport::Close();	// Closes any open Connections

	WSACleanup();
}

WsTransport::WsTransport(dword dwIpAddress) : SocTransport(dwIpAddress)
{
}

// Error strings for WSA errors

#ifdef DEBUG
struct {
	int nError;
	TCHAR *pszError;
} aSocErrors[] = {
	{ WSAEINTR, TEXT("EINTR") },
	{ WSAEBADF, TEXT("EBADF") },
	{ WSAEACCES, TEXT("EACCES") },
	{ WSAEFAULT, TEXT("EFAULT") },
	{ WSAEINVAL, TEXT("EINVAL") },
	{ WSAEMFILE, TEXT("EMFILE") },
	{ WSAEWOULDBLOCK, TEXT("EWOULDBLOCK") },
	{ WSAEINPROGRESS, TEXT("EINPROGRESS") },
	{ WSAEALREADY, TEXT("EALREADY") },
	{ WSAENOTSOCK, TEXT("ENOTSOCK") },
	{ WSAEDESTADDRREQ, TEXT("EDESTADDRREQ") },
	{ WSAEMSGSIZE, TEXT("EMSGSIZE") },
	{ WSAEPROTOTYPE, TEXT("EPROTOTYPE") },
	{ WSAENOPROTOOPT, TEXT("ENOPROTOOPT") },
	{ WSAEPROTONOSUPPORT, TEXT("EPROTONOSUPPORT") },
	{ WSAESOCKTNOSUPPORT, TEXT("ESOCKTNOSUPPORT") },
	{ WSAEOPNOTSUPP, TEXT("EOPNOTSUPP") },
	{ WSAEPFNOSUPPORT, TEXT("EPFNOSUPPORT") },
	{ WSAEAFNOSUPPORT, TEXT("EAFNOSUPPORT") },
	{ WSAEADDRINUSE, TEXT("EADDRINUSE") },
	{ WSAEADDRNOTAVAIL, TEXT("EADDRNOTAVAIL") },
	{ WSAENETDOWN, TEXT("ENETDOWN") },
	{ WSAENETUNREACH, TEXT("ENETUNREACH") },
	{ WSAENETRESET, TEXT("ENETRESET") },
	{ WSAECONNABORTED, TEXT("ECONNABORTED") },
	{ WSAECONNRESET, TEXT("ECONNRESET") },
	{ WSAENOBUFS, TEXT("ENOBUFS") },
	{ WSAEISCONN, TEXT("EISCONN") },
	{ WSAENOTCONN, TEXT("ENOTCONN") },
	{ WSAESHUTDOWN, TEXT("ESHUTDOWN") },
	{ WSAETOOMANYREFS, TEXT("ETOOMANYREFS") },
	{ WSAETIMEDOUT, TEXT("ETIMEDOUT") },
	{ WSAECONNREFUSED, TEXT("ECONNREFUSED") },
	{ WSAELOOP, TEXT("ELOOP") },
	{ WSAENAMETOOLONG, TEXT("ENAMETOOLONG") },
	{ WSAEHOSTDOWN, TEXT("EHOSTDOWN") },
	{ WSAEHOSTUNREACH, TEXT("EHOSTUNREACH") },
	{ WSAENOTEMPTY, TEXT("ENOTEMPTY") },
	{ WSAEPROCLIM, TEXT("EPROCLIM") },
	{ WSAEUSERS, TEXT("EUSERS") },
	{ WSAEDQUOT, TEXT("EDQUOT") },
	{ WSAESTALE, TEXT("ESTALE") },
	{ WSAEREMOTE, TEXT("EREMOTE") },
	{ WSAEDISCON, TEXT("EDISCON") },
	{ WSASYSNOTREADY, TEXT("SYSNOTREADY") },
	{ WSAVERNOTSUPPORTED, TEXT("VERNOTSUPPORTED") },
	{ WSANOTINITIALISED, TEXT("NOTINITIALISED") },
	{ WSAHOST_NOT_FOUND, TEXT("HOST_NOT_FOUND") },
	{ WSATRY_AGAIN, TEXT("TRY_AGAIN") },
	{ WSANO_RECOVERY, TEXT("NO_RECOVERY") },
	{ WSANO_DATA, TEXT("NO_DATA") },
};

const TCHAR *PszFromSocError(int nError)
{
	static TCHAR szError[80];
	TCHAR *pszError = TEXT("Unknown Socket error!");

	for (int i = 0; i < sizeof(aSocErrors) / (sizeof(int) + sizeof(char *));
			i++) {
		if (nError == aSocErrors[i].nError) {
			pszError = aSocErrors[i].pszError;
			break;
		}
	}

	wsprintf(szError, TEXT("%s (%d)"), pszError, nError);
	return szError;
}

#endif
