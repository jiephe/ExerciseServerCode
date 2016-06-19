#include "WgdServer.h"
#include <stdio.h>

int main()
{
	CWgdServer ws("192.168.1.101", 19887, 4);

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
