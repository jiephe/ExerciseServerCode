#ifndef _WGD_CONN_H_
#define _WGD_CONN_H_

#include "util.h"
#include "SimpleBuffer.h"

class CBaseSocket;

class CWGDConn : public CRefObject
{
public:
	CWGDConn();

	~CWGDConn();

public:

	void OnConnect(net_handle_t handle, CBaseSocket* pBaseSocket);

	virtual void OnConfirm() {}
	virtual void OnRead();
	virtual void OnWrite();
	virtual void OnClose() {}

	int Send(void* data, int len);

	int32_t ParseBuffer(CSimpleBuffer& simpleBuffer);

	int32_t DealBuffer(CSimpleBuffer& simpleBuffer);

	int32_t WriteFile(const std::string& strFilePath, void* data, uint32_t len);

	int32_t ReadFile(const std::string& strFilePath, CSimpleBuffer& buffer);

private:	
	CBaseSocket*			m_pBaseSocket;

	string					m_peer_ip;
	
	uint16_t				m_peer_port;

	CSimpleBuffer			m_in_buf;

	CSimpleBuffer			m_out_buf;

	bool					m_busy;
};

#endif

