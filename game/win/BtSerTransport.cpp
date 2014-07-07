#include "..\ht.h"
#include "..\Multiplayer.h"
#include <BtSerTransport.h>
#include <TCHAR.H>
#ifndef CE
#include <setupapi.h>
#include <devguid.h>
#include <WinIoCtl.h>
#endif

// The 'inbound' port must be used to 'Advertise' availability of this device 
// for others to connect to it. The 'outbound' port, when used, will invoke
// "Bluetooth Browser" UI for connecting to another device.

int BtSerTransport::GetTransportDescriptions(TransportDescription *atrad, int ctradMax)
{
	// Make sure the BT com port is there and available

	word wInboundPort = 0xffff, wOutboundPort = 0xffff;

#ifdef CE
	HKEY hkey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Drivers\\BuiltIn"), 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
		return 0;

	int i = 0;
	while (true) {
		TCHAR tszT[100];
		FILETIME ftT;
		DWORD cbtszT = sizeof(tszT);
		if (RegEnumKeyEx(hkey, i++, (TCHAR *)&tszT, &cbtszT, NULL, NULL, NULL, &ftT) != ERROR_SUCCESS)
			break;

		HKEY hkeyDevice;
		if (RegOpenKeyEx(hkey, tszT, 0, KEY_ALL_ACCESS, &hkeyDevice) != ERROR_SUCCESS)
			break;

		// Look for devices with "COM" prefixes

		TCHAR tszPrefix[50];
		DWORD cbT = sizeof(tszPrefix);
		if (RegQueryValueEx(hkeyDevice, TEXT("Prefix"), 0, NULL, (byte *)tszPrefix, &cbT) != ERROR_SUCCESS) {
			RegCloseKey(hkeyDevice);
			continue;
		}
		if (_tcscmp(tszPrefix, TEXT("COM")) != 0) {
			RegCloseKey(hkeyDevice);
			continue;
		}

		// Get this COM device's index

		DWORD dwIndex;
		cbT = sizeof(DWORD);
		if (RegQueryValueEx(hkeyDevice, TEXT("Index"), 0, NULL, (byte *)&dwIndex, &cbT) != ERROR_SUCCESS) {
			RegCloseKey(hkeyDevice);
			continue;
		}

#if 0 // don't do this because it can make UI ("Bluetooth Browser") pop up
		// See if we can open this port

		TCHAR tszComPort[20];
		_stprintf(tszComPort, TEXT("COM%d:"), dwIndex);
		HANDLE hf = CreateFile(tszComPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (hf == INVALID_HANDLE_VALUE) {
			RegCloseKey(hkeyDevice);
			continue;
		}
		CloseHandle(hf);
#endif

		// Get the device's friendly name

		TCHAR tszFriendly[100];
		cbT = sizeof(tszFriendly);
		if (RegQueryValueEx(hkeyDevice, TEXT("FriendlyName"), 0, NULL, (byte *)tszFriendly, &cbT) != ERROR_SUCCESS) {
//			sprintf(ptrad->szName, "COM%d", dwIndex);
			RegCloseKey(hkeyDevice);
			continue;

		}

		RegCloseKey(hkeyDevice);

		// Is this a Bluetooth port
		// UNDONE: friendly names are localized?

		_tcsupr(tszFriendly);
		if (_tcsstr(tszFriendly, TEXT("BLUETOOTH")) != NULL) {

			// Assign first Bluetooth port as the inbound port, second as the outbound port

			if (wInboundPort == 0xffff)
				wInboundPort = (word)dwIndex;
			else if (wOutboundPort == 0xffff)
				wOutboundPort = (word)dwIndex;
		}
	}

	RegCloseKey(hkey);

#else
	HDEVINFO hdevi = INVALID_HANDLE_VALUE;
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

	// Let's take a look at com ports

	hdevi = SetupDiGetClassDevs(&GUID_CLASS_COMPORT, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hdevi == INVALID_HANDLE_VALUE) {
		Assert("SetupDiGetClassDevs failed. (err=%lx)", GetLastError());
		return 0;
	}

	// Enumerate the serial ports

	SP_DEVICE_INTERFACE_DATA difd;
	DWORD dwDetDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
	pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[dwDetDataSize];

	// This is required, according to the documentation. Yes, it's weird.

	difd.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	for (int i = 0; true; i++) {
		if (!SetupDiEnumDeviceInterfaces(hdevi, NULL, &GUID_CLASS_COMPORT, i, &difd)) {
			DWORD err = GetLastError();
			if (err != ERROR_NO_MORE_ITEMS)
				Assert("SetupDiEnumDeviceInterfaces failed. (err=%lx)", err);
			break;
		}

		SP_DEVINFO_DATA devdata = { sizeof(SP_DEVINFO_DATA) };
		if (!SetupDiGetDeviceInterfaceDetail(hdevi, &difd, pDetData, dwDetDataSize, NULL, &devdata)) {
			Assert("SetupDiGetDeviceInterfaceDetail failed. (err=%lx)", GetLastError());
			break;
		}

		// Got a path to the device. Try to get some more info.

//		TCHAR szFriendly[256];
//		BOOL fSuccess = SetupDiGetDeviceRegistryProperty(hdevi, &devdata, SPDRP_FRIENDLYNAME, NULL, (PBYTE)szFriendly, sizeof(szFriendly), NULL);
		TCHAR szDesc[256];
		BOOL fSuccess = SetupDiGetDeviceRegistryProperty(hdevi, &devdata, SPDRP_DEVICEDESC, NULL, (PBYTE)szDesc, sizeof(szDesc), NULL);
		if (!fSuccess)
			continue;

		// Is this a Bluetooth port?

		strupr(szDesc);
		if ((strstr(szDesc, "BLUETOOTH")) != NULL) {

			// Get its PortName

			HKEY hkey = SetupDiOpenDevRegKey(hdevi, &devdata, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_ALL_ACCESS);
			if (hkey == NULL)
				continue;

			TCHAR szPortName[256];
			DWORD cbT = sizeof(szPortName);
			if (RegQueryValueEx(hkey, "PortName", 0, NULL, (byte *)szPortName, &cbT) != ERROR_SUCCESS) {
				RegCloseKey(hkey);
				continue;
			}

			RegCloseKey(hkey);

			// Assign first Bluetooth port as the inbound port, second as the outbound port

			if (wInboundPort == 0xffff)
				wInboundPort = (word)(szPortName[3] - '0');
			else if (wOutboundPort == 0xffff)
				wOutboundPort = (word)(szPortName[3] - '0');
			strupr(szPortName);
		}
	}

	delete pDetData;
	SetupDiDestroyDeviceInfoList(hdevi);
#endif

	if (wInboundPort == 0xffff || wOutboundPort == 0xffff)
		return 0;
	atrad[0].dwTransportSpecific = ((DWORD)wInboundPort << 16) | wOutboundPort;
	atrad[0].trat = ktratBluetoothSer;
	strcpy(atrad[0].szName, "BLUETOOTH SERIAL");
	atrad[0].pfnOpen = BtSerTransport::Open;

	return 1;
}

Transport *BtSerTransport::Open(TransportDescription *ptrad)
{
	Transport *ptra = new BtSerTransport(ptrad->dwTransportSpecific);
	if (ptra == NULL)
		return NULL;

	if (!ptra->Open())
		return NULL;
	return ptra;
}

BtSerTransport::BtSerTransport(dword dwPorts)
{
	m_wInboundPort = (word)(dwPorts >> 16);
	m_wOutboundPort = (word)(dwPorts & 0xffff);
	m_hfInbound = NULL;
	m_hfOutbound = NULL;
	m_cInboundPortRef = m_cOutboundPortRef = 0;
	m_wf = 0;
}

bool BtSerTransport::Open()
{
	// Nothing to do

	return Transport::Open();
}

void BtSerTransport::Close()
{
	Transport::Close();	// Closes any open Connections
}

HANDLE BtSerTransport::OpenInboundPort()
{
	if (m_hfInbound != NULL) {
		m_cInboundPortRef++;
		return m_hfInbound;
	}

	TCHAR tszPort[10];
	_stprintf(tszPort, TEXT("COM%d:"), m_wInboundPort);
	m_hfInbound = CreateFile(tszPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hfInbound == INVALID_HANDLE_VALUE) {
		HtMessageBox(kfMbWhiteBorder, "Communication Error", "Failed to open COM%d (%d).", m_wInboundPort, GetLastError());
		return INVALID_HANDLE_VALUE;
	}

	m_cInboundPortRef = 1;
	return m_hfInbound;
}

void BtSerTransport::CloseInboundPort()
{
	Assert(m_cInboundPortRef > 0);
	m_cInboundPortRef--;
	if (m_cInboundPortRef == 0) {
		CloseHandle(m_hfInbound);
		m_hfInbound = NULL;
	}
}

HANDLE BtSerTransport::OpenOutboundPort()
{
	if (m_hfOutbound != NULL) {
		m_cOutboundPortRef++;
		return m_hfOutbound;
	}

	TCHAR tszPort[10];
	_stprintf(tszPort, TEXT("COM%d:"), m_wOutboundPort);
	m_hfOutbound = CreateFile(tszPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (m_hfOutbound == INVALID_HANDLE_VALUE) {
		HtMessageBox(kfMbWhiteBorder, "Communication Error", "Failed to open COM%d (%d).", m_wOutboundPort, GetLastError());
		return INVALID_HANDLE_VALUE;
	}

	m_cOutboundPortRef = 1;
	return m_hfOutbound;
}

void BtSerTransport::CloseOutboundPort()
{
	Assert(m_cOutboundPortRef > 0);
	m_cOutboundPortRef--;
	if (m_cOutboundPortRef == 0) {
		CloseHandle(m_hfOutbound);
		m_hfOutbound = NULL;
	}
}

Connection *BtSerTransport::NewConnection()
{
	return new BtSerConnection();
}

// BeginGameSearch is called when the JoinOrHostMultiplayer form enters its
// modal loop. EndGameSearch is called when the form is destructed.

// Serial GameSearch 
// - create a non-blocking datagram socket to listen for SERVERINFO broadcasts
// - poll the socket to check for SERVERINFO broadcasts every 1/10th of a second
// - for each SERVERINFO broadcast received call the Transport's registered 
//   ITransportCallback::OnReceive method

bool BtSerTransport::BeginGameSearch()
{
	// Already searching?

	if ((m_wf & kfBstSearchingForGames) != 0)
		return false;

	if (OpenOutboundPort() == INVALID_HANDLE_VALUE)
		return false;

	// UNDONE: set a timeout on the port so future ReadFile's won't block forever?

	// Start timer

	gtimm.AddTimer(this, 10);	// Every 100ms, i.e., 1/10 second

	m_wf |= kfBstSearchingForGames;
	return true;
}

void BtSerTransport::EndGameSearch()
{
	if ((m_wf & kfBstSearchingForGames) == 0)
		return;

	// Stop timer

	gtimm.RemoveTimer(this);

	CloseOutboundPort();
	m_wf &= ~kfBstSearchingForGames;
}

bool BtSerTransport::AdvertiseGame(const char *pszGameName)
{
	// Remember the game name we'll be broadcasting

	strncpyz(m_szGameName, (char *)pszGameName, sizeof(m_szGameName));

	// Create a socket to broadcast through

	if (OpenInboundPort() == INVALID_HANDLE_VALUE)
		return false;

	// Start timer

	gtimm.AddTimer(this, 50);	// Every 500ms, i.e., 1/2 second

	// Broadcast game availability immediately

	OnTimer(0);
	m_wf |= kfBstAdvertisingGame;
	return true;
}

void BtSerTransport::UnadvertiseGame(bool fRetainConnections)
{
	if ((m_wf & kfBstAdvertisingGame) == 0)
		return;

	// Stop timer

	gtimm.RemoveTimer(this);

	// Close broadcast port

	CloseInboundPort();
	m_wf &= ~kfBstAdvertisingGame;
}

void BtSerTransport::OnTimer(long tCurrent)
{
	// If we're Advertising send a SERVERINFO message

	if ((m_wf & kfBstAdvertisingGame) != 0) {
		Assert(m_hfInbound != NULL);

		// Broadcast a notification that this device is hosting a game

		ServerInfoNetMessage sinm(m_szGameName);
		MpTrace("> %s", PszFromNetMessage(&sinm));

		dword cb;
		if (!WriteFile(m_hfInbound, &sinm, sizeof(sinm), &cb, NULL)) {
#ifdef DEBUG
			HostMessageBox(TEXT("WriteFile err: %d"), GetLastError());
#endif
		}
		Assert(cb == sizeof(sinm));
	}
	
	// If we're searching for hosts check for receipt of a SERVERINFO message

	if ((m_wf & kfBstSearchingForGames) != 0) {
		Assert(m_hfOutbound != NULL);

		// Anything coming in on the serial port?

		COMSTAT coms;
		DWORD dwErrors;
		if (!ClearCommError(m_hfOutbound, &dwErrors, &coms)) {
#ifdef DEBUG
			HostMessageBox(TEXT("ClearCommError returned %ld"), dwErrors);
#endif
			return;
		}

		if (coms.cbInQue >= sizeof(ServerInfoNetMessage)) {
			ServerInfoNetMessage sinm;
			dword cb;
			if (!ReadFile(m_hfOutbound, &sinm, sizeof(ServerInfoNetMessage), &cb, NULL)) {
#ifdef DEBUG
				HostMessageBox(TEXT("Failed ReadFile ServerInfoNetMessage (%d)"), GetLastError());
#endif
				return;
			}
			MpTrace("< GAMEHOSTFOUND");

			if (m_ptcb != NULL) {
				NetAddress nad;
				memset(&nad, 0, sizeof(nad));
				m_ptcb->OnGameHostFound(&nad);
			}
		}
	}
}

//---------------------------------------------------------------------------
// BtSerConnection implementation

BtSerConnection::BtSerConnection()
{
	m_hf = NULL;
	m_fListening = false;
	m_fInbound = false;
	gptra->AddConnection(this);
}

BtSerConnection::BtSerConnection(HANDLE hf, bool fInbound)
{
	m_hf = hf;
	m_fListening = false;
	m_fInbound = fInbound;
	gptra->AddConnection(this);
}

BtSerConnection::~BtSerConnection()
{
	Disconnect();
	gptra->RemoveConnection(this);
}

bool BtSerConnection::Poll()
{
	if (m_hf == NULL)
		return false;

	if (m_fListening) {

		// Accept a client connection and produce a new file handle for communicating with that client

		HANDLE hfInbound = ((BtSerTransport *)gptra)->OpenInboundPort();
		if (hfInbound != INVALID_HANDLE_VALUE) {
			Connection *pcon = new BtSerConnection(hfInbound, true);
			if (pcon == NULL)
				return false;

			if (m_pccb != NULL)
				m_pccb->OnConnect(pcon);
		}
	}

	{
		// Test if any incoming data is pending

		COMSTAT coms;
		DWORD dwErrors;
		if (!ClearCommError(m_hf, &dwErrors, &coms)) {
#ifdef DEBUG
			HostMessageBox(TEXT("ClearCommError returned %ld"), dwErrors);
#endif
			return false;
		}

		if (coms.cbInQue >= sizeof(NetMessage)) {
			NetMessage nm;
			dword cb;
			if (!ReadFile(m_hf, &nm, sizeof(NetMessage), &cb, NULL)) {
#ifdef DEBUG
				HostMessageBox(TEXT("Failed ReadFile NetMessage header (%d)"), GetLastError());
#endif
			}
			if (cb != sizeof(NetMessage)) {
				HandleRecvError();
				return false;
			}

			// UNDONE: issue for polymorphic NetMessages?
			int cbT = BigWord(nm.cb);
			NetMessage *pnm = (NetMessage *)new byte[cbT];
			if (pnm == NULL) {
				// Data is still pending but the assumption here is that the
				// caller is going to give up (out of memory) and close the socket
				// UNDONE: is this a good assumption?
				return false;
			}
			memcpy(pnm, &nm, sizeof(NetMessage));
			dword cbRemaining = cbT - sizeof(NetMessage);
			if (cbRemaining != 0) {
				dword cbActual;
				if (!ReadFile(m_hf, (pnm + 1), cbRemaining, &cbActual, NULL)) {
#ifdef DEBUG
					HostMessageBox(TEXT("Failed ReadFile full NetMessage (%d)"), GetLastError());
#endif
				}

				if (cbActual != cbRemaining) {
					delete pnm;
					HandleRecvError();
					return false;
				}
			}

			if (m_pccb != NULL) {
				// Before calling OnReceive, order in native byte order

				NetMessageByteOrderSwap(BigWord(pnm->nmid), pnm, false);

				MpTrace("< %s", PszFromNetMessage(pnm));

				m_pccb->OnReceive(this, pnm);
			}
			delete pnm;
		}
	}

	return true;
}

void BtSerConnection::HandleRecvError()
{
	Disconnect();
}

void BtSerConnection::HandleSendError()
{
	Disconnect();
}

bool BtSerConnection::AsyncListen()
{
	m_hf = ((BtSerTransport *)gptra)->OpenInboundPort();
	if (m_hf == INVALID_HANDLE_VALUE) {
		HtMessageBox(kfMbWhiteBorder, "Communication Error", "Failed to open inbound port.");
		return false;
	}

	m_fListening = true;
	return true;
}

// UNDONE: this implementation is synchronous

bool BtSerConnection::AsyncConnect(NetAddress *pnad)
{
	m_hf = ((BtSerTransport *)gptra)->OpenOutboundPort();
	if (m_hf == INVALID_HANDLE_VALUE) {
		HtMessageBox(kfMbWhiteBorder, "Communication Error", "Failed to connect.");
		return false;
	}

	// UNDONE: OnConnectComplete

	return true;
}

bool BtSerConnection::AsyncSend(NetMessage *pnm)
{
	if (m_hf == NULL)
		return false;

	MpTrace("> %s", PszFromNetMessage(pnm));

	// Before sending, order in network byte order

	int cb = pnm->cb;	// nab this before byte-swapping it!
	NetMessageByteOrderSwap(pnm->nmid, pnm, true);

	// UNDONE: this is synchronous

	dword cbActual;
	if (!WriteFile(m_hf, pnm, cb, &cbActual, NULL)) {
#ifdef DEBUG
		HostMessageBox(TEXT("WriteFile err: %d"), GetLastError());
#endif
	}
	if (cbActual != cb) {
		HandleSendError();
		return false;
	}

	return true;
}

void BtSerConnection::Disconnect()
{
	if (m_hf == NULL)
		return;

	if (m_fInbound)
		((BtSerTransport *)gptra)->CloseInboundPort();
	else
		((BtSerTransport *)gptra)->CloseOutboundPort();
	m_hf = NULL;

	if (m_pccb != NULL)
		m_pccb->OnDisconnect(this);
}
