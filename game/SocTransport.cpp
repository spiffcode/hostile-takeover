#ifdef PNO
#define USE_PALM_UNIX_HEADERS
#endif

#include "ht.h"
#include "mpshared/netmessage.h"
#include "SocTransport.h"

namespace wi {

#ifdef DEBUG
const TCHAR *PszFromSocError() secComm;
const TCHAR *PszFromSocError(int nError) secComm;
#endif

//---------------------------------------------------------------------------
// SocTransport implementation

SocTransport::SocTransport(dword dwIpAddress)
{
	m_socBroadcast = INVALID_SOCKET;
	m_socBroadcastListen = INVALID_SOCKET;
	m_socAcceptListen = INVALID_SOCKET;
	m_dwIpAddress = dwIpAddress;
	m_pconAsyncConnect = NULL;
}

bool SocTransport::GetLocalNetAddress(NetAddress *pnad)
{
	memset(pnad, 0, sizeof(*pnad));
	sockaddr_in *psoca = (sockaddr_in *)pnad;
	psoca->sin_family = AF_INET;
	psoca->sin_port = 0;
    
#ifdef IPHONE
	psoca->sin_addr.s_addr = htonl(m_dwIpAddress);
#else
	psoca->sin_addr.S_un.S_addr = htonl(m_dwIpAddress);
#endif
	return true;
}

// Stub so we can force the code section this destructor goes in

SocTransport::~SocTransport()
{
}

void SocTransport::GetAddressString(char *pszAddress, int cbMax)
{
	in_addr ina;
#ifdef IPHONE
    ina.s_addr = htonl(m_dwIpAddress);
#else
	ina.S_un.S_addr = htonl(m_dwIpAddress);
#endif
	strncpyz(pszAddress, inet_ntoa(ina), cbMax);
}

// UNDONE: this blocks in connect() (not Async!)

Connection *SocTransport::AsyncConnect(NetAddress *pnad)
{
	MpTrace("AsyncConnect(%s)", pnad != NULL ? inet_ntoa(((sockaddr_in *)pnad)->sin_addr) : "loopback");

	Connection *pcon;

	// A NULL pnad means this device is the server and is now trying to
	// connect to itself as a client. Use the LoopbackConnection to satisfy
	// this.

	if (pnad == NULL) {
		pcon = new LoopbackConnection(this);
		Assert(pcon != NULL, "out of memory!");
		if (pcon == NULL)
			return NULL;
		m_fAsyncConnectLoopback = true;

	} else {
		// Create the socket for connecting to the server

		SOCKET soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (soc == INVALID_SOCKET) {
#ifdef DEBUG
			HostMessageBox(TEXT("socket err: 0x%lx"), PszFromSocError());
#endif
			return NULL;
		}

		// Disable nagle algorithm

		int nTrue = 1;
		setsockopt(soc, IPPROTO_TCP, TCP_NODELAY, (NetBuff)&nTrue, sizeof(int));
                
		// Disable excessive 'lingering' (particularly a problem on PalmOS <=5 which has only 16 sockets)

		linger lngr;
		lngr.l_onoff = 1;
		lngr.l_linger = 0;
		setsockopt(soc, SOL_SOCKET, SO_LINGER, (NetBuff)&lngr, sizeof(lngr));

		// Connect to the server
		// UNDONE: On PalmOS <=5 this occasionally times out after 2 secs. Longer would be better

		if (connect(soc, (const sockaddr *)pnad, sizeof(sockaddr_in)) != 0) {
			switch (WSAGetLastError()) {
			case WSAEHOSTUNREACH:
				HtMessageBox(kfMbWhiteBorder, "Comm Problem", "Host unreachable.");
				break;

			case WSAECONNREFUSED:
				HtMessageBox(kfMbWhiteBorder, "Comm Problem", "Connection refused.");
				break;

			case WSAETIMEDOUT:
				HtMessageBox(kfMbWhiteBorder, "Comm Problem", "Timed out trying to connect to host.");
				break;

			default:
				HtMessageBox(kfMbWhiteBorder, "Comm Problem", "Unable to connect. Error %d", WSAGetLastError());
				break;
			}
			closesocket(soc);
			return NULL;
		}

		pcon = NewConnection(soc);
		if (pcon == NULL) {
			closesocket(soc);
			return NULL;
		}
		m_fAsyncConnectLoopback = false;
	}

	AddConnection(pcon);

	Assert(m_pconAsyncConnect == NULL, "Can only have one pending AsyncConnect at a time");
	m_pconAsyncConnect = pcon;

	return pcon;
}

SocConnection *SocTransport::NewConnection(SOCKET soc)
{
	return new SocConnection(soc);
}

void SocTransport::Poll()
{
	if (!IsOpen())
		return;

	// Users of Transport::AsyncConnect rely on it not completing the connection
	// immediately so they have an opportunity to hook the OnConnectComplete
	// callback.

	if (m_pconAsyncConnect != NULL) {
		if (m_fAsyncConnectLoopback)
			((LoopbackConnection *)m_pconAsyncConnect)->Connect();
		IConnectionCallback *pccb = m_pconAsyncConnect->GetCallback();
		if (pccb != NULL) {
			pccb->OnConnectComplete(m_pconAsyncConnect);
		}
		m_pconAsyncConnect = NULL;
	}

	// While a game is being advertised the server accepts client connections

	if (m_socAcceptListen != INVALID_SOCKET) {

		// Test if any client connection requests are pending

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(m_socAcceptListen, &fds);
		TIMEVAL tvTimeout;
		tvTimeout.tv_sec = 0;
		tvTimeout.tv_usec = 0;
		int nSelected = select(((int)m_socAcceptListen) + 1, &fds, NULL, NULL, &tvTimeout);
		if (nSelected == SOCKET_ERROR) {
#ifdef DEBUG
			HostMessageBox(TEXT("select err: %s"), PszFromSocError());
#endif
			if (m_ptcb != NULL)
				m_ptcb->OnTransportError(ktraeAdvertiseGameFailed);
			return;
		}

		// Accept a client connection and produce a new socket for communicating with that client

		if (nSelected == 1) {
			NetAddress nad;
			memset(&nad, 0, sizeof(nad));
#ifdef IPHONE
            socklen_t cbAddr = sizeof(nad);
#else
			int cbAddr = sizeof(nad);
#endif
			SOCKET socConn = accept(m_socAcceptListen, (sockaddr *)&nad, &cbAddr);
			if (socConn == INVALID_SOCKET) {
				// This error is seen on a Tungsten C hosting a game when a ux50 
				// times out while trying to connect. The best thing to do is just
				// ignore the connection attempt. Hopefully it will try again and
				// get it right this time

				if (WSAGetLastError() == 0)
					return;
#ifdef DEBUG
				HostMessageBox(TEXT("accept err: %s"), PszFromSocError());
#endif
				return;
			}
//			HostMessageBox("cbAddr = %d, sizeof(sockaddr_in) = %d", cbAddr, sizeof(sockaddr_in));
//			Assert(cbAddr == sizeof(sockaddr_in));

			Connection *pcon = new SocConnection(socConn);
			Assert(pcon != NULL, "out of memory!");
			if (pcon == NULL) {
				closesocket(socConn);
				return;
			}
			AddConnection(pcon);

			if (m_ptcb != NULL) {
				if (!m_ptcb->OnClientConnect(pcon))
					delete pcon;
			}
		}
	}

	Transport::Poll();
}

// BeginGameSearch is called when the JoinOrHostMultiplayer form enters its
// modal loop. EndGameSearch is called when the form is destructed.

// Winsock GameSearch
// - create a non-blocking datagram socket to listen for SERVERINFO broadcasts
// - poll the socket to check for SERVERINFO broadcasts every 1/10th of a second
// - for each SERVERINFO broadcast received call the Transport's registered
//   ITransportCallback::OnReceive method

bool SocTransport::BeginGameSearch()
{
	// Already searching?

	if (m_socBroadcastListen != INVALID_SOCKET)
		return false;

	m_fBlockNextGameHost = false;

	m_socBroadcastListen = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socBroadcastListen == INVALID_SOCKET) {
#ifdef DEBUG
		HostMessageBox(TEXT("socket err: %s"), PszFromSocError());
#endif
		HtMessageBox(kfMbWhiteBorder, "Comm Problem", "Failed to open network.");
		return false;
	}

	// Set the socket to non-blocking because we're going to poll it
	// every 1/10th of a second.

	unsigned long ulT = 1;
#if defined(PIL)
	// I think the NetSocket.c fcntl simulator routine (NetFCntl) is screwed
	// up, using the wrong constants and not conforming to its own documentation.
	// But for now I'll go with the flow...

	if (fcntl(m_socBroadcastListen, F_SETFL, FNDELAY) == SOCKET_ERROR) {
#elif defined(IPHONE)
    if (fcntl(m_socBroadcastListen, F_SETFL, fcntl(m_socBroadcastListen, F_GETFL, 0) | O_NONBLOCK) == -1) {
#else
	if (ioctlsocket(m_socBroadcastListen, FIONBIO, &ulT) == SOCKET_ERROR) {
#endif
#ifdef DEBUG
		HostMessageBox(TEXT("socket err: %s"), PszFromSocError());
#endif
		HtMessageBox(kfMbWhiteBorder, "Comm Problem", "Network error.");
		return false;
	}

    sockaddr_in soca;
	soca.sin_family = AF_INET;
//	soca.sin_addr.S_un.S_addr = htonl(m_dwIpAddress);
        
#ifdef IPHONE
	soca.sin_addr.s_addr = htonl(INADDR_ANY);        
#else
	soca.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#endif

	// The server broadcasts across 4 ports so we can run up to 4 clients on
	// the same machine without conflict. Loop through all the ports until
	// we find a free one (one not taken already by another running client).

	int i;
	for (i = 0; i < 4; i++) {
		soca.sin_port = htons(SERVERINFO_BROADCAST_PORT + i);
		if (bind(m_socBroadcastListen, (sockaddr *)&soca, sizeof(soca)) != SOCKET_ERROR)
			break;
	}

#ifdef DEBUG
	if (i == 4)
		HostMessageBox(TEXT("bind err: %s"), PszFromSocError());
#endif

	// Start timer

	gtimm.AddTimer(this, 10);	// Every 100ms, i.e., 1/10 second
	return true;
}

void SocTransport::EndGameSearch()
{
	if (m_socBroadcastListen == INVALID_SOCKET)
		return;

	// Stop timer

	gtimm.RemoveTimer(this);

	closesocket(m_socBroadcastListen);
	m_socBroadcastListen = INVALID_SOCKET;
}

void SocTransport::NextGameHost()
{
	m_fBlockNextGameHost = false;
}

// AdvertiseGame must clean up after itself if it fails, i.e.,
// do not assume that UnadvertiseGame will be called.

bool SocTransport::AdvertiseGame(const char *pszGameName)
{
	int err;

	// Remember the game name we'll be broadcasting

	strncpyz(m_szGameName, (char *)pszGameName, sizeof(m_szGameName));

	// Create a socket to broadcast through

	m_socBroadcast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socBroadcast == INVALID_SOCKET) {
#ifdef DEBUG
		HostMessageBox(TEXT("socket err: %s"), PszFromSocError());
#endif
		return false;
	}
	int broadcast = 1;
	setsockopt(m_socBroadcast, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof(broadcast));

	sockaddr_in soca;
	memset(&soca, 0, sizeof(soca));
	soca.sin_family = AF_INET;
//	soca.sin_addr.S_un.S_addr = htonl(m_dwIpAddress);
    
#ifdef IPHONE
	soca.sin_addr.s_addr = htonl(INADDR_ANY);    
#else
	soca.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#endif
	err = bind(m_socBroadcast, (sockaddr *)&soca, sizeof(soca));
	if (err == SOCKET_ERROR) {
#ifdef DEBUG
		HostMessageBox(TEXT("socket err: %s"), PszFromSocError());
#endif
		closesocket(m_socBroadcast);
		m_socBroadcast = INVALID_SOCKET;
		return false;
	}

	// Create the socket for listening to client connection requests

	m_socAcceptListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socAcceptListen == INVALID_SOCKET) {
#ifdef DEBUG
		HostMessageBox(TEXT("socket err: %s"), PszFromSocError());
#endif
		if (m_socBroadcast != INVALID_SOCKET) {
			closesocket(m_socBroadcast);
			m_socBroadcast = INVALID_SOCKET;
		}
		return false;
	}

	// Disable nagle algorithm -- should be inherited by all accepted sockets

	int nTrue = 1;
	setsockopt(m_socAcceptListen, IPPROTO_TCP, TCP_NODELAY, (NetBuff)&nTrue, sizeof(int));

	// Bind the listening socket to the local address and the connection port

	NetAddress nad;
	GetLocalNetAddress(&nad);
	((sockaddr_in *)&nad)->sin_port = htons(SERVER_LISTEN_PORT);
    
#ifdef IPHONE
	((sockaddr_in *)&nad)->sin_addr.s_addr = htons(INADDR_ANY);    
#else
	((sockaddr_in *)&nad)->sin_addr.S_un.S_addr = htons(INADDR_ANY);
#endif
    
//	HostMessageBox("addr %s, fam %d, port %d", inet_ntoa(((sockaddr_in *)&nad)->sin_addr), ((sockaddr_in *)&nad)->sin_family, ntohs(((sockaddr_in *)&nad)->sin_port));
	err = bind(m_socAcceptListen, (const sockaddr *)&nad, sizeof(sockaddr_in));
	if (err == SOCKET_ERROR) {
#ifdef DEBUG
		HostMessageBox(TEXT("bind err: %s"), PszFromSocError());
#endif
		if (m_socBroadcast != INVALID_SOCKET) {
			closesocket(m_socBroadcast);
			m_socBroadcast = INVALID_SOCKET;
		}
		closesocket(m_socAcceptListen);
		m_socAcceptListen = INVALID_SOCKET;
		return false;
	}

	// Prepare for client connections

	err = listen(m_socAcceptListen, SOMAXCONN);
	if (err == SOCKET_ERROR) {
#ifdef DEBUG
		HostMessageBox(TEXT("listen err: %s"), PszFromSocError());
#endif
		if (m_socBroadcast != INVALID_SOCKET) {
			closesocket(m_socBroadcast);
			m_socBroadcast = INVALID_SOCKET;
		}
		closesocket(m_socAcceptListen);
		m_socAcceptListen = INVALID_SOCKET;
		return false;
	}

	if (m_socBroadcast != INVALID_SOCKET) {
		// Start timer

		gtimm.AddTimer(this, 50);	// Every 500ms, i.e., 1/2 second

		// Broadcast game availability immediately

		OnTimer(0);
	}
	return true;
}

void SocTransport::UnadvertiseGame()
{
	// Close connection accepting socket

	closesocket(m_socAcceptListen);
	m_socAcceptListen = INVALID_SOCKET;

	// Close broadcast socket

	if (m_socBroadcast != INVALID_SOCKET) {
		closesocket(m_socBroadcast);
		m_socBroadcast = INVALID_SOCKET;

		// Stop timer

		gtimm.RemoveTimer(this);
	}
}

void SocTransport::OnTimer(long tCurrent)
{
	// If we're Advertising broadcast a SERVERINFO message

	if (m_socBroadcast != INVALID_SOCKET) {
		sockaddr_in soca;
		soca.sin_family = AF_INET;
        
#ifdef IPHONE
		soca.sin_addr.s_addr = INADDR_BROADCAST;        
#else
		soca.sin_addr.S_un.S_addr = INADDR_BROADCAST;
#endif

		ServerInfoNetMessage nm(m_szGameName);
		MpTrace("> %s", PszFromNetMessage(&nm));

		// Broadcast to 4 ports to support running 4 clients on the same machine

		for (int i = 0; i < 4; i++) {
			soca.sin_port = htons(SERVERINFO_BROADCAST_PORT + i);
			if (sendto(m_socBroadcast, (char *)&nm, sizeof(nm), 0, (sockaddr *)&soca, sizeof(soca)) == SOCKET_ERROR) {
#ifdef DEBUG
				HostMessageBox(TEXT("sendto err: %s"), PszFromSocError());
#endif
			}
		}
	}

	// If we're searching for hosts check for receipt of a SERVERINFO message

	if (m_socBroadcastListen != INVALID_SOCKET) {
		NetAddress nad;
        
#ifdef IPHONE
		socklen_t cbSocaServer = sizeof(nad);        
#else
		int cbSocaServer = sizeof(nad);
#endif
		ServerInfoNetMessage sinm;
		if (recvfrom(m_socBroadcastListen, (char *)&sinm, sizeof(sinm), 0, (sockaddr *)&nad, &cbSocaServer) == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err != WSAEWOULDBLOCK && err != WSAETIMEDOUT) {
#ifdef DEBUG
				HostMessageBox(TEXT("recvfrom err: %s"), PszFromSocError(err));

				// UNDONE: If we discover any errors that would cause us to want to abort the
				// game search operation...

//				if (m_ptcb != NULL)
//					m_ptcb->OnTransportError(ktraeGameSearchFailed);

#endif
				return;
			}
		} else {
			MpTrace("< GAMEHOSTFOUND");

			if (m_fBlockNextGameHost)
				return;

			// Fill in a NetAddress ourselves so we can be sure no random numbers are present

			NetAddress nadT;
			memset(&nadT, 0, sizeof(nadT));
			// HACK: this is the port the game is going to want to connect to
			((sockaddr_in *)&nadT)->sin_port = htons(SERVER_LISTEN_PORT);
			((sockaddr_in *)&nadT)->sin_family = ((sockaddr_in *)&nad)->sin_family;
			((sockaddr_in *)&nadT)->sin_addr = ((sockaddr_in *)&nad)->sin_addr;

			if (m_ptcb != NULL) {
				m_fBlockNextGameHost = true;
				m_ptcb->OnGameHostFound(&nadT);
			}
		}
	}
}

bool SocTransport::CanSpecifyAddress()
{
	return true;
}

bool ValidateIpAddress(const char *psz) secComm;
bool ValidateIpAddress(const char *psz)
{
	dword dwIpAddress = inet_addr(psz);
	if (dwIpAddress == INADDR_NONE) {
		HtMessageBox(kfMbWhiteBorder, "Input Error", "Invalid Internet address.");
		return false;
	}

	return true;
}

bool SocTransport::SpecifyAddress(NetAddress *pnad)
{
#if 0
	static char *s_apszChars[] = {
		"ABCDEFGHIJKLM",
		"NOPQRSTUVWXYZ",
		"0123456789.-",
	};
#else
	static char *s_apszChars[] = {
		"123",
		"456",
		"789",
		".0"
	};
#endif

	char szIpAddress[32];
	if (!DoInputPanelForm(s_apszChars, ARRAYSIZE(s_apszChars), "Address:", "", szIpAddress, sizeof(szIpAddress), ValidateIpAddress))
		return false;

	dword nlIpAddress = inet_addr(szIpAddress);

	sockaddr_in *psoca = (sockaddr_in *)pnad;
	psoca->sin_family = AF_INET;
	psoca->sin_port = htons(SERVER_LISTEN_PORT);
    
#ifdef IPHONE
	psoca->sin_addr.s_addr = nlIpAddress;    
#else
	psoca->sin_addr.S_un.S_addr = nlIpAddress;
#endif

	return true;
}

//---------------------------------------------------------------------------
// SocConnection implementation

SocConnection::SocConnection()
{
	m_soc = INVALID_SOCKET;

	m_fListening = false;
	m_fDisconnecting = false;
}

SocConnection::SocConnection(SOCKET soc)
{
	m_soc = soc;

	m_fListening = false;
	m_fDisconnecting = false;
}

SocConnection::~SocConnection()
{
	if (m_soc != INVALID_SOCKET)
		closesocket(m_soc);
	gptra->RemoveConnection(this);
}

bool SocConnection::Poll()
{
	if (m_fDisconnecting) {
		m_fDisconnecting = false;
		if (m_pccb != NULL)
			m_pccb->OnDisconnect(this);
		return false;
	}

	if (m_soc == INVALID_SOCKET)
		return false;

	// Test if any incoming data is pending

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_soc, &fds);
	TIMEVAL tvTimeout;
	tvTimeout.tv_sec = 0;
	tvTimeout.tv_usec = 0;
	fd_set fdsDummy;
	FD_ZERO(&fdsDummy);
//	MpTrace("m_soc = %d, fds = %ld", (UInt16)m_soc, (UInt32)fds);
	int nSelected = select(((int)m_soc) + 1, &fds, &fdsDummy, &fdsDummy, &tvTimeout);

	if (nSelected == SOCKET_ERROR) {
#ifdef DEBUG
		HostMessageBox(TEXT("select err: %s"), PszFromSocError());
#endif
		return false;
	}

	if (nSelected == 1) {
		NetMessage nm;
		int cb = recv(m_soc, (char *)&nm, sizeof(NetMessage), 0);
//		MpTrace("recv: cb %d", cb);
		if (cb != sizeof(NetMessage)) {
			HandleRecvError();
			return false;
		}
		int cbT = BigWord(nm.cb);
		NetMessage *pnm = (NetMessage *)new byte[cbT];
		if (pnm == NULL) {
			// Data is still pending but we can't do anything about it.
			// Follow the usual course of action (HandleRecvError will close the Connection)

			HandleRecvError();
			return false;
		}
		memcpy(pnm, &nm, sizeof(NetMessage));
		int cbRemaining = cbT - sizeof(NetMessage);

		// UNDONE: this now is a blocking operation until all of the message is
		// received. Better would be to accumulate the pieces of the message in
		// a temp buffer (SocConnection member) until it is complete, in a non-
		// blocking fashion.

		byte *pbT = (byte *)(pnm + 1);
		while (cbRemaining != 0) {
			cbT = recv(m_soc, (char *)pbT, cbRemaining, 0);
			if (cbT == SOCKET_ERROR) {
				int err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK) {
					delete pnm;
					HandleRecvError();
					return false;
				}
			}
			pbT += cbT;
			cbRemaining -= cbT;
		}

		if (m_pccb != NULL) {
			// Before calling OnReceive, order in native byte order

			NetMessageByteOrderSwap(pnm, false);

			MpTrace("< %s", PszFromNetMessage(pnm));
			m_pccb->OnReceive(this, pnm);
		}
		delete pnm;
	}

	return true;
}

void SocConnection::HandleRecvError()
{
	// Pretty much any error is fatal to a Connection but we'll start
	// by casing them out one by one so everything is considered

	int err = WSAGetLastError();

	// On CE.NET when a peer terminates ungracefully recv will return partial
	// messages. We detect this and treat it like any other fatal error.

	if (err < WSABASEERR)
		err = 0;

	switch (err) {
	case 0:					// When the peer gracefully closes its socket
	case WSAECONNRESET:		// When the peer crashes, etc (ungraceful termination)
	case WSAECONNABORTED:
	case netErrSocketClosedByRemote: // Couldn't map this to WSACONRESET or WSACONABORTED because they're already mapped to other Palm errors
		AsyncDisconnect();
		break;

	// Treat all errors as fatal to the Connection and Disconnect

	default:
#ifdef DEBUG
		HostMessageBox(TEXT("recv err: %s"), PszFromSocError(err));
#endif
		AsyncDisconnect();
		break;
	}
}

void SocConnection::HandleSendError()
{
	// Pretty much any error is fatal to a Connection but we'll start
	// by casing them out one by one so everything is considered

	int err = WSAGetLastError();
	switch (err) {
	case WSAECONNRESET:
		AsyncDisconnect();
		break;

	// Treat all errors as fatal to the Connection and Disconnect

	default:
#ifdef DEBUG
		HostMessageBox(TEXT("send err: %s"), PszFromSocError(err));
#endif
		AsyncDisconnect();
		break;
	}
}

bool SocConnection::AsyncSend(NetMessage *pnm)
{
	if (m_soc == INVALID_SOCKET)
		return false;

	MpTrace("> %s", PszFromNetMessage(pnm));

	// Before sending, order in network byte order

	int cb = pnm->cb;	// nab this before byte-swapping it!

	NetMessageByteOrderSwap(pnm, true);

	// UNDONE: this is synchronous
	int cbActual = send(m_soc, (NetBuff)pnm, cb, 0);

	// Swap it back in case of reuse!

	NetMessageByteOrderSwap(pnm, false);

	if (cbActual != cb) {
		HandleSendError();
		return false;
	}

	return true;
}

void SocConnection::AsyncDisconnect()
{
	if (m_soc == INVALID_SOCKET)
		return;

	shutdown(m_soc, SD_SEND);
	closesocket(m_soc);
	m_soc = INVALID_SOCKET;
	
	// Callers assume that the disconnect notification will be async
	// so we do a little extra work to satisfy this.

	m_fDisconnecting = true;
}

// Error strings for Socket errors

#ifdef DEBUG
const TCHAR *PszFromSocError()
{
	return PszFromSocError(WSAGetLastError());
}
#endif

} // namespace wi
