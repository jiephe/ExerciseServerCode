#ifndef __COMMON_NET_H__
#define __COMMON_NET_H__

#include "ostype.h"

typedef struct _tagWGDHEAD
{
	int32_t				nParentCmd;
	int32_t				nSubCmd;
	uint32_t			nDataLen;
}WGDHEAD;

typedef struct _tagSendData
{
	net_handle_t		fd;
	PVOID				pData;
	uint32_t			len;
}SendData;


#endif
