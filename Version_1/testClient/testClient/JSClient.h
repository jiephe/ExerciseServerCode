#ifndef _JS_CLIENT_H_
#define _JS_CLIENT_H_

#include <WinSock2.h>
#include <vector>

typedef std::vector<char>	ByteBuf_t;
#define MAX_SEND_RCV_LEN		(8192)

typedef struct _tagWGDHEAD
{
	int			nCmd;
	int			nDataLen;
}WGDHEAD;

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
	int Connect(const char* ip, int port);

	int Send(const char* buf, int len);

	void onData();

	static  DWORD WINAPI StartRoutine(LPVOID arg);

public:
	virtual int OnRead(SOCKET fd);
	virtual int OnWrite(SOCKET fd);
	virtual int OnExcept(SOCKET fd);

private:
	void BeginDetect(CJSClientListener* pListener);

private:
	SOCKET		m_fd;
	bool		m_bRunning;
	ByteBuf_t	m_buffer;
	CNotify*	m_pNotify;
};


#endif