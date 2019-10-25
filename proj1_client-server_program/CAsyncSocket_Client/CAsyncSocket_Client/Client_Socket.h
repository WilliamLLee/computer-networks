#pragma once
#include "afxsock.h"
class Client_Socket :
	public CAsyncSocket
{
public:
	Client_Socket();
	~Client_Socket();
	virtual void OnReceive(int nErrorCode);
};

