#include "EventDispatch.h"
#include "BaseSocket.h"
#include "WgdServer.h"

CDispatchTask::CDispatchTask(DispatchType type, SOCKET nFd, CWgdServer* pServer)
	:m_type(type), m_nFd(nFd), m_pServer(pServer)
{

}

CDispatchTask::~CDispatchTask()
{

}

void CDispatchTask::run()
{
	if (m_type == e_ReadType)
	{
		CBaseSocket* pSocket = m_pServer->FindBaseSocket((net_handle_t)m_nFd);
		if (pSocket)
		{
			pSocket->OnRead();
			//pSocket->ReleaseRef();
		}
	}
	else if (m_type == e_WriteType)
	{
		CBaseSocket* pSocket = m_pServer->FindBaseSocket((net_handle_t)m_nFd);
		if (pSocket)
		{
			pSocket->OnWrite();
			//pSocket->ReleaseRef();
		}
	}
	else if (m_type == e_ExcepType)
	{
		CBaseSocket* pSocket = m_pServer->FindBaseSocket((net_handle_t)m_nFd);
		if (pSocket)
		{
			pSocket->OnClose();
			//pSocket->ReleaseRef();
		}
	}
}




