#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include "ostype.h"
#include "Thread.h"
#include "Task.h"
#include <list>

using namespace std;

class CWgdServer;

class CWorkerThread 
{
public:
	CWorkerThread();
	~CWorkerThread();

	static  DWORD WINAPI StartRoutine(LPVOID arg);

	void StartDispatch(uint32_t wait_timeout);

	void Start();
	void Execute();
	void PushTask(CTask* pTask);

	void AddEvent(SOCKET fd, uint8_t socket_event);
	void RemoveEvent(SOCKET fd, uint8_t socket_event);

	void RemoveEvent();

	void SetThreadIdx(uint32_t idx) { m_thread_idx = idx; }

	void SetBaseServer(CWgdServer* pServer) {m_pBaseServer = pServer;}
	
private:

	uint32_t			m_thread_idx;

	uint32_t			m_task_cnt;

	unit32_long_t		m_thread_id;

	CThreadNotify		m_thread_notify;

	list<CTask*>		m_task_list;

	fd_set				m_read_set;

	fd_set				m_write_set;

	fd_set				m_excep_set;

	bool				m_bRunning;

	CLock				m_lock;

	CWgdServer*			m_pBaseServer;
};

class CThreadPool 
{
public:
	CThreadPool();
	virtual ~CThreadPool();

	int Init(uint32_t worker_size, CWgdServer* pServer);
	void AddTask(CTask* pTask);
	void Destory();

	uint32_t AddEvent(SOCKET fd, uint8_t socket_event);
private:
	uint32_t 		m_worker_size;
	CWorkerThread* 	m_worker_list;
};

#endif 
