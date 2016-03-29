#include "BaseSocket.h"
#include "Lock.h"
#include "WgdServer.h"

CBaseSocket::CBaseSocket(CWgdServer* pBaseServer)
{
	m_socket = INVALID_SOCKET;

	m_state = SOCKET_STATE_IDLE;

	m_Notify = NULL;

	m_pBaseServer = pBaseServer;
}

CBaseSocket::~CBaseSocket()
{
	printf("CBaseSocket::~CBaseSocket, socket=%d\n", m_socket);
}

int CBaseSocket::Listen(const char* server_ip, uint16_t port, IServerNotify* pNotify)
{
	m_local_ip = server_ip;
	m_local_port = port;
	m_Notify = pNotify;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		printf("socket failed, err_code=%d\n", _GetErrorCode());
		return NETLIB_ERROR;
	}

	printf("Main Socket[%d]...\n", m_socket);

	_SetReuseAddr(m_socket);
	_SetNonblock(m_socket);

	sockaddr_in serv_addr;
	_SetAddr(server_ip, port, &serv_addr);
    int ret = ::bind(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret == SOCKET_ERROR)
	{
		printf("bind failed, err_code=%d\n", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_ERROR;
	}

	ret = listen(m_socket, 64);
	if (ret == SOCKET_ERROR)
	{
		printf("listen failed, err_code=%d\n", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_ERROR;
	}

	m_state = SOCKET_STATE_LISTENING;

	printf("CBaseSocket::Listen on %s:%d\n", server_ip, port);

	m_pBaseServer->AddBaseSocket(this);
	m_pBaseServer->AddEventForMainSocket(m_socket, SOCKET_READ | SOCKET_EXCEP);
	return NETLIB_OK;
}

net_handle_t CBaseSocket::Connect(const char* server_ip, uint16_t port, IServerNotify* pNotify)
{
	printf("CBaseSocket::Connect, server_ip=%s, port=%d\n", server_ip, port);

	m_remote_ip = server_ip;

	m_remote_port = port;

	m_Notify = pNotify;

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		printf("socket failed, err_code=%d\n", _GetErrorCode());
		return NETLIB_INVALID_HANDLE;
	}

	_SetNonblock(m_socket);
	_SetNoDelay(m_socket);
	sockaddr_in serv_addr;
	_SetAddr(server_ip, port, &serv_addr);
	int ret = connect(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if ( (ret == SOCKET_ERROR) && (!_IsBlock(_GetErrorCode())) )
	{	
		printf("connect failed, err_code=%d\n", _GetErrorCode());
		closesocket(m_socket);
		return NETLIB_INVALID_HANDLE;
	}
	m_state = SOCKET_STATE_CONNECTING;
	m_pBaseServer->AddBaseSocket(this);
	m_pBaseServer->AddEvent(m_socket, SOCKET_ALL);
	
	return (net_handle_t)m_socket;
}

int CBaseSocket::Send(void* buf, int len)
{
	if (m_state != SOCKET_STATE_CONNECTED)
		return NETLIB_ERROR;

	int ret = send(m_socket, (char*)buf, len, 0);
	if (ret == SOCKET_ERROR)
	{
		int err_code = _GetErrorCode();
		if (_IsBlock(err_code))
		{
			m_pBaseServer->AddEvent(m_socket, SOCKET_WRITE);
			ret = 0;
		}
		else
		{
			
		}
	}

	return ret;
}

int CBaseSocket::Recv(void* buf, int len)
{
	return recv(m_socket, (char*)buf, len, 0);
}

int CBaseSocket::Close()
{
	closesocket(m_socket);

	delete this;

	return 0;
}

int CBaseSocket::OnRead()
{
	if (m_state == SOCKET_STATE_LISTENING)
	{
		_AcceptNewSocket();
	}
	else
	{
		u_long avail = 0;
		int nRet = ioctlsocket(m_socket, FIONREAD, &avail);
		if ( (nRet == SOCKET_ERROR) || (avail == 0) )
		{
			m_Notify->OnClose((net_handle_t)m_socket);
			return -1;
		}
		else
		{
			m_Notify->OnRead((net_handle_t)m_socket);
		}
	}

	return 0;
}

void CBaseSocket::OnWrite()
{
	if (m_state == SOCKET_STATE_CONNECTING)
	{
		int error = 0;
		socklen_t len = sizeof(error);
		getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
		if (error) 
		{
			m_Notify->OnClose((net_handle_t)m_socket);
		}
		else 
		{
			m_state = SOCKET_STATE_CONNECTED;
			m_Notify->ReConnect((net_handle_t)m_socket);
		}
	}
	else
	{
		m_Notify->OnWrite((net_handle_t)m_socket);
	}
}

void CBaseSocket::OnClose()
{
	m_state = SOCKET_STATE_CLOSING;

	m_Notify->OnClose((net_handle_t)m_socket);
}

void CBaseSocket::SetSendBufSize(uint32_t send_size)
{
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&send_size, 4);
	if (ret == SOCKET_ERROR)
	{
		//log("set SO_SNDBUF failed for fd=%d", m_socket);
	}

	socklen_t len = 4;
	int8_t size = 0;
	getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &size, &len);
	//log("socket=%d send_buf_size=%d", m_socket, size);
}

void CBaseSocket::SetRecvBufSize(uint32_t recv_size)
{
	int ret = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&recv_size, 4);
	if (ret == SOCKET_ERROR)
	{
		//log("set SO_RCVBUF failed for fd=%d", m_socket);
	}

	socklen_t len = 4;
	int8_t size = 0;
	getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &size, &len);
	//log("socket=%d recv_buf_size=%d", m_socket, size);
}

int CBaseSocket::_GetErrorCode()
{
	return WSAGetLastError();
}

bool CBaseSocket::_IsBlock(int error_code)
{
	return ( (error_code == WSAEINPROGRESS) || (error_code == WSAEWOULDBLOCK) );
}

void CBaseSocket::_SetNonblock(SOCKET fd)
{
	u_long nonblock = 1;
	int ret = ioctlsocket(fd, FIONBIO, &nonblock);
	if (ret == SOCKET_ERROR)
	{
		//log("_SetNonblock failed, err_code=%d", _GetErrorCode());
	}
}

void CBaseSocket::_SetReuseAddr(SOCKET fd)
{
	int reuse = 1;
	int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
	if (ret == SOCKET_ERROR)
	{
		//log("_SetReuseAddr failed, err_code=%d", _GetErrorCode());
	}
}

void CBaseSocket::_SetNoDelay(SOCKET fd)
{
	int nodelay = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
	if (ret == SOCKET_ERROR)
	{
		//log("_SetNoDelay failed, err_code=%d", _GetErrorCode());
	}
}

void CBaseSocket::_SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr)
{
	memset(pAddr, 0, sizeof(sockaddr_in));
	pAddr->sin_family = AF_INET;
	pAddr->sin_port = htons(port);
	pAddr->sin_addr.s_addr = inet_addr(ip);
	if (pAddr->sin_addr.s_addr == INADDR_NONE)
	{
		hostent* host = gethostbyname(ip);
		if (host == NULL)
		{
			//log("gethostbyname failed, ip=%s", ip);
			return;
		}

		pAddr->sin_addr.s_addr = *(uint32_t*)host->h_addr;
	}
}

void CBaseSocket::_AcceptNewSocket()
{
	SOCKET fd = 0;
	sockaddr_in peer_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
	char ip_str[64];
	while ( (fd = accept(m_socket, (sockaddr*)&peer_addr, &addr_len)) != INVALID_SOCKET )
	{
		CBaseSocket* pSocket = new CBaseSocket(m_pBaseServer);
		uint32_t ip = ntohl(peer_addr.sin_addr.s_addr);
		uint16_t port = ntohs(peer_addr.sin_port);

		snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

		printf("AcceptNewSocket, socket=%d from %s:%d\n", fd, ip_str, port);

		pSocket->SetSocket(fd);
		pSocket->SetState(SOCKET_STATE_CONNECTED);
		pSocket->SetRemoteIP(ip_str);
		pSocket->SetRemotePort(port);
		pSocket->SetNotify(m_Notify);

		_SetNoDelay(fd);
		_SetNonblock(fd);
		m_pBaseServer->AddBaseSocket(pSocket);
		m_Notify->OnConnect((net_handle_t)fd);
		m_pBaseServer->AddEvent(fd, SOCKET_READ | SOCKET_EXCEP);
	}
}

