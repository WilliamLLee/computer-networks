#pragma once
#include "afxsock.h"

class Server_Socket :
	public CAsyncSocket
{
public:
	Server_Socket();
	~Server_Socket();
	virtual void OnReceive(int nErrorCode);
};

