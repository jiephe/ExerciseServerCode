#ifndef __COMMON_DEFINE_H__
#define __COMMON_DEFINE_H__

#include "ostype.h"

#define					MAX_IP_LEN						(64)

#define					MIN_TIMER_DURATION				(100)

#define					MAX_SEND_RCV_LEN				(8192)

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

#endif
