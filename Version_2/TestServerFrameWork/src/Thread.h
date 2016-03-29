/*================================================================
*   Copyright (C) 2014 All rights reserved.
*   
*   文件名称：Thread.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年09月10日
*   描    述：
*
#pragma once
================================================================*/

#ifndef __THREAD_H__
#define __THREAD_H__

#ifndef _WIN32
#include <pthread.h>
#endif

#include "ostype.h"

class CThread
{
public:
	CThread();
	virtual ~CThread();
    
#ifdef _WIN32
	static unit32_long_t WINAPI StartRoutine(LPVOID lpParameter);
#else
	static void* StartRoutine(void* arg);
#endif
    
	virtual void StartThread(void);
	virtual void OnThreadRun(void) = 0;
protected:
#ifdef _WIN32
	unit32_long_t		m_thread_id;
#else
	pthread_t			m_thread_id;
#endif
};

class CEventThread : public CThread
{
public:
	CEventThread();
	virtual ~CEventThread();
    
 	virtual void OnThreadTick(void) = 0;
	virtual void OnThreadRun(void);
	virtual void StartThread();
	virtual void StopThread();
	bool IsRunning() { return m_bRunning; }
private:
	bool 		m_bRunning;
};

class CThreadNotify
{
public:
	CThreadNotify();
	~CThreadNotify();

#ifdef _WIN32
	void Lock() { EnterCriticalSection(&m_cs); }
	void Unlock() { LeaveCriticalSection(&m_cs); }
	DWORD Wait() { return WaitForSingleObject(m_hEvent, INFINITE);}
	void Reset() {ResetEvent(m_hEvent);}
	void Signal() { SetEvent(m_hEvent); }
#else
	void Lock() { pthread_mutex_lock(&m_mutex); }
	void Unlock() { pthread_mutex_unlock(&m_mutex); }
	void Wait() { pthread_cond_wait(&m_cond, &m_mutex); }
	void Signal() { pthread_cond_signal(&m_cond); }
#endif
	
private:

#ifdef _WIN32
	HANDLE				m_hEvent;		
	CRITICAL_SECTION	m_cs;		
#else
	pthread_mutex_t 	m_mutex;
	pthread_mutexattr_t	m_mutexattr;
	pthread_cond_t 		m_cond;
#endif
};


#endif
