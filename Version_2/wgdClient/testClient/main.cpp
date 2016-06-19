// NetClient.cpp : Defines the entry point for the console application.
//

#include "JSClient.h"
#include "CommonNet.h"
#include <stdio.h>

#define TEST_SIZE  (1024*1024)

class CMyNotify:public CNotify
{
public:
	CMyNotify() {};
	~CMyNotify() {};

public:
	 virtual int onData(void* data, int len);
};

int CMyNotify::onData(void* data, int len)
{
	WGDHEAD* pHead = (WGDHEAD*)data;
	
	printf("Have receive Server ack:[%d] totolSize:[%d]\n", pHead->nParentCmd, len);

	return 0;
}

int main()
{
 	CMyNotify* pNofity = new CMyNotify;
 
 	CJSClient cjsClient(pNofity);

 	int nRet = cjsClient.Connect("192.168.1.101", 19887);
	if (nRet == -1)
	{
		printf("Connect Main Server fail error:[%d]", GetLastError());

		return -1;
	}

	char* pbuf = new char[TEST_SIZE];
	for (int i = 0; i < TEST_SIZE; ++i)
	{
		pbuf[i] = 's';
	}

	int nTotalSize = sizeof(WGDHEAD) + TEST_SIZE;
	char* pM = new char[nTotalSize];

	for (int i = 0; i < 2; i++)
	{
		memset(pM, 0x0, nTotalSize);
		WGDHEAD head;
		head.nSubCmd = i;
		head.nDataLen = TEST_SIZE;
		memcpy(pM, &head, sizeof(WGDHEAD));
		memcpy(pM+sizeof(WGDHEAD), pbuf, TEST_SIZE);

		cjsClient.Send(pM, nTotalSize);
	}

	while (true)
	{
		Sleep(100);
	}
	
	return 0;
}

