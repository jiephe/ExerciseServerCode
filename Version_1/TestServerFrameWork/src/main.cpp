#include "WgdServer.h"
#include "ServerDeal.h"
#include <stdio.h>

int main()
{
	CWgdServer ws("127.0.0.1", 19889, 4);

	bool bRet = ws.init();
	if (bRet)
		printf("Server init Ok!\n");
	else
	{
		printf("Server init Fail!\n");
		return -1;
	}

	printf("Server begin to work....\n");

	ws.StartWork();

	printf("Server end to work....\n");

	return 0;
}
