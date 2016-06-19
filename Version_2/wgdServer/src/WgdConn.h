#ifndef _WGD_CONN_H_
#define _WGD_CONN_H_

#include "SimpleBuffer.h"
#include <string>

using namespace std;

class CBaseSocket;
class CWgdServer;

class CWGDConn
{
public:
	CWGDConn(CWgdServer* pNotify);

	~CWGDConn();

public:
	virtual int  OnRead();

	virtual void OnWrite();

	virtual void OnClose();

	void SetBaseSocket(CBaseSocket* pBaseSocket) {m_pBaseSocket = pBaseSocket;}

	int Send(void* data, int len);

	int32_t ParseBuffer(CSimpleBuffer& simpleBuffer);

	int32_t DealBuffer(CSimpleBuffer& simpleBuffer);

private:	
	CBaseSocket*			m_pBaseSocket;

	string					m_peer_ip;
	
	uint16_t				m_peer_port;

	CSimpleBuffer			m_in_buf;

	CSimpleBuffer			m_out_buf;

	CWgdServer*				m_pServer;
};

#endif

