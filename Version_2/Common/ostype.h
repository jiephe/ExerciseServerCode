#ifndef __OS_TYPE_H__
#define __OS_TYPE_H__

#include <WinSock2.h>
#include <Windows.h>
#include <WinBase.h>
#include <direct.h>

typedef char				int8_t;
typedef short				int16_t;
typedef int					int32_t;
typedef	long long			int64_t;
typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef	unsigned long long	uint64_t;
typedef int					socklen_t;
typedef unsigned long		unit32_long_t;
typedef unsigned char		uchar_t;
typedef int					net_handle_t;
typedef int					conn_handle_t;
typedef void*				HANDLE;
typedef void*				PVOID;

const uint32_t INVALID_UINT32	= (uint32_t) -1;

const uint32_t INVALID_VALUE	= 0;

#endif
