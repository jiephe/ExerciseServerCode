#include <stdlib.h>
#include "ThreadPool.h"
#include "WgdServer.h"
#include "WgdConn.h"
#include "CommonDef.h"

CWorkerThread::CWorkerThread()
{
	m_task_cnt = 0;

	FD_ZERO(&m_read_set);

	FD_ZERO(&m_write_set);

	FD_ZERO(&m_excep_set);

	m_bRunning = false;
}

CWorkerThread::~CWorkerThread()
{
	m_bRunning = false;
}

DWORD WINAPI CWorkerThread::StartRoutine(LPVOID arg)
{
	CWorkerThread* pThread = (CWorkerThread*)arg;

	pThread->Execute();

	return NULL;
}

void CWorkerThread::Start()
{
	CreateThread(NULL, 0, StartRoutine, this, 0, &m_thread_id);

	printf("Thread [%u] Starting Success...\n", m_thread_id);
}

void CWorkerThread::Execute()
{
	StartDispatch(100);
}

void CWorkerThread::AddEvent(SOCKET fd, uint8_t socket_event)
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

void CWorkerThread::RemoveEvent(SOCKET fd, uint8_t socket_event)
{
	CAutoLock func_lock(&m_lock);

	if ((socket_event & SOCKET_READ) != 0)
	{
		FD_CLR(fd, &m_read_set);
	}

	if ((socket_event & SOCKET_WRITE) != 0)
	{
		FD_CLR(fd, &m_write_set);
	}

	if ((socket_event & SOCKET_EXCEP) != 0)
	{
		FD_CLR(fd, &m_excep_set);
	}
}

void CWorkerThread::StartDispatch(uint32_t wait_timeout)
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
			CWGDConn* pConn = m_pBaseServer->FindWgdConn((net_handle_t)fd);
			if (pConn)
			{
				int nRet = pConn->OnRead();
				if (nRet == -1)
				{
					// ±ªπÿ±’¡À
					printf("Remove Socket[%d] All Event\n");
					RemoveEvent(fd, SOCKET_ALL);
				}
			}
		}

		for (u_int i = 0; i < write_set.fd_count; i++)
		{
			SOCKET fd = read_set.fd_array[i];
			CWGDConn* pConn = m_pBaseServer->FindWgdConn((net_handle_t)fd);
			if (pConn)
			{
				RemoveEvent(fd, SOCKET_WRITE);
				pConn->OnWrite();
			}
		}
	}
}

CThreadPool::CThreadPool()
{
	m_worker_size = 0;
	m_worker_list = NULL;
}

CThreadPool::~CThreadPool()
{

}

int CThreadPool::Init(uint32_t worker_size, CWgdServer* pServer)
{
	printf("Begin start %d threads...\n", worker_size);

    m_worker_size = worker_size;
	m_worker_list = new CWorkerThread [m_worker_size];
	if (!m_worker_list)
	{
		return 1;
	}

	for (uint32_t i = 0; i < m_worker_size; i++) 
	{
		m_worker_list[i].SetBaseServer(pServer);
		m_worker_list[i].SetThreadIdx(i);
		m_worker_list[i].Start();
	}

	return 0;
}

void CThreadPool::Destory()
{
    if (m_worker_list)
        delete [] m_worker_list;
}

uint32_t CThreadPool::AddEvent(SOCKET fd, uint8_t socket_event)
{
	uint32_t thread_idx = rand() % m_worker_size;

	printf("AddEvent Dispatch to [%u] thread deal....\n", thread_idx);

	m_worker_list[thread_idx].AddEvent(fd, socket_event);

	return thread_idx;
}


