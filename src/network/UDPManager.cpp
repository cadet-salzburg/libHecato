#include "UDPManager.h"

#ifdef WIN32
#include <Ws2tcpip.h>		// TCP/IP annex needed for multicasting
typedef int socklen_t;
#else
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#define closesocket(x) close(x)
#define ioctlsocket(a,b,c) ioctl(a,b,c)
#define WSAGetLastError() (errno)
#endif

//--------------------------------------------------------------------------------

bool UDPManager::m_bWinsockInit= false;

//--------------------------------------------------------------------------------
UDPManager::UDPManager()
{
	// was winsock initialized?
#ifdef WIN32
	if (!m_bWinsockInit) {
		WORD vr;
		WSADATA	wsaData;
		vr=	MAKEWORD(2,	2);
		WSAStartup(vr, &wsaData);
		m_bWinsockInit=	true;
	}
#else
	m_bWinsockInit=	true;
#endif

	m_hSocket= INVALID_SOCKET;
	m_lTimeoutReceive= DEFAULT_TIMEOUT;
	m_lTimeoutSend= DEFAULT_TIMEOUT;

	m_nLastError = 0;

	memset(&m_saRemote, 0, sizeof(m_saRemote));
	m_bHaveRemoteAddress= false;

};

//--------------------------------------------------------------------------------
UDPManager::~UDPManager()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		Close();
	}
}

//--------------------------------------------------------------------------------
///	Closes an open socket.
///	NOTE: A	closed socket cannot be	reused again without a call	to "Create()".
bool UDPManager::Close()
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(false);
	}
	if(closesocket(m_hSocket) == SOCKET_ERROR)
	{
		return(false);
	}
	m_hSocket= INVALID_SOCKET;

	return(true);
}

//--------------------------------------------------------------------------------
bool UDPManager::Create()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		return(false);
	}
	m_hSocket =	socket(AF_INET,	SOCK_DGRAM,	IPPROTO_UDP);
	if (m_hSocket != INVALID_SOCKET)
	{
		int val = 1;

#ifdef SO_REUSEADDR
		setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val));
#endif
#ifdef SO_REUSEPORT
		setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEPORT, (char*)&val, sizeof(val));
#endif

	}
	return(m_hSocket !=	INVALID_SOCKET);
}


bool UDPManager::Bind(const InetAddr &_addr)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(false);
	}

	if (bind(m_hSocket,(struct sockaddr*)&_addr,sizeof(_addr)) == SOCKET_ERROR)
	{
		//		int error = setLastError();
		return false;
	}
	m_saRemote = _addr;
	return true;
}

bool UDPManager::Bind(USHORT usPort)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(false);
	}

	struct sockaddr_in saServer;
	memset(&saServer, 0, sizeof(saServer));
	saServer.sin_family	= AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;
	//Port MUST	be in Network Byte Order
	saServer.sin_port =	htons(usPort);

	return Bind(saServer);
}

bool UDPManager::BindMcast(const InetAddr &_addr)
{
	// bind to port
	if (!Bind(_addr))
	{
		//		int error = setLastError();
		return false;
	}

	// join the multicast group
	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(ip_mreq));
	mreq.imr_multiaddr.s_addr = _addr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*) &_addr, sizeof (_addr)) == SOCKET_ERROR)
	{
		//		int error = setLastError();
		return false;
	}

	// multicast bind successful
	return true;
}

bool UDPManager::BindMcast(const char *pMcast, USHORT usPort)
{
	// bind to port
	if (!Bind(usPort))
	{
		//		int error = setLastError();
		return false;
	}

	// join the multicast group
	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(ip_mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(pMcast);
	mreq.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*) &mreq, sizeof (mreq)) == SOCKET_ERROR)
	{
		//		int error = setLastError();
		return false;
	}

	// multicast bind successful
	return true;
}


bool UDPManager::Connect(const InetAddr &_addr)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(false);
	}

	m_saRemote = _addr;
	m_bHaveRemoteAddress = true;
	return true;
}

bool UDPManager::ConnectMcast(const InetAddr &_addr)
{
	// associate the source socket's address with the socket
	if (!Bind(ntohs(_addr.sin_port)))
	{
		//		int error = setLastError();
		return false;
	}

	// set ttl to default
	if (!SetTTL(1))
	{
		//		int error = setLastError();
	}

	if (!Connect(_addr))
	{
		//		int error = setLastError();
		return false;
	}

	// multicast connect successful
	return true;
}

//--------------------------------------------------------------------------------
bool UDPManager::Connect(const char *pHost, USHORT usPort)
{
	memset(&m_saRemote, 0, sizeof(m_saRemote));

	if (m_hSocket == INVALID_SOCKET)
	{
		return(false);
	}

	m_saRemote.sin_family= AF_INET; // host byte order
	m_saRemote.sin_port = htons(usPort);	// short, network byte order

	if (isalpha(pHost[0])) {  // host address is a name
		struct hostent *he;
		if ((he = gethostbyname(pHost)) == NULL)
		{
			//			int error = setLastError();
			return false;
		}

		m_saRemote.sin_addr  = *((struct in_addr *)he->h_addr);
	}
	else
	{
		if ((m_saRemote.sin_addr.s_addr = inet_addr(pHost)) == INADDR_NONE)
		{
			return false;
		}
	}

	m_bHaveRemoteAddress = true;
	return true;
}

bool UDPManager::ConnectMcast(const char* pMcast, USHORT usPort)
{
	// associate the source socket's address with the socket
	if (!Bind(usPort))
	{
		//		int error = setLastError();
		return false;
	}

	// set ttl to default
	if (!SetTTL(1))
	{
		//		int error = setLastError();
	}

	if (!Connect(pMcast, usPort))
	{
		//		int error = setLastError();
		return false;
	}

	// multicast connect successful
	return true;
}

//--------------------------------------------------------------------------------
///	Return values:
///	SOCKET_TIMEOUT indicates timeout
///	SOCKET_ERROR in	case of	a problem.
int	UDPManager::Send(const void* pBuff, const int iSize)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(SOCKET_ERROR);
	}

	if (m_lTimeoutSend != NO_TIMEOUT)
	{
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(m_hSocket, &fd);
		struct timeval tv= { m_lTimeoutSend, 0 };
		int ret = select((int)m_hSocket+1, NULL, &fd, NULL, &tv);
		if (ret == 0)
		{
			return(SOCKET_TIMEOUT);
		}
		else if (ret < 0)
		{
			return(SOCKET_ERROR);
		}
	}

	int ret = sendto(m_hSocket, (char*)pBuff, iSize, 0, (const struct sockaddr*)&m_saRemote, sizeof(m_saRemote));
	if (ret	>= 0)
	{
	}
	else
	{
		//		int error = setLastError();
	}

	return(ret);
}

//--------------------------------------------------------------------------------
///	Return values:
///	SOCKET_TIMEOUT indicates timeout
///	SOCKET_ERROR in	case of	a problem.
int	UDPManager::SendAll(const void* pBuff, const int iSize)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(SOCKET_ERROR);
	}

	if (m_lTimeoutSend != NO_TIMEOUT)
	{
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(m_hSocket, &fd);
		struct timeval tv= { m_lTimeoutSend, 0 };
		int ret = select((int)m_hSocket+1, NULL, &fd, NULL, &tv);
		if (ret == 0)
		{
			return(SOCKET_TIMEOUT);
		}
		else if (ret < 0)
		{
			return(SOCKET_ERROR);
		}
	}

	int	total= 0;
	int	n= 0;

	while (total < iSize)
	{
		n =	sendto(m_hSocket, (char*)pBuff,	iSize, 0, (const struct sockaddr*)&m_saRemote, sizeof(m_saRemote));
		if (n == -1)
		{
			break;
		}
		total += n;
	}

	return n==-1?SOCKET_ERROR:total;
}



//--------------------------------------------------------------------------------
///	Return values:
///	SOCKET_TIMEOUT indicates timeout
///	SOCKET_ERROR in	case of	a problem.
int	UDPManager::Receive(void* pBuff, const int iSize)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(SOCKET_ERROR);
	}

	if (m_lTimeoutReceive != NO_TIMEOUT)
	{
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(m_hSocket, &fd);
		struct timeval tv= { m_lTimeoutReceive, 0 };
		int ret = select((int)m_hSocket+1, &fd, NULL, NULL, &tv);
		if (ret == 0)
		{
			return(SOCKET_TIMEOUT);
		}
		else if (ret < 0)
		{
			return(SOCKET_ERROR);
		}
	}

	memset(pBuff, 0, iSize);
	socklen_t nLen= sizeof(m_saRemote);
	int	ret= recvfrom(m_hSocket, (char*)pBuff, iSize, 0, (struct sockaddr*)&m_saRemote, &nLen);
	if (ret	>= 0)
	{
		m_bHaveRemoteAddress= true;
	}
	else
	{
		//		int error = setLastError();
		m_bHaveRemoteAddress= false;
	}

	return ret;
}


//--------------------------------------------------------------------------------
bool UDPManager::GetRemoteAddr(char* pAddress, USHORT* pPort)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(false);
	}
	if (!m_bHaveRemoteAddress)
	{
		return(false);
	}

	strcpy(pAddress, inet_ntoa((in_addr)m_saRemote.sin_addr));
	*pPort = ntohs(m_saRemote.sin_port);	// short, host byte order
	return true;
}

bool UDPManager::GetRemoteAddr(InetAddr &_addr)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return(false);
	}
	if (!m_bHaveRemoteAddress)
	{
		return(false);
	}
	memcpy(&_addr, &m_saRemote, sizeof(m_saRemote));
	return true;
}

int	UDPManager::GetMaxMsgSize()
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return 0;
	}

#ifdef SO_MAX_MSG_SIZE
	int	sizeBuffer=0;
	int	size = sizeof(sizeBuffer);
	getsockopt(m_hSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&sizeBuffer, &size);

	return sizeBuffer;
#else
	return 0;
#endif
}


int	UDPManager::GetReceiveBufferSize()
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return 0;
	}

	int	sizeBuffer=0;
	socklen_t size = sizeof(sizeBuffer);
	getsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&sizeBuffer, &size);

	return sizeBuffer;
}

bool UDPManager::SetReceiveBufferSize(int sizeInByte)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	return (setsockopt(m_hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&sizeInByte, sizeof(sizeInByte)) == 0);
}

int	UDPManager::GetSendBufferSize()
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return 0;
	}

	int	sizeBuffer=0;
	socklen_t size = sizeof(sizeBuffer);
	getsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sizeBuffer, &size);

	return sizeBuffer;
}

bool UDPManager::SetSendBufferSize(int sizeInByte)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	return (setsockopt(m_hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sizeInByte, sizeof(sizeInByte)) == 0);
}

bool UDPManager::SetReuseAddress(bool allowReuse)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	int	on = (allowReuse) ? 1:0;

	return (setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) == 0);
}


bool UDPManager::SetEnableBroadcast(bool enableBroadcast)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	int	on = (enableBroadcast) ? 1:0;

	return (setsockopt(m_hSocket, SOL_SOCKET, SO_BROADCAST, (char*)&on, sizeof(on)) == 0);
}


int UDPManager::GetTTL()
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return -1;
	}

	int nTTL;
	socklen_t nSize = sizeof(nTTL);
	if (getsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char*) &nTTL, &nSize) == SOCKET_ERROR)
	{
		//		int error = setLastError();
		return -1;
	}

	return nTTL;
}

bool UDPManager::SetTTL(int nTTL)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	// Set the Time-to-Live of the multicast.
	if (setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*)&nTTL, sizeof (nTTL)) == SOCKET_ERROR)
	{
		//		int error = setLastError();
		return false;
	}

	return true;
}

bool UDPManager::GetLocalHost(char* pName, char* pAddress, char* pBroadcast)
{
	pName[0] = '\0';
	pAddress[0] = '\0';
	pBroadcast[0] = '\0';

	char buffer[256];
	if (gethostname(buffer, sizeof(buffer)) == SOCKET_ERROR)
	{
		return false;
	}
	strcpy(pName, buffer);

	struct hostent *he;
	if ((he = gethostbyname(pName)) == NULL)
	{
	}
	else
	{
		struct in_addr in = *((struct in_addr *)he->h_addr);
		unsigned char *inBytes = (unsigned char *)&in;
		strcpy(pAddress, inet_ntoa(in));
		if ((inBytes[0] & 0x80) == 0)  // Class A
		{
			inBytes[1] = 255;
			inBytes[2] = 255;
			inBytes[3] = 255;
			strcpy(pBroadcast, inet_ntoa(in));
		}
		else if ((inBytes[0] & 0xC0) == 0x80)  // Class B
		{
			inBytes[2] = 255;
			inBytes[3] = 255;
			strcpy(pBroadcast, inet_ntoa(in));
		}
		else if ((inBytes[0] & 0xE0) == 0xC0)  // Class C
		{
			inBytes[3] = 255;
			strcpy(pBroadcast, inet_ntoa(in));
		}
	}
	return true;
}

bool UDPManager::IsReadable()
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(m_hSocket, &fd);
	struct timeval tv= { 0, 0 };
	int ret = select((int)m_hSocket+1, &fd, NULL, NULL, &tv);

	return ret > 0;
}

bool UDPManager::IsWriteable()
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(m_hSocket, &fd);
	struct timeval tv= { 0, 0 };
	int ret = select((int)m_hSocket+1, NULL, &fd, NULL, &tv);

	return ret > 0;
}

int UDPManager::setLastError()
{
	m_nLastError = WSAGetLastError();
	return m_nLastError;
}


int UDPManager::getLastError()
{
	int nRet = ( m_nLastError ? m_nLastError : WSAGetLastError());
	m_nLastError = 0;
	return nRet;
}

bool UDPManager::SetBlocking(bool bIsBlockingA)
{
	if (m_hSocket == INVALID_SOCKET)
	{
		return false;
	}

	unsigned long onL = (bIsBlockingA) ? 1:0;
	if (ioctlsocket(m_hSocket, FIONBIO, &onL) == SOCKET_ERROR)
	{
		//		int error = setLastError();
		return false;
	}
	return true;
}
