#ifndef _SERVER_DEAL_H_
#define _SERVER_DEAL_H_

#include "ostype.h"
#include "INotify.h"

class CServerDeal: public INotify
{
public:
	CServerDeal();

	virtual ~CServerDeal();

public:
	virtual int OnReceiveData(net_handle_t fd);
};

#endif