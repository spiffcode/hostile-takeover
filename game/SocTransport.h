// It would be better if there were a socket abstraction, that Transport used.
// This way, these platform differences would be kept in the socket
// implementations.

#ifdef PIL
// On Palm we try to make things look like they do on Windows so we
// can share code. Conveniently Palm provides mappings to Berkeley
// Sockets which gets us most the way there.

// Map non-Berkeley stuff

extern "C" {
#include <palm_socket.h>
}

#define SOCKET NetSocketRef
#define HOSTENT hostent
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define WSAGetLastError() errno
#define WSASetLastError(err) (errno = (err))
#define closesocket(soc) close(soc)
#define TIMEVAL timeval
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEMSGSIZE EMSGSIZE
#define WSABASEERR 0
#define WSAECONNRESET ECONNRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAETIMEDOUT netErrTimeout
#define WSAEHOSTUNREACH netErrUnreachableDest
#define WSAECONNREFUSED ECONNREFUSED
#define TRUE 1

namespace wi {
typedef void *NetBuff;
}
#endif

#if defined(IPHONE)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#define SOCKET int
#define HOSTENT hostent
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define WSAGetLastError() errno
#define WSASetLastError(err) (errno = (err))
#define closesocket(soc) close(soc)
#define TIMEVAL timeval
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEMSGSIZE EMSGSIZE
#define WSABASEERR 0
#define WSAECONNRESET ECONNRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAETIMEDOUT ETIMEDOUT
#define WSAEHOSTUNREACH EHOSTUNREACH
#define WSAECONNREFUSED ECONNREFUSED
#define TRUE 1
#define SD_SEND SHUT_WR
#define netErrSocketClosedByRemote -1 // this happens when read() returns 0
namespace wi {
typedef const char *NetBuff;
}
#endif

#if defined(WIN) || defined(CE)
#include <winsock.h>
#define netErrSocketClosedByRemote (WSABASEERR - 1)	// won't happen

namespace wi {
typedef const char *NetBuff;
}
#endif

namespace wi {

#define SERVER_LISTEN_PORT	22222	// 0x56ce
#define SERVERINFO_BROADCAST_PORT 22223	// 22223-22226
#define kcbSendBufferMax	8192

class SocConnection;

class SocTransport : public Transport, Timer // tra
{
public:
	SocTransport(dword dwIpAddress) secComm;
	virtual ~SocTransport() secComm;

	// Transport implementation

//	virtual bool Open();
//	virtual void Close();
	virtual Connection *AsyncConnect(NetAddress *pnad) secComm;
	virtual void Poll() secComm;

	virtual bool BeginGameSearch() secComm;
	virtual void NextGameHost() secComm;
	virtual void EndGameSearch() secComm;
	virtual bool AdvertiseGame(const char *pszGameName) secComm;
	virtual void UnadvertiseGame() secComm;

	virtual bool CanSpecifyAddress() secComm;
	virtual bool SpecifyAddress(NetAddress *pnad) secComm;
	virtual void GetAddressString(char *pszAddress, int cbMax) secComm;

	// Timer method

	virtual void OnTimer(long tCurrent) secComm;

	//

protected:
	virtual SocConnection *NewConnection(SOCKET soc) secComm;

protected:
	dword m_dwIpAddress;

private:
	bool GetLocalNetAddress(NetAddress *pnad) secComm;

	char m_szGameName[kcbGameName];
	SOCKET m_socBroadcast;
	SOCKET m_socBroadcastListen;
	SOCKET m_socAcceptListen;

	Connection *m_pconAsyncConnect;
	bool m_fAsyncConnectLoopback;
	bool m_fBlockNextGameHost;
};

class SocConnection : public Connection
{
public:
	SocConnection() secComm;
	SocConnection(SOCKET soc) secComm;
	~SocConnection() secComm;

	// Connection implementation

	virtual bool Poll() secComm;
	virtual bool AsyncSend(NetMessage *pnm) secComm;
	virtual void AsyncDisconnect() secComm;

private:
	void HandleRecvError() secComm;
	void HandleSendError() secComm;

	bool m_fListening;
	bool m_fDisconnecting;

protected:
	SOCKET m_soc;
};

} // namespace wi