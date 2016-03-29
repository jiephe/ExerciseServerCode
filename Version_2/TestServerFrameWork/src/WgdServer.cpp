#include "WgdServer.h"
#include "EventDispatch.h"
#include "WgdConn.h"

void CWgdServer::OnStatisticEnd()
{
	m_StatLock.lock();
	m_nCount++;
	if (m_nCount == 1)
		m_dwTime = GetTickCount();
	if (m_nCount == 100*10)
	{
		printf("Total cost time: %d\n", GetTickCount()-m_dwTime);
	}
	m_StatLock.unlock();
}

CWgdServer::CWgdServer(int8_t ip[], uint16_t port, int nThreadNum)
	: m_port(port), m_ThreadNum(nThreadNum)
{
	memcpy(m_ip, ip, sizeof(m_ip));

	m_ListenThreadPool.Init(m_ThreadNum, this);

	m_bRunning = false;

	FD_ZERO(&m_read_set);

	FD_ZERO(&m_write_set);

	FD_ZERO(&m_excep_set);

	m_nCount = 0;
}

CWgdServer::~CWgdServer()
{
	m_bRunning = false;

	int ret = NETLIB_OK;
	if (WSACleanup() != 0)
	{
		ret = NETLIB_ERROR;
	}
}

bool CWgdServer::init()
{
	int ret = NETLIB_OK;

	WSADATA wsaData;
	WORD wReqest = MAKEWORD(1, 1);
	if (WSAStartup(wReqest, &wsaData) != 0)
	{
		ret = NETLIB_ERROR;
	}

	if (ret == NETLIB_ERROR)
		return false;

	return true;
}

int CWgdServer::StartWork()
{
	CBaseSocket* pSocket = new CBaseSocket(this);
	if (!pSocket)
		return NETLIB_ERROR;

	int ret =  pSocket->Listen(m_ip, m_port, this);
	if (ret == NETLIB_ERROR)
	{		
		delete pSocket;
		return ret;
	}

	StartDispatch(100);

	return 0;
}

void CWgdServer::StartDispatch(uint32_t wait_timeout)
{
	fd_set read_set, write_set, excep_set;
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = wait_timeout * 1000;	// 10 millisecond

	if (m_bRunning)
		return;
	m_bRunning = true;

	while (m_bRunning)
	{
		if (!m_read_set.fd_count && !m_write_set.fd_count && !m_excep_set.fd_count)
		{
			Sleep(MIN_TIMER_DURATION);
			continue;
		}

		m_lock.lock();
		memcpy(&read_set, &m_read_set, sizeof(fd_set));
		memcpy(&write_set, &m_write_set, sizeof(fd_set));
		memcpy(&excep_set, &m_excep_set, sizeof(fd_set));
		m_lock.unlock();

		int nfds = select(0, &read_set, &write_set, &excep_set, &timeout);
		if (nfds == SOCKET_ERROR)
		{
			printf("select failed, error code: %d\n", GetLastError());
			Sleep(MIN_TIMER_DURATION);
			continue;			
		}

		if (nfds == 0)
		{
			continue;
		}

		for (u_int i = 0; i < read_set.fd_count; i++)
		{
			SOCKET fd = read_set.fd_array[i];
			CBaseSocket* pSocket = FindBaseSocket((net_handle_t)fd);
			if (pSocket)
			{
				pSocket->OnRead();
			}
		}
	}
}

void CWgdServer::AddEventForMainSocket(SOCKET fd, uint8_t socket_event)
{
	CAutoLock func_lock(&m_lock);

	if ((socket_event & SOCKET_READ) != 0)
	{
		FD_SET(fd, &m_read_set);
	}

	if ((socket_event & SOCKET_WRITE) != 0)
	{
		FD_SET(fd, &m_write_set);
	}

	if ((socket_event & SOCKET_EXCEP) != 0)
	{
		FD_SET(fd, &m_excep_set);
	}
}

void CWgdServer::AddEvent(SOCKET fd, uint8_t socket_event)
{
	uint32_t idx = m_ListenThreadPool.AddEvent(fd, socket_event);
}

void CWgdServer::AddBaseSocket(CBaseSocket* pSocket)
{
	m_sockMap_lock.lock();
	m_socket_map.insert(make_pair((net_handle_t)pSocket->GetSocket(), pSocket));
	m_sockMap_lock.unlock();
}

void CWgdServer::RemoveBaseSocket(CBaseSocket* pSocket)
{
	m_sockMap_lock.lock();
	m_socket_map.erase((net_handle_t)pSocket->GetSocket());
	m_sockMap_lock.unlock();
}

CBaseSocket* CWgdServer::FindBaseSocket(net_handle_t fd)
{
	CBaseSocket* pSocket = NULL;

	m_sockMap_lock.lock();
	SocketMap::iterator iter = m_socket_map.find(fd);
	if (iter != m_socket_map.end())
	{
		pSocket = iter->second;
		//pSocket->AddRef();
	}
	m_sockMap_lock.unlock();

	return pSocket;
}

int	CWgdServer::OnRead(net_handle_t fd)
{
	CWGDConn* pConn = FindWgdConn(fd);
	if (pConn)
		pConn->OnRead();

	return 0;
}

int	CWgdServer::OnClose(net_handle_t fd)
{
	CBaseSocket* pSocket = FindBaseSocket(fd);
	if (!pSocket)
		return NETLIB_ERROR;

	RemoveBaseSocket(pSocket);
	int ret = pSocket->Close();

	RemoveWgdConn(fd);

	return ret;
}

int	CWgdServer::OnConnect(net_handle_t fd)
{
	CWGDConn* pConn = new CWGDConn();
	pConn->OnConnect(fd, FindBaseSocket(fd));

	AddWgdConn(pConn, fd);

	return 0;
}

void CWgdServer::AddWgdConn(CWGDConn* pConn, net_handle_t fd)
{
	printf("Add Conn %d \n", fd);
	m_ConnMap_lock.lock();
	m_conn_map.insert(make_pair(fd, pConn));
	m_ConnMap_lock.unlock();
}

void CWgdServer::RemoveWgdConn(net_handle_t handle)
{
	printf("Remove Conn %d \n", handle);

	m_ConnMap_lock.lock();
	ConnMap_t::iterator iter = m_conn_map.find(handle);
	if (iter != m_conn_map.end())
	{
		CWGDConn* pConn = (CWGDConn*)iter->second;
		delete pConn;
		m_conn_map.erase(iter);
		m_ConnMap_lock.unlock();
		return ;
	}
	m_ConnMap_lock.unlock();
}

CWGDConn* CWgdServer::FindWgdConn(net_handle_t handle)
{
	CWGDConn* pConn = NULL;

	m_ConnMap_lock.lock();
	ConnMap_t::iterator iter = m_conn_map.find(handle);
	if (iter != m_conn_map.end())
	{
		pConn = iter->second;
		//pConn->AddRef();
	}
	m_ConnMap_lock.unlock();

	return pConn;
}

int	CWgdServer::ReConnect(net_handle_t fd)
{
	return 0;
}

int	CWgdServer::OnWrite(net_handle_t fd)
{
	return 0;
}

int	CWgdServer::OnExcept(net_handle_t fd)
{

	return 0;
}