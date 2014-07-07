/////////////////////////////////////////////////////////////////////////////
//
//  Name        BtIfClasses.h
//  $Header:
//
//  Function    this file contains Widcomm SDK class definitions
//
//  Date                 Modification
//  ----------------------------------
//  12/17/2000    JF   Create
//
//  Copyright (c) 2000-2002, WIDCOMM Inc., All Rights Reserved.
//  Proprietary and confidential.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _BTIFCLASSES_H
#define _BTIFCLASSES_H

#ifdef WIDCOMMSDK_EXPORTS
#define WIDCOMMSDK __declspec(dllexport)
#else
#define WIDCOMMSDK __declspec(dllimport)
#endif

#include "BtIfDefinitions.h"

// Resolve forward references
//
class CSdpDiscoveryRec;


// Ensure alignment across all builds
//
#ifdef _WIN32_WCE //for CE only
    #pragma pack (8)
    #define BT_CHAR TCHAR
    #define BT_CB_LPSTR  LPCTSTR   //char used in call back function
#else //for windows
    #pragma pack (1)
    #define BT_CHAR char
    #define BT_CB_LPSTR  WCHAR *  //char used in call back function
#endif

class CWBtAPI;
//
// Define status values that are returned in the "OnDeviceStatus" callback
//
#define BT_DEVST_DOWN               0           // Device is present, but down
#define BT_DEVST_UP                 1           // Device is present and UP
#define BT_DEVST_ERROR              2           // Device is in error (maybe being removed)
#define BT_DEVST_UNLOADED           3           // Stack is being unloaded

// Define master/slave role code for SwitchRole function
//
typedef enum
{
	NEW_MASTER,		
	NEW_SLAVE
} MASTER_SLAVE_ROLE;


// Define return code for Audio functions
//
typedef enum
{
    AUDIO_SUCCESS,
    AUDIO_UNKNOWN_ADDR, //if the ACL connection is not up
    AUDIO_BUSY,         //if another SCO being set up to the same BD address
    AUDIO_NO_RESOURCES, //if the max SCO limit has been reached
    AUDIO_ALREADY_STARTED,  //connection is already up.
    AUDIO_UNKNOWN_ERROR,
	AUDIO_INVALID_PARAM,
    AUDIO_INVALID_HANDLE = 0xffff
} AUDIO_RETURN_CODE;


//
// Since CE seems to have some problems with dll unloading, 
// define a function that can be called to shut down the SDK
//
WIDCOMMSDK  void WIDCOMMSDK_ShutDown(void);

////////////////////////////////////////////////////////////////////////////
// 
// Define a class for interfacing to the stack
//
class CBtIf_Impl;

class WIDCOMMSDK CBtIf
{
public:
	CBtIf();
	virtual ~CBtIf();

    // Define return code for Bond function
    //
    typedef enum
    {
        SUCCESS,
        ALREADY_BONDED,
	    BAD_PARAMETER,
	    FAIL
    } BOND_RETURN_CODE;

	typedef enum {
		DISCOVERY_RESULT_SUCCESS,
		DISCOVERY_RESULT_CONNECT_ERR,		    // Could not connect to remote device 
		DISCOVERY_RESULT_CONNECT_REJ,           // Remote device rejected the connection 
		DISCOVERY_RESULT_SECURITY,		        // Security failed 
		DISCOVERY_RESULT_BAD_RECORD,            // Remote Service Record Error 
		DISCOVERY_RESULT_OTHER_ERROR			// Other error
	} DISCOVERY_RESULT;

   	typedef enum {
		DEVST_DOWN,		// Device is present, but down
		DEVST_UP,		// Device is present and UP
		DEVST_ERROR,	// Device is in error (maybe being removed)
		DEVST_UNLOADED,	// Stack is being unloaded
		DEVST_RELOADED	// Stack reloaded after being unloaded
	} STACK_STATUS;

    // standard GUID values for common Bluetooth service classes
    static const GUID guid_SERVCLASS_SERVICE_DISCOVERY_SERVER;
    static const GUID guid_SERVCLASS_BROWSE_GROUP_DESCRIPTOR;
    static const GUID guid_SERVCLASS_PUBLIC_BROWSE_GROUP;
    static const GUID guid_SERVCLASS_SERIAL_PORT;    
    static const GUID guid_SERVCLASS_LAN_ACCESS_USING_PPP;
	static const GUID guid_SERVCLASS_PANU;
	static const GUID guid_SERVCLASS_NAP;
	static const GUID guid_SERVCLASS_GN;
    static const GUID guid_SERVCLASS_DIALUP_NETWORKING;
    static const GUID guid_SERVCLASS_IRMC_SYNC;
    static const GUID guid_SERVCLASS_OBEX_OBJECT_PUSH;
    static const GUID guid_SERVCLASS_OBEX_FILE_TRANSFER;
    static const GUID guid_SERVCLASS_IRMC_SYNC_COMMAND;
    static const GUID guid_SERVCLASS_HEADSET;
    static const GUID guid_SERVCLASS_CORDLESS_TELEPHONY;
    static const GUID guid_SERVCLASS_INTERCOM;
    static const GUID guid_SERVCLASS_FAX;
    static const GUID guid_SERVCLASS_HEADSET_AUDIO_GATEWAY;
    static const GUID guid_SERVCLASS_PNP_INFORMATION;
    static const GUID guid_SERVCLASS_GENERIC_NETWORKING;
    static const GUID guid_SERVCLASS_GENERIC_FILETRANSFER;
    static const GUID guid_SERVCLASS_GENERIC_AUDIO;
    static const GUID guid_SERVCLASS_GENERIC_TELEPHONY;
	static const GUID guid_SERVCLASS_BPP_PRINTING;
	static const GUID guid_SERVCLASS_HCRP_PRINTING;
	static const GUID guid_SERVCLASS_SPP_PRINTING;

#ifdef _WIN32_WCE
    typedef enum 
    {
		CONNECT_ALLOW_NONE,
		CONNECT_ALLOW_ALL, 
		CONNECT_ALLOW_PAIRED, 
		CONNECT_ALLOW_FILTERED
	} CONNECT_ALLOW_TYPE;

#endif

    // Application can use this function to start an inquiry.
    BOOL StartInquiry();

    // Application can use this function to stop an inquiry.
    void StopInquiry();

    // Application can use this function to start service discovery
    BOOL StartDiscovery (BD_ADDR p_bda, GUID *p_service_guid);
    
	BOND_RETURN_CODE Bond(BD_ADDR bda, BT_CHAR* pin_code);

	// query if a device is bonded
	BOOL BondQuery(BD_ADDR bda);

	// Remove Bonding
	BOOL UnBond(BD_ADDR bda);

    //Crate AudioConnection
    AUDIO_RETURN_CODE CreateAudioConnection(BD_ADDR bda, BOOL bIsClient, UINT16 *audioHandle);

    //Disconnect AudioConnection
    static AUDIO_RETURN_CODE RemoveAudioConnection(UINT16 audioHandle);

    //audion callback functions
    virtual void OnAudioConnected(UINT16 audioHandle){};
    virtual void OnAudioDisconnect(UINT16 audioHandle){};
    virtual void OnStackStatusChange(CBtIf::STACK_STATUS new_status) {}
    virtual void OnInquiryComplete (BOOL success, short num_responses) {}// {}
    virtual void OnDeviceResponded (BD_ADDR bda, DEV_CLASS devClass, BD_NAME bdName, BOOL bConnected) {} // = 0;
    virtual void OnDiscoveryComplete () {}// = 0;

    // Application can use this function to get list of services on the remote device
    int ReadDiscoveryRecords (BD_ADDR p_bda, int max_size, CSdpDiscoveryRec *p_list, GUID *p_guid_filter = NULL);

	// application can use this function from within the OnDiscoveryComplete callback
	// to find out the discovery results
	CBtIf::DISCOVERY_RESULT GetLastDiscoveryResult(BD_ADDR p_bda, UINT16 *p_num_recs);

	// server should call this method to switch role to master if
	// they want to accept multiple connections
	static BOOL SwitchRole(BD_ADDR bd_addr, MASTER_SLAVE_ROLE new_role);
    
    // sets the public variable m_BdAddr
	BOOL GetLocalDeviceInfo();

	//audio internal function
    typedef void (tAudio_CB) (UINT16);

	//internal use
    static AUDIO_RETURN_CODE CreateAudioConnection(BD_ADDR bda, BOOL bIsClient, UINT16 *audioHandle,
                                                 tAudio_CB *p_conn_cb, tAudio_CB *p_disc_cb);

    BOOL GetLocalServiceName(BT_CHAR *p_ServiceName, int bBuffLen);
    BOOL GetNextLocalServiceName(BT_CHAR *p_ServiceName, int bBuffLen);

    static BOOL SetLinkSupervisionTimeOut(BD_ADDR BdAddr, UINT16 timeout);

    BD_ADDR	m_BdAddr;	// Bluetooth address of local device

#ifdef _WIN32_WCE
    BOOL RadioOn();
    BOOL RadioOff();

    static AUDIO_RETURN_CODE ReadAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteR);
	static AUDIO_RETURN_CODE WriteAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteW);
    void AllowToConnect(CONNECT_ALLOW_TYPE ConnectType);
    void SetDiscoverable(BOOL bDiscoverable);
    BOOL ReloadStack();
#endif
protected:
    void SetOnDeviceStatusCallback();	

private:

    CBtIf_Impl      *m_pImpl;
	
    friend class CBtIfFriend;
    friend class CBtIf_Impl;

    HANDLE GetMutexHandle();
    void   SetMutexHandle(HANDLE hMutex);
    
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CBtIf(const CBtIf & x);
    CBtIf& operator= (const CBtIf & x);
};


////////////////////////////////////////////////////////////////////////////
//
// Define a class to control an L2CAP interface (for a specific PSM)
//
class CL2CapIf_Impl;

class WIDCOMMSDK CL2CapIf  
{
public:
    CL2CapIf();
    ~CL2CapIf();

    // Server should call this method without any parameter
    // to assign a new PSM value, or with a PSM value if it
    // is using a fixed PSM. Client should call this method
    // with PSM found from service discovery
    //
    BOOL    AssignPsmValue (GUID *p_guid, UINT16 psm = 0);

    // Both client and server sides should call this function
    // to register a PSM with L2CAP, once the PSM value is known
    //
    BOOL    Register ();

    // Both client and server sides should call this function
    // to de-register a PSM from L2CAP, when application is exiting
    //
    void    Deregister ();

    // Both client and server MUST call this function to set
    // the security level for connections on the assigned PSM.
    //
	BOOL SetSecurityLevel (BT_CHAR *p_service_name, UINT8 security_level, BOOL is_server);

    // Returns the PSM value currently in use.
    //
    UINT16 GetPsm() ;

private:
    friend class CL2CapConn;
    friend class CL2CapIf_Impl;
    CL2CapIf_Impl *m_pImpl;

    BOOL GetIsReg();
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CL2CapIf(const CL2CapIf & x);
    CL2CapIf& operator= (const CL2CapIf & x); 
};


////////////////////////////////////////////////////////////////////////////
//
// Define a class to control the L2CAP connections (both client and server
//

class Cl2CapConn_Impl;

class WIDCOMMSDK  CL2CapConn
{
public:

    // Construction/destruction
    //
    CL2CapConn ();
    virtual ~CL2CapConn();

    // Public variables
    //
    // BOOL        m_isCongested, Removed, please call GetCongested()
    //UINT16      m_RemoteMtu; Removed, please call GetRemoteMtu()
    //BD_ADDR     m_RemoteBdAddr;   Please call GetRemoteBdAddr()

    BOOL    GetCongested();
    UINT16  GetRemoteMtu();
    void    GetRemoteBdAddr(BD_ADDR bda);

    // Server should call this method to listen for an incoming
    // connection. Client should not call this method.
    //
    BOOL    Listen (CL2CapIf *p_if);

    // Server should call this method to switch role to master if
	// it wants to accept multiple connections
	//
	BOOL	SwitchRole(MASTER_SLAVE_ROLE new_role);

    BOOL    SetLinkSupervisionTimeOut(UINT16 timeout);

    //Crate AudioConnection
    AUDIO_RETURN_CODE CreateAudioConnection(BOOL bIsClient, UINT16 *audioHandle);

    //Disconnect AudioConnection
    AUDIO_RETURN_CODE RemoveAudioConnection(UINT16 audioHandle);

    //audion callback functions
    virtual void OnAudioConnected(UINT16 audioHandle){};
    virtual void OnAudioDisconnect(UINT16 audioHandle){};

    // Server should call this method to accept an incoming
    // connection, after he is notified of that connection.
    // If anything other than the default MTU is desired,
    // it should be passed as a parameter.
    //
    BOOL    Accept (UINT16 desired_mtu = L2CAP_DEFAULT_MTU);

    // Server should call this method to reject an incoming
    // connection, after he is notified of that connection.
    //
    BOOL    Reject (UINT16 reason);

    // Client should call thi smethod to create a connection
    // to a remote device. If anything other than the default 
    // MTU is desired, it should be passed as a parameter
    //
    BOOL    Connect (CL2CapIf *p_if, BD_ADDR p_bd_addr, UINT16 desired_mtu = L2CAP_DEFAULT_MTU);

    // Client or server may call this function to reconfigure
    // an existing connection.
    //
    BOOL    Reconfigure (tL2CAP_CONFIG_INFO *p_cfg);

    // Client or server may call this function to disconnect
    // an existing connection.
    //
    void    Disconnect (void);

    // Client or server may call this function to send data to
    // an existing connection.
    //
    BOOL    Write (void *p_data, UINT16 length, UINT16 *p_len_written);

    // Get Current Connection Statistics
    //
    BOOL GetConnectionStats (tBT_CONN_STATS *p_conn_stats);

    // Server may provide a function to handle incoming connection
    // notifications. Client should not.
    //
    virtual void OnIncomingConnection ();

    // Client may provide a function to handle connection pending
    // notifications.
    //
    virtual void OnConnectPendingReceived (void) {}

    // Client and server may provide a method to be notified
    // when a connection is established.
    //
    virtual void OnConnected() {}

    // Client and server may provide a method to be notified
    // when data is received from the remote side.
    //
    virtual void OnDataReceived (void *p_data, UINT16 length) {}

    // Client and server may provide a method to be notified
    // when a connection becomes congested or uncongested.
    //
    virtual void OnCongestionStatus (BOOL is_congested) {}

    // Client and server may provide a method to be notified
    // when a connection is disconnected.
    //
    virtual void OnRemoteDisconnected (UINT16 reason) {}

#ifdef _WIN32_WCE
	AUDIO_RETURN_CODE ReadAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteR);
	AUDIO_RETURN_CODE WriteAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteW);
#endif


private:
    
    static CL2CapConn   *m_p_first_conn;
    CL2CapConn          *m_p_next_conn;
    void           SetIdle();

    friend class CL2CapFriend;
    friend class CL2CapConn_Impl;
    CL2CapConn_Impl *m_pImpl;
    
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CL2CapConn(const CL2CapConn & x);
    CL2CapConn& operator= (const CL2CapConn & x); 
};



////////////////////////////////////////////////////////////////////////////
//
// Define a class to create and manage SDP service records
//
class CSdpService_Impl;

class WIDCOMMSDK CSdpService
{
public:
    CSdpService();
    virtual ~CSdpService();

    // This function adds a service class ID list to a service record. The service class ID
    // list should be the first attribute added for a service record.
    //
    SDP_RETURN_CODE AddServiceClassIdList (int num_guids, GUID *p_service_guids);

    // This function adds a name field to a service record.
    //
//    SDP_RETURN_CODE AddServiceName (char *p_service_name);
	SDP_RETURN_CODE AddServiceName (BT_CHAR *p_service_name);//CE defs

    // This function adds a profile descriptor list to a service record.
    //
    SDP_RETURN_CODE AddProfileDescriptorList (GUID *p_profile_guid, UINT16 version);

    // This function adds an L2CAP protocol descriptor list to a service record.
    //
    SDP_RETURN_CODE AddL2CapProtocolDescriptor (UINT16 psm);

    // This function adds an RFCOMM protocol descriptor list to a service record.
    //
    SDP_RETURN_CODE AddRFCommProtocolDescriptor (UINT8 scn);

    // This function adds a generic protocol descriptor list to a service record.
    // It should be only needed if the specific RFCOMM and L2CAP functions above
    // do not suffice.
    //
    SDP_RETURN_CODE AddProtocolList (int num_elem, tSDP_PROTOCOL_ELEM *p_elem_list);

    // This function adds the additional sequence of generic protocol descriptor lists to a service record.
    // It should be only needed if the specific RFCOMM and L2CAP functions above
    // do not suffice.
    //
    SDP_RETURN_CODE AddAdditionProtoLists (int num_list_elem, tSDP_PROTO_LIST_ELEM *p_proto_list);

    // This function adds a language base to a service record.
    //
    SDP_RETURN_CODE AddLanguageBaseAttrIDList (UINT16 lang, UINT16 char_enc, UINT16 base_id);

    // This function makes a service record public browsable.
    //
    SDP_RETURN_CODE MakePublicBrowseable (void);

    // This function sets the 'service availability' field of a service record.
    //
    SDP_RETURN_CODE SetAvailability (UINT8 availability);

    // This function adds an attribute to a service record. It is intended
    // to be used to add some other attribute not covered by the existing
    // functions. Note that the parameter should be in Big Endian order.
    //
    SDP_RETURN_CODE AddAttribute (UINT16 attr_id, UINT8 attr_type, UINT32 attr_len, UINT8 *p_val);

    // This function deletes an attribute from a service record.
    //
    SDP_RETURN_CODE DeleteAttribute (UINT16 attr_id);

	// This functions add a list (sequence) for the 'supported formats' attribute
	//
	SDP_RETURN_CODE AddSupportedFormatsList(UINT8 num_formats, 
										UINT8 pDataType[], UINT8 pDataTypeLength[], UINT8 *pDataTypeValue[]);

private:
    
    friend class CSdpService_Impl;
    CSdpService_Impl *m_pImpl;

    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CSdpService(const CSdpService & x);
    CSdpService& operator= (const CSdpService & x); 
};


class CSdpDiscoveryRec_Impl;

class WIDCOMMSDK CSdpDiscoveryRec
{
public:

    CSdpDiscoveryRec();
    ~CSdpDiscoveryRec();

	//char    m_service_name[BT_MAX_SERVICE_NAME_LEN + 1];
	BT_CHAR   m_service_name[BT_MAX_SERVICE_NAME_LEN + 1];


    GUID    m_service_guid;

    BOOL    FindRFCommScn (UINT8 *pScn);

    BOOL    FindL2CapPsm (UINT16 *pPsm);

    BOOL    FindProtocolListElem (UINT16 layer_uuid, tSDP_PROTOCOL_ELEM *p_elem);

    BOOL    FindAdditionalProtocolListElem (UINT16 layer_uuid, tSDP_PROTOCOL_ELEM *p_elem);

    BOOL    FindProfileVersion (GUID *p_profile_guid, UINT16 *p_version);

    BOOL    FindAttribute (UINT16 attr_id, SDP_DISC_ATTTR_VAL *p_val);

private:

    friend class CBtIf;
    friend class CSdpDiscoveryRec_Impl;
    CSdpDiscoveryRec_Impl *m_pImpl;
    void    Create(void *p);
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CSdpDiscoveryRec(const CSdpDiscoveryRec & x);
    CSdpDiscoveryRec& operator= (const CSdpDiscoveryRec & x); 
};


////////////////////////////////////////////////////////////////////////////
//
// Define a class to control the RfComm interface
//
class CRfCommIf_Impl;  

class WIDCOMMSDK CRfCommIf  
{
public:
    CRfCommIf();
    ~CRfCommIf();

    // Server should call this method without any parameter
    // to assign a new SCN value, or with a SCN value if it
    // is using a fixed SCN. Client should call this method
    // with SCN found from service discovery
    //
    BOOL AssignScnValue (GUID *p_service_guid, UINT8 scn = 0);

    // Returns the SCN value currently in use.
    //
    UINT8 GetScn();

    // Both client and server MUST call this function to set
    // the security level for connections on the assigned SCN.
    //
	BOOL SetSecurityLevel (BT_CHAR *p_service_name, UINT8 security_level, BOOL is_server);

    // Server should call this method to switch role to master if
	// it wants to accept multiple connections
	//
	BOOL	SwitchRole(MASTER_SLAVE_ROLE new_role);

private:
    friend class CRfCommIf_Impl;
    CRfCommIf_Impl *m_pImpl;
    
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CRfCommIf(const CRfCommIf & x);
    CRfCommIf& operator= (const CRfCommIf & x); 
};



///////////////////////////////////////////////////////////////////////////////////////
// Define a class to control the RFCOMM connections (both client and server)
//
class CRfCommPort_Impl;

class WIDCOMMSDK  CRfCommPort
{
public:

    // Construction/destruction
    //
    CRfCommPort ();
    virtual ~CRfCommPort();
//
// Define return code for RFCOMM port functions
//
typedef enum
{
    SUCCESS,
    UNKNOWN_ERROR,
    ALREADY_OPENED,         // Client tried to open port to existing DLCI/BD_ADDR
    NOT_OPENED,             // Function called before conn opened, or after closed
    LINE_ERR,               // Line error
    START_FAILED,           // Connection attempt failed
    PAR_NEG_FAILED,         // Parameter negotiation failed, currently only MTU
    PORT_NEG_FAILED,        // Port negotiation failed
    PEER_CONNECTION_FAILED, // Connection ended by remote side
    PEER_TIMEOUT,           
    INVALID_PARAMETER

} PORT_RETURN_CODE;


	// Open the RFComm serial port as a server (i.e. listen for
    // remote side to connect to us).
	//
   PORT_RETURN_CODE  OpenServer (UINT8 scn, UINT16 desired_mtu = RFCOMM_DEFAULT_MTU);

	// Open the RFComm serial port as a client (i.e. initiate
    // the connection).
	//
   PORT_RETURN_CODE  OpenClient (UINT8 scn, BD_ADDR RemoteBdAddr, UINT16 desired_mtu = RFCOMM_DEFAULT_MTU);

   // Close the RFComm serial port
   //
   PORT_RETURN_CODE  Close(void);

   // Check if connection is up, and if so to whom
   //
   BOOL IsConnected (BD_ADDR *p_remote_bdaddr);

   // Set port flow control - application level, not low level
   //
   PORT_RETURN_CODE SetFlowEnabled (BOOL enabled);

   // Set control leads see BtIfDefinitions.h
   //
   PORT_RETURN_CODE SetModemSignal (UINT8 signal);

   // Get control lead status
   //
   PORT_RETURN_CODE GetModemStatus (UINT8 *p_signal);

   // Send an error to the peer
   //
   PORT_RETURN_CODE SendError (UINT8 errors);

   // Purge transmit queue
   //
   PORT_RETURN_CODE Purge (UINT8 purge_flags);

   // Write data
   //
   PORT_RETURN_CODE Write (void *p_data, UINT16 len_to_write, UINT16 *p_len_written);

   // Get Current Connection Statistics
   //
   PORT_RETURN_CODE GetConnectionStats (tBT_CONN_STATS *p_conn_stats);

   // App may provide these functions
   //
   virtual void OnDataReceived (void *p_data, UINT16 len) {}
   virtual void OnEventReceived (UINT32 event_code) {}
   virtual void OnModemSignalChanged (UINT8 signals) {}
   virtual void OnFlowEnabled (BOOL enabled) {}

   BOOL	SwitchRole(MASTER_SLAVE_ROLE new_role);

   //Crate AudioConnection
   AUDIO_RETURN_CODE CreateAudioConnection(BOOL bIsClient, UINT16 *audioHandle);

   //Disconnect AudioConnection
   AUDIO_RETURN_CODE RemoveAudioConnection(UINT16 audioHandle);

   //audion callback functions
   virtual void OnAudioConnected(UINT16 audioHandle){};
   virtual void OnAudioDisconnect(UINT16 aidioHandle){};

   PORT_RETURN_CODE SetLinkSupervisionTimeOut(UINT16 timeout);

#ifdef _WIN32_WCE
	AUDIO_RETURN_CODE ReadAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteR);
	AUDIO_RETURN_CODE WriteAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteW);
#endif

private:

    static CRfCommPort  *m_p_first_port;
    CRfCommPort         *m_p_next_port;

    friend class CRfCommFriend;
    friend class CRfCommPort_Impl;
    CRfCommPort_Impl *m_pImpl;
 
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CRfCommPort(const CRfCommPort & x);
    CRfCommPort& operator= (const CRfCommPort & x); 
};

///////////////////////////////////////////////////////////////////////////////////////
// Define a class to control the FTP client sessions
//
class CFtpClient_Impl;

class WIDCOMMSDK  CFtpClient
{

public:

//
// Define return code for FTP Client functions
//
typedef enum
{
    SUCCESS,                // Operation initiated without error
    OUT_OF_MEMORY,          // Not enough memory to initiate operation
    SECURITY_ERROR,         // Error implementing requested security level
    FTP_RETURN_ERROR,       // FTP-specific error
    NO_BT_SERVER,           // cannot access the local Bluetooth COM server
    ALREADY_CONNECTED,      // Only one connection at a time supported for each instantiated CFtpClient object
    NOT_OPENED,             // Connection must be opened before requesting this operation
    UNKNOWN_RETURN_ERROR    // Any condition other than the above
} FTP_RETURN_CODE;

//
// Define return code for FTP response functions
//
typedef enum
{
    COMPLETED,              // operation completed without error
    BAD_ADDR,               // bad BD_ADDR
    FILE_EXISTS,            // file already exists
    BAD_STATE,              // could not handle request in present state
    BAD_REQUEST,            // invalid request
    NOT_FOUND,              // no such file
    NO_SERVICE,             // could not find the specified FTP server
    DISCONNECT,             // connection lost
    READ,                   // read error
    WRITE,                  // write error
    OBEX_AUTHEN,            // OBEX Authentication required
    DENIED,                 // request could not be honored
    DATA_NOT_SUPPORTED,     // server does not support the requested data
    CONNECT,                // error establishing connection
    PERMISSIONS,            // incorrect file or service permissions
    NOT_INITIALIZED,        // not initialized
    PARAM,                  // invalid parameter
    RESOURCES,              // out of file system resources (handles, disk space, etc)
    SHARING,                // sharing violation
    UNKNOWN_RESULT_ERROR    // Any condition other than the above
} FTP_RESULT_CODE;

// FTP
enum FtpFolder
{
    FTP_ROOT_FOLDER = 1,
    FTP_PARENT_FOLDER,
    FTP_SUBFOLDER
};
typedef enum FtpFolder tFtpFolder;

    // Construction/destruction
    //
    CFtpClient ();
    virtual ~CFtpClient();

    FTP_RETURN_CODE OpenConnection (BD_ADDR bdAddr, CSdpDiscoveryRec & sdp_rec);
    FTP_RETURN_CODE CloseConnection();
    FTP_RETURN_CODE PutFile(WCHAR * localFileName);
    FTP_RETURN_CODE GetFile(WCHAR * remoteFileName,  WCHAR * localFolder);
    FTP_RETURN_CODE FolderListing();
    FTP_RETURN_CODE ChangeFolder(WCHAR * szFolder);
    FTP_RETURN_CODE DeleteFile(WCHAR * szFile);
    FTP_RETURN_CODE Abort();
    FTP_RETURN_CODE Parent();
    FTP_RETURN_CODE Root();
    FTP_RETURN_CODE CreateEmpty(WCHAR * szFile);
    FTP_RETURN_CODE CreateFolder(WCHAR * szFolder);
    void SetSecurity(BOOL authentication, BOOL encryption);

   // Application may provide these functions to facilitate managing the connection
    virtual void OnOpenResponse(FTP_RESULT_CODE result_code) {}
    virtual void OnCloseResponse(FTP_RESULT_CODE result_code){}
    

//windows
#ifndef _WIN32_WCE
    virtual void OnProgress(FTP_RESULT_CODE result_code, WCHAR * name, long current, long total) {}
    virtual void OnPutResponse(FTP_RESULT_CODE result_code, WCHAR * name) {}
    virtual void OnGetResponse(FTP_RESULT_CODE result_code, WCHAR * name) {}
    virtual void OnCreateResponse(FTP_RESULT_CODE result_code, WCHAR * name) {}
    virtual void OnDeleteResponse(FTP_RESULT_CODE result_code, WCHAR * name){}
    
    virtual void OnChangeFolderResponse(FTP_RESULT_CODE result_code, tFtpFolder folder_type, WCHAR * szFolder) {}
    virtual void OnFolderListingResponse(FTP_RESULT_CODE result_code, tFTP_FILE_ENTRY * listing, long entries) {}
    virtual void OnXmlFolderListingResponse(FTP_RESULT_CODE rc, WCHAR * pfolder_listing, long folder_length ){}
#else //for ce
    virtual void OnProgress(FTP_RESULT_CODE result_code, LPCTSTR name, long current, long total) {}
    virtual void OnPutResponse(FTP_RESULT_CODE result_code, LPCTSTR name) {}
    virtual void OnGetResponse(FTP_RESULT_CODE result_code, LPCTSTR name) {}
    virtual void OnCreateResponse(FTP_RESULT_CODE result_code, LPCTSTR name) {}
    virtual void OnDeleteResponse(FTP_RESULT_CODE result_code, LPCTSTR name){}
    
    virtual void OnFolderListingResponse(FTP_RESULT_CODE result_code, tFTP_FILE_ENTRY * listing, BOOL last) {}
    virtual void OnXmlFolderListingResponse(FTP_RESULT_CODE rc, WCHAR * pfolder_listing, long folder_length, BOOL last ){}
    virtual void OnChangeFolderResponse(FTP_RESULT_CODE result_code, tFtpFolder folder_type, LPCTSTR szFolder) {}
#endif
    

virtual void OnAbortResponse(FTP_RESULT_CODE result_code) {}

private:
    
    friend class CFtpClientFriend;
    friend class CFtpClient_Impl;
    CFtpClient_Impl *m_pImpl;
 
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CFtpClient(const CFtpClient & x);
    CFtpClient& operator= (const CFtpClient & x); 
};


///////////////////////////////////////////////////////////////////////////////////////
// Define a class to control the OPP client sessions
//
class COppClient_Impl;

class WIDCOMMSDK  COppClient
{
public:

//
// Define return code for OPP Client functions
//
typedef enum
{
    OPP_CLIENT_SUCCESS,            // Operation initiated without error
    OUT_OF_MEMORY,      // Not enough memory to initiate operation
    SECURITY_ERROR,     // Error implementing requested security level
    OPP_ERROR,          // OPP-specific error
    ABORT_INVALID,      // Abort not valid, no operation is in progress
    UNKNOWN_ERROR       // Any condition other than the above
} OPP_RETURN_CODE;

//
// Define return code for OPP response functions
//
typedef enum
{
    COMPLETED,	        // operation completed without error
    BAD_ADDR,           // bad BD_ADDR
    BAD_STATE,	        // could not handle request in present state
    BAD_REQUEST,     	// invalid request
    NOT_FOUND,	        // no such file
    NO_SERVICE,      	// could not find the specified FTP server
    DISCONNECT,      	// connection lost
    READ,        	    // read error
    WRITE,	            // write error
    OBEX_AUTH,          // OBEX Authentication required
    DENIED,	            // request could not be honored
    DATA_NOT_SUPPORTED,	// server does not support the requested data
    CONNECT,	        // error establishing connection
    NOT_INITIALIZED,	// not initialized
    PARAM,	            // bad parameter
    BAD_INBOX,	        // inbox is not valid
    BAD_NAME,	        // bad name for object
    PERMISSIONS,	    // prohibited by file permissions
    SHARING, 	        // file is shared
    RESOURCES,	        // file system resource limit reached - may be file handles, disk space, etc.
    FILE_EXISTS,        // is closedfile alto attempt to perform function after connectionready exists
    UNKNOWN_RESULT_ERROR	    // Any condition other than the above
} OPP_RESULT_CODE;

//
// Define return code for OPP response functions
// (NOTE: these values match the ones defined in \middleware\opp\oppapp.h)
//
typedef enum
{
    OPP_PUT_TRANS =             1,
    OPP_GET_TRANS =             2,
    OPP_EXCHANGE_PUT_TRANS =    3,
    OPP_EXCHANGE_GET_TRANS =    4,
    OPP_ABORT_TRANS =           5

} OPP_TRANSACTION_CODE;

public:

    // Construction/destruction
    //
    COppClient ();
    virtual ~COppClient();

    OPP_RETURN_CODE Push(BD_ADDR bda, WCHAR * pszPathName, CSdpDiscoveryRec & sdp_rec);
    OPP_RETURN_CODE Pull(BD_ADDR bda, WCHAR * pszPathName, CSdpDiscoveryRec & sdp_rec);
    OPP_RETURN_CODE Exchange(BD_ADDR bda, WCHAR * pszName, WCHAR * pszFolder, CSdpDiscoveryRec & sdp_rec);
    OPP_RETURN_CODE Abort();
    void SetSecurity(BOOL authentication, BOOL encryption);


    virtual void OnAbortResponse (OPP_RESULT_CODE result_code) {}

#ifndef _WIN32_WCE
    virtual void OnProgress(OPP_RESULT_CODE result_code, BD_ADDR bda, WCHAR * string, long current, long total) {}
    virtual void OnPushResponse(OPP_RESULT_CODE result_code,  BD_ADDR bda, WCHAR * string) {}
    virtual void OnPullResponse(OPP_RESULT_CODE result_code , BD_ADDR bda, WCHAR * string) {}
    virtual void OnExchangeResponse(OPP_RESULT_CODE result_code, BD_ADDR bda, WCHAR * string) {}
    virtual void OnExchangeResponse(OPP_RESULT_CODE result_code, BD_ADDR bda, WCHAR * string, OPP_TRANSACTION_CODE transaction_code) {}
#else    //for CE
    virtual void OnProgress(OPP_RESULT_CODE result_code, BD_ADDR bda, LPCTSTR string, long current, long total) {}
    virtual void OnPushResponse(OPP_RESULT_CODE result_code,  BD_ADDR bda, LPCTSTR string) {}
    virtual void OnPullResponse(OPP_RESULT_CODE result_code , BD_ADDR bda, LPCTSTR string) {}
    virtual void OnExchangeResponse(OPP_RESULT_CODE result_code, BD_ADDR bda, LPCTSTR string) {}
    virtual void OnExchangeResponse(OPP_RESULT_CODE result_code, BD_ADDR bda, LPCTSTR string, OPP_TRANSACTION_CODE transaction_code) {}
#endif
    
private:
    friend class COppClientFriend;
    friend class COppClient_Impl;
    COppClient_Impl  *m_pImpl;
 
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    COppClient(const COppClient & x);
    COppClient& operator= (const COppClient & x); 
};

///////////////////////////////////////////////////////////////////////////////////////
// Define a class to control the LAP client sessions
//
class CLapClient_Impl;

class WIDCOMMSDK  CLapClient
{
public:

//
// Define return code for LAP Client functions
//
typedef enum
{
    SUCCESS,     	    // Operation initiated without error
    NO_BT_SERVER,        // COM server could not be started
    ALREADY_CONNECTED,   // attempt to connect before previous connection closed
    NOT_CONNECTED,       // attempt to close unopened connection
    NOT_ENOUGH_MEMORY,   // local processor could not allocate memory for open
    UNKNOWN_ERROR,	    // Any condition other than the above
    INVALID_PARAMETER   // One or more of function parameters are not valid
} LAP_RETURN_CODE;

//
// Define connection states
//
typedef enum
{
    LAP_CONNECTED,	        // port now connected
    LAP_DISCONNECTED	    // port now disconnected
} LAP_STATE_CODE;


public:

    // Construction/destruction
    //
    CLapClient ();
    virtual ~CLapClient();

	LAP_RETURN_CODE CreateConnection(BD_ADDR bda, GUID guid, CSdpDiscoveryRec & sdp_rec);
    LAP_RETURN_CODE CreateConnection(BD_ADDR bda, CSdpDiscoveryRec & sdp_rec);
    LAP_RETURN_CODE RemoveConnection();
    void SetSecurity(BOOL authentication, BOOL encryption);
	LAP_RETURN_CODE GetConnectionStats (tBT_CONN_STATS *p_conn_stats);

    virtual void OnStateChange(BD_ADDR bda, DEV_CLASS dev_class, BD_NAME name, short com_port, LAP_STATE_CODE state) = 0;

private:
    friend class CLapClientFriend;
    friend class CLapClient_Impl;
    CLapClient_Impl  *m_pImpl;
 
    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CLapClient(const CLapClient & x);
    CLapClient& operator= (const CLapClient & x); 
};

///////////////////////////////////////////////////////////////////////////////////////
// Define a class to control the DUN client sessions
//
class CDunClient_Impl;

class WIDCOMMSDK  CDunClient
{
public:

//
// Define return code for DUN Client functions
//
typedef enum
{
    SUCCESS,     	    // Operation initiated without error
    NO_BT_SERVER,        // COM server could not be started
    ALREADY_CONNECTED,   // attempt to connect before previous connection closed
    NOT_CONNECTED,       // attempt to close unopened connection
    NOT_ENOUGH_MEMORY,   // local processor could not allocate memory for open
    UNKNOWN_ERROR,	    // Any condition other than the above
    INVALID_PARAMETER   // One or more of function parameters are not valid
} DUN_RETURN_CODE;

//
// Define connection states
//
typedef enum
{
    DUN_CONNECTED,	        // port now connected
    DUN_DISCONNECTED	    // port now disconnected
} DUN_STATE_CODE;


public:

    // Construction/destruction
    //
    CDunClient ();
    virtual ~CDunClient();

    DUN_RETURN_CODE CreateConnection(BD_ADDR bda, CSdpDiscoveryRec & sdp_rec);
    DUN_RETURN_CODE RemoveConnection();
    void SetSecurity(BOOL authentication, BOOL encryption);
	DUN_RETURN_CODE GetConnectionStats (tBT_CONN_STATS *p_conn_stats);

    virtual void OnStateChange(BD_ADDR bda, DEV_CLASS dev_class, BD_NAME name, short com_port, DUN_STATE_CODE state) = 0;

private:
    friend class CDunClientFriend;
    friend class CDunClient_Impl;
    CDunClient_Impl  *m_pImpl;

    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CDunClient(const CDunClient & x);
    CDunClient& operator= (const CDunClient & x); 
};

///////////////////////////////////////////////////////////////////////////////////////
// Define a class to control the SPP client sessions
//
class CSppClient_Impl;

class WIDCOMMSDK  CSppClient
{
public:

//
// Define return code for SPP Client functions
//
typedef enum
{
    SUCCESS,     	     // Operation initiated without error
    NO_BT_SERVER,        // COM server could not be started
    ALREADY_CONNECTED,   // attempt to connect before previous connection closed
    NOT_CONNECTED,       // attempt to close unopened connection
    NOT_ENOUGH_MEMORY,   // local processor could not allocate memory for open
    UNKNOWN_ERROR,	     // Any condition other than the above
    INVALID_PARAMETER   // One or more of function parameters are not valid
} SPP_CLIENT_RETURN_CODE;


public:

    // Construction/destruction
    //
    CSppClient ();
    virtual ~CSppClient();

    SPP_CLIENT_RETURN_CODE CreateConnection(BD_ADDR bda, BT_CHAR *szServiceName);
    SPP_CLIENT_RETURN_CODE RemoveConnection();
	SPP_CLIENT_RETURN_CODE GetConnectionStats (tBT_CONN_STATS *p_conn_stats);

    virtual void OnClientStateChange(BD_ADDR bda, DEV_CLASS dev_class, BD_NAME name, short com_port, SPP_STATE_CODE state) = 0;

private:

    friend class CSppClientFriend;
    friend class CSppClient_Impl;
    CSppClient_Impl  *m_pImpl;

    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CSppClient(const CSppClient & x);
    CSppClient& operator= (const CSppClient & x); 
};


///////////////////////////////////////////////////////////////////////////////////////
// Define a class to control the SPP server sessions
//
class CSppServer_Impl;

class WIDCOMMSDK  CSppServer
{
public:

//
// Define return code for SPP Server functions
//
typedef enum
{
    SUCCESS,     	     // Operation initiated without error
    NO_BT_SERVER,        // COM server could not be started
    ALREADY_CONNECTED,   // attempt to connect before previous connection closed
    NOT_CONNECTED,       // attempt to close unopened connection
    NOT_ENOUGH_MEMORY,   // local processor could not allocate memory for open
	NOT_SUPPORTED,		 // requested service not available locally
    UNKNOWN_ERROR,	     // Any condition other than the above
    INVALID_PARAMETER   // One or more of function parameters are not valid
} SPP_SERVER_RETURN_CODE;

public:

    // Construction/destruction
    //
    CSppServer ();
    virtual ~CSppServer();

    SPP_SERVER_RETURN_CODE CreateConnection(BT_CHAR * szServiceName);
    SPP_SERVER_RETURN_CODE RemoveConnection();
    virtual void OnServerStateChange(BD_ADDR bda, DEV_CLASS dev_class, BD_NAME name, short com_port, SPP_STATE_CODE state) = 0;
    SPP_SERVER_RETURN_CODE GetConnectionStats (tBT_CONN_STATS *p_conn_stats);

private:
    friend class CSppServerFriend;
    friend class CSppServer_Impl;
    CSppServer_Impl  *m_pImpl;

    // This class will not support the compiler-supplied copy constructor or assignment operator,
    // so these are declared private to prevent inadvertent use by the application.
    CSppServer(const CSppServer & x);
    CSppServer& operator= (const CSppServer & x); 
};



////////////////////////////////////////////////////////////////////////////
// The CPrintClient class
//
//file types
#define TYPE_STRING_GIF			_T("image/gif:89A")

#define BPSF_TYPE	 0x01	//Mask value

struct BTPRINTSTRUCT
	{
		DWORD    dwSize;     // Must be sizeof(BTPRINTSTRUCT)
        UINT     mask;       // 0 or BPSF_TYPE
		LPCTSTR  pszType;    // Data type
	};


class CPrintInternal;		
class CPrintClient_Impl;

class WIDCOMMSDK CPrintClient
{
public:
 
	// Define the profiles supported by the Printing SDK
	//
	typedef enum
	{
		PRINT_PROFILE_BPP,
		PRINT_PROFILE_HCRP,
		PRINT_PROFILE_SPP

	} ePRINT_PROFILE;

	// Define the current state of the Printing SDK
	//
	typedef enum
	{
		PRINT_STATE_IDLE,
		PRINT_STATE_CONNECTING,		
		PRINT_STATE_PRINTING,
		PRINT_STATE_FLOW_CONTROLLED,
		PRINT_STATE_DISCONNECTING, 
		PRINT_STATE_DONE

	} ePRINT_STATE;

	// Define error codes returned by the Printing SDK
	//
	typedef enum
	{
		// Generic to all profiles
		//
		PRINT_RC_OK,
		PRINT_RC_FILE_PRINTED_OK,
		PRINT_RC_FILE_NOT_FOUND,
		PRINT_RC_FILE_READ_ERROR,
		PRINT_RC_ALREADY_PRINTING,
		PRINT_RC_UNKNOWN_PROFILE,
		PRINT_RC_SERVICE_NOT_FOUND,
		PRINT_RC_SECURITY_ERROR,
		PRINT_RC_CONNECT_ERROR,
		PRINT_RC_WRITE_ERROR,
		PRINT_RC_REMOTE_DISCONNECTED,
		PRINT_RC_INVALID_PARAM,


		// BPP Specific errors
		//
		PRINT_RC_BPP_SCN_NOT_FOUND,
		PRINT_RC_BPP_SCN_NOT_ASSIGNED,
		PRINT_RC_BPP_OBEX_ABORTED,
		PRINT_RC_BPP_OBEX_MISMATCH,

		// HCRP Specific errors
		//
	    PRINT_RC_HCRP_CTL_PSM_NOT_FOUND,
		PRINT_RC_HCRP_DATA_PSM_NOT_FOUND,

		// SPP Specific errors
		//
		PRINT_RC_SPP_SCN_NOT_FOUND,

	} ePRINT_ERROR;

	public:
		CPrintClient();
        ~CPrintClient();

		ePRINT_ERROR        Start (BD_ADDR pBDA, ePRINT_PROFILE eProfile, LPCTSTR pszFile, 
								    BTPRINTSTRUCT * pBtPrintStruct = NULL);

		void                Cancel();
		ePRINT_STATE        GetState();
		UINT				GetBytesSent();
		UINT				GetBytesTotal();
		ePRINT_ERROR        GetLastError(TCHAR **pDescr);

		virtual void OnStateChange (ePRINT_STATE NewState) { };
		static CPrintInternal *pIp;

    private:
        friend class CPrintClient_Impl;
        CPrintClient_Impl  *m_pImpl;

};

		
#pragma pack ()


#endif // !defined(AFX_WIDCOMMSDK_H__1F5ED990_6FC6_4B0D_882C_8D7C98C16A06__INCLUDED_)
