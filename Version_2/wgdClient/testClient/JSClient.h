#ifndef _JS_CLIENT_H_
#define _JS_CLIENT_H_

#include <WinSock2.h>
#include <vector>
#include <string>
#include "SimpleBuffer.h"

class CJSClientListener
{
public:
	virtual int OnRead(SOCKET fd) = 0;
	virtual int OnWrite(SOCKET fd) = 0;
	virtual int OnExcept(SOCKET fd) = 0;
};

class CNotify
{
public:
	virtual int onData(void* data, int len) = 0;
};

class CJSClient: public CJSClientListener
{
public:
	CJSClient(CNotify* pNotify);
	~CJSClient();

public:
	int CreateSocket();

	int Connect(const char* ip, int port);

	int ReConnect(const char* ip, int port);

	int Send(const char* buf, int len);

	int32_t ParseBuffer(CSimpleBuffer& simpleBuffer);

	int32_t DealBuffer(CSimpleBuffer& simpleBuffer);

	static  DWORD WINAPI StartRoutine(LPVOID arg);

public:
	virtual int OnRead(SOCKET fd);
	virtual int OnWrite(SOCKET fd);
	virtual int OnExcept(SOCKET fd);

private:
	void BeginDetect(CJSClientListener* pListener);

private:
	SOCKET			m_fd;
	bool			m_bRunning;
	CSimpleBuffer	m_buffer;
	CNotify*		m_pNotify;

	std::string		m_strIP;

	uint16_t		m_Port;
};


#endif