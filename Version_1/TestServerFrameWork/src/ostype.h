#ifndef __OS_TYPE_H__
#define __OS_TYPE_H__

#include <WinSock2.h>
#include <Windows.h>
#include <WinBase.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>
#include <hash_map>

using namespace stdext;
using namespace std;

typedef char			int8_t;
typedef short			int16_t;
typedef int				int32_t;
typedef	long long		int64_t;
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;
typedef	unsigned long long	uint64_t;
typedef int				socklen_t;
typedef unsigned long	unit32_long_t;
typedef unsigned char	uchar_t;
typedef int				net_handle_t;
typedef int				conn_handle_t;
typedef void*			HANDLE;

#define					MAX_IP_LEN						(64)

#define					MIN_TIMER_DURATION				(100)

#define					NETLIB_MAX_SOCKET_BUF_SIZE		(128 * 1024)

enum 
{
	SOCKET_READ		= 0x1,
	SOCKET_WRITE	= 0x2,
	SOCKET_EXCEP	= 0x4,
	SOCKET_ALL		= 0x7
};

typedef enum _EnumDispatchType
{
	e_ReadType,
	e_WriteType,
	e_ExcepType,
}DispatchType;

enum
{
	SOCKET_STATE_IDLE,
	SOCKET_STATE_LISTENING,
	SOCKET_STATE_CONNECTING,
	SOCKET_STATE_CONNECTED,
	SOCKET_STATE_CLOSING
};

enum
{
	NETLIB_OK		= 0,
	NETLIB_ERROR	= -1
};

#define NETLIB_INVALID_HANDLE	-1

enum
{
	NETLIB_MSG_CONNECT = 1,
	NETLIB_MSG_CONFIRM,
	NETLIB_MSG_READ,
	NETLIB_MSG_WRITE,
	NETLIB_MSG_CLOSE,
	NETLIB_MSG_TIMER,
    NETLIB_MSG_LOOP
};

const uint32_t INVALID_UINT32  = (uint32_t) -1;
const uint32_t INVALID_VALUE = 0;

typedef struct _tagWGDHEAD
{
	int				nCmd;
	int				nDataLen;
}WGDHEAD;


#endif
