#include "JSClient.h"
#include <stdio.h>
#include "Mstcpip.h"
#include "CommonNet.h"
#include "CommonDef.h"

CJSClient::CJSClient(CNotify* pNofity)
	:m_pNotify(pNofity)
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);

	CreateSocket();

	m_bRunning = true;
}

int CJSClient::CreateSocket()
{
	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(1985);
	serv_addr.sin_addr.s_addr = inet_addr("192.168.1.103");
	if (serv_addr.sin_addr.s_addr == INADDR_NONE)
	{
		hostent* host = gethostbyname("192.168.1.103");
		if (host != NULL)
		{
			serv_addr.sin_addr.s_addr = *(unsigned int*)host->h_addr;
		}
	}

	int nRet = ::bind(m_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if (nRet < 0)
	{
		int nErr = GetLastError();
		printf("Err: %d\n", nErr);
		return nRet;
	}

	int nErr = GetLastError();


#if 1
	{
		// 开启KeepAlive
		BOOL bKeepAlive = TRUE;
		int nRet = ::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive));
		if (nRet == SOCKET_ERROR)
		{

		}

		// 设置KeepAlive参数
		tcp_keepalive alive_in          = {0};
		tcp_keepalive alive_out         = {0};
		alive_in.keepalivetime          = 5000;                // 开始首次KeepAlive探测前的TCP空闭时间
		alive_in.keepaliveinterval      = 1000;                // 两次KeepAlive探测间的时间间隔
		alive_in.onoff                  = TRUE;
		unsigned long ulBytesReturn = 0;
		nRet = WSAIoctl(m_fd, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in), &alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL);
		if (nRet == SOCKET_ERROR)
		{

		}
	}
#endif

	return nRet;
}

CJSClient::~CJSClient()
{
	m_bRunning = false;

	closesocket(m_fd);

	WSACleanup();
}

int CJSClient::Connect(const char* ip, int port)
{
	if (ip == NULL || port <= 0)
	{
		return -1;
	}

	m_strIP = ip;

	m_Port = port;

	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	int ret = connect(m_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret < 0)
	{
		printf("Connect Server[%s]--Port[%d] fail, error[%d]", ip, port, WSAGetLastError());

		return ret;
	}

	u_long nonblock = 1;
	ret = ioctlsocket(m_fd, FIONBIO, &nonblock);

	DWORD thread_id;

	CreateThread(NULL, 0, StartRoutine, this, 0, &thread_id);

	return ret;
}

int CJSClient::ReConnect(const char* ip, int port)
{
	sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	
	int nTime = 1;

	//尝试连接之前是非阻塞的 防止主进程阻塞在connect函数 无法处理其他事情
	u_long nonblock = 1;
	int ret = ioctlsocket(m_fd, FIONBIO, &nonblock);

	ret = connect(m_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
	while(ret == -1/* && nTime < 100000*/)
	{
		//连接已经存在 不用再创建
		if (GetLastError() == WSAEISCONN)
		{
			break;
		}
		printf("\ntry to ReConnect Server[%s]--Port[%d] error:[%d]\n", ip, port, GetLastError());
		nTime++;

		Sleep(300);

		ret = connect(m_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
	}

// 	if (ret == -1)
// 	{
// 		printf("ReConnect Server[%s]--Port[%d] fail, error[%d]\n", ip, port, WSAGetLastError());
// 
// 		return ret;
// 	}

	printf("ReConnect Server[%s]--Port[%d] Success\n", ip, port);

	{
		char* pbuf = new char[1024];
		for (int i = 0; i < 1024; ++i)
		{
			pbuf[i] = 's';
		}

		int nTotalSize = sizeof(WGDHEAD) + 1024;
		char* pM = new char[nTotalSize];

		for (int i = 0; i < 1; i++)
		{
			memset(pM, 0x0, nTotalSize);
			WGDHEAD head;
			head.nSubCmd = 100;
			head.nDataLen = 1024;
			memcpy(pM, &head, sizeof(WGDHEAD));
			memcpy(pM+sizeof(WGDHEAD), pbuf, 1024);

			Send(pM, nTotalSize);
		}
	}

	//连接成功之后要设置成阻塞的
	nonblock = 0;
	ret = ioctlsocket(m_fd, FIONBIO, &nonblock);

	return 0;
}

DWORD WINAPI CJSClient::StartRoutine(LPVOID arg)
{
	CJSClient* pClient = (CJSClient*)arg;

	pClient->BeginDetect(pClient);

	return 0;
}

void CJSClient::BeginDetect(CJSClientListener* pListener)
{
	if (NULL == pListener)
	{
		return ;
	}

	fd_set read_set, write_set, excep_set;
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100 * 1000;	// 10 millisecond

	while (m_bRunning)
	{
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_ZERO(&excep_set);
		FD_SET(m_fd, &read_set);
		FD_SET(m_fd, &write_set);
		FD_SET(m_fd, &excep_set);

		int nfds = select(0, &read_set, &write_set, &excep_set, &timeout);

		if (nfds == SOCKET_ERROR)
		{
			printf("select failed, error code: %d\n", GetLastError());
			Sleep(100);
			continue;			// select again
		}

		if (nfds == 0)
		{
			Sleep(100);
			continue;
		}

		for (u_int i = 0; i < read_set.fd_count; i++)
		{
			pListener->OnRead(read_set.fd_array[i]);	
		}

		for (u_int i = 0; i < write_set.fd_count; i++)
		{
			pListener->OnWrite(write_set.fd_array[i]);	
		}

		for (u_int i = 0; i < excep_set.fd_count; i++)
		{
			pListener->OnExcept(excep_set.fd_array[i]);	
		}

		Sleep(100);
	}

	return ;
}

int CJSClient::OnRead(SOCKET fd)
{
	u_long avail = 0;
	if ( (ioctlsocket(m_fd, FIONREAD, &avail) == SOCKET_ERROR) || (avail == 0) )
	{
		closesocket(m_fd);

		CreateSocket();
		ReConnect(m_strIP.c_str(), m_Port);

		return 0;
	}

	int READ_BUF_SIZE = 8192;

	for (;;)
	{
		uint32_t free_buf_len = m_buffer.GetAllocSize() - m_buffer.GetWriteOffset();
		if (free_buf_len < READ_BUF_SIZE)
			m_buffer.Extend(READ_BUF_SIZE);

		int nRet = recv(fd, (char*)(m_buffer.GetBuffer() + m_buffer.GetWriteOffset()), READ_BUF_SIZE, 0);
		if (SOCKET_ERROR == nRet)
		{
			if ((WSAGetLastError() == WSAEINTR))
				continue;
			else
				break;
		}

		m_buffer.IncWriteOffset(nRet);
	}
	
	while( ParseBuffer(m_buffer) != -1)
	{

	}

	return 0;
}


int32_t CJSClient::ParseBuffer(CSimpleBuffer& simpleBuffer)
{
	uint32_t uintHave = simpleBuffer.GetWriteOffset();
	int32_t nHeadSize = sizeof(WGDHEAD);

	if (uintHave < nHeadSize)
	{
		return -1;
	}

	uchar_t* pBuffer = simpleBuffer.GetBuffer();
	WGDHEAD* pHead = (WGDHEAD*)pBuffer;
	int32_t nTotalSize = nHeadSize + pHead->nDataLen;
	if (uintHave < nTotalSize)
	{
		return -1;
	}

	return DealBuffer(simpleBuffer);
}

int32_t CJSClient::DealBuffer(CSimpleBuffer & simpleBuffer)
{
	uchar_t* pBuffer = simpleBuffer.GetBuffer();
	WGDHEAD* pHead = (WGDHEAD*)pBuffer;
	int32_t nTotalSize = sizeof(WGDHEAD) + pHead->nDataLen;

	m_pNotify->onData(pBuffer, nTotalSize);

	simpleBuffer.Read(NULL, nTotalSize);

	return 0;
}

int CJSClient::Send(const char* buf, int len)
{
	if (NULL == buf || len <= 0)
	{
		return -1;
	}

	int offset = 0;
	int remain = len;

	while (remain > 0)
	{
		int send_size = remain;
		if (send_size > MAX_SEND_RCV_LEN)
			send_size = MAX_SEND_RCV_LEN;

		int nSend = send(m_fd, (char*)buf+offset, send_size, 0);
		if (nSend <= 0)
		{
			if (WSAEWOULDBLOCK == WSAGetLastError())
			{
				//printf("Send fail, error[%d] continue \n", WSAGetLastError());
				continue;
			}
			else
			{
				printf("Send fail, error[%d]\n", WSAGetLastError());
				break;
			}
		}

		offset += nSend;
		remain -= nSend;
	}

	return 0;
}

int CJSClient::OnWrite(SOCKET fd)
{

	return 0;
}

int CJSClient::OnExcept(SOCKET fd)
{
	closesocket(m_fd);

	return 0;
}