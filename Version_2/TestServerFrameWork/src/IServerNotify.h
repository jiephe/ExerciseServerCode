#ifndef _ISERVER_NOTIFY_H_
#define _ISERVER_NOTIFY_H_

class IServerNotify
{
public:
		virtual int OnExcept(net_handle_t fd)			= 0;

		virtual int OnConnect(net_handle_t fd)			= 0;

		virtual int ReConnect(net_handle_t fd)			= 0;

		virtual int OnReceivedNotify(net_handle_t fd, void* pData, int len)	= 0;
};


#endif