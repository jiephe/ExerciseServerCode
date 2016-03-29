#include <stdlib.h>
#include "util.h"
#include "ThreadPool.h"

CWorkerThread::CWorkerThread()
{
	m_task_cnt = 0;
}

CWorkerThread::~CWorkerThread()
{

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
	while (true) 
	{
		m_thread_notify.Wait();
		m_thread_notify.Reset();

		CTask* pTask = NULL;

		m_thread_notify.Lock();
		if (!m_task_list.empty())
		{
			pTask = m_task_list.front();
			m_task_list.pop_front();
		}
		m_thread_notify.Unlock();

		if (pTask)
		{
			pTask->run();

			delete pTask;

			m_task_cnt++;

			printf("%d have the execute %d task\n", m_thread_idx, m_task_cnt);
		}
	}
}

void CWorkerThread::PushTask(CTask* pTask)
{
	m_thread_notify.Lock();
	m_task_list.push_back(pTask);
	m_thread_notify.Signal();
	m_thread_notify.Unlock();
}

CThreadPool::CThreadPool()
{
	m_worker_size = 0;
	m_worker_list = NULL;
}

CThreadPool::~CThreadPool()
{

}

int CThreadPool::Init(uint32_t worker_size)
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
		m_worker_list[i].SetThreadIdx(i);
		m_worker_list[i].Start();
	}

	return 0;
}

void CThreadPool::Destory()
{
    if(m_worker_list)
        delete [] m_worker_list;
}

void CThreadPool::AddTask(CTask* pTask)
{
	uint32_t thread_idx = rand() % m_worker_size;

	printf("AddTask Dispatch to [%u] thread deal....\n", thread_idx);

	m_worker_list[thread_idx].PushTask(pTask);
}

