#ifndef __EVENT_DISPATCH_H__
#define __EVENT_DISPATCH_H__

#include "ostype.h"
#include "util.h"
#include "Task.h"

class CWgdServer;

class CDispatchTask : public CTask
{
public:
	CDispatchTask(DispatchType type, SOCKET nFd, CWgdServer* pServer);

	virtual ~CDispatchTask();

	virtual void run();

private:
	DispatchType	m_type;

	SOCKET			m_nFd;

	CWgdServer*		m_pServer;
};

#endif
