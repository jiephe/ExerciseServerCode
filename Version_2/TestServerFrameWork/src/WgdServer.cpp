#include "WgdServer.h"
#include "EventDispatch.h"
#include "WgdConn.h"
#include "CommonDef.h"

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

void CWgdServer::StartSendThread()
{
	//CreateThread(NULL, 0, StartRoutine, this, 0, &m_SendThread_id);
}

DWORD WINAPI CWgdServer::StartRoutine(LPVOID arg)
{
	CWgdServer* pThread = (CWgdServer*)arg;

	pThread->Execute();

	return NULL;
}

void CWgdServer::Execute()
{
#if 0
	while (true)
	{
		int nSize = 0;
		SendData sd;
		m_send_lock.lock();
		nSize = m_thread_list.size();
		if (nSize > 0)
		{
			sd = m_thread_list.front();
			m_thread_list.pop();
		}
		m_send_lock.unlock();

		if (nSize <= 0)
		{
			Sleep(MIN_TIMER_DURATION);
			continue;
		}

		CBaseSocket* pSocket = FindBaseSocket(sd.fd);

		pSocket->Send(sd.pData, sd.len);

		if (sd.pData)
			delete sd.pData;
	}
#endif
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
	m_pMainSocket = new CBaseSocket();
	if (!m_pMainSocket)
		return NETLIB_ERROR;

	m_pMainSocket->SetMainServer(this);

	int ret =  m_pMainSocket->Listen(m_ip, m_port);
	if (ret == NETLIB_ERROR)
	{		
		delete m_pMainSocket;
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
			if (m_pMainSocket)
			{
				m_pMainSocket->OnRead();
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
	m_ListenThreadPool.AddEvent(fd, socket_event);
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

int	CWgdServer::OnExcept(net_handle_t fd)
{

	return 0;
}

int	CWgdServer::OnReceivedNotify(net_handle_t fd, void* pData, int len)
{
	if (NULL == pData || len <= 0)
	{
		return -1;
	}

	WGDHEAD* pHead = (WGDHEAD*)pData;

	OnTest(fd, pData, len);

// 	switch(pHead->nSubCmd)
// 	{
// 	case 1:
// 		OnTest(fd, pData, len);
// 		break;
// 	default:
		//break;
// 	}

	return 0;
}

void	CWgdServer::OnTest(net_handle_t fd, void* pData, int len)
{
	WGDHEAD* pHead = (WGDHEAD*)pData;

	printf("Have Receive Data Cmd: %d\n", pHead->nSubCmd);

	CWGDConn* pConn = FindWgdConn(fd);

	WGDHEAD wh;
	wh.nParentCmd = pHead->nSubCmd;
	wh.nDataLen = 1024;

	char buf[1024];
	for (int i = 0; i < 1024; ++i)
		buf[i] = 'a';

	int nTotalSize = sizeof(WGDHEAD)+1024;
	char* pSendData = new char[nTotalSize];
	memcpy(pSendData, &wh, sizeof(WGDHEAD));
	memcpy(pSendData+sizeof(WGDHEAD), buf, 1024);

	pConn->Send(pSendData, nTotalSize);

	delete [] pSendData;
}