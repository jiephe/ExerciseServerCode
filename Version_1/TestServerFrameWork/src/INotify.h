#ifndef _I_NOTIFY_H_
#define _I_NOTIFY_H_

#include "ostype.h"

class INotify
{
public:
		virtual int OnReceiveData(net_handle_t fd)				= 0;
};


#endif