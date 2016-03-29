// NetClient.cpp : Defines the entry point for the console application.
//

#include "JSClient.h"

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
	char* pString = (char*)data;

	printf("receive data : [%s]", pString);

	return 0;
}

int main()
{
 	CMyNotify* pNofity = new CMyNotify;
 
 	CJSClient cjsClient(pNofity);

 	int nRet = cjsClient.Connect("127.0.0.1", 19887);

	char* pbuf = new char[TEST_SIZE];
	for (int i = 0; i < TEST_SIZE; ++i)
	{
		pbuf[i] = 's';
	}

	int nTotalSize = sizeof(WGDHEAD) + TEST_SIZE;
	char* pM = new char[nTotalSize];

	for (int i = 0; i < 100; i++)
	{
		memset(pM, 0x0, nTotalSize);
		WGDHEAD head;
		head.nCmd = i;
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

