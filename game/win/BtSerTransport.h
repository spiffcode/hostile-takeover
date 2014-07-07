class BtSerConnection;

class BtSerTransport: public Transport, Timer // tra
{
	friend BtSerConnection;

public:
	static int GetTransportDescriptions(TransportDescription *atrad, int ctradMax);
	static Transport *Open(TransportDescription *ptrad);

	BtSerTransport(dword dwPorts);
	virtual bool Open();
	virtual void Close();
	virtual Connection *NewConnection();

	virtual bool BeginGameSearch();
	virtual void EndGameSearch();
	virtual bool AdvertiseGame(const char *pszGameName);
	virtual void UnadvertiseGame(bool fRetainConnections);

	// Timer method

	virtual void OnTimer(long tCurrent);

private:
#if 0
	word GetInboundPort() {
		return m_wInboundPort;
	}

	word GetOutboundPort() {
		return m_wOutboundPort;
	}
#endif

	HANDLE OpenInboundPort();
	void CloseInboundPort();
	HANDLE OpenOutboundPort();
	void CloseOutboundPort();

private:
	char m_szGameName[kcbGameName];
	word m_wInboundPort;
	word m_wOutboundPort;
	HANDLE m_hfOutbound;
	HANDLE m_hfInbound;
	int m_cInboundPortRef;
	int m_cOutboundPortRef;
	word m_wf;
};

const word kfBstAdvertisingGame = 0x0001;
const word kfBstSearchingForGames = 0x0002;

class BtSerConnection : public Connection
{
public:
	BtSerConnection();
	BtSerConnection(HANDLE hf, bool fInbound = false);
	~BtSerConnection();
	void HandleRecvError();
	void HandleSendError();

	// Connection overrides

	virtual bool Poll();
	virtual bool AsyncListen();
	virtual bool AsyncConnect(NetAddress *pnad);
	virtual bool AsyncSend(NetMessage *pnm);
	virtual void Disconnect();

private:
	HANDLE m_hf;
	bool m_fListening;
	bool m_fInbound;
};
