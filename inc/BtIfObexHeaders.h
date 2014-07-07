/////////////////////////////////////////////////////////////////////////////
//
//  Name        BtIfObexHeaders.h
//  $Header:
//
//  Function    this file contains Widcomm SDK class definitions
//
//  Date                 Modification
//  ----------------------------------
//  24Apr2001    JWF   Create
//
//  Copyright (c) 2000-2002, WIDCOMM Inc., All Rights Reserved.
//  Proprietary and confidential.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _BTIFOBEXHEADERS_H
#define _BTIFOBEXHEADERS_H

#ifdef WIDCOMMSDK_EXPORTS
#define WIDCOMMSDK __declspec(dllexport)
#else
#define WIDCOMMSDK __declspec(dllimport)
#endif

//#include "obex_api.h"
//#include "obex_capi.h"
//#include "obex_sapi.h"
#include "BtIfDefinitions.h"
#include "BtIfClasses.h"

// Ensure alignment across all builds
//
#pragma pack (1)

class CObexHeaders;

////////////////////////////////////////////////////////////////////////////
// 
// Class for OBEX client side applications
//
class CObexClient_Impl;
  
class WIDCOMMSDK CObexClient  
{
public:

	~CObexClient();
	CObexClient();

	tOBEX_ERRORS Open (UINT8 scn, BD_ADDR bd_addr, CObexHeaders *p_request, UINT16 mtu = OBEX_DEFAULT_MTU);
	tOBEX_ERRORS SetPath (CObexHeaders *p_request, BOOL backup, BOOL create);
	tOBEX_ERRORS Put (CObexHeaders *p_request, BOOL final);
	tOBEX_ERRORS Get (CObexHeaders *p_request, BOOL final);
	tOBEX_ERRORS Abort (CObexHeaders *p_request);
	tOBEX_ERRORS Close (CObexHeaders *p_request);

	virtual void OnOpen(CObexHeaders *p_confirm, UINT16 tx_mtu, tOBEX_ERRORS code, tOBEX_RESPONSE_CODE response) = 0;
	virtual void OnClose(CObexHeaders *p_confirm, tOBEX_ERRORS code, tOBEX_RESPONSE_CODE response) = 0;
	virtual void OnAbort (CObexHeaders *p_confirm, tOBEX_ERRORS code, tOBEX_RESPONSE_CODE response) {}
	virtual void OnPut (CObexHeaders *p_confirm, tOBEX_ERRORS  code, tOBEX_RESPONSE_CODE response) {}
	virtual void OnGet (CObexHeaders *p_confirm, tOBEX_ERRORS  code, BOOL final, tOBEX_RESPONSE_CODE response) {}
	virtual void OnSetPath (CObexHeaders *p_confirm, tOBEX_ERRORS code, tOBEX_RESPONSE_CODE response) {}

    //Create AudioConnection
    AUDIO_RETURN_CODE CreateAudioConnection(BOOL bIsClient, UINT16 *audioHandle);

    //Disconnect AudioConnection
    AUDIO_RETURN_CODE RemoveAudioConnection(UINT16 audioHandle);

    //Audio callback functions
    virtual void OnAudioConnected(UINT16 audioHandle){};
    virtual void OnAudioDisconnect(UINT16 audioHandle){};

#ifdef _WIN32_WCE
	AUDIO_RETURN_CODE ReadAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteR);
	AUDIO_RETURN_CODE WriteAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteW);
#endif

    tOBEX_ERRORS    SetLinkSupervisionTimeOut(UINT16 timeout);

private:

    static CObexClient   *m_p_first_client;
    CObexClient          *m_p_next_client;
    friend class CObexClientFriend;
    friend class CObexClient_Impl;
    CObexClient_Impl  *m_pImpl;
    tOBEX_HEADERS * GetBinaryPtr(CObexHeaders *p_headers_object);

};


////////////////////////////////////////////////////////////////////////////
// 
// Class for OBEX server side applications
//
class CObexServer_Impl;  

class WIDCOMMSDK CObexServer  
{
public:

	~CObexServer();
	CObexServer();

	tOBEX_ERRORS Register (UINT8 scn, UINT8 *p_target = NULL);
	tOBEX_ERRORS Unregister();
	tOBEX_ERRORS OpenCnf (tOBEX_ERRORS  obex_errors, tOBEX_RESPONSE_CODE rsp_code, CObexHeaders * p_response, UINT16 mtu= OBEX_DEFAULT_MTU);
	tOBEX_ERRORS SetPathCnf (tOBEX_ERRORS  obex_errors, tOBEX_RESPONSE_CODE rsp_code, CObexHeaders * p_response);
	tOBEX_ERRORS PutCnf (tOBEX_ERRORS  obex_errors, tOBEX_RESPONSE_CODE rsp_code, CObexHeaders * p_response);
	tOBEX_ERRORS PutCreateCnf (tOBEX_ERRORS  obex_errors, tOBEX_RESPONSE_CODE rsp_code, CObexHeaders * p_response);
	tOBEX_ERRORS PutDeleteCnf (tOBEX_ERRORS  obex_errors, tOBEX_RESPONSE_CODE rsp_code, CObexHeaders * p_response);
	tOBEX_ERRORS GetCnf (tOBEX_ERRORS  obex_errors, tOBEX_RESPONSE_CODE rsp_code, BOOL final, CObexHeaders * p_response);
	tOBEX_ERRORS AbortCnf (tOBEX_ERRORS  obex_errors, tOBEX_RESPONSE_CODE rsp_code, CObexHeaders * p_response);
	tOBEX_ERRORS CloseCnf (tOBEX_ERRORS  obex_errors, tOBEX_RESPONSE_CODE rsp_code, CObexHeaders * p_response);
    BOOL         SwitchRole(MASTER_SLAVE_ROLE new_role);

    //Create AudioConnection
    AUDIO_RETURN_CODE CreateAudioConnection(BOOL bIsClient, UINT16 *audioHandle);

    //Disconnect AudioConnection
    AUDIO_RETURN_CODE RemoveAudioConnection(UINT16 audioHandle);

    //Audio callback functions
    virtual void OnAudioConnected(UINT16 audioHandle){};
    virtual void OnAudioDisconnect(UINT16 audinHandle){};

#ifdef _WIN32_WCE
	AUDIO_RETURN_CODE ReadAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteR);
	AUDIO_RETURN_CODE WriteAudioData(void *pBuff, DWORD dwLen, DWORD *dwByteW);
#endif


    void		 GetRemoteBDAddr(BD_ADDR_PTR p_bd_addr);

	virtual void OnOpenInd(CObexHeaders *p_request) = 0;
	virtual void OnSetPathInd (CObexHeaders * p_request, BOOL backup, BOOL create) = 0;
	virtual void OnPutInd (CObexHeaders * p_request, BOOL final) = 0;
	virtual void OnPutCreateInd(CObexHeaders * p_request) = 0;
	virtual void OnPutDeleteInd (CObexHeaders * p_request) = 0;
	virtual void OnGetInd (CObexHeaders * p_request, BOOL final) = 0;
	virtual void OnAbortInd (CObexHeaders * p_request) = 0;
	virtual void OnCloseInd(CObexHeaders *p_request) = 0;

    tOBEX_ERRORS    SetLinkSupervisionTimeOut(UINT16 timeout);
private:


static CObexServer   *m_p_first_server;
    CObexServer          *m_p_next_server;
    friend class CObexServerFriend;
    friend class CObexServer_Impl;
    CObexServer_Impl    *m_pImpl;  
	tOBEX_HEADERS * GetBinaryPtr(CObexHeaders *p_headers_object);
};

////////////////////////////////////////////////////////////////////////////
// 
// Class for OBEX user defined parameter header
//
class CObexUserDefined_Impl;  

class WIDCOMMSDK CObexUserDefined  
{
public:

	~CObexUserDefined();
	CObexUserDefined();
	BOOL SetHeader (UINT8 id, UINT8 byte);
	BOOL SetHeader (UINT8 id, UINT32 four_byte);
	BOOL SetHeader (UINT8 id, UINT16 * p_text);
	BOOL SetHeader (UINT8 id, UINT8 * p_array, UINT16 length);
	UINT8 GetUserType (UINT8 * p_id);
	BOOL GetByte (UINT8 *p_byte);
	BOOL GetFourByte (UINT32 *p_fourbyte);
	UINT16 GetLength ();
	BOOL GetText (UINT16 *p_text);
	BOOL GetOctets (UINT8 *p_octet_array);

private:
	tOBEX_USER_HDR ud_hdr;
    friend class CObexUserDefined_Impl;
    CObexUserDefined_Impl  *m_pImpl;


    // This class will not support the compiler-supplied copy constructor or assignment operator, tOBEX_CSESSION_HANDLE session_handle,
    // so these are declared private to prevent inadvertant use by the application.
    CObexUserDefined(const CObexUserDefined & x);
    CObexUserDefined& operator= (const CObexUserDefined & x); 
};

 
////////////////////////////////////////////////////////////////////////////
// 
// Class for OBEX headers object, tOBEX_CSESSION_HANDLE session_handle, a container for all possible OBEX header values
//
class CObexHeaders_Impl;  

class WIDCOMMSDK CObexHeaders  
{
	friend tOBEX_HEADERS * CObexClient::GetBinaryPtr(CObexHeaders *p_headers_object);
	friend class CObexClientFriend;
	friend tOBEX_HEADERS * CObexServer::GetBinaryPtr(CObexHeaders *p_headers_object);
	friend class CObexServerFriend;

public:

	~CObexHeaders();
	CObexHeaders();	// locally allocate container, tOBEX_CSESSION_HANDLE session_handle, with all null header values

private:

	// These functions are accessable only from friends on the lower OBEX levels
	// They permit efficient use of existing OBEX header structures
	CObexHeaders(tOBEX_HEADERS *p_external_hdrs);	// externally constructed container
	tOBEX_HEADERS * GetBinaryPtr();

    friend class CObexHeaders_Impl;
    CObexHeaders_Impl   *m_pImpl;

    // This class will not support the compiler-supplied copy constructor or assignment operator, tOBEX_CSESSION_HANDLE session_handle,
    // so these are declared private to prevent inadvertant use by the application.
    CObexHeaders(const CObexHeaders & x);
    CObexHeaders& operator= (const CObexHeaders & x); 

public:

	void SetCount(UINT32 count);
	void DeleteCount();
	BOOL GetCount(UINT32 * p_count);

	BOOL SetName (UINT16 * p_array);
	void DeleteName();
	BOOL GetNameLength (UINT32 * p_len_incl_null);
	BOOL GetName (UINT16 * p_array);

	BOOL SetType (UINT8 * p_array, UINT32 length);
	void DeleteType();
	BOOL GetTypeLength (UINT32 * p_length);
	BOOL GetType (UINT8 * p_array);

	void SetLength(UINT32 length);
	void DeleteLength();
	BOOL GetLength(UINT32 * p_length);

	BOOL SetTime(char * p_str_8601);
	void DeleteTime();
	BOOL GetTime(char * p_str_8601);

	BOOL SetDescription (UINT16 * p_array);
	void DeleteDescription();
	BOOL GetDescriptionLength (UINT32 * p_len_incl_null);
	BOOL GetDescription (UINT16 * p_array);

	BOOL AddTarget (UINT8 * p_array, UINT32 length);
	UINT32 GetTargetCnt ();
	BOOL DeleteTarget (UINT16 index);
	BOOL GetTargetLength (UINT32  * p_length, UINT16 index);
	BOOL GetTarget (UINT8 * p_array, UINT16 index);

	BOOL AddHttp (UINT8 * p_array, UINT32 length);
	UINT32 GetHttpCnt ();
	BOOL DeleteHttp (UINT16 index);
	BOOL GetHttpLength (UINT32  * p_length, UINT16 index);
	BOOL GetHttp (UINT8 * p_array, UINT16 index);

	BOOL SetBody (UINT8 * p_array, UINT32 length, BOOL body_end);
	void DeleteBody();
	BOOL GetBodyLength ( UINT32 * p_length);
	BOOL GetBody (UINT8 * p_array, BOOL *p_body_end);

	BOOL SetWho (UINT8 * p_array, UINT32 length);
	void DeleteWho();
	BOOL GetWhoLength ( UINT32 * p_length);
	BOOL GetWho (UINT8 * p_array);

	BOOL AddAppParam (UINT8 tag, UINT8 length, UINT8 * p_array);
	UINT32 GetAppParamCnt ();
	BOOL DeleteAppParam(UINT16 index);
	BOOL GetAppParamLength(UINT8  * p_length, UINT16 index);
	BOOL GetAppParam (UINT8 * p_tag, UINT8 *p_array, UINT16 index);

	BOOL AddAuthChallenge (UINT8 tag, UINT8 length, UINT8 * p_array);
	UINT32 GetAuthChallengeCnt ();
	BOOL DeleteAuthChallenge (UINT16 index);
	BOOL GetAuthChallengeLength (UINT8  * p_length, UINT16 index);
	BOOL GetAuthChallenge (UINT8 * p_tag, UINT8 *p_array, UINT16 index);

	BOOL AddAuthResponse (UINT8 tag, UINT8 length, UINT8 * p_array);
	UINT32 GetAuthResponseCnt ();
	BOOL DeleteAuthResponse (UINT16 index);
	BOOL GetAuthResponseLength (UINT8  * p_length, UINT16 index);
	BOOL GetAuthResponse (UINT8 * p_tag, UINT8 *p_array, UINT16 index);

	BOOL SetObjectClass (UINT8 * p_array, UINT32 length);
	void DeleteObjectClass ();
	BOOL GetObjectClassLength ( UINT32 * p_length);
	BOOL GetObjectClass (UINT8 * p_array);

	BOOL AddUserDefined (CObexUserDefined * p_user_defined);
	UINT32 GetUserDefinedCnt ();
	BOOL DeleteUserDefined (UINT16 index);
	BOOL GetUserDefinedLength (UINT16  * p_length, UINT16 index);
	BOOL GetUserDefined (CObexUserDefined * p_user_defined, UINT16 index);
	
private:

	void SetDefaults(tOBEX_HEADERS *p_headers); // all header fields become null
	void FreeInternal(tOBEX_HEADERS *p_headers);	// release all memory malloced to this object
	void SetFlag (UINT32 mask);
	void ClearFlag(UINT32 mask);
	BOOL TestFlag(UINT32 mask);
	BOOL locally_allocated;

private:

	tOBEX_HEADERS *p_hdrs;
};



#pragma pack ()

#endif 
