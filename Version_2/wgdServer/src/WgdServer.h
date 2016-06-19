#ifndef  _WGD_SERVER_H_
#define  _WGD_SERVER_H_

#include "ostype.h"
#include "IServerNotify.h"
#include "BaseSocket.h"
#include "ThreadPool.h"
#include "CommonNet.h"
#include "CommonDef.h"
#include <map>
#include <hash_map>
#include <list>

class CWGDConn;

using namespace std;

typedef hash_map<net_handle_t, CBaseSocket*>	SocketMap;
typedef map<net_handle_t, CWGDConn*>			ConnMap_t;
typedef list<SendData>							list_data_t;

class CWgdServer : public IServerNotify
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
	void		StartSendThread();

	static  DWORD WINAPI StartRoutine(LPVOID arg);

	void		Execute();

	unit32_long_t		m_SendThread_id;

	list_data_t			m_thread_list;

	CLock				m_send_lock;

public:
	virtual int			OnExcept(net_handle_t fd);

	virtual int			OnConnect(net_handle_t fd) {return 0;}

	virtual int			ReConnect(net_handle_t fd);

	virtual int			OnReceivedNotify(net_handle_t fd, void* pData, int len);

	void	OnTest(net_handle_t fd, void* pData, int len);

public:
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

	CThreadPool			m_ListenThreadPool;

	uint32_t			m_ThreadNum;

	bool				m_bRunning;

	fd_set				m_read_set;

	fd_set				m_write_set;

	fd_set				m_excep_set;

	CLock				m_lock;

	CLock				m_ConnMap_lock;

	ConnMap_t			m_conn_map;

	CBaseSocket*		m_pMainSocket;
};


#endif