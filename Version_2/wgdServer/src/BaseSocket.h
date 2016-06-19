#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "ostype.h"
#include <string>

class CWgdServer;
class CWGDConn;

class CBaseSocket
{
public:
	CBaseSocket();
	
	virtual ~CBaseSocket();

	SOCKET GetSocket() { return m_socket; }
	void SetSocket(SOCKET fd) { m_socket = fd; }
	void SetState(uint8_t state) { m_state = state; }

	void SetRemoteIP(char* ip) { m_remote_ip = ip; }
	void SetRemotePort(uint16_t port) { m_remote_port = port; }
	void SetSendBufSize(uint32_t send_size);
	void SetRecvBufSize(uint32_t recv_size);

	void SetIsListenSocket(bool bListen) {m_bListenSocket = bListen;}
	bool IsListenSocket() {return m_bListenSocket;}
	
	void SetMainServer(CWgdServer* pServer) {m_pBaseServer = pServer;}

	void SetConn(CWGDConn* pConn) {m_pConn = pConn;}

	const char*	GetRemoteIP() { return m_remote_ip.c_str(); }
	uint16_t	GetRemotePort() { return m_remote_port; }
	const char*	GetLocalIP() { return m_local_ip.c_str(); }
	uint16_t	GetLocalPort() { return m_local_port; }

public:
	int Listen(
		const char*		server_ip, 
		uint16_t		port);

	net_handle_t Connect(
		const char*		server_ip, 
		uint16_t		port);

	int Send(void* buf, int len);

	int Recv(void* buf, int len);

	int Close();

	CWgdServer*		m_pBaseServer;			// for listen socket

	CWGDConn*		m_pConn;				// for connect socket -- main working for send and receive data

public:	
	int  OnRead();
	void OnWrite();
	void OnClose();

private:	
	int _GetErrorCode();
	bool _IsBlock(int error_code);

	void _SetNonblock(SOCKET fd);
	void _SetReuseAddr(SOCKET fd);
	void _SetNoDelay(SOCKET fd);
	void _SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr);

	void _AcceptNewSocket();

private:
	std::string		m_remote_ip;
	uint16_t		m_remote_port;
	std::string		m_local_ip;
	uint16_t		m_local_port;

	uint8_t			m_state;
	SOCKET			m_socket;	

	bool			m_bListenSocket;
};

#endif
