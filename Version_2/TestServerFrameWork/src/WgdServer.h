#ifndef  _WGD_SERVER_H_
#define  _WGD_SERVER_H_

#include "ostype.h"
#include "IServerNotify.h"
#include "BaseSocket.h"
#include "ThreadPool.h"
#include <map>

class CWGDConn;

typedef hash_map<net_handle_t, CBaseSocket*>	SocketMap;
typedef map<net_handle_t, CWGDConn*>			ConnMap_t;

class CWgdServer: public IServerNotify
{
public:
	CWgdServer(int8_t ip[], uint16_t port, int nThreadNum);

	virtual ~CWgdServer();

public:
	void OnStatisticEnd();

	CLock m_StatLock;

	int		m_nCount;
	DWORD	m_dwTime;

public:
	virtual int			OnRead(net_handle_t fd);

	virtual int			OnWrite(net_handle_t fd);

	virtual int			OnExcept(net_handle_t fd);

	virtual	int			OnClose(net_handle_t fd);

	virtual int			OnConnect(net_handle_t fd);

	virtual int			ReConnect(net_handle_t fd);

public:
	void AddBaseSocket(CBaseSocket* pSocket);

	void RemoveBaseSocket(CBaseSocket* pSocket);

	CBaseSocket* FindBaseSocket(net_handle_t fd);

	void AddEventForMainSocket(SOCKET fd, uint8_t socket_event);

	void AddEvent(SOCKET fd, uint8_t socket_event);

	void run();

	CWGDConn* FindWgdConn(net_handle_t handle);

	void AddWgdConn(CWGDConn* pConn, net_handle_t fd);

	void RemoveWgdConn(net_handle_t handle);

public:
	bool	init();

	int		StartWork();

	void	StartDispatch(uint32_t wait_timeout);

private:
	int8_t				m_ip[MAX_IP_LEN];

	uint16_t			m_port;

	SocketMap			m_socket_map;

	CLock				m_sockMap_lock;

	CThreadPool			m_ListenThreadPool;

	uint32_t			m_ThreadNum;

	bool				m_bRunning;

	fd_set				m_read_set;

	fd_set				m_write_set;

	fd_set				m_excep_set;

	CLock				m_lock;

	CLock				m_ConnMap_lock;

	ConnMap_t			m_conn_map;
};


#endif