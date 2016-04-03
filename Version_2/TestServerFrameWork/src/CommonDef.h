#ifndef __COMMON_DEFINE_H__
#define __COMMON_DEFINE_H__

#include "ostype.h"
#include <list>
#include <queue>

typedef struct _tagWGDHEAD
{
	int				nParentCmd;
	int				nSubCmd;
	int				nDataLen;
}WGDHEAD;


typedef struct _tagSendData
{
	net_handle_t	fd;
	void*			pData;
	int				len;
}SendData;

typedef std::queue<SendData>		list_data_t;

//typedef SAFE_DELETE(POINTER)		(if(POINTER) delete POINTER)

#endif
