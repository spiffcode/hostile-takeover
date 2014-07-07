#include <winsock.h>
#include "..\SocTransport.h"

class WsTransport : public SocTransport // tra
{
public:
	static int GetTransportDescriptions(TransportDescription *atrad, int ctradMax);
	static Transport *Open(TransportDescription *ptrad);
	WsTransport(dword dwIpAddress);

	// Transport implementation

	virtual bool Open();
	virtual void Close();
};
