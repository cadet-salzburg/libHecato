#ifndef  ___UDPMANAGER__H__
#define  ___UDPMANAGER__H__

#pragma once

#include "InetAddr.h"

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

// Implementation of a UDP socket.
class UDPManager
{
public:

	//constructor
	UDPManager();

	//destructor
	virtual ~UDPManager();

	bool Close();
	bool Create();
	bool Connect(const char *pHost, USHORT usPort);
	bool ConnectMcast(const char *pMcast, USHORT usPort);
	bool Connect(const InetAddr &_addr);
	bool ConnectMcast(const InetAddr &_addr);
	bool Bind(USHORT usPort);
	bool BindMcast(const char *pMcast, USHORT usPort);
	bool Bind(const InetAddr &_addr);
	bool BindMcast(const InetAddr &_addr);

	int Send(const void* pBuff, const int iSize);
	int SendAll(const void* pBuff, const int iSize);	//all data will be sent guaranteed.
	int Receive(void* pBuff, const int iSize);

	bool GetRemoteAddr(char* pAddress, USHORT* pPort);	//returns IP/Port of last received packet
	bool GetRemoteAddr(InetAddr &_addr);				//returns IP/Port of last received packet

	bool SetReuseAddress(bool allowReuse);
	bool SetEnableBroadcast(bool enableBroadcast);
	bool SetReceiveBufferSize(int sizeInByte);
	bool SetSendBufferSize(int sizeInByte);
	int GetReceiveBufferSize();
	int GetSendBufferSize();
	int GetMaxMsgSize();

	int GetTTL();	/// returns -1 on failure
	bool SetTTL(int nTTL);

	void SetTimeoutSend(long timeoutInSeconds)
	{
		m_lTimeoutSend= timeoutInSeconds;
	}
	void SetTimeoutReceive(long timeoutInSeconds)
	{
		m_lTimeoutReceive= timeoutInSeconds;
	}
	long GetTimeoutSend() const
	{
		return m_lTimeoutSend;
	}
	long GetTimeoutReceive() const
	{
		return m_lTimeoutReceive;
	}

	static bool GetLocalHost(char* pName, char* pAddress, char* pBroadcast);

	bool IsReadable();
	bool IsWriteable();

	SOCKET GetSocket() { return m_hSocket; }

	int getLastError();

	bool SetBlocking(bool bIsBlockingA);

protected:

	int setLastError();

	SOCKET m_hSocket;

	long m_lTimeoutReceive;
	long m_lTimeoutSend;

	InetAddr m_saRemote;
	bool m_bHaveRemoteAddress;

	int m_nLastError;

	static bool m_bWinsockInit;
};

#endif // ___UDPMANAGER__H__
